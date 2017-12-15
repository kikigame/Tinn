/* License and copyright go here*/

#include "level.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "terrain.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "random.hpp"
#include "labyrinth.hpp"
#include "shrine.hpp"

#include <algorithm> // max/min
#include <random>
#include <vector>
#include <string>
#include <sstream>

// define a level in the dungeon

class levelImpl;



// instantiate abstract methods. Note that these are still declarations,
// no must come before their use.
template<>
bool zoneArea<monster>::onExit(std::shared_ptr<monster>, itemHolder&) { return true; }
template<>
bool zoneArea<monster>::onEnter(std::shared_ptr<monster>, itemHolder&) { return true; }
template<>
bool zoneArea<monster>::onMoveWithin(std::shared_ptr<monster>) { return true; }
template<>
bool zoneArea<monster>::onAttack(monster&, monster&) { return true; }
template<>
bool zoneArea<item>::onMoveWithin(std::shared_ptr<item>) {return true; }
template<>
bool zoneArea<item>::onEnter(std::shared_ptr<item>, itemHolder&) { return true; }
template<>
bool zoneArea<item>::onExit(std::shared_ptr<item>, itemHolder&) { return true; }
template<>
bool zoneArea<item>::onAttack(monster&, item&) { return true; }



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

  // add a shrine. Returns first=top-left, second=bottom-right
  std::pair<coord, coord> addShrine();

  // possibly entrap a room:
  void addTraps(const std::pair<coord,coord> &coords);

  // possibly add some monsters to the room:
  void addMonsters(std::vector<std::pair<coord,coord>>);

  // possibly add some items to the room:
  void addItems(const std::pair<coord,coord> &);

  // add a monster to the room, but not at c: (used by addMonsters)
  void addMonster(const std::shared_ptr<monster> mon, const coord &c, const std::pair<coord,coord> &);

  // adds a corridor starting at from and ending at to. Very basic for now.
  // returns first square adjacent to "from"
  coord addCorridor(const coord from, const coord to);

  // average two coords and return their midpoint.
  coord mid(const std::pair<coord,coord> &p) const {
    return coord((p.first.first + p.second.first) / 2,
		 (p.first.second + p.second.second) / 2);
  }

  template<class iter>
  void place(const iter & begin,
 	     const iter & end,
	     terrainType terrainType);

  void place(const coord &c, terrainType terrainType);

private:
  void changeTerrain(coord pos, terrainType from, terrainType to);

public:
  // call to place ramps (in case of adjacent level requirements)
  virtual void negotiateRamps(optionalRef<levelGen> next) = 0;
  // override to specify position of our upramp
  virtual coord upRampPos() { return coord(-1,-1); }
  // call the build() function to add rooms, monsters etc.
  virtual void build() = 0;
};


