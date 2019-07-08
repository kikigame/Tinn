/* License and copyright go here*/

#include "level.hpp"
#include "levelGen.hpp"
#include "levelFactory.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "terrain.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "random.hpp"
#include "labyrinth.hpp"
#include "shrine.hpp"
#include "religion.hpp"
#include "role.hpp"
#include "transport.hpp"
#include "ref.hpp"
#include "combat.hpp"

#include <algorithm> // max/min
#include <random>
#include <vector>
#include <string>
#include <sstream>

// define a level in the dungeon

class levelImpl;



// instantiate abstract methods. Note that these are still declarations,
// must come before their use.
template<>
bool zoneArea<monster>::onExit(monster &, itemHolder&) { return true; }
template<>
bool zoneArea<monster>::onEnter(monster &, itemHolder&) { return true; }
template<>
bool zoneArea<monster>::onMoveWithin(monster &, const coord &) { return true; }
template<>
bool zoneArea<monster>::onAttack(monster&, monster&) { return true; }
template<>
bool zoneArea<item>::onMoveWithin(item &, const coord &) {return true; }
template<>
bool zoneArea<item>::onEnter(item &, itemHolder&) { return true; }
template<>
bool zoneArea<item>::onExit(item &, itemHolder&) { return true; }
template<>
bool zoneArea<item>::onAttack(monster&, item&) { return true; }


