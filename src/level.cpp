/* License and copyright go here*/

#include "level.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "terrain.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "random.hpp"
#include "labyrinth.hpp"

#include <algorithm> // max/min
#include <random>
#include <vector>
#include <string>
#include <sstream>

// define a level in the dungeon

class levelImpl;


// helper class to generate a random layout of a level
class levelGen {
private:

  levelImpl* const level_; // non-owning pointer
  level& pub_; // non-owning reference

public:
  levelGen(levelImpl* const level, ::level& pub) :
    level_(level), pub_(pub) {}
  virtual ~levelGen() {}

protected:
  // add an individual room. Returns first=top-left, second=bottom-right
  std::pair<coord,coord> addRoom();

  // possibly entrap a room:
  void addTraps(const std::pair<coord,coord> &coords);

  // possibly add some monsters to the room:
  void addMonsters(std::vector<std::pair<coord,coord>>);

  // possibly add some items to the room:
  void addItems(const std::pair<coord,coord> &);

  // add a monster to the room, but not at c: (used by addMonsters)
  void addMonster(const std::shared_ptr<monster> mon, const coord &c, const std::pair<coord,coord> &);

  // adds a corridor starting at from and ending at to. Very basic for now.
  void addCorridor(const coord from, const coord to);

  // average two coords and return their midpoint.
  coord mid(const std::pair<coord,coord> &p) const {
    return coord((p.first.first + p.second.first) / 2,
		 (p.first.second + p.second.second) / 2);
  }

  template<class iter>
  void place(const iter & begin,
 	     const iter & end,
	     terrainType terrainType);

public:
  // call the build() function to add rooms, monsters etc.
  virtual void build() = 0;
};

// a level generator that fills the level with a labyrinth
class labyGen : private levelGen {
private:
  const bool addDownRamp_; // do we need a downwards stairwell?
public:
  labyGen(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),  
    addDownRamp_(addDownRamp) {}
  virtual ~labyGen() {}
  virtual void build() {
    auto lab = // TODO: not sure if this will work of if we need more terrain types
    labyrinth<terrainType>(terrainType::UP, // in
			   (addDownRamp_ ? terrainType::DOWN : terrainType::GROUND), // out
			   terrainType::DOWN, // unassigned
			   terrainType::ROCK, // impass
			   terrainType::GROUND, // pass
			   terrainType::GROUND, // join
			   level::MAX_WIDTH,
			   level::MAX_HEIGHT);
    lab.build();

    std::vector<std::pair<coord,coord>> pos; // empty; there are no rooms
    addMonsters(pos);
  }
};

// a level generator that fills the level with randomly-placed rooms
class roomGen : private levelGen {
private:
  const bool addDownRamp_; // do we need a downwards stairwell?
public:
  roomGen(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),
    addDownRamp_(addDownRamp) {}
  virtual ~roomGen() {}
  virtual void build() {
    // let's have 3-5 rooms:
    int roomCount = numRooms();
    std::vector<std::pair<coord,coord>> pos;
    for (int i=0; i < roomCount; ++i) {
      std::pair<coord,coord> c(addRoom());
      pos.push_back(c);
    }

    // for now & for the sake of ease, let's just connect the midpoints of each room with
    // basic corridors to make a map:
    for (int i=1; i < roomCount; ++i)
      addCorridor(mid(pos[i-1]), mid(pos[i]));

    // place an entrance in the first room and an exit in the last.
    // TODO: if all rooms, by chance, have the same midpoint, where does the down ramp go?
    place(pos.begin(), pos.end(), terrainType::UP);
    if (addDownRamp_)
      place(pos.rbegin(), pos.rend(), terrainType::DOWN);

    addMonsters(pos);
  }
};

// manages the items in a given cell by adapting levelImpl to the itemHolder interface
// at specified coords
class itemHolderLevel : 
  public itemHolder,
  public iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > {
private:
  levelImpl& level_;
  const coord coord_;
  std::vector<std::shared_ptr<item>> copy_;
public:
  itemHolderLevel(levelImpl & level, const coord & c) :
    iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > >(copy_),
    level_(level), coord_(c) {}
  virtual ~itemHolderLevel() {}
  bool addItem(std::shared_ptr<item> item);
  iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > contents();
  virtual std::vector<std::shared_ptr<item>>::iterator begin();
  virtual std::vector<std::shared_ptr<item>>::iterator end();
  virtual void erase(std::vector<std::shared_ptr<item>>::iterator pos);

};


