/* License and copyright go here*/

// properties that a monster can have

#include "monsterIntrinsics.hpp"
#include "damage.hpp"
#include "terrain.hpp"
#include "itemholder.hpp"
#include "items.hpp"
#include "monster.hpp"
#include <map>
#include <bitset>

enum class bonusType {
  eatVeggie, // TODO
    dblAttack, // TODO
    speedy, // 1 slot on speed
    hearing, // TODO
    seeing, // TODO
    flying,
    swimming, // TODO (need water too)
    climbing, // fast at escaping pits
    fearless, // affected by petrify (or other fear) actions
    throws, //can the monster lob things at another
    zap, // can the monster zap another with a wand
    END
    };

// shared "method":
speed adjustSpeed(const bonus &s, const speed &fastness) {
  switch (fastness) {
  case speed::stop: return speed::stop;
  case speed::slow3:   return s == bonus(true) ? speed::slow2 : speed::slow3;
  case speed::slow2:   return s == bonus(true) ? speed::perturn : s == bonus(false) ? speed::slow3 : speed::slow2;
  case speed::perturn: return s == bonus(true) ? speed::turn2 : s == bonus(false) ? speed::slow2 : speed::perturn;
  case speed::turn2:   return s == bonus(true) ? speed::turn3 : s == bonus(false) ? speed::perturn : speed::turn2;
  case speed::turn3:   return s == bonus(false) ? speed::turn2 : speed::turn3;
  default: throw std::out_of_range("unknown speed enum constant in monsterIntrinsics::adjust");
  }
}

class monsterIntrinsicsImpl {
private:
  std::bitset<9> damageProof_; // TODO: Magic constant is number of damageType enum values
public:
  int turnsToEscape_;
  mutable std::map<bonusType, bonus> bonuses_; // mutable allows [] to auto-fill default
  std::map<damageType *, char> resistLevel_;
  std::map<damageType *, char> extraDamageLevel_;
  std::map<terrainType, bool> terrainMove_;
  monsterIntrinsicsImpl() :
    damageProof_(), turnsToEscape_(0), bonuses_(), resistLevel_(), extraDamageLevel_(), terrainMove_() {
    // all creatures move on ground by default:
    terrainMove_[terrainType::ROCK] = false;
    terrainMove_[terrainType::GROUND] = true;
    terrainMove_[terrainType::UP] = true;
    terrainMove_[terrainType::DOWN] = true;
  }
  bool proof(const damageType & t) const {
    auto typ = static_cast<typename std::underlying_type<damageType>::type>(t);
    return damageProof_.test(typ);
  }
  void proof(const damageType & t, const bool isProof) {
    auto typ = static_cast<typename std::underlying_type<damageType>::type>(t);
    damageProof_.set(typ, isProof);
  }
};

monsterAbilities::~monsterAbilities() {}


monsterIntrinsics::monsterIntrinsics() :
  pImpl_(new monsterIntrinsicsImpl()) {};

monsterIntrinsics::monsterIntrinsics(const monsterIntrinsics &other) :
  pImpl_(new monsterIntrinsicsImpl(*(other.pImpl_))) {};

monsterIntrinsics::~monsterIntrinsics() {}

  // monsters may be inherantly proof (bonus) against a damage type:
void monsterIntrinsics::proof(const damage & type, const bool isProof) {
  pImpl_->proof(type.type(), isProof);
}
const bool monsterIntrinsics:: proof(const damage & type) const {
  return pImpl_->proof(type.type());
}

// and there may be some simple flags too:
void monsterIntrinsics::eatVeggie(const bonus & isBonus) {
  pImpl_->bonuses_[bonusType::eatVeggie] = isBonus;
}
const bonus monsterIntrinsics::eatVeggie() const {
  return pImpl_->bonuses_[bonusType::eatVeggie];
}

