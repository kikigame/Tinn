/* License and copyright go here*/

// Objects for monster motion

#ifndef TRANSPORT_HPP__
#define TRANSPORT_HPP__

#include "items.hpp"
#include "terrain.hpp"

// transport items are normal items, but if placed on the correct kind
// of square, they can "override" the terrain type for that square,
// and provide locomotion over otherwise non-movable terrain.
class transport : public virtual shared_item {
private:
  // where must this be located to provide alternate terrain?
  const terrain &terrainToActivate_;
  // what terrain does this transport allow?
  const terrain &terrainToAllow_;
public:
  // create a transport that is usable on the "activate" terrain, allowing movement based on "allow"
  transport(terrainType activate, terrainType allow);
  transport(const transport &other) = delete;
  transport(transport &&other) = delete;
  virtual ~transport();
  // return the effective terrain for this vehicle. ie base if deactivated, or allow if activates base.
  const terrain &terrainFor(const terrain &base) const;
};

#endif //ndef TRANSPORT_HPP__
