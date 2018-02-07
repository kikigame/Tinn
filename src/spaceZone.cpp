/* License and copyright go here*/

// implement the behaviour of space tiles

#include "spaceZone.hpp"
#include "level.hpp"
#include "terrain.hpp"
#include "monster.hpp"
#include "items.hpp"

spaceZone::spaceZone(std::unique_ptr<geometry> &&geometry, level &lvl) :
  geometry_(), lvl_(lvl), mdir_(), idir_() {
  geometry_.swap(geometry);
}

spaceZone::~spaceZone() {}

bool spaceZone::contains(coord a) {
  return geometry_->contains(a);
}

bool spaceZone::onMoveWithin(monster &mon, const coord &dest) {
  switch (lvl_.terrainAt(dest).type()) {
  case terrainType::SPACE:
    return moveInSpace(mon, dest);
  case terrainType::BULKHEAD:
    return bounce(mon, dest);
  default:
    return true;
  }
}

bool spaceZone::onExit(monster &mon, itemHolder &next) {
  std::map<monster*, dir>::const_iterator it = mdir_.find(&mon);
  if (it != mdir_.end()) // stop tracking
    mdir_.erase(it);
  return true;
}

bool spaceZone::onMoveWithin(item &it, const coord &dest) {
  switch (lvl_.terrainAt(dest).type()) {
  case terrainType::SPACE:
    return moveInSpace(it, dest);
  case terrainType::BULKHEAD:
    return bounce(it, dest);
  default:
    return true;
  }
}

bool spaceZone::onExit(item &ite, itemHolder &next) {
  std::map<item*, dir>::const_iterator it = idir_.find(&ite);
  if (it != idir_.end()) // stop tracking
    idir_.erase(it);
  return true;
}

// when 1 coord away from the edge, we wrap around.
// we don't allow to go right to the edge, because level.cpp prevents
// us from trying to move off the edge of the map.
coord wraparound(coord p) {
  // NB: We slide slightly when wrapping around, allowing the player to explore more of the level.
  if (p.first < 1) { p.first = level::MAX_WIDTH-2; p.second++; }
  if (p.first >= level::MAX_WIDTH-1) { p.first = 1; p.second++; }
  if (p.second < 1) { p.second = level::MAX_HEIGHT-2; p.first++; }
  if (p.second >= level::MAX_HEIGHT-1) { p.second = 1; p.first++; } // move to edge or slide off
  if (p.first >= level::MAX_WIDTH-1) { p.first = 1; } // in case we "slide" off. 
  return p;
}

bool spaceZone::moveInSpace(monster &mon, const coord &dest) {
  if (!mon.isPlayer() && mon.name().find(L"space ") == 0)
    return true; // space monsters can move freely in space.
  std::map<monster*, dir>::const_iterator it = mdir_.find(&mon);
  if (it == mdir_.end()) {
    mdir_[&mon] = lvl_.posOf(mon).dirTo(dest);
    return true; // first one's free...
  }
  // which way is the monster *actually* moving?
  auto dir = it->second;
  // where's the monster *actually* going?
  coord pos = lvl_.posOf(mon);
  pos = pos.inDir(dir);
  pos = wraparound(pos);
  // what's there?
  auto &t = lvl_.terrainAt(pos);
  switch (t.type()) {
  case terrainType::BULKHEAD:
    bounce(mon, pos);
    return false;
  default: // impassible terrain already handled in level::moveTo()
    if (!contains(pos)) onExit(mon, lvl_.holder(pos));
    lvl_.teleportTo(mon, pos);
    return false;
  }
}

bool spaceZone::bounce(monster &mon, const coord &dest) {
  auto before = mon.injury().cur();
  auto max = mon.injury().max();
  auto lost = mon.wound(mon, 1, damageRepo::instance()[damageType::bashing]);
  if (before + lost >= max) return false; // monster has died. Let their corpse float here.
  //... setup new momentum trajectory:
  auto pos = lvl_.posOf(mon);
  auto dir =  pos.dirFrom(dest);
  mdir_[&mon] = dir;
  moveInSpace(mon, pos.inDir(dir)); // NB: This will break if there are 2 bulkheads close together with space in between.
  return false;
}


bool spaceZone::moveInSpace(item &item, const coord &dest) {
  //std::map<item*, dir>::const_iterator 
  auto it = idir_.find(&item);
  if (it == idir_.end()) {
    idir_[&item] = lvl_.posOf(item).dirTo(dest);
    return true; // first one's free...
  }
  // which way is the item *actually* moving?
  auto dir = it->second;
  // where's the item now?
  coord oPos = lvl_.posOf(item);
  // where's the item *actually* going?
  coord pos = oPos.inDir(dir);
  pos = wraparound(pos);
  // what's there?
  auto &t = lvl_.terrainAt(pos);
  switch (t.type()) {
  case terrainType::BULKHEAD:
    bounce(item, pos);
    return false;
  default: // impassible terrain already handled in level::moveTo()
    if (!contains(pos)) onExit(item, lvl_.holder(oPos));
    lvl_.holder(pos).addItem(item);
    return false;
  }
}

bool spaceZone::bounce(item &item, const coord &dest) {
  auto i = item.shared_from_this();
  item.strike(damageType::bashing);
  if (!i) return false; // item has beed destroyed.
  //... setup new momentum trajectory:
  auto pos = lvl_.posOf(item);
  auto dir =  pos.dirFrom(dest);
  idir_[&item] = dir;
  moveInSpace(item, pos.inDir(dir)); // NB: This will break if there are 2 bulkheads close together with space in between.
  return false;
}
