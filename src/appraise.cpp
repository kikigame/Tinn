/* License and copyright go here*/

// how much is my thingy worth?

#include "appraise.hpp"
#include "damage.hpp"
#include "monster.hpp"

bool isDamaged(const item &);

double appraise(const monster &monster, const item &thing) {

  auto value = thing.weight();
  // switch on category type
  switch (thing.render()) {
    // food
  case L'%': value *= 0.5; break;
    // weapons
  case L'!': value *= 10; break;
    // missiles
  case L'¬': value *= 3; break;
    // clothing - worth much less if damaged
  case L'[': if (isDamaged(thing)) value *= 0.1; break;
    // stylii - worth lots if magical
  case L'❘': if (thing.enchantment() > 0) value *= 10; break;
    // bottles
  case L'8': break;
    // readables
  case L'¶': break;
    // containers
  case L'=': break;
    // Liquids
  case L'~': break;
    // jewellery - lightweight but more valuable
  case L'*': value *= 30; break;
    // valuables - inherantly valuable
  case L'$': value *= 40; break;
    // tools - utility value
  case L'(': value *= 2; break;
  }
  if (thing.isBlessed()) value *= 2;
  if (thing.isCursed()) value *= 0.5;
  if (thing.isUnidentified()) value *= 0.9;
  if (thing.isSexy()) value *= 1.5;
  {
    int proofs = 0;
    for (auto dt = damageRepo::instance().begin(); dt != damageRepo::instance().end(); ++dt) {
      if (thing.isProof(dt->first)) proofs++;
    }
    value += value * 0.2 * proofs;
  }
  value += monster.appearance().cur(); // TODO: buy/sell; tweak
  value *= (100 + thing.enchantment()) / 100.;
  return value;
}


bool isDamaged(const item &item) {
  auto &dr = damageRepo::instance();
  for (auto dt : dr) // map of damageType to damage
    if (item.damageOfType(dt.first) != 0) return true;
  return false;
}
