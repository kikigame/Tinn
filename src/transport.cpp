/* License and copyright go here*/

#include "transport.hpp"
#include "coord.hpp"

transport::transport(terrainType activate, 
		     terrainType allow,
		     const movementType &move) :
  terrainToActivate_(*tFactory.get(activate)),
  terrainToAllow_(*tFactory.get(allow)),
  movement_(move),
  active_(false) {}

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
  switch (movement_.goTo_) {
  case goTo::none: return; // does not move
  case goTo::wander:
    // TODO: wander
    return;
  case goTo::player:
    // player control; nothing to do here (handled by onPlayerMove())
    return;
  case goTo::up:
    // TODO: seek out < (idea: escape pod on scifi level)
    return;
  case goTo::down:
    // TODO: seek out >
    return;
  }
}

// TODO: speed of movement

void transport::deactivate() {
  active_ = false;
}