// a templated level generator that fills the level with a labyrinth
// T is the labyrinth type. See labyrinth.hpp for options.
template <typename T>
class labyGenT : public levelGen {
private:
  const bool addDownRamp_; // do we need a downwards stairwell?
public:
  labyGenT(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),  
    addDownRamp_(addDownRamp) {}
  virtual ~labyGenT() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual void build() {
    place(upRampPos(), terrainType::UP);
    place(downRampPos(), terrainType::DOWN);
    T lab = T(terrainType::UP, // in
	      (addDownRamp_ ? terrainType::DOWN : terrainType::GROUND), // out
	      terrainType::PIT_HIDDEN, // unassigned
	      terrainType::ROCK, // impass
	      terrainType::GROUND, // pass
	      terrainType::PIT, // join (can't be same as ground)
	      level::MAX_WIDTH-4,
	      level::MAX_HEIGHT-2);
    lab.build();

    for (int y=0; y <= level::MAX_HEIGHT-2; ++y)
      for (int x=0; x <= level::MAX_WIDTH-4; ++x)
	place(coord(x,y+1), lab.begin()[x][y]);

    std::vector<std::pair<coord,coord>> pos = locateRooms(lab); // empty if there are no rooms

    addMonsters(pos);
    for (auto p : pos) {
      addTraps(p);
      addItems(p);
    }
  }
  virtual coord upRampPos() {
    return coord(0,1);
  }
  virtual coord downRampPos() {
    return coord(level::MAX_WIDTH/2+2,
		 level::MAX_HEIGHT); // current algorithm limitation
  }
private:
  std::vector<std::pair<coord, coord> > locateRooms(const T &lab) const {
    // identify any 2x2 squares
    std::vector<std::pair<coord, coord> > locs;
    for (int y=0; y <= level::MAX_HEIGHT-3; ++y)
      for (int x=0; x <= level::MAX_WIDTH-5; ++x)
	if ((lab.begin()[x][y] != terrainType::ROCK) &&
	    (lab.begin()[x][y+1] != terrainType::ROCK) &&
	    (lab.begin()[x+1][y] != terrainType::ROCK) &&
	    (lab.begin()[x+1][y+1] != terrainType::ROCK)) {
	  locs.emplace_back(coord(x,y), coord(x+1,y+1));
	}
    int count;
    do {
      // h-merge: merge any horizontally-adjacent areas with the same vertical height
      count = hmerge(locs);
      // v-merge: merge any vertially-adjacent areas with the same horizontal width
      count += vmerge(locs);
    } while (count > 0);

    // eliminate anything overlapping (or we get too many monsters!)
    for (int i=0; i < locs.size(); ++i)
      for (int j=i+1; j < locs.size();)
	if ((locs[j].first.first > locs[i].first.first && locs[j].first.first < locs[i].second.first) &&
	    (locs[j].first.second > locs[i].first.second && locs[j].first.second < locs[i].second.second) &&
	    (locs[j].second.first > locs[i].first.first && locs[j].first.first < locs[i].second.first) &&
	    (locs[j].second.second > locs[i].first.second && locs[j].first.second < locs[i].second.second))
	  locs.erase(locs.begin() + j);
	else ++j;

    return locs;
  }
  int hmerge(std::vector<std::pair<coord, coord> > &locs) const {
    int size = locs.size();
    for (int i=0; i < locs.size(); ++i)
      for (int j=i+1; j < locs.size();)
	if (locs[i].first.second == locs[j].first.second && locs[i].second.second == locs[j].second.second &&
	    locs[i].second.first >= locs[j].first.first && locs[i].second.first <= locs[j].second.first) {
	  locs[i].second.first = locs[j].second.first;
	  locs.erase(locs.begin() + j);
	} else ++j;
    return locs.size() - size;
  }
  int vmerge(std::vector<std::pair<coord, coord> > &locs) const {
    int size = locs.size();
    for (int i=0; i < locs.size(); ++i)
      for (int j=i+1; j < locs.size();)
	if (locs[i].first.first == locs[j].first.first && locs[i].second.first == locs[j].second.first &&
	    locs[i].second.second >= locs[j].first.second && locs[i].second.second <= locs[j].second.second) {
	  locs[i].second.second = locs[j].second.second;
	  locs.erase(locs.begin() + j);
	} else ++j;
    return locs.size() - size;
  }
};

// a level generator that fills the level with a labyrinth
typedef labyGenT<labyrinth<terrainType> > labyGen;

// a level generator that fills the entire level with rooms and divider walls
typedef labyGenT<labyrinth<terrainType, emptyfiller<terrainType>, true, false, imperfectRecursor<terrainType, emptyfiller<terrainType>, true, false> > > labyRoomGen;

// a level generator that fills the level with a very imperfect labyrinth; lots of rocks
typedef labyGenT<labyrinth<terrainType, pathfiller<terrainType>, true, false, imperfectRecursor<terrainType, pathfiller<terrainType>, true, false > > > labySmallGen;

// a labyrinth generator that prefers horizontal lines
typedef labyGenT<labyrinth<terrainType, castellationFillerH<terrainType> > > labyHGen;

// a labyrinth generator that prefers vertical lines
typedef labyGenT<labyrinth<terrainType, castellationFillerW<terrainType> > > labyWGen;