// implementation of level class 
class levelImpl : public renderByCoord {
public:
  // the dungeon this level is in
  dungeon& dungeon_;
  // how many levels deep are we?
  const int depth_;
  // map of coordinate to stack of items at that location
  ::std::multimap<coord, ::std::shared_ptr<item> > items_;
  // map of coordinate to all monsters at that location
  ::std::multimap<coord, ::std::shared_ptr<monster> > monsters_;
  // terrain type by coordinate
  ::std::map<coord, ::std::shared_ptr<terrain>> terrain_;
  // tell the user when stuff happens:
  std::shared_ptr<io> io_;


  // constructor fills the level with something suitable
  levelImpl(dungeon &dungeon, int depth, std::shared_ptr<io> ios, level& pub, bool downRamp) :
    dungeon_(dungeon),
    depth_(depth),
    io_(ios) {
    using namespace std;
    for (int x=0; x < level::MAX_WIDTH ; ++x)
      for (int y=0; y < level::MAX_HEIGHT ; ++y) {
	coord c(x,y);
	terrain_[c] = tFactory.get(terrainType::ROCK);
      }
    roomGen(this, pub, downRamp).build();
  }
  virtual ~levelImpl() {}

  drawIter drawBegin() const  {
    return drawIter(*this, coord(0,0), level::MAX_WIDTH, level::MAX_HEIGHT);
  }

  drawIter drawEnd() const  {
    return drawIter(*this, coord(-1,-1), level::MAX_WIDTH, level::MAX_HEIGHT);
  }

  virtual const renderable & renderableAt(const coord & pos) const {
    // show monsters if any:
    auto mn = monsters_.equal_range(pos);
    if (mn.first != mn.second) return *(mn.first->second);
    // show items if any:
    auto it = items_.equal_range(pos);
    if (it.first != it.second) return *(it.first->second);
    // show terrain if nothing else:
    return *(terrain_.at(pos));
  }

  coord findTerrain(const terrainType type) const {
    using namespace std;
    for (int y=0; y < level::MAX_HEIGHT; ++y)
      for (int x=0; x < level::MAX_WIDTH; ++x) {
	coord c(x,y);
	if (terrain_.at(c)->type() == type) 
	  return c;
      }
    throw wstring(L"Terrain type ") + to_string(type) + wstring(L" not found on level ") + to_wstring(depth_);
  }

  // dead creatures are removed from the level (shared_ptr).
  // aggressor can't die, so can be passed as references
  // target can, so needs to be a shared_ptr here.
  void attack(monster &aggressor, monster &target) {
    //    std::cerr << aggressor.name() << " is attacking " << target.name() << std::endl;
    const auto tName = target.name();
    const auto result = aggressor.attack(target);
    std::wstring rtn(result.text_);
    bool longMsg = false;
    if (static_cast<unsigned char>(aggressor.injury().cur()) == aggressor.injury().max()) {
      rtn = rtn + L"\n" + aggressor.name() + L" dies.";
      longMsg = true;
    }
    if (static_cast<unsigned char>(result.injury_.cur()) == result.injury_.max()) {
      rtn = rtn + L"\n" + tName + L" dies.";
      longMsg = true;
    }
    //    std::cerr << rtn << " ( monster health is " << target.injury() << ")" << std::endl;
    if (longMsg) io_->longMsg(rtn);
    else io_->message(rtn);
  }

