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
  eatVeggie, // double benefit from eating veggies
    dblAttack, // TODO
    speedy, // 1 slot on speed
    hearing, // hear messages even if deafened
    seeing, // see things even if blinded
    touch, // can feel things if not wearing gloves (for future use)
    taste, // taste messages (for future use)
    smell, // warn of foul odours; affected by gasses (for future use)
    sixth, // inherently receive divine/divination messages (for future use)
    tele, // telepathic; sense thoughts/brains of unseen monsters  (for future use)
    mag, // magnetic: sense magnets & attacks with magnetic weapons (for future use)
    flying,
    climbing, // fast at escaping pits
    fearless, // affected by petrify (or other fear) actions
    throws, //can the monster lob things at another
    zap, // can the monster zap another with a wand
    sleeps, // affected by wand of slumber
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
  std::bitset<static_cast<const int>(damageType::END)> damageProof_;
public:
  int turnsToEscape_;
  mutable std::map<bonusType, bonus> bonuses_; // mutable allows [] to auto-fill default
  std::map<damageType *, char> resistLevel_;
  std::map<damageType *, char> extraDamageLevel_;
  std::map<terrainType, bool> terrainMove_;
  int carryWeightN_;
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

const bool monsterIntrinsics::hasSense(const sense::sense &t) const {
  static auto nope = bonus(false);
  if (t & sense::SIGHT) return pImpl_->bonuses_[bonusType::seeing] != nope;
  if (t & sense::SOUND) return pImpl_->bonuses_[bonusType::hearing] != nope;
  if (t & sense::TOUCH) return pImpl_->bonuses_[bonusType::touch] != nope;
  if (t & sense::TASTE) return pImpl_->bonuses_[bonusType::taste] != nope;
  if (t & sense::SMELL) return pImpl_->bonuses_[bonusType::smell] != nope;
  if (t & sense::SIXTH) return pImpl_->bonuses_[bonusType::sixth] != nope;
  if (t & sense::TELE) return pImpl_->bonuses_[bonusType::tele] != nope;
  if (t & sense::MAG) return pImpl_->bonuses_[bonusType::mag] != nope;
  return false;
}
void monsterIntrinsics::sense(const sense::sense &t, const bool value) {
  if (t & sense::SIGHT) pImpl_->bonuses_[bonusType::seeing] = bonus(value);
  if (t & sense::SOUND) pImpl_->bonuses_[bonusType::hearing] = bonus(value);
  if (t & sense::TOUCH) pImpl_->bonuses_[bonusType::touch] = bonus(value);
  if (t & sense::TASTE) pImpl_->bonuses_[bonusType::taste] = bonus(value);
  if (t & sense::SMELL) pImpl_->bonuses_[bonusType::smell] = bonus(value);
  if (t & sense::SIXTH) pImpl_->bonuses_[bonusType::sixth] = bonus(value);
  if (t & sense::TELE)  pImpl_->bonuses_[bonusType::tele]  = bonus(value);
  if (t & sense::MAG)   pImpl_->bonuses_[bonusType::mag]   = bonus(value);
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
// can you zap wands at monsters?
void monsterIntrinsics::sleeps(const bool snoozy) {
  pImpl_->bonuses_[bonusType::sleeps] = bonus(snoozy);
}
const bool monsterIntrinsics::sleeps() const {
  return pImpl_->bonuses_[bonusType::sleeps] == bonus(true);
}

// adjust the given enum based on the speed bonus/penalty
speed monsterIntrinsics::adjust(const speed & fastness) {
  const bonus & s = speedy();
  return adjustSpeed(s, fastness);
}
void monsterIntrinsics::carryWeightN(const int n) {
  pImpl_->carryWeightN_ = n;
}
const int monsterIntrinsics::carryWeightN() const {
  return pImpl_->carryWeightN_;
}


monsterAbilityMods::monsterAbilityMods(itemHolder &mon, std::shared_ptr<monsterIntrinsics> intrinsics) :
  mon_(mon),
  intrinsics_(intrinsics), mod_(new monsterIntrinsicsImpl()) {};

  // monsters may be inherantly proof (bonus) against a damage type:
void monsterAbilityMods::proof(const damage & type, const bool isProof) {
  mod_->proof(type.type(), isProof);
}
const bool monsterAbilityMods::proof(const damage & type) const {
  return intrinsics_->proof(type) || mod_->proof(type.type());
}
  // and there may be some simple flags too:
void monsterAbilityMods::eatVeggie(const bonus & isBonus) {
  mod_->bonuses_[bonusType::eatVeggie] = isBonus;
}
const bonus monsterAbilityMods::eatVeggie() const {
  return intrinsics_->eatVeggie() + mod_->bonuses_[bonusType::eatVeggie];
}
// bonus - 2 x attacks / round; penalty - 2 x rounds / attack
void monsterAbilityMods::dblAttack(const bonus & isDblAttack) {
  mod_->bonuses_[bonusType::dblAttack] = isDblAttack;
}
const bonus monsterAbilityMods::dblAttack() const {
  return intrinsics_->dblAttack() + mod_->bonuses_[bonusType::dblAttack];
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
  return std::max(mod_->resistLevel_[&t], intrinsics_->resist(type));
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
  return std::max(mod_->extraDamageLevel_[&t], intrinsics_->extraDamage(type));
}
  // can you move through a given terrain?
void monsterAbilityMods::move(const terrain & type, const bool isMove) {
  mod_->terrainMove_[type.type()] = isMove;
}
const bool monsterAbilityMods::move(const terrain & type) const {
  return intrinsics_->move(type) || mod_->terrainMove_[type.type()];
}
// acts on the least significant bit that is set to 1
const bool monsterAbilityMods::hasSense(const sense::sense &t) const {
  if (t & sense::SIGHT) return mod_->bonuses_[bonusType::seeing].apply(intrinsics_->hasSense(sense::SIGHT));
  if (t & sense::SOUND) return mod_->bonuses_[bonusType::hearing].apply(intrinsics_->hasSense(sense::SOUND));
  if (t & sense::TOUCH) {
    if (mod_->bonuses_[bonusType::touch] == bonus(true)) return true;
    if (mod_->bonuses_[bonusType::touch] == bonus(false)) return false;
    const monster* mon = dynamic_cast<const monster*>(&mon_);
    if (!mon) return false;
    // a monster can only touch if they have ring slots (ie fingers) and are not wearing gloves or gauntlets
    // NB: A creature that keeps its ring slots via polymorphing while wearing a cursed item must therefore be able to touch UNLESS also wearing gloves!
    auto ringSlot = slotBy(slotType::ring_right_index);
    return
      intrinsics_->hasSense(t) &&
      (mon->slotAvail(ringSlot) || mon->inSlot(ringSlot)) &&
      mon->slotAvail(slotBy(slotType::gloves)) &&
      mon->slotAvail(slotBy(slotType::gauntlets));
  }
  if (t & sense::TASTE) return mod_->bonuses_[bonusType::taste].apply(intrinsics_->hasSense(sense::TASTE));
  if (t & sense::SMELL) return mod_->bonuses_[bonusType::smell].apply(intrinsics_->hasSense(sense::SMELL));
  if (t & sense::SIXTH) return mod_->bonuses_[bonusType::sixth].apply(intrinsics_->hasSense(sense::SIXTH));
  if (t & sense::TELE) return mod_->bonuses_[bonusType::tele].apply(intrinsics_->hasSense(sense::TELE));
  if (t & sense::MAG) return mod_->bonuses_[bonusType::mag].apply(intrinsics_->hasSense(sense::MAG));
  return false;
}
void monsterAbilityMods::sense(const sense::sense &t, const bonus value) {
  if (t & sense::SIGHT) mod_->bonuses_[bonusType::seeing] = value;
  if (t & sense::SOUND) mod_->bonuses_[bonusType::hearing] = value;
  if (t & sense::TOUCH) mod_->bonuses_[bonusType::touch] = value;
  if (t & sense::TASTE) mod_->bonuses_[bonusType::taste] = value;
  if (t & sense::SMELL) mod_->bonuses_[bonusType::smell] = value;
  if (t & sense::SIXTH) mod_->bonuses_[bonusType::sixth] = value;
  if (t & sense::TELE) mod_->bonuses_[bonusType::tele] = value;
  if (t & sense::MAG) mod_->bonuses_[bonusType::mag] = value;
}
// can you fly?
void monsterAbilityMods::fly(const bool canFly) {
  mod_->bonuses_[bonusType::flying] = canFly;
}
const bool monsterAbilityMods::fly() const {
  return intrinsics_->fly() || mod_->bonuses_[bonusType::flying] == bonus(true);
}
  // affected by petrify/fear actions? (false = double effect)
void monsterAbilityMods::fearless(const bonus &fearless) {
  mod_->bonuses_[bonusType::fearless] = fearless;
}
const bonus monsterAbilityMods::fearless() const {
  return intrinsics_->fearless() + mod_->bonuses_[bonusType::fearless];
}
// entrapment ignores modifiers
void monsterAbilityMods::entrap(const int ticksToEscape) {
  intrinsics_->entrap(ticksToEscape);
}
const bool monsterAbilityMods::entrapped() const {
  return intrinsics_->entrapped(); // ignores modifiers
}
  // good at climbing?
void monsterAbilityMods::climb(const bonus & canClimb) {
  mod_->bonuses_[bonusType::climbing] = canClimb;
}
const bonus monsterAbilityMods::climb() const {
  return intrinsics_->climb() + mod_->bonuses_[bonusType::climbing];
}
  // does *this* monster have a speed bonus/penalty
void monsterAbilityMods::speedy(const bonus & fast) {
  mod_->bonuses_[bonusType::speedy] = fast;
}
const bonus monsterAbilityMods::speedy() const {
  return intrinsics_->speedy() + mod_->bonuses_[bonusType::speedy];
}
// adjust the given enum based on the speed bonus/penalty
// NB: For each 3000N (~1/3tonne, in Earth gravity) the monster carries,
// rate is reduced by 1 slot. This means a human warrier can carry about a tonne.
speed monsterAbilityMods::adjust(const speed & fastness) {
  const monster* mon = dynamic_cast<const monster*>(&mon_);
  auto w = mon ? mon->abilities()->carryWeightN() : 3000;
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
  return intrinsics_->throws() || mod_->bonuses_[bonusType::throws] == bonus(true);
}
// can you zap wands at monsters?
void monsterAbilityMods::zap(const bool zap) {
  mod_->bonuses_[bonusType::zap] = bonus(zap);
}
const bool monsterAbilityMods:: zap() const {
  return intrinsics_->zap() || mod_->bonuses_[bonusType::zap] == bonus(true);
}
void monsterAbilityMods::sleeps(const bool snoozy) {
  mod_->bonuses_[bonusType::sleeps] = bonus(snoozy);
}
const bool monsterAbilityMods::sleeps() const {
  return mod_->bonuses_[bonusType::sleeps] == bonus(true);
}
void monsterAbilityMods::carryWeightN(const int n) {
  intrinsics_->carryWeightN(n); // can't be bothered to make this transient. Will create flags if needed.
}
const int monsterAbilityMods::carryWeightN() const {
  return intrinsics_->carryWeightN();
}