// bonus - 2 x attacks / round; penalty - 2 x rounds / attack
void monsterIntrinsics::dblAttack(const bonus & isDblAttack) {
  pImpl_->bonuses_[bonusType::dblAttack] = isDblAttack;
}
const bonus monsterIntrinsics:: dblAttack() const {
  return pImpl_->bonuses_[bonusType::dblAttack];
}

// fearless - ignores fear; penalty - 2 x rounds affected by fear
void monsterIntrinsics::fearless(const bonus & isFearless) {
  pImpl_->bonuses_[bonusType::fearless] = isFearless;
}
const bonus monsterIntrinsics:: fearless() const {
  return pImpl_->bonuses_[bonusType::fearless];
}

// resist damage of a given type (nullptr for all) - by 5% increments
// (may be negative for extra damage)
void monsterIntrinsics::resist(const damage * type, char level) {
  if (type == nullptr) {
    pImpl_->resistLevel_[nullptr] = level;
  } else {
    damageType t = type->type();
    pImpl_->resistLevel_[&t] = level;
  }
}
// total intrinsic damage resistance of a given type in 5% increments
// (may be negative for extra damage)
const char monsterIntrinsics::resist(const damage & type) const {
  damageType t = type.type();
  char rtn = pImpl_->resistLevel_[&t];
  rtn += pImpl_->resistLevel_[nullptr];
  return rtn;
}

// deal extra damage of a given type (nullptr for all) - by 5% increments
// (may be negative for healing attack)
void monsterIntrinsics::extraDamage(const damage * type, char level) {
  if (type == nullptr) {
    pImpl_->extraDamageLevel_[nullptr] = level;
  } else {
    damageType t = type->type();
    pImpl_->extraDamageLevel_[&t] = level;
  }
}
// total intrinsic extra damage of a given type in 5% increments
// (may be negative for healing attack)
const char monsterIntrinsics::extraDamage(const damage & type) const {
  damageType t = type.type();
  char rtn = pImpl_->extraDamageLevel_[&t];
  rtn += pImpl_->extraDamageLevel_[nullptr];
  return rtn;
}

// can you move through a given terrain?
void monsterIntrinsics::move(const terrain & type, const bool isMove) {
  pImpl_->terrainMove_[type.type()] = isMove;
}
const bool monsterIntrinsics:: move(const terrain & type) const {
  return pImpl_->terrainMove_[type.type()];
}

// can you hear monsters?
void monsterIntrinsics::hear(const bool hearing) {
  pImpl_->bonuses_[bonusType::hearing] = bonus(hearing);
}
const bool monsterIntrinsics:: hear() const {
  return pImpl_->bonuses_[bonusType::hearing] == bonus(true);
}
// can you see monsters?
void monsterIntrinsics::see(const bool sight) {
  pImpl_->bonuses_[bonusType::seeing] = bonus(sight);
}
const bool monsterIntrinsics:: see() const {
  return pImpl_->bonuses_[bonusType::seeing] == bonus(true);
}
// moving in water
void monsterIntrinsics::swim(const bool swim) {
  pImpl_->bonuses_[bonusType::swimming] = bonus(swim);
}
const bool monsterIntrinsics:: swim() const {
  return pImpl_->bonuses_[bonusType::swimming] == bonus(true);
}
// can you fly?
void monsterIntrinsics::fly(const bool fly) {
  pImpl_->bonuses_[bonusType::flying] = bonus(fly);
}
const bool monsterIntrinsics:: fly() const {
  return pImpl_->bonuses_[bonusType::flying] == bonus(true);
}
// are you trapped right now?
void monsterIntrinsics::entrap(const int turnsToEscape) {
  pImpl_->turnsToEscape_ += turnsToEscape;
}
const bool monsterIntrinsics::entrapped() const {
  return pImpl_->turnsToEscape_ > 0;
}
// how fast can we get out of a pit?
void monsterIntrinsics::climb(const bonus & sight) {
  pImpl_->bonuses_[bonusType::climbing] = bonus(sight);
}
const bonus monsterIntrinsics:: climb() const {
  return pImpl_->bonuses_[bonusType::climbing];
}
// does *this* monster have a speed bonus/penalty
void monsterIntrinsics::speedy(const bonus & isDblAttack) {
  pImpl_->bonuses_[bonusType::speedy] = isDblAttack;
}
const bonus monsterIntrinsics:: speedy() const {
  return pImpl_->bonuses_[bonusType::speedy];
}
// can you throw things at monsters?
void monsterIntrinsics::throws(const bool throws) {
  pImpl_->bonuses_[bonusType::throws] = bonus(throws);
}
const bool monsterIntrinsics::throws() const {
  return pImpl_->bonuses_[bonusType::throws] == bonus(true);
}
// can you zap wands at monsters?
void monsterIntrinsics::zap(const bool zap) {
  pImpl_->bonuses_[bonusType::zap] = bonus(zap);
}
const bool monsterIntrinsics:: zap() const {
  return pImpl_->bonuses_[bonusType::zap] == bonus(true);
}