// a templated level generator that fills the level with a labyrinth
// T is the labyrinth type. See labyrinth.hpp for options.
template <typename T>
class labyGenT : public levelGen {
private:
  const bool addDownRamp_; // do we need a downwards stairwell?
  int maxX_, maxY_;
public:
  labyGenT(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),  
    addDownRamp_(addDownRamp), 
    maxX_(level::MAX_WIDTH - 4),
    maxY_(level::MAX_HEIGHT - 2) {}
  virtual ~labyGenT() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual void build() {
    place(upRampPos(), terrainType::UP);
    place(downRampPos(), terrainType::DOWN);
    switch (dPc() / 10) {
    case 0: maxX_ -=12; maxY_ -=4; break;
    case 1: maxX_ -=12; maxY_ -=2; break;
    case 2: maxX_ -=12; break;
    case 3: maxX_ -=8; maxY_ -=4; break;
    case 4: maxX_ -=8; maxY_ -=2; break;
    case 5: maxX_ -=8; break;
    case 6: maxX_ -=4; maxY_ -=4; break;
    case 7: maxX_ -=4; maxY_ -=2; break;
    case 8: maxX_ -=4; break;
    case 9: maxY_ -=4; break;
    case 10: maxY_ -=2; break;
    }
    T lab = T(terrainType::UP, // in
	      (addDownRamp_ ? terrainType::DOWN : terrainType::GROUND), // out
	      terrainType::PIT_HIDDEN, // unassigned
	      terrainType::ROCK, // impass
	      terrainType::GROUND, // pass
	      terrainType::PIT, // join (can't be same as ground)
	      maxX_,
	      maxY_);
    lab.build();

    for (int y=0; y < level::MAX_HEIGHT; ++y)
      for (int x=0; x < level::MAX_WIDTH; ++x)
	if ( x <= maxX_ && y <= maxY_)
	  place(coord(x,y+1), lab.begin()[x][y]);
	else place(coord(x,y+1), terrainType::ROCK);

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
    for (int y=0; y <= maxY_-1; ++y)
      for (int x=0; x <= maxX_-1; ++x)
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
    for (unsigned int i=0; i < locs.size(); ++i)
      for (unsigned int j=i+1; j < locs.size();)
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
    for (unsigned int i=0; i < locs.size(); ++i)
      for (unsigned int j=i+1; j < locs.size();)
	if (locs[i].first.second == locs[j].first.second && locs[i].second.second == locs[j].second.second &&
	    locs[i].second.first >= locs[j].first.first && locs[i].second.first <= locs[j].second.first) {
	  locs[i].second.first = locs[j].second.first;
	  locs.erase(locs.begin() + j);
	} else ++j;
    return locs.size() - size;
  }
  int vmerge(std::vector<std::pair<coord, coord> > &locs) const {
    unsigned int size = locs.size();
    for (unsigned int i=0; i < locs.size(); ++i)
      for (unsigned int j=i+1; j < locs.size();)
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
    try {
      if (dPc() <= 10) {
	auto shrinePos = addShrine();
	++shrinePos.second.first;
	++shrinePos.second.second;
	auto shrineDir = addCorridor(mid(shrinePos), mid(pos[0]));
	coord alterPos = shrineDir.next(mid(shrinePos));
	place(alterPos, terrainType::ALTAR);
      }
    } catch (std::wstring s) {
      // ignore; we can manage without a shrine
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
  virtual ~itemHolderLevel() {}
  virtual bool addItem(item &item); // overridden to handle movement callbacks
  virtual bool removeItemForMove(item &item, itemHolder &next); // overridden to handle movement callbacks
};


std::wstring nth(const int i) {
  std::wstringstream rtn;
  rtn << i;
  if ((i % 100) >= 11 && (i % 100) <= 13)
    rtn << L"th";
  else switch (i % 10) {
  case 1: rtn << L"st"; break; // 1st // 11th // 21st // 31st // 41st // 51st // 61st // 71st // 81st // 91st // 101th // 111th 
  case 2: rtn << L"nd"; break;
  case 3: rtn << L"rd"; break;
  default: rtn << L"th"; break;
  }
  return rtn.str();
}

class terrainRef : public ref<const terrain> {
public:
  terrainRef() :
    ref<const terrain>(tFactory.get(terrainType::GROUND)) {}
  terrainRef(const terrain &other) :
    ref<const terrain>(other) {}
  terrainType type() const { return value().type(); }
};

// implementation of level class 
class levelImpl : public renderByCoord {
public:
  // the dungeon this level is in
  dungeon& dungeon_;
  // how many levels deep are we?
  const int depth_;
  // map of coordinate to all monsters at that location
  ::std::multimap<coord, ::std::shared_ptr<monster> > monsters_;
  // terrain type by coordinate
  ::std::map<coord, terrainRef> terrain_;
  // what special zones are in this level?
  std::vector<std::shared_ptr<zoneArea<item> > > itemZones_;
  std::vector<std::shared_ptr<zoneArea<monster> > > monsterZones_;

  // hold the itemHolderLevels
  std::map<coord, ::std::unique_ptr<itemHolder> > holders_;

  // some sort of name for the level.
  std::wstring name_;

  // constructor fills the level with something suitable
  levelImpl(dungeon &dungeon, int depth) :
    dungeon_(dungeon),
    depth_(depth),
    name_(L"The " + nth(depth) + L" Area of Adventure") {
    using namespace std;
    for (coord c : coordRectIterator(0, 0, level::MAX_WIDTH-1, level::MAX_HEIGHT-1))
      terrain_[c] = tFactory.get(terrainType::ROCK);
  }
  virtual ~levelImpl() {}

  std::wstring name() const {
    return name_;
  }

  drawIter drawBegin() const {
    return drawIter(*this, coord(0,0), level::MAX_WIDTH, level::MAX_HEIGHT);
  }

  drawIter drawEnd() const {
    return drawIter(*this, coord(-1,-1), level::MAX_WIDTH, level::MAX_HEIGHT);
  }

  virtual const renderable & renderableAt(const coord & pos) const {
    // show monsters if any:
    auto mn = monsters_.equal_range(pos);
    if (mn.first != mn.second) return *(mn.first->second);
    // show items if any:
    auto it = holder(pos).firstItem();
    if (it) return it.value();
    // show terrain if nothing else:
    return terrain_.at(pos).value();
  }

  const terrain &terrainAt(const coord & c) const {
    return terrain_.at(c).value();
  }

  coord findTerrain(const terrainType &type) const {
    using namespace std;
    for (coord c : coordRectIterator(0,0,level::MAX_WIDTH-1, level::MAX_HEIGHT - 1))
      if (terrain_.at(c).type() == type) 
	return c;
    throw wstring(L"Terrain type ") + to_string(type) + wstring(L" not found on level ") + to_wstring(depth_);
  }

  std::vector<coord> findAllTerrain(const terrainType &type) const {
    using namespace std;
    vector<coord> rtn;
    for (coord c : coordRectIterator(0,0,level::MAX_WIDTH-1, level::MAX_HEIGHT - 1))
      if (terrain_.at(c).type() == type) 
	rtn.emplace_back(c);
    return rtn;;
  }
  
  coord findTerrain(const terrainType t, const int width, const int height) const {
    bool found = false;
    int xPos, yPos;
    for (int triesLeft=100; triesLeft>0 && !found; --triesLeft) {
      // X pos is between 1 (to allow for border on left) and MAX_WIDTH - width - 2
      std::uniform_int_distribution<int> xPosD(1,level::MAX_WIDTH - width - 2);
      // Y pos is between 1 (to allow for border on top) and MAX_HEIGHT - height - 2
      std::uniform_int_distribution<int> yPosD(1,level::MAX_HEIGHT - height - 2);
      xPos = xPosD(generator), yPos = yPosD(generator);
	  
      found = true;
      for (coord c : coordRectIterator(xPos, yPos, width + xPos-1, height + yPos - 1))
	if (terrain_.at(c).type() != t) {
	  found = false;
	  break;
	}
      if (found) return coord(xPos,yPos);
    }
    using namespace std;
    throw wstring(L"Terrain type ") + to_string(t) + wstring(L" not found in required size on level ") + to_wstring(depth_);
  }


  // dead creatures are removed from the level (shared_ptr).
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

  optionalRef<item> thrownWeapon(monster &m) {
    if (!m.abilities()->throws()) return optionalRef<item>();
    return m.firstItem([&m](item &i){
	auto pUse = dynamic_cast<useInCombat*>(&i);
	return pUse && i.render() == L'¬' && pUse->shouldUse(m);
      });
  }
  optionalRef<item> zapItem(monster &m) {
    if (!m.abilities()->zap()) return optionalRef<item>();
    return m.firstItem([&m](item &i){
	auto pUse = dynamic_cast<useInCombat*>(&i);
	return pUse && pUse->shouldUse(m);
      });
  }


  optionalRef<monster> lineOfSightTarget(monster &m, const coord  &mPos) {
    if (!m.abilities()->see()) return optionalRef<monster>(); // can't see targets
    for (auto it = monsters_.begin(); it != monsters_.end(); ++it) {
      const coord &tPos = it->first;
      std::shared_ptr<monster> t = it->second; // target monster
      // don't self-flagellate
      if (&(*t) == &m) continue;
      // only non-coaligned monsters attack
      if (m.align().coalignment(t->align()) >= 3) continue;
      // never throw into a temple (let's say any zone for ease)
      auto zones = zonesAt(posOf(*t), true);
      if (zones.begin() != zones.end()) continue;
      // skip anyone charming
      if (std::find_if(m.charmedBegin(), m.charmedEnd(),
		       [&t](const std::pair<const monster*, const monster*> &p){return p.second == &(*t);}) != m.charmedEnd()) continue;
      // check for a line of movement
      coord c;
      for (c = mPos; c != tPos; c = c.towards(tPos))
	if (!m.abilities()->move(terrainAt(c)))
	  break; // give up
      if (c == tPos) return optionalRef<monster>(*t); // found one!
    }
    return optionalRef<monster>(); // no targets found
  }

  void moveOrFight(monster &m, const ::dir &dir, bool avoidTraps) {
    coord c = posOf(m);
    coord cc = c.inDir(dir);
    if (cc.first < 0 || cc.second < 0 || cc.first == level::MAX_WIDTH || cc.second == level::MAX_HEIGHT)
      return; // can't move above top of map
    if (!movable(c,cc,m,avoidTraps, false)) return; // can't move into terrain
    auto mn = monsters_.find(cc);
    while (mn != monsters_.end()) {
      attack(m, *(mn->second)); // can't move into monster
      return;
    }
    if (!m.isPlayer()) {
      // if the monster can throw AND has a thrown item, or can zap AND has a wand, then we should check for monsters in line of sight.
      // if there are any, then attack that way.
      auto thrown = thrownWeapon(m);
      auto zapped = zapItem(m);
      auto target = lineOfSightTarget(m,c); // we only really care if any target exists at this point.
      if ((thrown || zapped) && target && dPc() <= 40 /* ~ 33% chance */ ) {
	if (thrown || (thrown && zapped && dPc() <= 50)) {
	  // lob/fire at the target:
	  dynamic_cast<useInCombat&>(thrown.value()).useForCombat();
	} else {
	  // zap zapped.value() at a target
	  zapped.value().use();
	}
	return;
      }
    }
    move(m, dir, avoidTraps);
  }
  void up(monster &m) {
    coord c = posOf(m);
    switch (terrain_.at(c).type()) {
    case terrainType::UP:
      if (depth_ <= 1) {
	auto &role = dung().pc()->job();
	for (auto pQ = role.questsBegin(); pQ != role.questsEnd(); ++pQ) {
	  if (!pQ->isSuccessful() && !ioFactory::instance().ynPrompt(pQ->incompletePrompt()))
	    return;
	  endGame();
	}
	return;
      }
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
    switch (terrain_.at(c).type()) {
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
    for (auto &h : holders_) {
      auto i = h.second->firstItem([&it](item &i) { return &i == &it; });
      if (i) return h.first;
    }
    return coord(-1,-1);
  }
  coord posOf(const monster &m) const {
    auto bm = dynamic_cast<const ::bigMonster*>(&m);
    if (bm) return bm->mainPos();
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
  optionalRef<monster> findMonster(monster &from, const wchar_t dir) const {
    if (dir == L',') return optionalRef<monster>(from);
    coord pos = posOf(from);
    coord towards;
    switch (dir) {
    case '<': { // up: find any flying monsters on the current square
      auto mAt = monstersAt(pos);
      for (auto i : mAt)
	if (i.value().abilities()->fly())
	  return optionalRef<monster>(i.value());
      return optionalRef<monster>();
    }
    case '>': {// down: find any burrowing monsters on the current square
      auto &rock = tFactory.get(terrainType::ROCK);
      auto mAt = monstersAt(pos);
      for (auto i : mAt)
	if (i.value().abilities()->move(rock))
	  return optionalRef<monster>(i.value());
      return optionalRef<monster>();
    }
    case 'W': case 'w': // north
      towards = coord(pos.first, 0);
      break;
    case 'A': case 'a': // west
      towards = coord(0, pos.second);
      break;
    case 'S': case 's': // south
      towards = coord(pos.first, level::MAX_HEIGHT);
      break;
    case 'D': case 'd': // east
      towards = coord(level::MAX_WIDTH, pos.second);
      break;
    }
    do {
      coord cc=pos;
      pos = pos.towards(towards);
      if (!from.curLevel().movable(cc,pos, from, false, false))
	return optionalRef<monster>();
      auto mAt = monstersAt(pos);
      if (!mAt.empty())
	return optionalRef<monster>(mAt[0].value());
    } while (pos != towards);
    return optionalRef<monster>();
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
  // determines if a monster is still alive at the given position.
  bool stillOnLevel(const monster *mon) const {
    for (auto p : monsters_)
      if (p.second.get() == mon) return true;
    return false;
  }
  // teleport a monster. NB: This will move the monster regardless of any traps, items or other things in the way
  // UNLESS zone effects do not allow it
  void moveTo(monster &m, const coord &dest) {
    auto bm = dynamic_cast<::bigMonster*>(&m);
    if (bm) bm->setPos(dest);
    // NB: This repeats the loop each time in case any callback removes or moves the monster.
    zoneActions<monster> zones;
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      auto pM = i->second;
      if (*pM == m) {
	zones = zoneActions<monster>(zonesAt(i->first, true), zonesAt(dest, true));
	for (auto z : zones.same())
	  if (!z->onMoveWithin(*pM, dest)) return;
      }
    }
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      auto pM = i->second;
      if (*pM == m)
	for (auto z : zones.leaving())
	  if (!z->onExit(*pM, holder(dest))) return;
    }
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      auto pM = i->second;
      if (*pM == m)
	for (auto z : zones.entering())
	  if (!z->onEnter(*pM, holder(i->first))) return;
    }
    for (auto i = monsters_.begin(); i != monsters_.end(); ++i) {
      auto pM = i->second;
      if (*pM == m) {
	teleportTo(i, dest);
	return;
      }
    }
  }
  void teleportTo(monster &m, const coord &dest) {
    std::shared_ptr<monster> pM;
    auto i = monsters_.begin();
    while (i != monsters_.end())
      if (*(i->second) == m) {
	teleportTo(i, dest); //invalidates i
	return;
      } else ++i;
  }
  void teleportTo(::std::multimap<coord, ::std::shared_ptr<monster> >::iterator i, const coord &dest) {
    auto pM = i->second;
    if(pM->onMove(dest, terrain_.at(dest).value())) {
      if (posOf(*pM).first >= 0) { // in case monster has died/left the level in onMove()
	monsters_.erase(i);
	monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(dest, pM));
      }
      // reveal any pits:
      if (!pM->abilities()->fly() && terrain_.at(dest).type() == terrainType::PIT_HIDDEN)
	terrain_[dest] = tFactory.get(terrainType::PIT);
      // any single-shot traps:
      if (terrain_.at(dest).type() == terrainType::PIANO_HIDDEN)
	terrain_[dest] = tFactory.get(terrainType::GROUND);
      if (pM->isPlayer())
	describePlayerLoc(*pM, dest);
      pM->postMove(dest, terrain_.at(dest).value());
    }
  }

  void describePlayerLoc(const monster &m, const coord &pcLoc) const {
    std::vector<std::wstring> msg;
    auto monsters = monsters_.equal_range(pcLoc);
    for (auto i = monsters.first; i != monsters.second; ++i)
      if (*(i->second) != m)
	msg.emplace_back(L"live " + i->second->name());
    auto items = holder(pcLoc);
    items.forEachItem([&msg](const item&, std::wstring name) {
	msg.emplace_back(name);
      });
    
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
    if (pos.first >= level::MAX_WIDTH) pos.first = level::MAX_WIDTH-1;
    if (pos.second >= level::MAX_HEIGHT) pos.second = level::MAX_HEIGHT-1;
  }

  void changeTerrain(const coord &c, terrainType t) {
    terrain_[c] = tFactory.get(t);
  }
  
  bool movable(const coord &oldPos, const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) {
    auto &t = terrainAt(pos);
    auto v = vehicleMovable(pos, m, t); // moving on to a vehicle
    if (!v) v = vehicleTransportable(oldPos, t, m);  // moving by vehicle
    if (v) {
      transport &tr = dynamic_cast<transport&>(v.value());
      tr.isOnLevel(dungeon_[depth_]);
      tr.onMonsterMove(oldPos, holder(pos), pos, t);
      vehicleLeaving(oldPos, pos, t);
      return true;
    }
    if (!m.abilities()->move(t)) return false;
    if (avoidTraps && t.entraps(m, false)) return false;
    if (avoidHiddenTraps && t.entraps(m, true)) return false;
    return true;
  }
private:
  // determine any vehicles we're moving on to
  optionalRef<item> vehicleMovable(const coord &pos, const monster &m, const terrain &t) {
    auto &h = holder(pos);
    return h.firstItem([&m, &t, this](item &i) { // returns true if finds any item
	auto pV = dynamic_cast<transport *>(&i);
	if (pV) pV->isOnLevel(dungeon_[depth_]);
	return pV && m.abilities()->move(pV->terrainFor(t)); // returns item if movable
      });
  }
  // determine any vehicles that are moving us
  optionalRef<item> vehicleTransportable(const coord &oldPos, const terrain &t, const monster &m) {
    auto &h = holder(oldPos);
    return h.firstItem([&m, &t, this](item &i) { // returns true if finds any item
	auto pV = dynamic_cast<transport *>(&i);
	if (pV) pV->isOnLevel(dungeon_[depth_]);
	return pV && pV->moveOnto(t) && m.abilities()->move(pV->terrainFor(t));
      });
  }
  // determine any vehicles that we're stepping off of
  void vehicleLeaving(const coord &oldPos, const coord &pos, const terrain &t) {
    auto &h = holder(oldPos);
    h.forEachItem([&oldPos, &pos, &t, this](item &i, std::wstring) { // returns true if finds any item
	auto pV = dynamic_cast<transport *>(&i);
	if (pV) pV->isOnLevel(dungeon_[depth_]);
	if(pV) pV->onMonsterMove(oldPos, holder(pos), pos, t);
      });
  }
public:

  // move a monster by direction, with optional safety
  void move(monster &m, const ::dir dir, const bool avoidTraps) {
    coord cc=coord(posOf(m));
    coord pos = cc.inDir(dir);
    
    sanitiseCoords(pos);

    if (!movable(cc,pos, m, avoidTraps, false) || // monster can't pass this way
	monsters_.find(pos) != monsters_.end()) { // monsters can't *generally* move into each other.
      // can't move this way.
      return;
    }
    if (!avoidTraps || !terrainAt(pos).entraps(m, false)) // avoid traps if we should & can see them
      moveTo(m, pos); // moveTo handles entrapped()
  }
  void removeMonster(const monster &m) {
    bool isBig = dynamic_cast<const ::bigMonster*>(&m);
    for (auto i = monsters_.begin(); i != monsters_.end();) {
      auto erIter = i++; // erased iter would become invalid, so increment first
      if (*(erIter->second) == m) {
	monsters_.erase(erIter);
	if (!isBig)
	  return;
      }
    }
  }
  void removeDeadMonster(monster &m, bool allowCorpse) {
    // monster's inventory is dropped
    const coord c = posOf(m);
    auto &h = holder(c);
    m.forEachItem([&h](item &i, std::wstring) {
	h.addItem(i);
      });
    // corpse is added
    if (allowCorpse) {
      auto &corpse = createCorpse(m);
      holder(c).addItem(corpse);
    }
    const monsterType &t = m.type();
    // monster is removed
    removeMonster(m);
    // spawn a ghost occasionally
    const mutation &ghostType = mutationFactory::instance()[mutationType::GHOST];
    if (ghostType.appliesTo(t) && depth() > 20 && dPc() < 105) {
      auto mon = t.spawn(dungeon_[depth()]);
      mon->mutate(mutationType::GHOST);
      addMonster(mon, c);
    }
  }
  // replace the positions of the monster with what matches its copy.
  void bigMonster(monster &m, std::vector<coord> &pos) {
    std::shared_ptr<monster> pM;
    for (auto mn = monsters_.begin(); mn != monsters_.end();)
      if (mn->second.get() == &m) {
	pM = mn->second;
	mn = monsters_.erase(mn);
      } else ++mn;
    for (auto c : pos)
      monsters_.emplace(c, pM);
  }


  void addMonster(const std::shared_ptr<monster> mon, const coord c) {
    auto bm = std::dynamic_pointer_cast<::bigMonster>(mon);
    auto mn = monsters_.equal_range(c);
    if (mn.first == mn.second) {
      // no existing monster; place where requested
      monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(c, mon));
      if (bm) bm->setPos(c);
    } else {
      auto 
	maxX = std::min(level::MAX_WIDTH, c.first + 2),
	maxY = std::min(level::MAX_HEIGHT, c.second + 2);
      coord i;
      for (i.first = std::max(0, c.first - 2); i.first < maxX; ++i.first)
	for (i.second = std::max(0, c.second - 2); i.second < maxY; ++i.second) {
	  if (! movable(i,i,*mon, true, true)) continue;
	  auto mni = monsters_.equal_range(i);
	  if (mni.first == mni.second) {
	    // free nearby passible space
	    monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(i, mon));
	    if (bm) bm->setPos(i);
	    return;
	  }
	}
      // still here; nowhere is suitable; give up and stack 'em up:
      monsters_.emplace(::std::pair<coord, std::shared_ptr<monster> >(c, mon));
      if (bm) bm->setPos(c);
    }
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
	//holder(pos).removeItem(pos, it);
      }});
  }

  std::vector<ref<monster> > monstersAt(const coord &pos) const {
    auto range = monsters_.equal_range(pos);
    std::vector<ref<monster> > rtn;
    for (auto i = range.first; i != range.second; ++i)
      rtn.push_back(*(i->second.get()));
    return rtn;
  }

  void forEachMonster(std::function<void(monster &)> f) {
    // iterate over a copy, in case a monster moves (eg charmed)
    std::vector<::std::shared_ptr<monster>> monsters;
    for (auto &p : monsters_)
      monsters.emplace_back(p.second);
    for (auto &p : monsters)
      f(*p);
  }

  dungeon & dung() { return dungeon_; }

  const dungeon & dung() const { return dungeon_; }

  itemHolder &holder(const coord c) {
    auto rtn = holders_.find(c);
    if (rtn == holders_.end()) 
      holders_.emplace(c, std::unique_ptr<itemHolder>(new itemHolderLevel(*this, c)));
    return *(holders_[c]);
  }

  const itemHolder &holder(const coord c) const {
    // safe becase it's lazy-loaded.
    // better than mutable because we only allow creating of blank holders.
    return const_cast<levelImpl *>(this)->holder(c);
  }

  coord createPrison() {
    bool found = false;
    for (int triesLeft=10; triesLeft > 0 && !found; --triesLeft) {
      coord c,pos = findTerrain(terrainType::ROCK, 3, 3);
      found = true;
      auto iter = coordRectIterator(pos.first, pos.second, pos.first+3, pos.second+3);
      for (auto pC = iter.begin(); pC != iter.end(); ++pC) {
	c = *pC;
	auto ma = monstersAt(c);
	if (ma.empty()) {
	  found = false;
	  break; // too many rocky monsters
	}
	auto &h = holder(c);
	auto i = h.firstItem([](item &) {return true;});
	if (!i) {
	  found = false;
	  break; // too many items in the rock
	}
      }
      if (found) {
	coord mid(c.first+1, c.second+1);
	terrain_.at(mid) = tFactory.get(terrainType::GROUND);
	return mid;
      }
    }
    throw std::wstring(L"Too many rocky things to create a prison!");
  }

  void crack(level &pub) {
    auto rocks = findAllTerrain(terrainType::ROCK);
    auto grounds = findAllTerrain(terrainType::GROUND);
    int crackCounter=0;
    for (coord r : rocks)
      // are we adjacent to a ground?
      for (coord g : grounds) {
	if (r.towards(g) == g)
	  if (dPc() <= 10) {
	    terrain_[r] = tFactory.get(terrainType::CRACK);
	    if (++crackCounter == 10) {
	      auto mok = monsterTypeRepo::instance()[monsterTypeKey::mokumokuren].spawn(pub);
	      addMonster(mok, r);
	      crackCounter = 0;
	    }
	  }
      }
  }
  
