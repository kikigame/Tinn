/* License and copyright go here*/

// Objects for monster motion

#ifndef TRANSPORT_HPP__
#define TRANSPORT_HPP__

#include "items.hpp"
#include "terrain.hpp"
#include "movement.hpp"

class coord;

// transport items are normal items, but if placed on the correct kind
// of square, they can "override" the terrain type for that square,
// and provide locomotion over otherwise non-movable terrain.
class transport : public virtual shared_item {
private:
  // where must this be located to provide alternate terrain?
  const terrain &terrainToActivate_;
  // what terrain does this transport allow?
  const terrain &terrainToAllow_;
  // how does the transport move?
  movementType movement_;
  // is the transport active?
  bool active_;
  // current level, if known. (If not, either we can't move or nothing's moving on the level anyway).
  level * lvl_;
public:
  // create a transport that is usable on the "activate" terrain, allowing movement based on "allow"
  // if move.goTo == none, this trasport does not move
  // if move.goTo == player, this transport is under player control.
  // currently, "unaligned" and "coaligned" work the same as player.
  transport(terrainType activate, terrainType allow, const movementType &move);
  transport(const transport &other) = delete;
  transport(transport &&other) = delete;
  virtual ~transport();
  // return the effective terrain for this vehicle. ie base if deactivated, or allow if activates base.
  const terrain &terrainFor(const terrain &base) const;

  // called whenever the player moves onto or off the transport
  void onMonsterMove(const coord &oldPos, itemHolder &newPos, const coord &pos, const terrain &newTerrain);

  // can this vehicle move onto the given terrain type by its own power?
  bool moveOnto(const terrain &t) const;

  void isOnLevel(level &);
private:
  void activate(); // called by level when monster activates the transport
  void deactivate(); // called by level when monster steps off the transport
  void selfMove();
};

#endif //ndef TRANSPORT_HPP__
