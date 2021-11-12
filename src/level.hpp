/* License and copyright go here*/

// define a level in the dungeon

#ifndef LEVEL_HPP__
#define LEVEL_HPP__

#include <ostream>
#include <map>
#include <vector>
#include <utility> // for pair
#include <memory> // for shared_ptr

#include "coord.hpp"
#include "zone.hpp"
#include "ref.hpp"

class itemHolder;
class item;
class monster;
class terrain;
enum class terrainType;
enum class monsterTypeKey;

class levelImpl;
class dungeon;
class formatter;
class drawIter;
class role;

class renderByCoord;

class level;
class levelFactoryImpl;
class levelFactory {
private:
  std::shared_ptr<levelFactoryImpl> pImpl_;
public:
  levelFactory(dungeon &dungeon, const int numLevels, role &);
  std::vector<level*>::iterator begin();
  std::vector<level*>::iterator end();
};

class level {
private:
  std::unique_ptr<levelImpl> pImpl_;
public:
  static const int MAX_WIDTH = 70;
  static const int MAX_HEIGHT = 20;
  level(levelImpl *);
  level(const level &other) = delete; // disallow copying
  level(const level &&other) = delete; // disallow moving
  ~level();
  std::wstring name() const;
  drawIter drawBegin() const;
  drawIter drawEnd() const;
  /*
   * Return the terrain at the given coordinates.
   * NB: You probably don't want to use this unless you're thinking carefully about transport.
   */
  const terrain &terrainAt(const coord & c) const;
  /*
   * Test if AT LEAST ONE of the coords ADJACENT to c are of type t.
   */
  bool isTerrainAdjacent(const terrainType &t, const coord &c) const;
  /*
   * Find any coordinate (probably deterministicly)
   * with the given terrain type. not a "posOf" as it may not be unique.
   */
  coord findTerrain(const terrainType &type) const;
  /*
   * Find all the terrain of a given type
   */
  std::vector<coord> findAllTerrain(const terrainType &type) const;
  /*
   * Return the postiion of a monster, which may be the player.
   */
  coord posOf(const monster & m) const;
  /*
   * Return the postiion of an item
   */
  coord posOf(const item & i) const;
  /*
   * Attempt to find a monster in the given direction from another.
   * Assumes that any square (from) can move through is passible for the search.
   * (i.e. will search through rocks if from can burrow, but not if from cannot)
   * returns "from" if dir == L'.' (hence "from" is not const).
   */
  optionalRef<monster> findMonster(monster &from, const wchar_t dir) const;
  /*
   * Which special features (if any) exist at the given location?
   */
  template <typename T>
  filteredIterable<std::shared_ptr<zoneArea<T> >,std::vector<std::shared_ptr<zoneArea<T> > > > zonesAt(const coord &);
  /*
   * Syntax sugar for posOf(player)
   */
  const coord pcPos() const;
  int depth() const;

  // request specific movement of monsters:
  void moveOrFight(monster &m, const ::dir &d, bool avoidTraps);
  void up(monster & m);
  void down(monster & m);
  void moveTo(terrainType terrain); // teleport the player to somewhere with the given terrain (must exist)
  /*
   * move by the given delta (1 square max)
   * true - do not move into traps or walls
   * false - move into traps regardless (like the old computer game Zombies)
   */
  void move(monster &monster, const ::dir dir, const bool avoidTraps);
  /*
   * move to given coordinates, applying zone effects & traps
   */
  void moveTo(monster &monster, const coord &targetPos);
  /*
   * move to given coordinates, ignoring zone restrictions & applying traps.
   * Use carefully; should probably only be used in zones.
   * May still be skipped if the monster refuses the move.
   */
  void teleportTo(monster &monster, const coord &targetPos);
  void addMonster(std::shared_ptr<monster> monster, const coord &targetPos);
  void removeDeadMonster(monster & m, bool allowCorpse); // called by monster so can't get the shared_ptr.
  // modify the location of a monster that can occupy multiple coordinates:
  void bigMonster(monster &m, std::vector<coord> &pos);

  /*
   * Player picks up items from the current level square
   */
  void pickUp();

  /*
   * Iterators over all the monsters at the given coordinates.
   * NB: coordinates need not exist on the map.
   */
  std::vector<ref<monster> > monstersAt(const coord &pos) const;

  /*
   * Iterate over each monster on the level
   */
  void forEachMonster(std::function<void(monster &)> f);

  // return a mutable holder for an item on the level
  itemHolder& holder(const item &item);

  // return a mutable holder for items in the given square
  itemHolder& holder(const coord &c);

  // create a prison in an empty portion of the map.
  // throws std::wstring if no part of the map could be found (after 100 tries)
  coord createPrison();

  // get a reference to the dungeon
  dungeon & dung();
  const dungeon & dung() const;

  // get a reference to I/O
  formatter msg() const;
  
  // determine if a given position is passible or not to the given monster
  // oldPos - needed in case a transport at the old pos can be used. Pass "pos" for teleporting or creating monsters.
  bool movable(const coord &oldPos, const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) const;

  // dangerous method; it does not do anything with any monsters on this square. Use sparingly.
  void changeTerrain(const coord &c, terrainType t);

  // determine if a monster still exists at the given location.
  // Returns false if the monster has died. (So can't take a reference as it might be invalid).
  bool stillOnLevel(const monster *mon) const;

  // prevent copying
  level operator =(level &other) = delete;
  // prevent moving
  level operator =(level &&other) = delete;

  operator const renderByCoord&() const;

  // find a non-coaligned monster in line of sight:
  optionalRef<monster> lineOfSightTarget(monster &m);

  // crack the level, eg for an earthquake
  void crack();

  // access all coords, eg for rnd() (coord.hpp)
  coordRectIterator allCoords();
};

# endif // ndef  LEVEL_HPP__