private:
  void endGame() {
    auto &ios = ioFactory::instance();
    auto &role = dung().pc()->job();
    ios.clear();
    ios.message(L"You have left the dungeon.");
    for (auto pQ = role.questsBegin(); pQ != role.questsEnd(); ++pQ)
      if (pQ->isSuccessful()) 
	ios.message(pQ->completeMsg());
    ios.message(L"You have survived.");
    dung().quit();
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

  auto &ground = tFactory.get(terrainType::GROUND);
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
  const int width = 5, height = 5;
  // look for a contiguous 5x5 place on the map that is only rock.
  coord topLeft = level_->findTerrain(terrainType::ROCK, width, height);
  int xPos = topLeft.first, yPos = topLeft.second;
  coord btmRight(xPos+width-2, yPos+height-2);

  auto loc = std::pair<coord,coord>(topLeft,btmRight);

  addShrine(topLeft, btmRight);
  return loc;
}

void levelGen::addShrine(const coord &topLeft, const coord &btmRight, optionalRef<deity> d) {
  const int width = btmRight.first - topLeft.first + 1;
  const int height = btmRight.second  - topLeft.second + 1;
  auto shr = d ? std::make_shared<shrine>(topLeft, btmRight, d.value()) :
    std::make_shared<shrine>(topLeft, btmRight);
  int xPos = topLeft.first, yPos = topLeft.second;

  auto &ground = tFactory.get(terrainType::GROUND);
  for (int y=0; y < height-1; ++y) {
    for (int x=0; x < width-1; ++x) {
      coord c(x + xPos,y + yPos);
      level_->terrain_.at(c) = ground;
    }
  }
  itemZone(shr);
  monsterZone(shr);
  level_->holder(coord(xPos+2,yPos+2)).addItem(createHolyBook(shr->align()));
  if (shr->align().element() == Element::plant &&
      shr->align().domination() == Domination::aggression &&
      shr->align().outlook() == Outlook::cruel) {
    addMonster(monsterTypeKey::venusTrap, coord(xPos+1,yPos+1));
    addMonster(monsterTypeKey::venusTrap, coord(xPos+1,yPos+3));
    addMonster(monsterTypeKey::venusTrap, coord(xPos+3,yPos+1));
    addMonster(monsterTypeKey::venusTrap, coord(xPos+3,yPos+3));
  }
}

