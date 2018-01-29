/* License and copyright go here*/

// define movement within the dungeon

#ifndef MOBILE_HPP
#define MOBILE_HPP

#include "movement.hpp"
#include "level.hpp"

/*
 * Concept: mobile.
 * Provides T.cuLevel(), returns reference to level on which mobile should move
 * Provides T.movement(), returns a reference to movementType struct describing movement.
 * Provides T.align(), returns the alignment for movement/coalignment purposes (reference to deity)
 */

class monster;

template<class T>
void moveMobile(T &mon);
void monsterAttacks(monster &mon);


template <class T>
struct movementTraits{
  // default: return the unmodified speed. For monsters, this is modified by the abilities.
  static speed adjust(T &t, const movementType &type) { return type.speed_; }
  // default: get position by asking the level.
  static coord position(level &level, T &t) { return level.posOf(t); }
};



#endif //ndef MOBILE_HPP