// a level generator that fills the level with randomly-placed rooms
class roomGen : public levelGen {
private:
  const bool addDownRamp_; // do we need a downwards exit?
  coord downRampPos_; // if set
public:
  roomGen(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),
    addDownRamp_(addDownRamp),
    downRampPos_(-1,-1) {}
  virtual ~roomGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {
    if (addDownRamp_ && next) {
      downRampPos_ = next.value().upRampPos();
      if (downRampPos_.first >= 0)
	place(downRampPos_, terrainType::DOWN);
    }
  }
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
    if (addDownRamp_) {
      if (downRampPos_.first < 0)
	place(pos.rbegin(), pos.rend(), terrainType::DOWN);
      else
	addCorridor(mid(pos[0]), downRampPos_);
    }

    addMonsters(pos);

    // place a shrine sometimes
    if (dPc() <= 10) {
      auto shrinePos = addShrine();
      auto shrineDir = addCorridor(mid(shrinePos), mid(pos[0]));
      coord alterPos = shrineDir.next(mid(shrinePos));
      place(alterPos, terrainType::ALTAR);
    }
  }
};

// manages the items in a given cell by adapting levelImpl to the itemHolder interface
// at specified coords
class itemHolderLevel : 
  public itemHolder {
private:
  levelImpl& level_;
  const coord coord_;
public:
  itemHolderLevel(levelImpl & level, const coord & c) :
    level_(level), coord_(c) {}
  virtual bool addItem(item &item); // overridden to handle movement callbacks
  virtual bool removeItemForMove(item &item, itemHolder &next); // overridden to handle movement callbacks
  virtual ~itemHolderLevel() {}
};

// implementation of level class 
class levelImpl : public renderByCoord {
public:
  // the dungeon this level is in
  dungeon& dungeon_;
  // how many levels deep are we?
  const int depth_;
  // map of coordinate to stack of items at that location - TODO: do we need this with holders_?
  ::std::multimap<coord, ::std::weak_ptr<item> > items_;
  // map of coordinate to all monsters at that location
  ::std::multimap<coord, ::std::shared_ptr<monster> > monsters_;
  // terrain type by coordinate
  ::std::map<coord, ::std::shared_ptr<terrain> > terrain_;
  // what special zones are in this level?
  std::vector<std::shared_ptr<zoneArea<item> > > itemZones_;
  std::vector<std::shared_ptr<zoneArea<monster> > > monsterZones_;

  // hold the itemHolderLevels
  std::map<coord, ::std::unique_ptr<itemHolder> > holders_;

