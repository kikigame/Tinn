/* License and copyright go here*/

// define a simple trystate type

#ifndef BONUS_HPP
#define BONUS_HPP

class bonus {
private:
  bool bonus_;
  bool penalty_;
public:
  bonus(const bonus & rhs);
  // true to create a bonus, false to create a penalty:
  bonus(bool bonusOrPenalty);
  // create a non-bonus, non-penalty (ie the default state)
  bonus();
  bool operator == (const bonus & rhs) const;
  bool operator != (const bonus & rhs) const;
  bool operator > (const bonus & rhs) const;
  bool operator < (const bonus & rhs) const;
  bonus operator + (const bonus & rhs) const;
  bonus operator - (const bonus & rhs) const;
  bonus & operator += (const bonus & rhs);
  bonus & operator -= (const bonus & rhs);
  bonus & operator = (const bonus & rhs);
};


#endif // BONUS_HPP
