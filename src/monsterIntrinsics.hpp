/* License and copyright go here*/

// properties that a monster can have

#ifndef MONSTER_INTRINSICS_HPP
#define MONSTER_INTRINSICS_HPP

#include "bonus.hpp"
#include "movement.hpp"
#include <memory> // unique_ptr

class damage;
class terrain;

class monsterIntrinsicsImpl;
class monsterIntrinsics {
private:
const std::unique_ptr<monsterIntrinsicsImpl> pImpl_;
public:
  monsterIntrinsics();
  ~monsterIntrinsics();
  // monsters may be inherantly proof (bonus) against a damage type:
  void proof(const damage & type, const bool isProof);
  const bool proof(const damage & type) const;
  // and there may be some simple flags too:
  void eatVeggie(const bonus & isBonus);
  const bonus & eatVeggie() const;
  // bonus - 2 x attacks / round; penalty - 2 x rounds / attack
  void dblAttack(const bonus & isDblAttack);
  const bonus & dblAttack() const;
  // resist damage of a given type (nullptr for all) - by 5% increments
  // (may be negative for extra damage)
  void resist(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative for extra damage)
  const char resist(const damage & type) const;
  // extra damage of a given type (nullptr for all) - by 5% increments
  // (may be negative to deal healing)
  void extraDamage(const damage * type, char level);
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative to deal healing)
  const char extraDamage(const damage & type) const;
  // can you move through a given terrain?
  void move(const terrain & type, const bool isMove);
  const bool move(const terrain & type) const;
  // can you hear monsters?
  void hear(const bool hearing);
  const bool hear() const;
  // can you see monsters?
  void see(const bool sight);
  const bool see() const;
  // can you move through water?
  void swim(const bool canSwim);
  const bool swim() const;
  // good at climbing?
  void climb(const bonus & canClimb);
  const bonus & climb() const;
  // does *this* monster have a speed bonus/penalty
  void speedy(const bonus & fast);
  const bonus & speedy() const;
  // adjust the given enum based on the speed bonus/penalty
  speed adjust(const speed & fastness);
};


#endif //MONSTER_INTRINSICS_HPP
