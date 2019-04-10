/* License and copyright go here*/

// define a simple trystate type

#include "bonus.hpp"

bonus::bonus(bool bonusOrPenalty) :
  bonus_(bonusOrPenalty == true),
  penalty_(bonusOrPenalty == false) {}
bonus::bonus() : 
  bonus_(false),
  penalty_(false) {}
bonus::bonus(const bonus & rhs) :
  bonus_(rhs.bonus_),
  penalty_(rhs.penalty_) {}

/*
 * TRUTH TABLE:
 *
 * A     \B| Bonus  | Default | penalty_
 * ------------------------------------
 * Bonus   | A == B | A > B   | A > B
 * Default | A < B  | A == B  | A > B
 * Penalty_ | A < B  | A < B   | A == B
 */

bool bonus::operator == (const bonus & rhs) const {
  return bonus_ == rhs.bonus_ && penalty_ == rhs.penalty_;
}
bool bonus::operator != (const bonus & rhs) const {
  return !(bonus_ == rhs.bonus_ && penalty_ == rhs.penalty_);
}
bool bonus::operator > (const bonus & rhs) const {
  if (bonus_ && !rhs.bonus_) return true;
  if (!bonus_ && !penalty_ && rhs.penalty_) return true;
  return false;
}
bool bonus::operator < (const bonus & rhs) const {
  if (penalty_ && !rhs.penalty_) return true;
  if (!penalty_ && !bonus_ && rhs.bonus_) return true;
  return false;
}
bonus bonus::operator + (const bonus & rhs) const {
  // equal stays the same:
  if (*this == rhs) return bonus(rhs);
  // bonus and penalty_ cancel:
  if ((penalty_ ^ rhs.penalty_) && (bonus_ ^ rhs.penalty_)) return bonus();
  // at least one bonus => bonus
  if (bonus_ || rhs.bonus_) return bonus(true);
  // at least one penalty_ => penalty_
  if (penalty_ || rhs.penalty_) return bonus(false);
  // no score draw:
  return bonus();
}
// can't think of a use for this, but seems legitimate:
bonus bonus::operator - (const bonus & rhs) const {
  // bonuses cancel:
  if (bonus_ && rhs.bonus_) return bonus();
  // penalties cancel too:
  if (penalty_ && rhs.penalty_) return bonus();
  // bonus_x - penalty_ == bonus_
  // penalty_ - bonus_ == penalty_
  // equal stays the same:
  return bonus(*this);
}
bonus & bonus::operator += (const bonus & rhs) {
  *this = (*this) + rhs;
  return *this;
}
bonus & bonus::operator -= (const bonus & rhs) {
  *this = (*this) - rhs;
  return *this;
}
bonus & bonus::operator = (const bonus & rhs) {
  bonus_ = rhs.bonus_;
  penalty_ = rhs.penalty_;
  return *this;
}