// adjust the given enum based on the speed bonus/penalty
speed monsterIntrinsics::adjust(const speed & fastness) {
  const bonus & s = speedy();
  return adjustSpeed(s, fastness);
}



monsterAbilityMods::monsterAbilityMods(itemHolder &mon, monsterIntrinsics &intrinsics) :
  mon_(mon),
  intrinsics_(intrinsics), mod_(new monsterIntrinsicsImpl()) {};

  // monsters may be inherantly proof (bonus) against a damage type:
void monsterAbilityMods::proof(const damage & type, const bool isProof) {
  mod_->proof(type.type(), isProof);
}
const bool monsterAbilityMods::proof(const damage & type) const {
  return intrinsics_.proof(type) || mod_->proof(type.type());
}
  // and there may be some simple flags too:
void monsterAbilityMods::eatVeggie(const bonus & isBonus) {
  mod_->bonuses_[bonusType::eatVeggie] = isBonus;
}
const bonus monsterAbilityMods::eatVeggie() const {
  return intrinsics_.eatVeggie() + mod_->bonuses_[bonusType::eatVeggie];
}
// bonus - 2 x attacks / round; penalty - 2 x rounds / attack
void monsterAbilityMods::dblAttack(const bonus & isDblAttack) {
  mod_->bonuses_[bonusType::dblAttack] = isDblAttack;
}
const bonus monsterAbilityMods::dblAttack() const {
  return intrinsics_.dblAttack() + mod_->bonuses_[bonusType::dblAttack];
}
  // resist damage of a given type (nullptr for all) - by 5% increments
  // (may be negative for extra damage)
void monsterAbilityMods::resist(const damage * type, char level) {
  if (type == nullptr) mod_->resistLevel_[nullptr] = level;
  else {
    damageType t = type->type();
    mod_->resistLevel_[&t] = level;
  }
}
  // total intrinsic damage resistance of a given type in 5% increments
  // (may be negative for extra damage)
const char monsterAbilityMods::resist(const damage & type) const {
  damageType t = type.type();
  return std::max(mod_->resistLevel_[&t], intrinsics_.resist(type));
}
  // extra damage of a given type (nullptr for all) - by 5% increments
  // (may be negative to deal healing)
void monsterAbilityMods::extraDamage(const damage * type, char level) {
  if (type == nullptr) mod_->extraDamageLevel_[nullptr] = level;
  else {
    damageType t = type->type();
    mod_->extraDamageLevel_[&t] = level;
  }
}
// total intrinsic damage resistance of a given type in 5% increments
// (may be negative to deal healing)
const char monsterAbilityMods::extraDamage(const damage & type) const {
  damageType t = type.type();
  return std::max(mod_->extraDamageLevel_[&t], intrinsics_.extraDamage(type));
}
  // can you move through a given terrain?
void monsterAbilityMods::move(const terrain & type, const bool isMove) {
  mod_->terrainMove_[type.type()] = isMove;
}
const bool monsterAbilityMods::move(const terrain & type) const {
  return intrinsics_.move(type) || mod_->terrainMove_[type.type()];
}
  // can you hear monsters?