void levelGen::addShrine(std::unique_ptr<geometry> &&loc, optionalRef<deity> d) {
  int count=0;
  deity &path = d ? d.value() : rndAlign();
  auto &ground = tFactory.get(terrainType::GROUND);
  for (int x=0; x < level::MAX_WIDTH; ++x)
    for (int y=0; y < level::MAX_HEIGHT; ++y) {
      coord c(x,y);
      if (loc->contains(c)) {
	level_->terrain_.at(c) = ground;
	if (++count == 2)
	  level_->holder(c).addItem(createHolyBook(d.value()));      
      }
    }
  auto shr = std::make_shared<shrine>(std::move(loc), path);
  itemZone(shr);
  monsterZone(shr);
}



void levelGen::itemZone(std::shared_ptr<zoneArea<item>> z) {
  level_->itemZones_.push_back(z);
}

void levelGen::monsterZone(std::shared_ptr<zoneArea<monster>> z) {
  level_->monsterZones_.push_back(z);
}

void levelGen::setName(const std::wstring &name) {
  level_->name_ = name;
}

// all new monsters at level-gen time come through this method.
void levelGen::addMonster(std::shared_ptr<monster> m, const coord &c) {
  level_->addMonster(m,c);

  // in the case of Kelpies, nice to give them some water:
  if (m->type().type() == monsterTypeKey::kelpie) {
    auto &water = tFactory.get(terrainType::WATER);
    // look for any other non-water monsters on same square:
    bool found = false;
    auto ms = level_->monstersAt(c);
    for (auto m : ms)
      if (!m.value().abilities()->move(water)) {
	found = true; 
	break;
      }
    // otherwise make it watery
    if (!found && level_->terrainAt(c).type() == terrainType::GROUND)
      level_->terrain_[c] = tFactory.get(terrainType::WATER);
  }
}