  // NB: Moving of non-player monsters doesn't use cardinals
  void north(monster &m) {
    coord c = posOf(m);
    if (c.second == 0) return; // can't move above top of map
    c.second--;
    if (!terrain_[c]->movable(m)) return; // can't move into terrain
    auto mn = monsters_.find(c);
    while (mn != monsters_.end()) {
      attack(m, *(mn->second)); // can't move into monster
      return;
    }
    moveTo(m, c);
    //      pcPos_ = c;
  }
  void south(monster &m) {
    coord c = posOf(m);
    c.second++;
    if (c.second == level::MAX_HEIGHT) return; // can't move below bottom of map
    if (!terrain_[c]->movable(m)) return; // can't move into terrain
    auto mn = monsters_.find(c);
    if (mn != monsters_.end()) {
      attack(m, *(mn->second)); // can't move into monster
      return;
    }
    moveTo(m, c);
  }
  void east(monster &m) {
    coord c = posOf(m);
    c.first++;
    if (c.first == level::MAX_WIDTH) return; // can't move after right of map
    if (!terrain_[c]->movable(m)) return; // can't move into terrain
    auto mn = monsters_.find(c);
    if (mn != monsters_.end()) {
      attack(m, *(mn->second)); // can't move into monster
      return;
    }
    moveTo(m, c);
  }
  void west(monster &m) {
    coord c = posOf(m);
    if (c.first == 0) return; // can't move before left of map
    c.first--;
    if (!terrain_[c]->movable(m)) return; // can't move into terrain
    auto mn = monsters_.find(c);
    if (mn != monsters_.end()) {
      attack(m, *(mn->second)); // can't move into monster
      return;
    }
    moveTo(m, c);
  }
  void up(monster &m) {
    coord c = posOf(m);
    if (depth_ <= 1) {
      io_->message(L"Leaving the dungeon isn't implemented yet.");
      return;
    }
    switch (terrain_[c]->type()) {
    case terrainType::UP:
      removeMonster(m);
      if (m.isPlayer())
	dungeon_.upLevel();
      m.onLevel(&dungeon_.cur_level());
      break;
    default:
      io_->message(L"There is no way up here.");
    }
  }
  void down(monster &m) {
    coord c = posOf(m);
    if (depth_ == dungeon_.maxLevel()) {
      io_->message(L"You are already at the bottom of the game.");
      return;
    }
    switch (terrain_[c]->type()) {
    case terrainType::DOWN:
      removeMonster(m);
      if (m.isPlayer())
	dungeon_.downLevel();
      m.onLevel(&dungeon_.cur_level());
      break;
    default:
      io_->message(L"There is no way down here.");
    }
  }
  coord posOf(const item &it) const {
    for (auto i = items_.begin(); i != items_.end(); ++i) {
      if (&(*(i->second)) == &it) {
	return i->first;
      }
    }
    return coord(-1,-1);
  }
  coord posOf(const monster &m) const {
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (*(i->second) == m) {
	return i->first;
      }
    }
    return coord(-1,-1);
  }
  const coord pcPos() const {
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (i->second->isPlayer()) {
	return i->first;
      }
    }
    return coord(-1,-1);
  }
  int depth() const {
    return depth_;
  }
  // find a piece of terrain and move to it (NB: This won't work with Nethack-style branch levels)
  void moveTo(const terrainType terrain) {
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (i->second->isPlayer()) {
	moveTo(*(i->second), findTerrain(terrain));
	return;
      }
    }
  }
  // teleport a monster. NB: This will move the monster regardless of any traps, items or other things in the way.
  void moveTo(monster &m, const coord &dest) {
    std::shared_ptr<monster> pM;
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (*(i->second) == m) {
	pM = i->second;
	monsters_.erase(i);
	break;
      }
    }
    addMonster(pM, dest);
    // reveal any pits:
    if (terrain_[dest]->type() == terrainType::PIT_HIDDEN) {
      if (m.isPlayer()) // TODO: Pit traps should not be revealed by flying monsters, or should they?
	io_->message(L"It's a (pit) trap!"); // ref: Admiral Ackbar, Star Wars film Episode VI: Return of the Jedi.
      terrain_[dest] = tFactory.get(terrainType::PIT);
    }
    m.onMove(dest, *(terrain_[dest]));
    if (m.isPlayer()) {
      describePlayerLoc(m, dest);
    }
  }

  void describePlayerLoc(const monster &m, const coord &pcLoc) const {
    std::vector<std::wstring> msg;
    auto monsters = monsters_.equal_range(pcLoc);
    for (auto i = monsters.first; i != monsters.second; ++i)
      if (*(i->second) != m)
	msg.emplace_back(std::wstring(L"live ") + i->second->name());
    auto items = items_.equal_range(pcLoc);
    for (auto i = items.first; i != items.second; ++i)
      msg.emplace_back(i->second->name());
    
    std::wstring message;
    switch (msg.size()) {
    case 0:
      return; // no message
    case 1:
      message += L"There's a " + msg[0] + L" here";
      io_->message(message);
      break;
    default:
      message = L"In your space are:\n";
      for (auto m : msg) message += m + L"\n";
      io_->longMsg(message);
    }
  }

  void sanitiseCoords(coord &pos) const {
    if (pos.first < 0) pos.first = 0;
    if (pos.second < 0) pos.second = 0;
    if (pos.first >= level::MAX_WIDTH) pos.first = level::MAX_WIDTH;
    if (pos.second >= level::MAX_HEIGHT) pos.second = level::MAX_HEIGHT;
  }

  // move a monster by direction, with optional safety
  void move(monster &m, const std::pair<char,char> dir, const bool avoidTraps) {
    coord pos = coord(posOf(m));
    pos.first += dir.first; // -1, 0, or +1
    pos.second += dir.second; // -1, 0, or +1
    
    sanitiseCoords(pos);

    if (!terrain_[pos]->movable(m) || // monster can't pass this way
	monsters_.find(pos) != monsters_.end()) { // monsters can't *generally* move into each other.
      // can't move this way.
      return;
    }
    // TODO: traps and their avoidance
    moveTo(m, pos);
    // TODO: monster items and inventory- collect the new stuff?
  }
  void removeMonster(const monster &m) {
    //io_->longMsg(std::wstring("I am removing ") + name);
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (*(i->second) == m) {
	monsters_.erase(i);
	return; // iterator is now invalid, but we found it.
      }
    }
  }

  void addMonster(const std::shared_ptr<monster> mon, const coord c) {
    auto mn = monsters_.equal_range(c);
    if (mn.first == mn.second) {
      // no existing monster; place where requested
      monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(c, mon));
    } else {
      auto 
	maxX = std::min(level::MAX_WIDTH, c.first + 2),
	maxY = std::min(level::MAX_HEIGHT, c.second + 2);
      coord i;
      for (i.first = std::max(0, c.first - 2); i.first < maxX; ++i.first)
	for (i.second = std::max(0, c.second - 2); i.second < maxY; ++i.second) {
	  if (! terrain_[i]->movable(mon)) continue;
	  auto mni = monsters_.equal_range(i);
	  if (mni.first == mni.second) {
	    // free nearby passible space
	    monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(i, mon));
	    return;
	  }
	}
      // still here; nowhere is suitable; give up and stack 'em up:
      monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(c, mon));
    }
  }

  void addItem(const std::shared_ptr<item> item, const coord c) {
    items_.emplace(c, item); // items may always occupy the same square
  }

  void pickUp() {
    auto pos = pcPos();
    auto v = itemsAt(pos); // take a copy
    if (v.empty()) {
      io_->message(L"You can see nothing to collect here.");
    }
    for (auto it : v)
      if (io_->ynPrompt(std::wstring(L"Do you want to collect: ") + it->name() + L"?")) {
	dungeon_.pc()->addItem(it);
	removeItem(pos, it);
      }
  }

  std::pair<std::multimap<coord, std::shared_ptr<monster> >::iterator,
	    std::multimap<coord, std::shared_ptr<monster> >::iterator> monstersAt(const coord &pos) {
    return monsters_.equal_range(pos);
  }

  dungeon & dung() { return dungeon_; }

  std::unique_ptr<itemHolder> holder(const coord c) {
    return std::unique_ptr<itemHolderLevel>(new itemHolderLevel(*this, c));
  }

  // used by itemHolderLevel:
  std::vector<std::shared_ptr<item>> itemsAt(const coord & pos) const {
    std::vector<std::shared_ptr<item>> rtn;
    auto it = items_.equal_range(pos);
    for (auto i = it.first; i != it.second; ++i) rtn.push_back(i->second);
    return rtn;
  }
  // used by itemHolderLevel:
  void removeItem(const coord & pos, const std::shared_ptr<item> & item) {
    auto it = items_.equal_range(pos);
    for (auto i = it.first; i != it.second; ++i)
      if (i->first == pos && i->second == item) {
	items_.erase(i);
	return;
      }
  }
};

