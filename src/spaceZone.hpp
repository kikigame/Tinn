/* License and copyright go here*/

// implement the behaviour of space tiles

#ifndef SPACE_HPP__
#define SPACE_HPP__

#include "zone.hpp"
#include "geometry.hpp"

#include <map>
#include <memory>

class monster;
class item;
class level;

class spaceZone : public zoneArea<monster>, public zoneArea<item> {
private:
  std::unique_ptr<geometry> geometry_;
  //const coord ul_;
  //  const coord lr_;
  level &lvl_;
  std::map<monster*, dir> mdir_;
  std::map<item*, dir> idir_;
public:
  spaceZone(std::unique_ptr<geometry> &&, level &lev);
  virtual ~spaceZone();

  // determine if a given square contains this zone
  virtual bool contains(coord area);

  virtual bool onMoveWithin(monster &, const coord &dest);
  //virtual bool onEnter(monster &, itemHolder &prev);
  virtual bool onExit(monster &, itemHolder &prev);

  virtual bool onMoveWithin(item &, const coord &dest);
  //virtual bool onEnter(item &, itemHolder &prev);
  virtual bool onExit(item &, itemHolder &next);

private:
  bool moveInSpace(monster &, const coord &dest, bool bouncing = false);
  bool bounce(monster &, const coord &dest);
  bool moveInSpace(item &, const coord &dest);
  bool bounce(item &, const coord &dest);
  bool wearingMagBoots(const monster &) const;
};


#endif //ndef SPACE_HPP__