monster &levelGen::addMonster(monsterTypeKey m, const coord &c) {
  auto &mt = monsterTypeRepo::instance()[m];
  auto mon = mt.spawn(pub_);
  addMonster(mon, c);
  return *mon;
}

monster &levelGen::addMonster(monsterBuilder &b, const coord &c) {
  auto &mt = monsterTypeRepo::instance()[b.type().type()];
  b.startOn(pub_);
  auto mon = mt.spawn(b);
  addMonster(mon, c);
  return *mon;
}

std::vector<monster *> levelGen::addMonsters(std::vector<std::pair<coord,coord>> coords /*by value*/,
					     std::function<bool(const monsterType*)> f) {
  std::vector<std::pair<unsigned int, monsterType*>> types =
    spawnMonsters(level_->depth(), coords.size(), f);
  std::vector<monster *> rtn;
  
  for (auto i : types) {
    if (coords.begin() == coords.end()) break;
    auto room = rndPick(coords.begin(), coords.end());
    // we'll avoid the middle square, as that's likely to be where the player starts. Bit hacky.
    // add initial monsters to the room.
    const coord midPoint = mid(*room);

    for (unsigned int c=0; c < i.first; ++c) {
      auto &mt = *(i.second);
      //auto m = ofType<mt>(pub_);
      std::shared_ptr<monster> m = mt.spawn(pub_);
      addMonster(m, midPoint, *room);
      // make sure the monster was added before returning it
      // (pointer won't be wild so long as m is in scope)
      auto added = pub_.monstersAt(midPoint);
      if (std::find(added.begin(), added.end(), ref<monster>(*m)) != added.end())
	rtn.emplace_back(&*m);
    }
    coords.erase(room); // don't use the same room twice; tend to avoid the packs of monsters starting together
  }
  return rtn;
}

