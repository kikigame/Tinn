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

class itemHolder;
class item;
class monster;
class terrain;
enum class terrainType;

class levelImpl;
class dungeon;
class drawIter;

class renderByCoord;

class level;
class levelFactoryImpl;
class levelFactory {
private:
  std::shared_ptr<levelFactoryImpl> pImpl_;
public:
  levelFactory(dungeon &dungeon, const int numLevels);
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
  drawIter drawBegin() const;
  drawIter drawEnd() const;
  /*
   * Find any coordinate (probably deterministicly)
   * with the given terrain type. not a "posOf" as it may not be unique.
   */
  coord findTerrain(const terrainType type) const;
  /*
   * Return the postiion of a monster, which may be the player.
   */
  coord posOf(const monster & m) const;
  /*
   * Return the postiion of an item
   */
  coord posOf(const item & i) const;
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
  void north(monster & m);
  void south(monster & m);
  void east(monster & m);
  void west(monster & m);
  void up(monster & m);
  void down(monster & m);
  void moveTo(terrainType terrain); // teleport the player to somewhere with the given terrain (must exist)
  /*
   * move by the given delta (1 square max)
   * true - do not move into traps or walls
   * false - move into traps regardless (like the old computer game Zombies)
   */
  void move(monster &monster, const std::pair<char,char> dir, const bool avoidTraps);
  void moveTo(monster &monster, const coord targetPos);
  void addMonster(std::shared_ptr<monster> monster, const coord targetPos);
  void removeDeadMonster(monster & m); // called by monster so can't get the shared_ptr.

  /*
   * Player picks up items from the current level square
   */
  void pickUp();

  /*
   * Iterators over all the monsters at the given coordinates.
   * NB: coordinates need not exist on the map.
   */
  std::pair<std::multimap<coord, std::shared_ptr<monster> >::iterator,
	    std::multimap<coord, std::shared_ptr<monster> >::iterator> monstersAt(const coord &pos);

  // return a mutable holder for an item on the level
  itemHolder& holder(const item &item);

  // return a mutable holder for items in the given square
  itemHolder& holder(const coord c);

  // get a reference to the dungeon
  dungeon & dung();

  // determine if a given position is passible or not to the given monster
  bool movable(const coord &pos, const monster &m, bool avoidTraps, bool avoidHiddenTraps) const;

  // prevent copying
  level operator =(level &other) = delete;

  operator const renderByCoord&() const;
};

# endif // ndef  LEVEL_HPP__
