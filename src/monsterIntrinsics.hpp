/* License and copyright go here*/

// properties that a monster can have

#ifndef MONSTER_INTRINSICS_HPP
#define MONSTER_INTRINSICS_HPP

#include "bonus.hpp"
#include "movement.hpp"
#include "sense.hpp"
#include <memory> // unique_ptr

class damage;
class terrain;
class itemHolder;

class monsterAbilities {
public:
  virtual ~monsterAbilities();
  // monsters may be inherantly proof (bonus) against a damage type:
  virtual void proof(const damage & type, const bool isProof) = 0;
  virtual const bool proof(const damage & type) const = 0;
  // and there may be some simple flags too:
  virtual void eatVeggie(const bonus & isBonus) = 0;
  virtual const bonus eatVeggie() const = 0;
  // bonus - 2 x attacks / round = 0; penalty - 2 x rounds / attack
  virtual void dblAttack(const bonus & isDblAttack) = 0;
  virtual const bonus dblAttack() const = 0;
  // resist damage of a given type (nullptr for all) - by 5% increments
  // (may be negative for extra damage)
  virtual void resist(const damage * type, char level) = 0;
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative for extra damage)
  virtual const char resist(const damage & type) const = 0;
  // extra damage of a given type (nullptr for all) - by 5% increments
  // (may be negative to deal healing)
  virtual void extraDamage(const damage * type, char level) = 0;
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative to deal healing)
  virtual const char extraDamage(const damage & type) const = 0;
  // can you move through a given terrain?
  virtual void move(const terrain & type, const bool isMove) = 0;
  virtual const bool move(const terrain & type) const = 0;
  // do you have a particular sense?
  virtual void sense(const sense::sense &t, bool s) = 0;
  virtual const bool hasSense(const sense::sense &t) const = 0;
  // can you fly?
  virtual void fly(const bool canFly) = 0;
  virtual const bool fly() const = 0;
  // affected by petrify/fear actions? (false = double effect)
  virtual void fearless(const bonus &fearless) = 0;
  virtual const bonus fearless() const = 0;
  // are you stuck in a trap right now?
  virtual void entrap(const int ticksToEscape) = 0;
  virtual const bool entrapped() const = 0;  
  // good at climbing?
  virtual void climb(const bonus & canClimb) = 0;
  virtual const bonus climb() const = 0;
  // does *this* monster have a speed bonus/penalty
  virtual void speedy(const bonus & fast) = 0;
  virtual const bonus speedy() const = 0;
  // adjust the passed enum based on the speed bonus/penalty
  virtual speed adjust(const speed & fastness) = 0;
  // can this monster use thrown weapons?
  virtual void throws(const bool lobs) = 0;
  virtual const bool throws() const = 0;
  // can this monster use wands? (eg ranged attack / healing)
  virtual void zap(const bool useWands) = 0;
  virtual const bool zap() const = 0;
  // can this monster rest?
  virtual void sleeps(const bool snoozy) = 0;
  virtual const bool sleeps() const = 0;
  // how much weight can we comfortably carry?
  virtual void carryWeightN(const int ) = 0;
  virtual const int carryWeightN() const = 0;
};

