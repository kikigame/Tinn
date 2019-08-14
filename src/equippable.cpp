/* License and copyright go here*/

// define equippable items

#include "equippable.hpp"
#include "monsterType.hpp"
#include "items.hpp"
#include "random.hpp"

// used to find outermost slots for armour/weapons:
std::vector<const slot*> coverSearch::deeper(const slot* const & sl) {
  return sl->covered();
};


equippable::equippable(std::vector<const slot *>slots) :
  equipment_() {
  // create all slots as empty initially
  for (auto slot : slots)
    equipment_.emplace(slot, optionalRef<item>());
}

equippable::~equippable() {}


bool equippable::equip(item &item, const std::pair<slotType, slotType> &slots) {
  std::array<const slot *, 2> sl{ slotBy(slots.first), slotBy(slots.second) };
  return equip(item, sl);
}

bool equippable::equip(item &item, const std::array<const slot *,2> &slots) {
  if (slots[0] == slots[1]) return equip(item, slots[0]);

  for (auto s : slots) {
    if (s == nullptr) continue;
    auto i = equipment_.find(s);
    if (i == equipment_.end()) return false; // monster doesn't have this slot
    if (i->second) return false; // already occupied
  }

  onEquip(item, slots[0], slots[1]);
  return true;
}

bool equippable::equip(item &item, const slotType slot) {
  auto s = slotBy(slot);
  return equip(item, s);
}

bool equippable::equip(item &item, const slot *s) {
  auto i = equipment_.find(s);
  if (i == equipment_.end()) return false; // monster doesn't have this slot
  if (i->second) return false; // already occupied
  onEquip(item, s, s);
  return true;
}

void equippable::onEquip(item &item, const slot *s1, const slot *s2) {
  // equip the item
  equipment_[s1] = item;
  equipment_[s2] = item;
}

bool equippable::unequip(item &item) {
  bool rtn = false;
  auto eend = equipment_.end();
  for (auto e = equipment_.begin(); e != eend; ++e)
    if (e->second && &(e->second.value()) == &item) {
      // we can equip the item. 
      // unequip the item
      optionalRef<::item> nullItem;
      e->second = nullItem;
      rtn = true; // don't break! some items have multiple slots
    }
  return rtn;
}

std::array<const slot *, 2> equippable::forceUnequip(item &it) {
  auto arr = slotsOf(it);
  optionalRef<::item> nullItem;
  for (auto slot : arr)
    if (slot) equipment_[slot] = nullItem;
  return arr;
}

// monster.isEquipped defined in items.cpp

bool equippable::slotAvail(const slot *s) const {
  return equipment_.find(s) != equipment_.end();
}
optionalRef<item> equippable::inSlot(slotType slot) {
  auto it = equipment_.find(slotBy(slot));
  if (it == equipment_.end()) return optionalRef<item>();
  return optionalRef<item>(it->second);
}
const std::array<const slot *,2> equippable::slotsOf(const item &item) const {
  std::array<const slot *,2> rtn = {nullptr, nullptr };
  bool foundOne = false;
  for (auto i : equipment_)
    if (i.second && &(i.second.value()) == &item) {
      rtn[foundOne ? 1 : 0] = i.first;
      foundOne = true;
    }
  if (rtn[1] == nullptr) rtn[1] = rtn[0];
  return rtn;
}

// TODO: should this not be finding the first wielded item?
optionalRef<item> equippable::findWeapon() const {
  auto weap = equipment_.find(slotBy(slotType::primary_weapon));
  if (weap == equipment_.end()) weap = equipment_.find(slotBy(slotType::secondary_weapon));
  optionalRef<item> weapon;
  if (weap != equipment_.end()) weapon = weap->second;
  return weapon;
}

optionalRef<item> equippable::findArmour() const {
  coverSearch cs;
  std::vector<item *> outerArmour;
  for (auto i : equipment_) {
    if (i.second && 
	i.second.value().equippable() == item::equipType::worn &&
	!cs.isCovered(equipment_, i.first)) {// is it covered?
      auto item = i.second;
      outerArmour.emplace_back(&item.value());
    }
  }
  optionalRef<item> rtn;
  if (!outerArmour.empty()) {
    auto it = *rndPick(outerArmour.begin(), outerArmour.end());
    rtn = optionalRef<item>(*it);
  }
  return rtn;
}

