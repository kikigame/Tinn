/* License and copyright go here*/

// properties that a monster can have

#ifndef MONSTER_INTRINSICS_HPP
#define MONSTER_INTRINSICS_HPP

#include "bonus.hpp"
#include "movement.hpp"
#include <memory> // unique_ptr

class damage;
class terrain;

class monsterAbilities {
public:
  // monsters may be inherantly proof (bonus) against a damage type:
  virtual void proof(const damage & type, const bool isProof) = 0;
  virtual const bool proof(const damage & type) const = 0;
  // and there may be some simple flags too:
  virtual void eatVeggie(const bonus & isBonus) = 0;
  virtual const bonus & eatVeggie() const = 0;
  // bonus - 2 x attacks / round = 0; penalty - 2 x rounds / attack
  virtual void dblAttack(const bonus & isDblAttack) = 0;
  virtual const bonus & dblAttack() const = 0;
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
  // can you hear monsters?
  virtual void hear(const bool hearing) = 0;
  virtual const bool hear() const = 0;
  // can you see monsters?
  virtual void see(const bool sight) = 0;
  virtual const bool see() const = 0;
  // can you move through water?
  virtual void swim(const bool canSwim) = 0;
  virtual const bool swim() const = 0;
  // can you fly?
  virtual void fly(const bool canSwim) = 0;
  virtual const bool fly() const = 0;
  // are you stuck in a trap right now?
  virtual void entrap(const int ticksToEscape) = 0;
  virtual const bool entrapped() const = 0;
  // good at climbing?
  virtual void climb(const bonus & canClimb) = 0;
  virtual const bonus & climb() const = 0;
  // does *this* monster have a speed bonus/penalty
  virtual void speedy(const bonus & fast) = 0;
  virtual const bonus & speedy() const = 0;
  // adjust the given enum based on the speed bonus/penalty
  virtual speed adjust(const speed & fastness) = 0;
};

class monsterIntrinsicsImpl;
class monsterIntrinsics : public monsterAbilities {
private:
const std::unique_ptr<monsterIntrinsicsImpl> pImpl_;
public:
  monsterIntrinsics();
  ~monsterIntrinsics();
  // monsters may be inherantly proof (bonus) against a damage type:
  virtual void proof(const damage & type, const bool isProof);
  virtual const bool proof(const damage & type) const;
  // and there may be some simple flags too:
  virtual void eatVeggie(const bonus & isBonus);
  virtual const bonus & eatVeggie() const;
  // bonus - 2 x attacks / round; penalty - 2 x rounds / attack
  virtual void dblAttack(const bonus & isDblAttack);
  virtual const bonus & dblAttack() const;
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
  // can you hear monsters?
  virtual void hear(const bool hearing);
  virtual const bool hear() const;
  // can you see monsters?
  virtual void see(const bool sight);
  virtual const bool see() const;
  // can you move through water?
  virtual void swim(const bool canSwim);
  virtual const bool swim() const;
  // can you fly?
  virtual void fly(const bool canSwim);
  virtual const bool fly() const;
  // are you stuck in a trap right now?
  virtual void entrap(const int ticksToEscape);
  virtual const bool entrapped() const;
  // good at climbing?
  virtual void climb(const bonus & canClimb);
  virtual const bonus & climb() const;
  // does *this* monster have a speed bonus/penalty
  virtual void speedy(const bonus & fast);
  virtual const bonus & speedy() const;
  // adjust the given enum based on the speed bonus/penalty
  virtual speed adjust(const speed & fastness);
};


#endif //MONSTER_INTRINSICS_HPP