std::pair<coord,coord> levelGen::addRoom() {
  int width = roomWidth(), height = roomHeight();
  // for now, just place the room. Don't care if they overlap.
  // X pos is between 1 (to allow for border on left) and MAX_WIDTH - width - 2
  std::uniform_int_distribution<int> xPosD(1,level::MAX_WIDTH - width - 2);
  // Y pos is between 1 (to allow for border on top) and MAX_HEIGHT - height - 2
  std::uniform_int_distribution<int> yPosD(1,level::MAX_HEIGHT - height - 2);
  int xPos = xPosD(generator), yPos = yPosD(generator);

  auto ground = tFactory.get(terrainType::GROUND);
  for (int y=0; y < height; ++y) {
    for (int x=0; x < width; ++x) {
      coord c(x + xPos,y + yPos);
      //      std::cout << "placing room at " << c << std::endl;
      //      std::cout << "terrain was " << level_->terrain_.at(c)->type() << std::endl;
      level_->terrain_.at(c) = ground;
    }
  }
  //  std::cout << "Added a room " << std::flush;
  coord topLeft(xPos,yPos);
  coord btmRight(xPos+width, yPos+height);
  //std::cout << "at " << topLeft << " -> " << btmRight << std::endl;
  auto loc = std::pair<coord,coord>(topLeft,btmRight);

  addTraps(loc);

  addItems(loc);

  return loc;
}

