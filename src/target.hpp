/* License and copyright go here*/

// targeting methods

#ifndef TARGET_HPP
#define TARGET_HPP

class monster;

// pick a target
template <bool lineOfSight>
monster *pickTarget(monster &src, bool allowOnSelf = false);

// test if the given target is viable.
// NB: this may return a target inside a zone where it can't be attacked
// (eg shrine). The deity may intercede to prevent te attack, but the
// attacking monster should still hang around the shrine waiting...
bool viableTarget(monster &attacker, monster &test);

#endif //ndef TARGET_HPP
