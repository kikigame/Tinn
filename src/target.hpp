/* License and copyright go here*/

// targeting methods

#ifndef TARGET_HPP
#define TARGET_HPP

class monster;

// pick a target
template <bool lineOfSight>
monster *pickTarget(monster &src);


#endif //ndef TARGET_HPP