class monsterIntrinsicsImpl;
class monsterIntrinsics : public monsterAbilities {
private:
const std::unique_ptr<monsterIntrinsicsImpl> pImpl_;
public:
  monsterIntrinsics();
  // explicit copy constructor as we create intrinsics by copying from the monster type:
  monsterIntrinsics(const monsterIntrinsics &);
  virtual ~monsterIntrinsics();
  // monsters may be inherantly proof (bonus) against a damage type:
  virtual void proof(const damage & type, const bool isProof);
  virtual const bool proof(const damage & type) const;
  // and there may be some simple flags too:
  virtual void eatVeggie(const bonus & isBonus);
  virtual const bonus eatVeggie() const;
  // bonus - 2 x attacks / round; penalty - 2 x rounds / attack
  virtual void dblAttack(const bonus & isDblAttack);
  virtual const bonus dblAttack() const;
  // resist damage of a given type (nullptr for all) - by 5% increments
  // (may be negative for extra damage)
  virtual void resist(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative for extra damage)
  virtual const char resist(const damage & type) const;
  // extra damage of a given type (nullptr for all) - by 5% increments
  // (may be negative to deal healing)
  virtual void extraDamage(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative to deal healing)
  virtual const char extraDamage(const damage & type) const;
  // can you move through a given terrain?
  virtual void move(const terrain & type, const bool isMove);
  virtual const bool move(const terrain & type) const;
  // do you have a particular sense? (NB: sixth sense always returns false here because it's not reliable)
  virtual void sense(const sense::sense &t, bool s);
  virtual const bool hasSense(const sense::sense &t) const;
  // can you fly?
  virtual void fly(const bool canFly);
  virtual const bool fly() const;
  // affected by petrify/fear actions? (false = double effect)
  virtual void fearless(const bonus &fearless);
  virtual const bonus fearless() const;
  // are you stuck in a trap right now?
  virtual void entrap(const int ticksToEscape);
  virtual const bool entrapped() const;
  // good at climbing?
  virtual void climb(const bonus & canClimb);
  virtual const bonus climb() const;
  // does *this* monster have a speed bonus/penalty
  virtual void speedy(const bonus & fast);
  virtual const bonus speedy() const;
  // adjust the given enum based on the speed bonus/penalty
  virtual speed adjust(const speed & fastness);
  // can this monster use thrown weapons? (eg ranged attack / healing)
  virtual void throws(const bool useWands);
  virtual const bool throws() const;
  // can this monster use wands? (eg ranged attack / healing)
  virtual void zap(const bool useWands);
  virtual const bool zap() const;
  // can this monster rest?
  virtual void sleeps(const bool snoozy);
  virtual const bool sleeps() const;
  // how much weight can we comfortably carry?
  virtual void carryWeightN(const int );
  virtual const int carryWeightN() const;
};

// abilites work the same as intrinsics, but may be granted by a more 
// transient action, such as wearing a magic item, and can be more easily revoked.
class monsterAbilityMods : public monsterIntrinsics {
private:
  const itemHolder &mon_;
  std::shared_ptr<monsterIntrinsics> intrinsics_;
  std::shared_ptr<monsterIntrinsicsImpl> mod_;
public:
  monsterAbilityMods(itemHolder &mon, std::shared_ptr<monsterIntrinsics> intrinsics);
  virtual ~monsterAbilityMods() {}
  // monsters may be inherantly proof (bonus) against a damage type:
  virtual void proof(const damage & type, const bool isProof);
  virtual const bool proof(const damage & type) const;
  // and there may be some simple flags too:
  virtual void eatVeggie(const bonus & isBonus);
  virtual const bonus eatVeggie() const;
  // bonus - 2 x attacks / round; penalty - 2 x rounds / attack
  virtual void dblAttack(const bonus & isDblAttack);
  virtual const bonus dblAttack() const;
  // resist damage of a given type (nullptr for all) - by 5% increments
  // (may be negative for extra damage)
  virtual void resist(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative for extra damage)
  virtual const char resist(const damage & type) const;
  // extra damage of a given type (nullptr for all) - by 5% increments
  // (may be negative to deal healing)
  virtual void extraDamage(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative to deal healing)
  virtual const char extraDamage(const damage & type) const;
  // can you move through a given terrain?
  virtual void move(const terrain & type, const bool isMove);
  virtual const bool move(const terrain & type) const;
  // grant/revoke particular senses by magical means.
  virtual void sense(const sense::sense &t, bonus s);
  // do you have a particular sense? (NB: pc can still receive sixth sense messages if this returns false)
  // acts on the least significant bit set to 1
  virtual const bool hasSense(const sense::sense &t) const;
  // can you fly?
  virtual void fly(const bool canFly);
  virtual const bool fly() const;
  // affected by petrify/fear actions? (false = double effect)
  virtual void fearless(const bonus &fearless);
  virtual const bonus fearless() const;
  // are you stuck in a trap right now?
  virtual void entrap(const int ticksToEscape);
  virtual const bool entrapped() const;
  // good at climbing?
  virtual void climb(const bonus & canClimb);
  virtual const bonus climb() const;
  // does *this* monster have a speed bonus/penalty
  virtual void speedy(const bonus & fast);
  virtual const bonus speedy() const;
  // adjust the given enum based on the speed bonus/penalty
  virtual speed adjust(const speed & fastness);
  // can this monster use thrown weapons? (eg ranged attack / healing)
  virtual void throws(const bool useWands);
  virtual const bool throws() const;
  // can this monster use wands? (eg ranged attack / healing)
  virtual void zap(const bool useWands);
  virtual const bool zap() const;
  // can this monster rest?
  virtual void sleeps(const bool snoozy);
  virtual const bool sleeps() const;
  // how much weight can we comfortably carry?
  virtual void carryWeightN(const int n); // NB: Mutator is permanent.
  virtual const int carryWeightN() const;
};


#endif //MONSTER_INTRINSICS_HPP
