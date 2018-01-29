/* License and copyright go here*/

// move monsters around a level

#include "mobile.hpp"
#include "monster.hpp"
#include "random.hpp"
#include "dungeon.hpp"
#include "religion.hpp"
#include "terrain.hpp"
#include "pathfinder.hpp"
#include "output.hpp"
#include <sstream>


/*
 * Algorithm to find next available moves within a level.
 */
template<bool avoidTraps, bool avoidHiddenTraps>
class nextLevelMoves {
private:
  const monster &mon_;
  const level &level_;
public:
  nextLevelMoves(monster &mon) :
    mon_(mon), level_(mon.curLevel()) {};
  std::set<coord> operator()(const coord &c) const {
    std::set<coord> rtn;
    for (int x=c.first-1; x<=c.first+1; ++x)
      for (int y=c.second-1; y<=c.second+1; ++y) {
	coord pos(x,y);
	if (pos == c) continue; // same square
	if (pos.first < 0 || pos.second < 0 ||
	    pos.first >= level::MAX_WIDTH || pos.second >= level::MAX_HEIGHT)
	  continue; // off the map; here be monsters...
	if (level_.movable(pos, pos, mon_, avoidTraps, avoidHiddenTraps)) 
	  rtn.emplace(pos);
      }	
    return rtn;
  }
};

/*
 * Algorithm to find next available moves within a level.
 * Returns only cardinal moves; faster for longer distances but may miss some monster-only paths.
 */
template<bool avoidTraps, bool avoidHiddenTraps>
class nextLevelCardMoves {
private:
  const monster &mon_;
  const level &level_;
public:
  nextLevelCardMoves(monster &mon) :
    mon_(mon), level_(mon.curLevel()) {};
  std::set<coord> operator()(const coord &c) const {
    static std::set<dir> all({{dir(-1,0), dir(0,-1), dir(1,0), dir(0,1)}});
    std::set<coord> rtn;
    for (auto dir : all) {
      auto pos = c.inDir(dir);
      if (pos.first < 0 || pos.second < 0 ||
	  pos.first >= level::MAX_WIDTH || pos.second >= level::MAX_HEIGHT)
	continue; // off the map; here be monsters...
      if (level_.movable(pos, pos, mon_, avoidTraps, avoidHiddenTraps)) 
	rtn.emplace(pos);
    }	
    return rtn;
  }
};

template <>
struct movementTraits<monster> {
  // default: return the unmodified speed. For monsters, this is modified by the abilities (including encumbrance).
  static speed adjust(monster &t, const movementType &type) { return t.abilities().adjust(type.speed_); }
  // default: get position by asking the level.
  static coord position(level &level, monster &t) { return level.posOf(t); }
};