void levelGen::addMonsters(std::vector<std::pair<coord,coord>> coords /*by value*/) {
  std::vector<std::pair<unsigned int, monsterType*>> types =
    spawnMonsters(level_->depth(), coords.size());

  for (auto i : types) {
    if (coords.begin() == coords.end()) break;
    auto room = rndPick(coords.begin(), coords.end());
    // we'll avoid the middle square, as that's likely to be where the player starts. Bit hacky.
    // add initial monsters to the room.
    const coord midPoint = mid(*room);

    for (unsigned int c=0; c < i.first; ++c) {
      auto &mt = *(i.second);
      auto monster = ofType(mt, pub_, level_->io_);
      addMonster(monster, midPoint, *room);
    }
    coords.erase(room); // don't use the same room twice; tend to avoid the packs of monsters starting together
  }
  /*
  int monsterCount = numMonsters(); // may be < 0, to increase chance of no initial monsters in a given room
  ::std::cerr << monsterCount << " monsters in level " << level_->depth_
	      << "'s room at " << coords.first << " to " << coords.second
	      << std::endl;*/
}

void levelGen::addMonster(std::shared_ptr<monster> mon, const coord &m, const std::pair<coord, coord> & coords) {
  //::std::cerr << "Placing monster " << i << " of " << monsterCount << std::endl;
  //  std::shared_ptr<monster> mon = roaming(pub_, level_->io_);
  for (int y = coords.first.second; y < coords.second.second; ++y)
    for (int x = coords.first.first; x < coords.second.first; ++x) {
      coord c = coord(x,y);
      if (c == m) continue;
      if (!level_->terrain_[c]->movable(mon)) continue; // roaming monster can't go on this terrain
      level_->addMonster(mon, c);
      return;
    }
}

void levelGen::addTraps(const std::pair<coord,coord> &coords) {
  // chance of a trap in any given room is 1 in (100-L), where L is level depth, but with a bell curve.
  // our special dice adjusts this so that there is a tiny chance of a trap on L0, or no trap on L100,
  // whereas half the rooms on level 50 (on average) will have one.
  // HOWEVER - a trap may only appear where another dungeon feature is not, so the actual chance of a trap
  // will go down a tiny bit too.
  //  if (dPc() < level_->depth_) 
  {
  
    // IT'S A TRAP! // ref: Admiral Ackbar, Star Wars film Episode VI: Return of the Jedi.
    // Pick one at random... we've only got one this release...
    std::uniform_int_distribution<int> dx(coords.first.first+1, coords.second.first - 2);
    std::uniform_int_distribution<int> dy(coords.first.second+1, coords.second.second - 2);
    const coord c(dx(generator), dy(generator));
    if (level_->terrain_.at(c)->type() == terrainType::GROUND)
      level_->terrain_.at(c) = tFactory.get(terrainType::PIT_HIDDEN);
  }
}

void levelGen::addItems(const std::pair<coord,coord> &coords) {
  int itemCount = numItems(); // may be < 0, to increase chance of no initial items in a given room
  std::uniform_int_distribution<int> dx(coords.first.first+1, coords.second.first - 2);
  std::uniform_int_distribution<int> dy(coords.first.second+1, coords.second.second - 2);
  for (int i=0; i < itemCount; ++i) {
    const coord c(dx(generator), dy(generator));
    auto item = createRndItem(level_->depth(), *(level_->io_));
    level_->addItem(item, c);
  }
}


void levelGen::addCorridor(const coord from, const coord to) {
  int delta;
  switch(corridorDir()) {
  case 0: // vertical first
    if (to.second < from.second) delta = -1; else delta = +1;
    for (int y=from.second; y != to.second; y+= delta)
      level_->terrain_[coord(from.first, y)]= tFactory.get(terrainType::GROUND);
    if (to.first < from.first) delta = -1; else delta = +1;
    for (int x=from.first; x != to.first; x+= delta)
      level_->terrain_[coord(x, to.second)]= tFactory.get(terrainType::GROUND);  
    break;
  case 1: // horizontal first
    if (to.first < from.first) delta = -1; else delta = +1;
    for (int x=from.first; x != to.first; x+= delta)
      level_->terrain_[coord(x, from.second)]= tFactory.get(terrainType::GROUND);  
    if (to.second < from.second) delta = -1; else delta = +1;
    for (int y=from.second; y != to.second; y+= delta)
      level_->terrain_[coord(to.first, y)]= tFactory.get(terrainType::GROUND);
    break;
  }
  
}

