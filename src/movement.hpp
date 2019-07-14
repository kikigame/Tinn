/* License and copyright go here*/

// how does this monster (or whatever) move?

#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

enum class speed {
  stop, // movement attempts cause 3 ticks and error
  slow3, // move only on turn % 3 == 0
  slow2, // move only on turn % 3 == 0
  perturn, // move each turn
  turn2, // move twice per turn
  turn3 // move thrice per turn
};

enum class goTo {
  none, // does not move; speed is ignored for movement
  wander, // move in a random direction
  player, // seek out the player. For transport, this means it's controlled by the monster using it.
  coaligned, // seek out the player if coaligned, or any coaligned temple, or any coaligned monster, or stay put.
  unaligned, // seek out the player if unaligned, or any unaligned monster, elso stay put.
  up, // find an up ramp and sit on it
  down, // find a down ramp and sit on it
  crack, // find any crack terran at random, not necessarily the one we're on
  web, // find any web terran at random, not necessarily the one we're on
};

enum class goBy {
  avoid, // go away from the target, stop for traps and walls
  zomavoid, // go away from the target, ignoring traps and walls. stop only where impossible
  zombeeline, // go straight, ignoring traps and walls. stop only where impossible
  beeline, // go straight, stop for traps and walls
  smart, // go straight, go around traps and walls
  teleport // teleport to the target
};

/*
 * Tinn creates complexity by cross-joining. Each monster type can move in one a number of different
 * movement styles, by combining the different properties above.
 * There are 31 of these at time of writing this comment.
 */
struct movementType {
  speed speed_;
  goTo goTo_;
  goBy goBy_;
  unsigned char jitterPc; // chance of deviating by one square from path
};

extern movementType stationary; //{ speed::stop, goTo::none, goBy::avoid, 0 };


#endif // ndef MOVEMENT_HPP~