template<class T>
void moveMobile(T &mon) {
  level & level = mon.curLevel();
  auto pcPos = level.pcPos();
  if (pcPos.first < 0) return; // only bother moving if player is on the level.
  const movementType &type = mon.movement();

  auto fastness = movementTraits<T>::adjust(mon, type);
  auto myPos = movementTraits<T>::position(level, mon);

  // do we move and - if so - how many times?
  int loopMax=0;
  switch (fastness) {
  case speed::slow3: if ((time::moveCount()) % 3 == 0) ++loopMax; break;
  case speed::slow2: if ((time::moveCount()) % 2 == 0) ++loopMax; break;
  case speed::perturn: ++loopMax; break;
  case speed::turn2: loopMax = 2; break;
  case speed::turn3: loopMax = 3; break;
  default: throw type.speed_;
  }

  for (int counter = 0; counter < loopMax; ++counter) {
    ::dir dir(0,0);
    coord targetPos;

    bool charmed = false;
    if (mon.charmedBegin() != mon.charmedEnd()) {
      auto pM = rndPick(mon.charmedBegin(), mon.charmedEnd());
      if (dPc() < (*pM)->appearance().cur()) {
	targetPos = level.posOf(**pM);
	dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	charmed = true;
      }
    }

    if (!charmed) switch (type.goTo_) {
    case goTo::none: 
      return;  // this does not move
    case goTo::wander:
      {
	const std::vector<char> dirs({-1, 0, +1 }); // could use boost::counting_iterator here, but I don't want the dependency
	dir.first = *rndPick(dirs.begin(), dirs.end());
	dir.second = *rndPick(dirs.begin(), dirs.end());
      }
      break;
    case goTo::coaligned:
      if (pcPos.first < 0 ||
	  level.dung().pc()->align().coalignment(mon.align()) >= 3) {
	mon.curLevel().forEachMonster([&mon, &dir, &level, &targetPos, &pcPos, &myPos](monster &m){
	    if (m.align().coalignment(mon.align()) >= 3) {
	      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
	      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	    }
	  });
	// TODO: Would be nice to move to coaligned zone if no monster exists
	break;
      } // else fall right through to player
    case goTo::unaligned:
      if (type.goTo_ == goTo::unaligned && (
	  pcPos.first < 0 ||
	  level.dung().pc()->align().coalignment(mon.align()) < 3)) {
	mon.curLevel().forEachMonster([&mon, &dir, &level, &targetPos, &pcPos, &myPos](monster &m){
	    if (m.align().coalignment(mon.align()) >= 3) {
	      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
	      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	    }
	  });
      break;
      } // else fall through to goTo::player
    case goTo::player:
      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::up:
      targetPos = level.findTerrain(terrainType::UP); 
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::down:
      targetPos = level.findTerrain(terrainType::DOWN);
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;     
      }
      break;
    default:
      throw type.goTo_;
    }

    // special case: work out best direction before applying jitter; ignore dir worked out above
    if (type.goBy_ == goBy::smart) {
      if (pathfinder<2>::absdistance(myPos, pcPos) < 4) {
	// very close; use a more accurate pathfinder
	auto nlm = nextLevelMoves<true, true>(mon); // up to 8 options
	std::function<std::set<coord >(const coord &)> nextMoves = nlm;
	dir = pathfinder<2>(nextMoves).find(myPos, targetPos); // order: 8 ^ 2 = 64
      } else {
	// only consider cardinal moves. Jitter may still help.
	auto nlm = nextLevelCardMoves<true, true>(mon); // up to 4 options
	std::function<std::set<coord >(const coord &)> nextMoves = nlm;
	dir = pathfinder<6>(nextMoves).find(myPos, targetPos); // order: 4 ^ 6 = 4096
      }
    }


    // apply jitter:
    if (type.jitterPc > 0) {
      auto jit = dPc();
      if (jit < type.jitterPc)
	switch (jit % 8) {
	case 0: --dir.first; --dir.second; break;
	case 1: --dir.first; break;
	case 2: --dir.first; ++dir.second; break;
	case 3: --dir.second; break;
	case 4: ++dir.second; break;
	case 5: ++dir.first; --dir.second; break;
	case 6: ++dir.first; break;
	case 7: ++dir.first; ++dir.second; break;
	}
    }

    // jitter should not affect movement speed:
    if (dir.first > 1) dir.first=1;
    if (dir.second > 1) dir.second=1;
    if (dir.first < -1) dir.first=-1;
    if (dir.second < -1) dir.second=-1;

    // now move the monster:
    switch (type.goBy_) {
    case goBy::avoid:
      dir.first =- dir.first; dir.second =- dir.second;
      // no break
    case goBy::beeline:
      level.move(mon, dir, true);
      break;
    case goBy::zomavoid:
      dir.first =- dir.first; dir.second =- dir.second;
      // no break
    case goBy::zombeeline:
      level.move(mon, dir, false);
      break;
    case goBy::smart:
      level.move(mon, dir, true);
      break;
    case goBy::teleport:
      level.moveTo(mon, targetPos);
      break;
    default:
      throw type.goBy_;
    }
  }
}

void ignored(monster &m) {
  moveMobile<monster>(m);
}


void monsterAttacks(monster &mon) {
  level & level = mon.curLevel();

  // we don't allow attacking until the player reaches us
  if (&(level.dung().cur_level()) != &level) return;

  // currently, all monsters will attack anything that
  // a) they are adjacent to, and
  // b) they are a different alignment to, on at least one axis OR both unaligned.
  // This needs improving, with multiple monster attacks.

  auto myPos = level.posOf(mon);
  auto &dam = mon.injury();
  auto &malign = mon.align();
  for (int dx=-1; dx <= +1; ++dx)
    for (int dy=-1; dy <= +1; ++dy) {
      coord pos(myPos.first + dx, myPos.second + dy);
      auto m = level.monstersAt(pos);
      // take a copy, in case (eg a monster dies) the collection changes
      std::vector<ref<monster> > monstersAt = m;
      for (auto ren : monstersAt) {
	auto &en = ren.value();
	if (&en == &mon) continue; // monsters don't usually fight themselves
	if (en.align().coalignment(malign) >= 3) continue; // monsters don't usually fight other creautures of the same alignment
	if (en.type() == mon.type()) continue; // monsters don't usually fight other creatures of the same class
	std::wstringstream msg;
	msg << (mon.isPlayer() ? L"You" : mon.name())
	    << myPos
	    << L" attacks "
	    << (en.isPlayer() ? L"you" : en.name())
	    << pos
	    << L": ";
	auto result = mon.attack(en); // may invalidate refernec mon.
	msg << result.text_;

	auto m = msg.str();

	auto &ios = ioFactory::instance();
	if (m.find(L"\n") != m.npos)
	  ios.longMsg(m);
	else
	  ios.message(m);

	if (dam.cur() == dam.max())
	  return; // defensive coding against resistive attacks from other monsters
      }
    }
}
