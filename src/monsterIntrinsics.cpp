/* License and copyright go here*/

// properties that a monster can have

#include "monsterIntrinsics.hpp"
#include "damage.hpp"
#include "terrain.hpp"
#include <map>
#include <bitset>

enum class bonusType {
  eatVeggie, // TODO
    dblAttack, // TODO
    speedy, // TODO: for players
    hearing, // TODO
    seeing, // TODO
    swimming, // TODO (need water too)
    climbing // TODO (need pits)
    };

class monsterIntrinsicsImpl {
private:
  std::bitset<9> damageProof; // TODO: Magic constant is number of damageType enum values
public:
  mutable std::map<bonusType, bonus> bonuses; // mutable allows [] to auto-fill default
  std::map<damageType *, char> resistLevel;
  std::map<damageType *, char> extraDamageLevel;
  std::map<terrainType, bool> terrainMove;
  monsterIntrinsicsImpl() {
    // all creatures move on ground by default:
    terrainMove[terrainType::ROCK] = false;
    terrainMove[terrainType::GROUND] = true;
    terrainMove[terrainType::UP] = true;
    terrainMove[terrainType::DOWN] = true;
  }
  bool proof(const damageType & t) const {
    auto typ = static_cast<typename std::underlying_type<damageType>::type>(t);
    return damageProof.test(typ);
  }
  void proof(const damageType & t, const bool isProof) {
    auto typ = static_cast<typename std::underlying_type<damageType>::type>(t);
    damageProof.set(typ, isProof);
  }
};

monsterIntrinsics::monsterIntrinsics() :
  pImpl_(new monsterIntrinsicsImpl()) {};

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
  pImpl_->bonuses[bonusType::eatVeggie] = isBonus;
}
const bonus & monsterIntrinsics::eatVeggie() const {
  return pImpl_->bonuses[bonusType::eatVeggie];
}

// bonus - 2 x attacks / round; penalty - 2 x rounds / attack
void monsterIntrinsics::dblAttack(const bonus & isDblAttack) {
  pImpl_->bonuses[bonusType::dblAttack] = isDblAttack;
}
const bonus & monsterIntrinsics:: dblAttack() const {
  return pImpl_->bonuses[bonusType::dblAttack];
}

// resist damage of a given type (nullptr for all) - by 5% increments
// (may be negative for extra damage)
void monsterIntrinsics::resist(const damage * type, char level) {
  if (type == nullptr) {
    pImpl_->resistLevel[nullptr] = level;
  } else {
    damageType t = type->type();
    pImpl_->resistLevel[&t] = level;
  }
}
// total intrinsic damage resistance of a given type in 5% increments
// (may be negative for extra damage)
const char monsterIntrinsics::resist(const damage & type) const {
  damageType t = type.type();
  char rtn = pImpl_->resistLevel[&t];
  rtn += pImpl_->resistLevel[nullptr];
  return rtn;
}

// deal extra damage of a given type (nullptr for all) - by 5% increments
// (may be negative for healing attack)
void monsterIntrinsics::extraDamage(const damage * type, char level) {
  if (type == nullptr) {
    pImpl_->extraDamageLevel[nullptr] = level;
  } else {
    damageType t = type->type();
    pImpl_->extraDamageLevel[&t] = level;
  }
}
// total intrinsic extra damage of a given type in 5% increments
// (may be negative for healing attack)
const char monsterIntrinsics::extraDamage(const damage & type) const {
  damageType t = type.type();
  char rtn = pImpl_->extraDamageLevel[&t];
  rtn += pImpl_->extraDamageLevel[nullptr];
  return rtn;
}

// can you move through a given terrain?
void monsterIntrinsics::move(const terrain & type, const bool isMove) {
  pImpl_->terrainMove[type.type()] = isMove;
}
const bool monsterIntrinsics:: move(const terrain & type) const {
  return pImpl_->terrainMove[type.type()];
}

// can you hear monsters?
void monsterIntrinsics::hear(const bool hearing) {
  pImpl_->bonuses[bonusType::hearing] = bonus(hearing);
}
const bool monsterIntrinsics:: hear() const {
  return pImpl_->bonuses[bonusType::hearing] == bonus(true);
}
// can you see monsters?
void monsterIntrinsics::see(const bool sight) {
  pImpl_->bonuses[bonusType::seeing] = bonus(sight);
}
const bool monsterIntrinsics:: see() const {
  return pImpl_->bonuses[bonusType::seeing] == bonus(true);
}
// moving in water
void monsterIntrinsics::swim(const bool sight) {
  pImpl_->bonuses[bonusType::swimming] = bonus(sight);
}
const bool monsterIntrinsics:: swim() const {
  return pImpl_->bonuses[bonusType::swimming] == bonus(true);
}
// how fast can we get out of a pit?
void monsterIntrinsics::climb(const bonus & sight) {
  pImpl_->bonuses[bonusType::climbing] = bonus(sight);
}
const bonus & monsterIntrinsics:: climb() const {
  return pImpl_->bonuses[bonusType::climbing];
}
// does *this* monster have a speed bonus/penalty
void monsterIntrinsics::speedy(const bonus & isDblAttack) {
  pImpl_->bonuses[bonusType::speedy] = isDblAttack;
}
const bonus & monsterIntrinsics:: speedy() const {
  return pImpl_->bonuses[bonusType::speedy];
}
// adjust the given enum based on the speed bonus/penalty
speed monsterIntrinsics::adjust(const speed & fastness) {
  const bonus & s = speedy();
  switch (fastness) {
  case speed::slow3:   return s == bonus(true) ? speed::slow2 : speed::slow3;
  case speed::slow2:   return s == bonus(true) ? speed::perturn : s == bonus(false) ? speed::slow3 : speed::slow2;
  case speed::perturn: return s == bonus(true) ? speed::turn2 : s == bonus(false) ? speed::slow2 : speed::perturn;
  case speed::turn2:   return s == bonus(true) ? speed::turn3 : s == bonus(false) ? speed::perturn : speed::turn2;
  case speed::turn3:   return s == bonus(false) ? speed::turn2 : speed::turn3;
  default: throw std::out_of_range("unknown speed enum constant in monsterIntrinsics::adjust");
  }
}