void levelGen::addMonster(std::shared_ptr<monster> mon, const coord &m, const std::pair<coord, coord> & coords) {
  for (int y = coords.first.second; y < coords.second.second; ++y)
    for (int x = coords.first.first; x < coords.second.first; ++x) {
      coord c = coord(x,y);
      if (c == m) continue;
      if (!level_->movable(c,c,*mon, false,true)) continue; // roaming monster can't go on this terrain
      addMonster(mon, c);

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
      if (level_->terrain_.at(c).type() == terrainType::GROUND) {
	switch(dPc() % 10) {
	case 0: case 1:
	case 2: case 3:
	case 4: case 5:
	case 6: case 7:
	  level_->terrain_.at(c) = tFactory.get(terrainType::PIT_HIDDEN);
	  break;
	case 8: case 9:
	  level_->terrain_.at(c) = tFactory.get(terrainType::PIANO_HIDDEN);
	}
      }
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
  auto pT = level_->terrain_.find(c);
  if (pT->second == tFactory.get(from))
    pT->second = tFactory.get(to);
}

coord levelGen::addCorridor(const coord &from, const coord &to) {
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
    if (level_->terrainAt(mid(*i)).type() == terrainType::GROUND) {
      level_->terrain_[mid(*i)] = tFactory.get(type);
      return;
    }
  }
  // fallback (in case all rooms are in the same place): place the down by the up:
  coord c = mid(*begin);
  c.first++;
  place(c,type);
}