void monsterAbilityMods::hear(const bool hearing) {
  mod_->bonuses_[bonusType::hearing] = hearing;
}
const bool monsterAbilityMods::hear() const {
  return intrinsics_.hear() || mod_->bonuses_[bonusType::hearing] == bonus(true);
}
  // can you see monsters?
void monsterAbilityMods::see(const bool sight) {
  mod_->bonuses_[bonusType::seeing] = sight;
}
const bool monsterAbilityMods::see() const {
  return intrinsics_.see() || mod_->bonuses_[bonusType::seeing] == bonus(true);
}
  // can you move through water?
void monsterAbilityMods::swim(const bool canSwim) {
  mod_->bonuses_[bonusType::swimming] = canSwim;
}
const bool monsterAbilityMods::swim() const {
  return intrinsics_.swim() || mod_->bonuses_[bonusType::swimming] == bonus(true);
}
  // can you fly?
void monsterAbilityMods::fly(const bool canFly) {
  mod_->bonuses_[bonusType::flying] = canFly;
}
const bool monsterAbilityMods::fly() const {
  return intrinsics_.fly() || mod_->bonuses_[bonusType::flying] == bonus(true);
}
  // affected by petrify/fear actions? (false = double effect)
void monsterAbilityMods::fearless(const bonus &fearless) {
  mod_->bonuses_[bonusType::fearless] = fearless;
}
const bonus monsterAbilityMods::fearless() const {
  return intrinsics_.fearless() + mod_->bonuses_[bonusType::fearless];
}
// entrapment ignores modifiers
void monsterAbilityMods::entrap(const int ticksToEscape) {
  intrinsics_.entrap(ticksToEscape);
}
const bool monsterAbilityMods::entrapped() const {
  return intrinsics_.entrapped(); // ignores modifiers
}
  // good at climbing?
void monsterAbilityMods::climb(const bonus & canClimb) {
  mod_->bonuses_[bonusType::climbing] = canClimb;
}
const bonus monsterAbilityMods::climb() const {
  return intrinsics_.climb() + mod_->bonuses_[bonusType::climbing];
}
  // does *this* monster have a speed bonus/penalty
void monsterAbilityMods::speedy(const bonus & fast) {
  mod_->bonuses_[bonusType::speedy] = fast;
}
const bonus monsterAbilityMods::speedy() const {
  return intrinsics_.speedy() + mod_->bonuses_[bonusType::speedy];
}
// adjust the given enum based on the speed bonus/penalty
// NB: For each 3000N (~1/3tonne, in Earth gravity) the monster carries,
// rate is reduced by 1 slot. This means a human warrier can carry about a tonne.
speed monsterAbilityMods::adjust(const speed & fastness) {
  const monster* mon = dynamic_cast<const monster*>(&mon_);
  auto w = mon ? mon->type().carryWeightN() : 3000;
  const bonus & s = speedy();
  auto rtn = adjustSpeed(s, fastness);
  double totalWeight=mon_.totalWeight();
  
  while (totalWeight > w && rtn != speed::stop) {
    rtn = static_cast<speed>(static_cast<int>(rtn)-1);
    totalWeight -= w;
  }
  return rtn;
}

// can you throw things at monsters?
void monsterAbilityMods::throws(const bool throws) {
  mod_->bonuses_[bonusType::throws] = bonus(throws);
}
const bool monsterAbilityMods:: throws() const {
  return intrinsics_.throws() || mod_->bonuses_[bonusType::throws] == bonus(true);
}
// can you zap wands at monsters?
void monsterAbilityMods::zap(const bool zap) {
  mod_->bonuses_[bonusType::zap] = bonus(zap);
}
const bool monsterAbilityMods:: zap() const {
  return intrinsics_.zap() || mod_->bonuses_[bonusType::zap] == bonus(true);
}

