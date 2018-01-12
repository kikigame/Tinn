/* License and copyright go here*/

// build levels in the dungeon

#ifndef LEVELGEN_HPP__
#define LEVELGEN_HPP__

#include "monsterType.hpp"
#include "coord.hpp"
#include "terrain.hpp"
#include "optionalRef.hpp"
#include <vector>

class level;
class levelImpl;
class monster;
class deity;

// helper class to generate a random layout of a level
class levelGen {
protected:

  levelImpl* const level_; // non-owning pointer
  level& pub_; // non-owning reference

public:
  levelGen(levelImpl* const level, ::level& pub) :
    level_(level), pub_(pub) {
    // ensure the method is created by the compiler:
    place(coordRectIterator(0,0,0,0), terrainType::GROUND);
  }
  virtual ~levelGen() {}

protected:
  // name the level (optional; default is based on depth)
  void setName(const std::wstring & name);

  // add an individual room. Returns first=top-left, second=bottom-right
  std::pair<coord,coord> addRoom();

  // add a shrine. Returns first=top-left, second=bottom-right
  // NB: Does not place an altar, as its placement depends on the access direction
  std::pair<coord, coord> addShrine();

  // place a shrine at given position (pass empty d for random)
  // NB: Does not place an altar, as its placement depends on the access direction
  void addShrine(const coord &tl, const coord &br, optionalRef<deity> d = optionalRef<deity>());

  // register zones:
  void itemZone(std::shared_ptr<zoneArea<item> >);
  void monsterZone(std::shared_ptr<zoneArea<monster> >);

  // possibly entrap a room:
  void addTraps(const std::pair<coord,coord> &coords);

  // possibly add some monsters to the room:
  void addMonsters(std::vector<std::pair<coord,coord>>);

  // possibly add some items to the room:
  void addItems(const std::pair<coord,coord> &);

  // add a monster to the room, but not at c: (used by addMonsters)
  void addMonster(std::shared_ptr<monster> mon, const coord &c, const std::pair<coord,coord> &);

  // add a monster at c:
  monster &addMonster(monsterTypeKey mon, const coord &c);

  // add a monster at c:
  monster &addMonster(monsterBuilder &b, const coord &c);

  // add a monster at c: (others eventually call this):
  void addMonster(std::shared_ptr<monster> mon, const coord &c);

  // adds a corridor starting at from and ending at to. Very basic for now.
  // returns first square adjacent to "from"
  coord addCorridor(const coord from, const coord to);

  // average two coords and return their midpoint.
  coord mid(const std::pair<coord,coord> &p) const {
    return coord((p.first.first + p.second.first) / 2,
		 (p.first.second + p.second.second) / 2);
  }

  template<class iter>
  void place(const iter & begin,
 	     const iter & end,
	     terrainType terrainType);

  template<class iter>
  void place(iter it,
	     terrainType terrainType);

  void place(const coord &c, terrainType terrainType);

  terrainType at(const coord &c) const;

  // as level::findTerrain, but returning any square, not the first.
  // very inefficient for small number of available squares.
  coord findRndTerrain(terrainType t) const;

private:
  void changeTerrain(coord pos, terrainType from, terrainType to);

public:
  // call to place ramps (in case of adjacent level requirements)
  virtual void negotiateRamps(optionalRef<levelGen> next) = 0;
  // override to specify position of our upramp
  virtual coord upRampPos() { return coord(-1,-1); }
  // call the build() function to add rooms, monsters etc.
  virtual void build() = 0;
};



#endif //ndef LEVELGEN_HPP__
