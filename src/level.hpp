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

class itemHolder;
class item;
class monster;
class terrain;
enum class terrainType;

class levelImpl;
class dungeon;
class drawIter;

class io;

class level {
private:
  std::shared_ptr<levelImpl> pImpl_;
public:
  static const int MAX_WIDTH = 70;
  static const int MAX_HEIGHT = 20;
  level(dungeon & dungeon_, ::std::shared_ptr<io> ios, int depth, bool hasDownRamp);
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
  void removeDeadMonster(const monster & m); // called by monster so can't get the shared_ptr.

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

  /*
   * Add an item to the level
   */
  void addItem(const std::shared_ptr<item> item, const coord c);

  // return a mutable holder for an item on the level
  std::unique_ptr<itemHolder> holder(const std::shared_ptr<item> item);

  // return a mutable holder for items in the given square
  std::unique_ptr<itemHolder> holder(const coord c);

  // get a reference to the dungeon
  dungeon & dung();

  // prevent copying
  level operator =(level &other) = delete;
};

# endif // ndef  LEVEL_HPP__