template<class iter>
void levelGen::place(const iter & begin,
 		    const iter & end,
		    terrainType type) {
  for (auto i=begin; i != end; ++i) {
    if (level_->terrain_[mid(*i)]->type() == terrainType::GROUND) {
      level_->terrain_[mid(*i)]= tFactory.get(type);
      return;
    }
  }
  // fallback (in case all rooms are in the same place): place the down by the up:
  coord c = mid(*begin);
  c.first++;
  level_->terrain_[c]= tFactory.get(type);
}


// the pImpl (pointer-to-implementation) pattern keeps the structure of the class out of the header file:

level::level(dungeon &dungeon, std::shared_ptr<io> ios, const int depth, bool addDownRamp) : 
  pImpl_(::std::make_shared<levelImpl>(dungeon, depth, ios, *this, addDownRamp)) {
}
level::~level() { 
}

drawIter level::drawBegin() const {
  return pImpl_->drawBegin();
}
drawIter level::drawEnd() const {
  return pImpl_->drawEnd();
}

coord level::findTerrain(const terrainType type) const {
  return pImpl_->findTerrain(type);
}

void level::north(monster &m) {
  return pImpl_->north(m);
}
void level::south(monster &m) {
  return pImpl_->south(m);
}
void level::east(monster &m) {
  return pImpl_->east(m);
}
void level::west(monster &m) {
  return pImpl_->west(m);
}
void level::up(monster &m) {
  return pImpl_->up(m);
}
void level::down(monster &m) {
  return pImpl_->down(m);
}
coord level::posOf(const item & it) const {
  return pImpl_->posOf(it);
}
coord level::posOf(const monster & m) const {
  return pImpl_->posOf(m);
}
const coord level::pcPos() const {
  return pImpl_->pcPos();
}
int level::depth() const {
  return pImpl_->depth();
}
void level:: moveTo(const terrainType terrain) {
  pImpl_->moveTo(terrain);
}
void level::moveTo(monster &monster, coord targetPos) {
  pImpl_->moveTo(monster, targetPos);
}
void level:: move(monster &m, const std::pair<char,char> dir, const bool avoidTraps) {
  pImpl_->move(m, dir, avoidTraps);
}
void level::addMonster(std::shared_ptr<monster> monster, coord targetPos) {
  pImpl_->addMonster(monster, targetPos);
}
void level::removeDeadMonster(const monster &m) {
  pImpl_->removeMonster(m);
}
void level::pickUp() {
  pImpl_->pickUp();
}
std::pair<std::multimap<coord, std::shared_ptr<monster> >::iterator,
	  std::multimap<coord, std::shared_ptr<monster> >::iterator> level::monstersAt(const coord &pos) {
  return pImpl_->monstersAt(pos);
}
void level::addItem(const std::shared_ptr<item> item, const coord c) {
  return pImpl_->addItem(item, c);
}
std::unique_ptr<itemHolder> level::holder(const std::shared_ptr<item> item) {
  return holder(posOf(*item)); // no need to bother pImpl for a simple wrapper
}
std::unique_ptr<itemHolder> level::holder(const coord c) {
  return pImpl_->holder(c);
}
dungeon & level::dung() {
  return pImpl_->dung();
}

bool itemHolderLevel::addItem(std::shared_ptr<item> item) {
  // Can't add liquid items to a level
  if (item->material() == materialType::liquid)
    return false;
  level_.addItem(item, coord_);
  return true;
}
iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > itemHolderLevel::contents() {
  return *this;
}
std::vector<std::shared_ptr<item>>::iterator itemHolderLevel::begin() {
  copy_ = level_.itemsAt(coord_);
  return copy_.begin();
}
std::vector<std::shared_ptr<item>>::iterator itemHolderLevel::end() {
  copy_ = level_.itemsAt(coord_);
  return copy_.end();
}
void itemHolderLevel::erase(std::vector<std::shared_ptr<item>>::iterator pos) {
  copy_.erase(pos);
  level_.removeItem(coord_, *pos);
}

// these need to be defined (not just declared) in order to take a reference to them, as in (eg)
// std::max(x, level::MAX_WIDTH); - oddities of the dark corners of C++ I guess.
// see http://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int
const int level::MAX_WIDTH;
const int level::MAX_HEIGHT;
