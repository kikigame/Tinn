/* License and copyright go here*/

// pure virtual interfaces for things that can have action at a distance

#ifndef COMBAT_HPP__
#define COMBAT_HPP__

// interface for things which monsters may use instead of moving during combat.
class useInCombat {
public:
  // always a virtual destructor
  virtual ~useInCombat() {}
  /*
   * Determine if this specific object should be used.
   * m - monster considering using this item
   */
  virtual bool shouldUse(const monster &m) const = 0;
  /*
   * perform the action
   */
  virtual bool useForCombat() = 0;
};

#endif //ndef COMBAT_HPP__
