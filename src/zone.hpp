/* License and copyright go here*/

// areas of a level with special behaviour

#ifndef ZONE_HPP__
#define ZONE_HPP__

#include "coord.hpp"
#include "itemholder.hpp"
#include <memory> // shared_ptr


// abstract class for a zone providing specific behaviour
// T may be "monster", "player" or "item"
template<class T>
class zoneArea {
public:
  // determine if a given square contains this zone
  virtual bool contains(coord area) = 0;

  // NB "moving" means moving for a monster/player, or
  // being fired, thrown, kicked, etc for an item

  // called whenever entering the zone
  // NB: For an item, this can also be by being dropped
  virtual bool onEnter(std::shared_ptr<T>, const itemHolder &prev) = 0;
  // called whenever leaving the zone
  // NB: For an item, this can also be by being taken
  virtual bool onExit(std::shared_ptr<T>, const itemHolder &prev) = 0;
  // called whenever moving between two squares in the zone
  virtual bool onMoveWithin(std::shared_ptr<T>) = 0; 
  // called whenever the location is a coord within the zone and a tick passes
  virtual bool onStandWithin(std::shared_ptr<T>) = 0;  

};


#endif //ndef ZONE_HPP__