template<class iter>
void levelGen::place(iter it,
		    terrainType type) {
  for (coord c : it)
    level_->terrain_[c] = tFactory.get(type);
}

void levelGen::place(const coord &c, terrainType type) {
  level_->terrain_[c] = tFactory.get(type);  
}

terrainType levelGen::at(const coord &c) const {
  return level_->terrain_.at(c).type();
}

coord levelGen::findRndTerrain(terrainType t) const {
  coord c;
  do {
    c.first = rndPickI(1, level::MAX_WIDTH-1);
    c.second = rndPickI(1, level::MAX_HEIGHT-1);
  } while (at(c) != t);
  return c;
}



// the pImpl (pointer-to-implementation) pattern keeps the structure of the class out of the header file:

level::level(levelImpl *pImpl) :
  pImpl_(pImpl) {}
level::~level() { 
}

std::wstring level::name() const {
  return pImpl_->name();
}

drawIter level::drawBegin() const {
  return pImpl_->drawBegin();
}
drawIter level::drawEnd() const {
  return pImpl_->drawEnd();
}

const terrain &level::terrainAt(const coord & c) const {
  return pImpl_->terrainAt(c);
}

coord level::findTerrain(const terrainType &type) const {
  return pImpl_->findTerrain(type);
}

std::vector<coord> level::findAllTerrain(const terrainType &type) const {
  return pImpl_->findAllTerrain(type);
}

