/* License and copyright go here*/

// how much is my thingy worth?

#include "appraise.hpp"
#include "damage.hpp"
#include "monster.hpp"

double appraise(const monster &monster, const item &thing) {

  auto value = thing.weight();
  // TODO: switch on category type
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
