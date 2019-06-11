/* License and copyright go here*/

#include "transport.hpp"
#include "coord.hpp"
#include "time.hpp"
#include "random.hpp"
#include "level.hpp"
#include "pathfinder.hpp"

transport::transport(terrainType activate, 
		     terrainType allow,
		     const movementType &move) :
  terrainToActivate_(tFactory.get(activate)),
  terrainToAllow_(tFactory.get(allow)),
  movement_(move),
  active_(false),
  lvl_(0) {}

transport::~transport() {}

const terrain &transport::terrainFor(const terrain &base) const {
  return base.type() == terrainToActivate_.type() ? terrainToAllow_ : base;
}

bool transport::moveOnto(const terrain &t) const {
  return
    // we must be a mobile transport:
    movement_.goTo_ != goTo::none &&
    // we must be activated by the destination terrain:
    t.type() == terrainToActivate_.type();
}

void transport::onMonsterMove(const coord &oldPos, itemHolder &newPos, const coord &pos, const terrain &newTerrain) {
  auto &it = *shared_from_this();
  if (pos == oldPos)
    // stepping onto the transport
    activate();
  else if (movement_.goTo_ == goTo::player && newTerrain.type() == terrainToActivate_.type())
    // active & moving with player while on active terrain:
    newPos.addItem(it);
  else
    // stepping off the transport
    deactivate();
}

void transport::activate() {
  active_ = true;
  // TODO: call selfMove() each tick until deactivated
}

void transport::isOnLevel(level & lvl) {
  lvl_ = &lvl;
}

// NB: This is similar to mobile.cpp's moveMobile<> method, but differs:
// - goTo::player means player-controlled
// - there is no jitter
void transport::selfMove() {
  auto pThis = shared_from_this();
  auto curPos = lvl_->posOf(*pThis);
  if (curPos.first < 0) {
    // not on a level; perhaps carried?
    deactivate();
    return;
  }

  coord targetPos;
  switch (movement_.goTo_) {
  case goTo::none: return; // does not move
  case goTo::wander: {
    std::vector<coord> opts;
    for (int x = curPos.first - 1; x <= curPos.first +1; ++x)
      for (int y = curPos.second - 1; y <= curPos.second +1; ++y)
	if (x >= 0 && y >= 0 && x < level::MAX_WIDTH && y < level::MAX_HEIGHT) {
	  coord nextCoord(x,y);
	  if (nextCoord != curPos && lvl_->terrainAt(nextCoord) == terrainToAllow_)
	    opts.emplace_back(x,y);
	}
    if (!opts.empty()) {
      auto pNextPos = rndPick(opts.begin(), opts.end());
      lvl_->holder(*pNextPos).addItem(*pThis);
    }
    return;
  }
  case goTo::coaligned:
  case goTo::unaligned:
  case goTo::player:
  default:
    // player control; nothing to do here (handled by onPlayerMove())
    return;
  case goTo::up:
    // seek out < (idea: escape pod on scifi level)
    targetPos = lvl_->findTerrain(terrainType::UP);
    return;
  case goTo::down:
    // seek out >
    targetPos = lvl_->findTerrain(terrainType::DOWN);
    return;
  }

  dir d = pathfinder<12>([this](const coord &c){
      if (c.first < 0 || c.second < 0 ||
	  c.first > level::MAX_WIDTH ||
	  c.second > level::MAX_HEIGHT)
	return false;
      return lvl_->terrainAt(c) == terrainToAllow_;
    }).find(curPos, targetPos);

  lvl_->holder(curPos.inDir(d)).addItem(*pThis);
}

// TODO: speed of movement

void transport::deactivate() {
  active_ = false;
}