void level::moveOrFight(monster &m, const ::dir &dir, bool at) {
  return pImpl_->moveOrFight(m, dir, at);
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
optionalRef<monster> level::findMonster(monster &from, const wchar_t dir) const {
  return pImpl_->findMonster(from, dir);
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
void level::moveTo(monster &monster, const coord &targetPos) {
  pImpl_->moveTo(monster, targetPos);
}
void level::teleportTo(monster &monster, const coord &targetPos) {
  pImpl_->teleportTo(monster, targetPos);
}
void level:: move(monster &m, const ::dir dir, const bool avoidTraps) {
  pImpl_->move(m, dir, avoidTraps);
}
bool level::movable(const coord &oldPos, const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) const {
  return pImpl_->movable(oldPos, pos, m, avoidTraps, avoidHiddenTraps);
}
void level::changeTerrain(const coord &c, terrainType t) {
  pImpl_->changeTerrain(c, t);
}
void level::addMonster(std::shared_ptr<monster> monster, const coord &targetPos) {
  pImpl_->addMonster(monster, targetPos);
}
void level::removeDeadMonster(monster &m, bool allowCorpse) {
  pImpl_->removeDeadMonster(m, allowCorpse);
}
void level::bigMonster(monster &m, std::vector<coord> &pos) {
  pImpl_->bigMonster(m, pos);
}
void level::pickUp() {
  pImpl_->pickUp();
}
std::vector<ref<monster> > level::monstersAt(const coord &pos) const {
  return pImpl_->monstersAt(pos);
}
void level::forEachMonster(std::function<void(monster &)> f) {
  pImpl_->forEachMonster(f);
}
itemHolder &level::holder(const item& item) {
  return holder(posOf(item)); // no need to bother pImpl for a simple wrapper
}
itemHolder &level::holder(const coord &c) {
  return pImpl_->holder(c);
}
dungeon & level::dung() {
  return pImpl_->dung();
}
const dungeon & level::dung() const {
  return pImpl_->dung();
}

bool itemHolderLevel::addItem(item &item) {
  // Can't add liquid items to a level
  if (item.material() == materialType::liquid)
    return false;
  auto &map = itemHolderMap::instance();
  if (!map.beforeFirstAdd(item)) {
    for (auto z : level_.zonesAt(coord_))
      if (!z->onEnter(item, item.holder())) return false;
  }
  itemHolder::addItem(item);
  return true;
}
bool itemHolderLevel::removeItemForMove(item &item, itemHolder &next) {
  auto pos = level_.posOf(item);
  for (auto z : level_.zonesAt(coord_))
    if (!z->onExit(item, next)) return false;
  itemHolder::removeItemForMove(item, next);
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
  static levelImpl::itemZoneIter zonesAt(levelImpl &p, const coord &c) { return p.zonesAt(c); } 
};
template<>
struct zoneTraits<monster>{ 
  static levelImpl::monsterZoneIter zonesAt(levelImpl &p, const coord &c) { return p.zonesAt(c, true); } 
};

template <typename T>
filteredIterable<std::shared_ptr<zoneArea<T> >,std::vector<std::shared_ptr<zoneArea<T> > > > 
level::zonesAt(const coord & c) {
  return zoneTraits<T>::zonesAt(*pImpl_, c);
}

bool level::stillOnLevel(const monster *mon) const {
  return pImpl_->stillOnLevel(mon);
}
optionalRef<monster> level::lineOfSightTarget(monster &m) {
  return pImpl_->lineOfSightTarget(m, posOf(m));
}
void level::crack() {
  return pImpl_->crack(*this);
}

coordRectIterator level::allCoords() {
  return coordRectIterator(0,0,level::MAX_WIDTH-1, level::MAX_HEIGHT-1);
}


class levelFactoryImpl {
private:
  int numLevels_;
  std::vector<levelImpl*> levels_;
  std::vector<level*> levelPubs_;
  std::vector<std::unique_ptr<levelGen> > levelGen_;
  role &role_;
public:
  levelFactoryImpl(dungeon &dungeon, const int numLevels, role &role) : 
    numLevels_(numLevels),
    role_(role) {
    for (int i=0; i <= numLevels; ++i) {
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
    for (auto i = begin; i != end; ++i)
      (*i)->negotiateRamps(
	 i == end || i+1 == end ? optionalRef<levelGen>() : optionalRef<levelGen>(**(i+1)));
    int depth=0;
    auto l = levelPubs_.begin();
    for (auto i = begin; i != end; ++i) {
      (*i)->build();
      for (std::vector<quest>::iterator pQ = role_.questsBegin(); pQ != role_.questsEnd(); ++pQ)
	pQ->setupLevel(**i, **l, depth);
      ++depth, ++l;
    }
  }
  std::vector<level*>::iterator begin() {
    return levelPubs_.begin();
  }
  std::vector<level*>::iterator end() {
    return levelPubs_.end();
  }
private:
  levelGen *createGen(int depth, levelImpl *l, level *level) {
    for (auto qI = role_.questsBegin(); qI != role_.questsEnd(); ++qI)
      if (qI->isQuestLevel(depth))
	return qI->newQuestLevelGen(*l, *level);
    bool addDownRamp = depth < numLevels_;
    switch (depth) {
    case 10:
      return newGen(specialLevelKey::WATER, l, level, addDownRamp);
    case 20:
      return newGen(specialLevelKey::SPACE, l, level, addDownRamp);
    default:
      ;// fall through
    }
    if (dPc() < depth)
      switch (depth / 10) {
      case 0:
      case 1:
    	try {
    	  return new labyRoomGen(l, *level, addDownRamp);
    	} catch (char const * msg) { /*std::cerr << "labyRoomGen: " << msg << std::endl; */}
      case 2:
      case 3:
      case 4:
      case 5:
    	try {
    	  return new labySmallGen(l, *level, addDownRamp);
    	} catch (char const * msg) { /*std::cerr << "labySmallGen: " << msg << std::endl; */}
      case 6:
      case 7:
      case 8:
      default:
    	try {
    	  return new labyGen(l, *level, addDownRamp);
    	} catch (char const * msg) { /*std::cerr << "labyGen: " << msg << std::endl; */}
      }
    // default case & fall-through:
    return new roomGen(l, *level, addDownRamp); 
  }
};

levelFactory::levelFactory(dungeon &dungeon, const int numLevels, role &job) :
  pImpl_(new levelFactoryImpl(dungeon, numLevels, job)) {
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

// instantiate templates
void unused1(level &l) { l.template zonesAt<monster>(coord(0,0)); }
void unused2(level &l) { l.template zonesAt<item>(coord(0,0)); }