  // constructor fills the level with something suitable
  levelImpl(dungeon &dungeon, int depth) :
    dungeon_(dungeon),
    depth_(depth) {
    using namespace std;
    for (int x=0; x < level::MAX_WIDTH ; ++x)
      for (int y=0; y < level::MAX_HEIGHT ; ++y) {
	coord c(x,y);
	terrain_[c] = tFactory.get(terrainType::ROCK);
      }
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
    if (it.first != it.second) {
      auto rtn = it.first->second.lock();
      if (rtn) {
	return *(rtn);	
      }
      else {
	rtn = rtn;
      }
    }
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
    for (auto z : zonesAt(posOf(aggressor), true))
      if (!z->onAttack(aggressor, target)) return;
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
    if (longMsg) ioFactory::instance().longMsg(rtn);
    else ioFactory::instance().message(rtn);
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
    move(m, std::make_pair<char,char>(0,-1), true);
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
    move(m, std::make_pair<char,char>(0,+1), true);
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
    move(m, std::make_pair<char,char>(+1,0), true);
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
    move(m, std::make_pair<char,char>(-1,0), true);
  }
  void up(monster &m) {
    coord c = posOf(m);
    if (depth_ <= 1) {
      ioFactory::instance().message(L"Leaving the dungeon isn't implemented yet.");
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
      ioFactory::instance().message(L"There is no way up here.");
    }
  }
  void down(monster &m) {
    coord c = posOf(m);
    if (depth_ == dungeon_.maxLevel()) {
      ioFactory::instance().message(L"You are already at the bottom of the game.");
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
      ioFactory::instance().message(L"There is no way down here.");
    }
  }
  coord posOf(const item &it) const {
    for (auto i = items_.begin(); i != items_.end(); ++i) {
      if (&(*(i->second.lock())) == &it) {
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

  typedef filteredIterable<std::shared_ptr<zoneArea<item> >, std::vector<std::shared_ptr<zoneArea<item> > > >
  itemZoneIter;
  typedef filteredIterable<std::shared_ptr<zoneArea<monster> >, std::vector<std::shared_ptr<zoneArea<monster> > > >
  monsterZoneIter;
  // you can't specialize member methods, so we'll just overload here, and use a traits class to switch between:
  itemZoneIter
  zonesAt(const coord &c) { return itemZoneIter(itemZones_, [&c] (std::shared_ptr<zoneArea<item> > z) {
	return z->contains(c);});};
  monsterZoneIter
  zonesAt(const coord &c, bool) { return monsterZoneIter(monsterZones_, [&c] (std::shared_ptr<zoneArea<monster> > z) {
	return z->contains(c);});};

  // find a piece of terrain and move to it (NB: This won't work with Nethack-style branch levels)
  void moveTo(const terrainType terrain) {
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (i->second->isPlayer()) {
	moveTo(*(i->second), findTerrain(terrain));
	return;
      }
    }
  }
  // teleport a monster. NB: This will move the monster regardless of any traps, items or other things in the way
  // UNLESS zone effects do not allow it
  void moveTo(monster &m, const coord &dest) {
    std::shared_ptr<monster> pM;
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (*(i->second) == m) {
	pM = i->second;
	zoneActions<monster> zones(zonesAt(i->first, true), zonesAt(dest, true));
	for (auto z : zones.same())
	  if (!z->onMoveWithin(pM)) return;
	for (auto z : zones.leaving())
	  if (!z->onExit(pM, holder(dest))) return;
	for (auto z : zones.entering())
	  if (!z->onEnter(pM, holder(i->first))) return;
	monsters_.erase(i);
	break;
      }
    }
    addMonster(pM, dest);
    // reveal any pits:
    if (!m.abilities().fly() && terrain_[dest]->type() == terrainType::PIT_HIDDEN) {
      if (m.isPlayer()) // TODO: Pit traps should not be revealed by flying monsters, or should they?
	ioFactory::instance().message(L"It's a (pit) trap!"); // ref: Admiral Ackbar, Star Wars film Episode VI: Return of the Jedi.
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
      msg.emplace_back(i->second.lock()->name());
    
    std::wstring message;
    switch (msg.size()) {
    case 0:
      return; // no message
    case 1:
      message += L"There's a " + msg[0] + L" here";
      ioFactory::instance().message(message);
      break;
    default:
      message = L"In your space are:\n";
      for (auto m : msg) message += m + L"\n";
      ioFactory::instance().longMsg(message);
    }
  }

  void sanitiseCoords(coord &pos) const {
    if (pos.first < 0) pos.first = 0;
    if (pos.second < 0) pos.second = 0;
    if (pos.first >= level::MAX_WIDTH) pos.first = level::MAX_WIDTH;
    if (pos.second >= level::MAX_HEIGHT) pos.second = level::MAX_HEIGHT;
  }

  
  bool movable(const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) {
    auto t = terrain_[pos];
    if (!t->movable(m)) return false;
    if (avoidTraps && t->entraps(m, false)) return false;
    if (avoidHiddenTraps && t->entraps(m, true)) return false;
    return true;
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
    if (!avoidTraps || !terrain_[pos]->entraps(m, false)) // avoid traps if we should & can see them
      if (!m.abilities().entrapped()) // can't move if trapped regardless
	moveTo(m, pos);
    // TODO: monster items and inventory- collect the new stuff?
  }
  void removeMonster(const monster &m) {
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      if (*(i->second) == m) {
	monsters_.erase(i);
	return; // iterator is now invalid, but we found it.
      }
    }
  }
  void removeDeadMonster(monster &m) {
    // monster's inventory is dropped
    const coord c = posOf(m);
    auto &h = holder(c);
    m.forEachItem([&h](item &i, std::wstring) {
	h.addItem(i);
      });
    // corpse is added
    auto &corpse = createCorpse(m.type(), m.injury().max());
    holder(c).addItem(corpse);
    // monster is removed
    removeMonster(m);
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

  // called by itemHolderLevel; don't use directly
  void addItem(item &item, const coord c) {
    items_.emplace(c, item.shared_from_this()); // items may always occupy the same square
  }

  void pickUp() {
    auto pos = pcPos();
    auto h = holder(pos);
    if (h.empty()) {
      ioFactory::instance().message(L"You can see nothing to collect here.");
    }
    h.forEachItem([this,pos](item &it, std::wstring name) {
      if (ioFactory::instance().ynPrompt(L"Do you want to collect: " + name + L"?")) {
	dungeon_.pc()->addItem(it);
	removeItem(pos, it);
      }});
  }

  std::pair<std::multimap<coord, std::shared_ptr<monster> >::iterator,
	    std::multimap<coord, std::shared_ptr<monster> >::iterator> monstersAt(const coord &pos) {
    return monsters_.equal_range(pos);
  }

  dungeon & dung() { return dungeon_; }

  itemHolder &holder(const coord c) {
    auto rtn = holders_.find(c);
    if (rtn == holders_.end()) 
      holders_.emplace(c, std::unique_ptr<itemHolder>(new itemHolderLevel(*this, c)));
    return *(holders_[c]);
  }

  // used by itemHolderLevel:
  std::vector<std::shared_ptr<item>> itemsAt(const coord & pos) const {
    std::vector<std::shared_ptr<item>> rtn;
    auto it = items_.equal_range(pos);
    for (auto i = it.first; i != it.second; ++i) rtn.push_back(i->second.lock());
    return rtn;
  }
  // used by itemHolderLevel:
  void removeItem(const coord & pos, item &item) {
    auto it = items_.equal_range(pos);
    for (auto i = it.first; i != it.second; ++i)
      if (i->first == pos && i->second.lock().get() == &item) {
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

std::pair<coord,coord> levelGen::addShrine() {
  int width = 5, height = 5;

  auto rock = tFactory.get(terrainType::ROCK);

  // look for a contiguous 5x5 place on the map that is only rock.
  bool found;
  int xPos, yPos;
  do {
    // for now, just place the room. Don't care if they overlap.
    // X pos is between 1 (to allow for border on left) and MAX_WIDTH - width - 2
    std::uniform_int_distribution<int> xPosD(1,level::MAX_WIDTH - width - 2);
    // Y pos is between 1 (to allow for border on top) and MAX_HEIGHT - height - 2
    std::uniform_int_distribution<int> yPosD(1,level::MAX_HEIGHT - height - 2);
    xPos = xPosD(generator), yPos = yPosD(generator);

    found = true;
    for (int y=0; y < height; ++y)
      for (int x=0; x < width; ++x) {
	coord c(x + xPos,y + yPos);
	if (level_->terrain_.at(c) != rock) {
	  found = false;
	  break;
	}
      }

  } while (!found);

  auto ground = tFactory.get(terrainType::GROUND);
  for (int y=1; y < height-1; ++y) {
    for (int x=1; x < width-1; ++x) {
      coord c(x + xPos,y + yPos);
      level_->terrain_.at(c) = ground;
    }
  }

  coord topLeft(xPos,yPos);
  coord btmRight(xPos+width, yPos+height);

  auto loc = std::pair<coord,coord>(topLeft,btmRight);

  auto shr = std::make_shared<shrine>(topLeft, btmRight);
  level_->itemZones_.push_back(shr);
  level_->monsterZones_.push_back(shr);
  level_->holder(coord(xPos+2,yPos+2)).addItem(createHolyBook(shr->align()));

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
      auto monster = ofType(mt, pub_);
      addMonster(monster, midPoint, *room);
    }
    coords.erase(room); // don't use the same room twice; tend to avoid the packs of monsters starting together
  }
}

void levelGen::addMonster(std::shared_ptr<monster> mon, const coord &m, const std::pair<coord, coord> & coords) {
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
    if (coords.second.first - 2 > coords.first.first+1 && 
	coords.second.second - 2 > coords.first.second + 1) {
      std::uniform_int_distribution<int> dx(coords.first.first+1, coords.second.first - 2);
      std::uniform_int_distribution<int> dy(coords.first.second+1, coords.second.second - 2);
      const coord c(dx(generator), dy(generator)); // coords=(0,0)-(2,1) but c=gibberish
      if (level_->terrain_.at(c)->type() == terrainType::GROUND)
	level_->terrain_.at(c) = tFactory.get(terrainType::PIT_HIDDEN);
    }
  }
}

void levelGen::addItems(const std::pair<coord,coord> &coords) {
  int itemCount = numItems(); // may be < 0, to increase chance of no initial items in a given room
  std::uniform_int_distribution<int> dx(coords.first.first+1, coords.second.first - 2);
  std::uniform_int_distribution<int> dy(coords.first.second+1, coords.second.second - 2);
  for (int i=0; i < itemCount; ++i) {
    const coord c(dx(generator), dy(generator));
    item &item = createRndItem(level_->depth());
    level_->holder(c).addItem(item);
  }
}

void levelGen::changeTerrain(coord c, terrainType from, terrainType to) {
  if (level_->terrain_[c] == tFactory.get(from))
    level_->terrain_[c] = tFactory.get(to);
}

coord levelGen::addCorridor(const coord from, const coord to) {
  coord rtn;
  int delta;
  switch(corridorDir()) {
  case 0: // vertical first
    if (to.second < from.second) delta = -1; else delta = +1;
    for (int y=from.second; y != to.second; y+= delta) {
      changeTerrain(coord(from.first, y), terrainType::ROCK, terrainType::GROUND);
      rtn = coord(from.first,from.second+delta);
    }
    if (to.first < from.first) delta = -1; else delta = +1;
    for (int x=from.first; x != to.first; x+= delta)
      changeTerrain(coord(x, to.second), terrainType::ROCK, terrainType::GROUND);
    break;
  case 1: // horizontal first
    if (to.first < from.first) delta = -1; else delta = +1;
    for (int x=from.first; x != to.first; x+= delta) {
      changeTerrain(coord(x, from.second), terrainType::ROCK, terrainType::GROUND);
      rtn = coord(from.first+delta,from.second);
    }
    if (to.second < from.second) delta = -1; else delta = +1;
    for (int y=from.second; y != to.second; y+= delta)
      changeTerrain(coord(to.first, y), terrainType::ROCK, terrainType::GROUND);
    break;
  }
  return rtn;
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
  place(c,type);
}

void levelGen::place(const coord &c, terrainType type) {
  level_->terrain_[c]= tFactory.get(type);  
}


// the pImpl (pointer-to-implementation) pattern keeps the structure of the class out of the header file:

level::level(levelImpl *pImpl) :
  pImpl_(pImpl) {}
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
bool level::movable(const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) const {
  return pImpl_->movable(pos, m, avoidTraps, avoidHiddenTraps);
}
void level::addMonster(std::shared_ptr<monster> monster, coord targetPos) {
  pImpl_->addMonster(monster, targetPos);
}
void level::removeDeadMonster(monster &m) {
  pImpl_->removeDeadMonster(m);
}
void level::pickUp() {
  pImpl_->pickUp();
}
std::pair<std::multimap<coord, std::shared_ptr<monster> >::iterator,
	  std::multimap<coord, std::shared_ptr<monster> >::iterator> level::monstersAt(const coord &pos) {
  return pImpl_->monstersAt(pos);
}
itemHolder &level::holder(const item& item) {
  return holder(posOf(item)); // no need to bother pImpl for a simple wrapper
}
itemHolder &level::holder(const coord c) {
  return pImpl_->holder(c);
}
dungeon & level::dung() {
  return pImpl_->dung();
}

bool itemHolderLevel::addItem(item &item) {
  // Can't add liquid items to a level
  if (item.material() == materialType::liquid)
    return false;
  auto &map = itemHolderMap::instance();
  if (!map.beforeFirstAdd(item)) {
    for (auto z : level_.zonesAt(coord_))
      if (!z->onEnter(item.shared_from_this(), item.holder())) return false;
  }
  itemHolder::addItem(item);
  level_.addItem(item, coord_);
  return true;
}
bool itemHolderLevel::removeItemForMove(item &item, itemHolder &next) {
  auto pos = level_.posOf(item);
  for (auto z : level_.zonesAt(coord_))
    if (!z->onExit(item.shared_from_this(), next)) return false;
  level_.removeItem(coord_, item);
  return true;
}

// hack to pretend we implement a readonly version of the interface:
level::operator const renderByCoord&() const {
  return *pImpl_;
}


// this is a bit convoluted, but keeps a simple interface to zonesAt() while allowing a
// different implementation per type, operating off a different field:
template <typename T>
struct zoneTraits {};
template<>
struct zoneTraits<item>{ 
  levelImpl::itemZoneIter zonesAt(levelImpl &p, const coord &c) { return p.zonesAt(c); } 
};
template<>
struct zoneTraits<monster>{ 
  levelImpl::monsterZoneIter zonesAt(levelImpl &p, const coord &c) { return p.zonesAt(c, true); } 
};

template <typename T>
filteredIterable<std::shared_ptr<zoneArea<T> >,std::vector<std::shared_ptr<zoneArea<T> > > > 
level::zonesAt(const coord & c) {
  return zoneTraits<T>::zonesAt(*pImpl_, c);
}


class levelFactoryImpl {
private:
  int numLevels_;
  std::vector<levelImpl*> levels_;
  std::vector<level*> levelPubs_;
  std::vector<std::unique_ptr<levelGen> > levelGen_;
public:
  levelFactoryImpl(dungeon &dungeon, const int numLevels) : 
    numLevels_(numLevels) {
    for (int i=0; i < numLevels; ++i) {
      levelImpl *l = new levelImpl(dungeon, i);
      levels_.push_back(l);
      levelPubs_.emplace_back(new level(l));
      auto &level = *(levelPubs_.rbegin());
      levelGen_.emplace_back(createGen(i, l, level));
    }
  }
  void build() {
    auto begin = levelGen_.begin();
    auto end = levelGen_.end();
    volatile int pos=0; // for debugging (TODO:removeme)
    for (auto i = begin; i != end; ++i, ++pos)
      (*i)->negotiateRamps(
	 i == end || i+1 == end ? optionalRef<levelGen>() : optionalRef<levelGen>(**(i+1)));
    pos=0;
    for (auto i = begin; i != end; ++i, ++pos)
      (*i)->build();
  }
  std::vector<level*>::iterator begin() {
    return levelPubs_.begin();
  }
  std::vector<level*>::iterator end() {
    return levelPubs_.end();
  }
private:
  levelGen *createGen(int depth, levelImpl *l, level *level) {
    bool addDownRamp = depth < numLevels_;
    if (dPc() < depth)
      switch (depth / 10) {
      case 0:
	try {
	  return new  labyRoomGen(l, *level, addDownRamp);
	} catch (char const * msg) { /*std::cerr << "labyRoomGen: " << msg << std::endl; */}
      case 1:
      case 2:
      case 3:
      case 4:
	try {
	  return new  labySmallGen(l, *level, addDownRamp);
	} catch (char const * msg) { /*std::cerr << "labySmallGen: " << msg << std::endl; */}
      case 5:
      case 6:
      case 7:
	try {
	  return new  labyHGen(l, *level, addDownRamp);
	} catch (char const * msg) { /*std::cerr << "labyHGen: " << msg << std::endl; */}
      case 8:
	try {
	  return new  labyWGen(l, *level, addDownRamp);
	} catch (char const * msg) { /*std::cerr << "labyWGen: " << msg << std::endl; */}
      default:
	try {
	  return new  labyGen(l, *level, addDownRamp);
	} catch (char const * msg) { /*std::cerr << "labyGen: " << msg << std::endl; */}
      }
    // default case & fall-through:
    return new  roomGen(l, *level, addDownRamp); 
  }
};

levelFactory::levelFactory(dungeon &dungeon, const int numLevels) :
  pImpl_(new levelFactoryImpl(dungeon, numLevels)) {
  pImpl_->build();
}

std::vector<level*>::iterator levelFactory::begin() {
  return pImpl_->begin();
}
std::vector<level*>::iterator levelFactory::end() {
  return pImpl_->end();
}

// these need to be defined (not just declared) in order to take a reference to them, as in (eg)
// std::max(x, level::MAX_WIDTH); - oddities of the dark corners of C++ I guess.
// see http://stackoverflow.com/questions/5391973/undefined-reference-to-static-const-int
const int level::MAX_WIDTH;
const int level::MAX_HEIGHT;