void equippable::forEachWeapon(std::function<void(const item &)> per) const {
  for (auto i : equipment_) {
    if (i.second &&
	i.second.value().equippable() == item::equipType::wielded)
      per(i.second.value());
  }
}

long equippable::modDamage(long pc, const damage &type) const {
  for (auto i : equipment_) {
    auto &item = i.second;
    if (item) pc = item.value().modDamage(pc, type);
  }
  return pc;
}


void equippable::polymorphCategory(monsterCategory c) {
  const auto &slots = slotsFor(c);
  std::vector<const slot*> toRemove;
  for (auto p : equipment_)
    if (std::find(slots.begin(), slots.end(), p.first) == slots.end())
      toRemove.push_back(p.first);
  for (auto s : slots)
    if (equipment_.count(s) == 0)
      equipment_.emplace(s, optionalRef<item>()); // new empty slot
  for (auto s : toRemove) {
    if (equipment_[s])
      // remove item from the slot if possible. If item is cursed, the slot stays.
      // (yes, you can get more equipment slots by polymorping with cursed items; they've got to be
      // good for something).
      unequip(equipment_[s].value()); // can fail (eg cursed)
    if (!equipment_[s])
      equipment_.erase(s);
  }
}

// calculate the current strength bonus from equipment
int equippable::strBonus() const {
  int rtn = 0;
  for (auto i : equipment_) {
    auto it = i.second;
    if (it && it.value().equippable() == item::equipType::wielded) {
      // +1 so that even an unenchanted weapon does some damage:
      rtn += it.value().enchantment() + 1;
    } else if (it && i.first == slotBy(slotType::gauntlets)) {
        coverSearch cs;
	if (!cs.isCovered(equipment_, i.first)) {
	  if (it.value().material() == materialType::metallic || it.value().material() == materialType::stony)
	    rtn += it.value().enchantment() + 1;
	  else
	    rtn += it.value().enchantment() / 2 + 1;
	}
    } else if (it && i.first == slotBy(slotType::secondary_weapon)) {
      // secondary wearables == shields. aside from Roman tower shields, shields are basically active punching weapons.
      rtn += it.value().enchantment() / 2 + 1;
    }
  }
  return rtn;
}

// calculate the current strength bonus from equipment
int equippable::dodBonus() const {
  int rtn = 0;
  for (auto i : equipment_)
    if (i.second && i.second.value().equippable() == item::equipType::worn) {
      // considering occupied worn slots
      auto &item = i.second.value();
      double defence;
      // base armour = (weight) * (material armour multiplicand) / 10:
      switch (item.material()) {
      case materialType::glassy: defence = (1/3.); break; // basically untoughened leather
      case materialType::woody: defence = 0.5; break; // basically untoughened leather
      case materialType::fleshy: defence = (2/3.); break; // basically untoughened leather
      case materialType::leathery: defence = 1; break; // standard armour
      case materialType::stony: defence = 1.5; break; // very strong if very heavy...
      case materialType::metallic: defence = 3; break; // best armour is metal, but also the heaviest
      default: defence = 0;
      }
      defence *= item.weight() / 10;
      defence += item.enchantment(); // each bonus adds +1 (=> +5%). Note that you can wear a *lot*, so enchantment adds up quite fast.
      if (item.isBlessed()) defence *= 1.5;
      if (item.isCursed()) defence *= 0.5;
      rtn += static_cast<int>(defence);
    }
  return rtn;
}

// calculate the current appearance bonus from equipment
int equippable::appBonus() const {
  int rtn = 0;
  coverSearch cs;
  for (auto i : equipment_)
    // considering non-weapon slots
    // non-weapon item in slot i.first is i.second
    // we don't consider enchantments of armour in slots that are covered
    if (i.second // is it occupied?
	&& i.second.value().equippable() == item::equipType::worn // by worn item
	&& !cs.isCovered(equipment_, i.first)) {// is it covered?
      auto &item = i.second;
      rtn += item.value().enchantment();
      if (item.value().isSexy()) ++rtn;
      if (item.value().isSexy() && item.value().isBlessed()) ++rtn;
      // NB: tshirts get sexy bonuses for being torn or wet (double if blessed).
      // the tshirt bonus is easy to get, but not that useful as it will generally be covered.
    }
  return rtn;
};
