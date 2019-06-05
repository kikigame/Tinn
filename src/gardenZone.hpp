/* License and copyright go here*/

// implement the behaviour of garden tiles

#ifndef GARDEN_HPP__
#define GARDEN_HPP__

#include "zone.hpp"
#include "geometry.hpp"
#include "time.hpp"
#include "terrain.hpp"
#include "itemTypes.hpp"
#include <map>
#include <memory>

class monster;
class item;
class level;

class gardenCallback;
/*
 * Gardens: Works on water and ground.
 *
 * When planting a water plant on water, or a ground plant on ground,
 * The plant will come back in the 9 surrounding squares a few turns later.
 *
 * A plant-aligned monster will leave plants in their wake while walking through it, if non-hostile.
 *
 * A hostile garden zone may replace ground tiles with knotweed.
 */
class gardenZone : public zoneArea<monster>, public zoneArea<item> {
private:
  std::unique_ptr<geometry> geometry_;
  bool hostile_;
  //const coord ul_;
  //  const coord lr_;
  level &lvl_;
  std::vector<std::shared_ptr<gardenCallback>> callbacks_;
public:
  gardenZone(std::unique_ptr<geometry> &&, level &lev, bool hostile);
  virtual ~gardenZone();

  // determine if a given square contains this zone
  virtual bool contains(coord area);

  virtual bool onMoveWithin(monster &, const coord &dest);
  //virtual bool onEnter(monster &, itemHolder &prev);
  virtual bool onExit(monster &, itemHolder &prev);

  virtual bool onMoveWithin(item &, const coord &dest);
  //virtual bool onEnter(item &, itemHolder &prev);
  virtual bool onExit(item &, itemHolder &next);

};


class gardenCallback : public temporal::callback {
private:
  std::vector<std::shared_ptr<gardenCallback>> &callbacks_;
  coord dest_;
  level &lvl_;
  const terrainType tType_;
  const itemTypeKey typeKey_;
public:
  gardenCallback(std::vector<std::shared_ptr<gardenCallback>> &callbacks,
		 const coord &dest, level &lvl,
		 const terrainType & tType, const itemTypeKey &typeKey);
  virtual ~gardenCallback();
  void operator()();
};



#endif //ndef GARDEN_HPP__
