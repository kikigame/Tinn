/* License and copyright go here*/

#include "items.hpp"
#include "equippable.hpp"
#include "random.hpp"

extern std::vector<damageType> allDamageTypes;

// implementation of item and basic item methods.
// built up of all visible properties.
std::wstring item::description() const {
  std::wstring buffer = name();
  const std::size_t len = buffer.length();
  buffer += L"\n";
  buffer += std::wstring(len, L'=');
  buffer += typeDescription();
  return buffer;
}

itemHolder& item::holder() const {
  auto &map = itemHolderMap::instance();
  // sanity check to avoid null reference:
  if (map.beforeFirstAdd(*this))
    throw name() + L" not yet in any holder";
  return map.forItem(*this);
}

bool item::equip(monster &) {
  return false; // can't equip this item type by default
}
void item::onUnequip(monster &) {
  // do nothing by default
}
item::equipType item::equippable() const { 
  return equipType::none; 
}
// destroy an item in inventory
bool item::destroy() {
  if (holder().contains(*this) )
    return holder().destroyItem(*this);
  else
    throw L"Item not in supplied container"; // should not fall through if procondition met
}

// try to use the object
item::useResult item::use () {
  return item::useResult::FAIL; // no effect by default
}

// basicItem::description() defined in items.cpp.

basicItem::basicItem(const itemType& type) :
  item(),
  flags_(),
  enchantment_(0),
  type_(type) {
  const damageRepo &dr = damageRepo::instance();
  for (auto dt : allDamageTypes)
    if (dr[dt].canDamage(type_.material()))
      damageTrack_.emplace(dt, 0);
  }
basicItem::~basicItem() {}
// delegate to itemType by default
const wchar_t basicItem::render() const {
  return type_.render();
}
// return the simple name for this item type; overridden in corpse/necklace etc.
std::wstring basicItem::simpleName() const {
  return type_.name();
}

// built up of itemType and adjectives etc.
std::wstring basicItem::name() const {
  buffer_ = L"";
  if (enchantment_ < 0) buffer_ += std::to_wstring(enchantment_) + L' ';
  if (enchantment_ > 0) buffer_ += L'+' + std::to_wstring(enchantment_) + L' ';
  for (auto a : adjectives())
      buffer_ += a + L" ";
  buffer_ += simpleName();
  return buffer_;
}



// what is the object made of?
materialType basicItem::material() const {
  return type_.material();
}
// hom much does it weigh?
double basicItem::weight() const {
  // basic: just return the base weight
  return type_.baseWeight();
}
damageType basicItem::weaponDamage(bool) {
  return damageType::bashing;
}
int basicItem::damageOfType(const damageType &type) const {
  auto i = damageTrack_.find(type);
  if (i == damageTrack_.end()) return 0;
  return i->second;
}
// list of all adjectives applicable to type
std::vector<std::wstring> basicItem::adjectives() const {
  std::vector<std::wstring> rtn;
  if (isUnidentified())
    return rtn;
  if (type_ == itemTypeRepo::instance()[itemTypeKey::spring_water])
    rtn.push_back(L"natural");
  if (isBlessed()) rtn.push_back(L"blessed");
  if (isCursed()) rtn.push_back(L"cursed");
  if (isSexy()) rtn.push_back(L"sexy");
  auto &dr = damageRepo::instance();
  auto m = type_.material();
  for (auto dt : allDamageTypes) {
    if (isProof(dt)) {
      // adjective for being (this material) being proof to this damage type:
      auto ptr = dr[dt].proofAdj(m);
      if (ptr != nullptr)
	rtn.push_back(std::wstring(ptr));
      continue;
    }
    int d = damageOfType(dt);
    if (d == 0) continue;
    auto adj = dr[dt].damageAdj(m);
    if (d == 1) rtn.push_back(std::wstring(L"barely ") + adj);
    if (d == 2) rtn.push_back(std::wstring(adj));
    if (d == 3) rtn.push_back(std::wstring(L"very ") + adj);
    if (d >= 4) rtn.push_back(std::wstring(L"thoroughly ") + adj);
  }
  return rtn;
}

// damage the item in some way (return false only if no effect)
bool basicItem::strike(const damageType &type) {
  if (isProof(type)) return false;
  auto i = damageTrack_.find(type);
  // many items should be destroyed if they get too damaged. Do it here or transmute won't work.
  if (i == damageTrack_.end()) {
    if (highlight()) return false; // highlighted items can't be destroyed this way; they're need for quests.
    int e = enchantment();
    if (dPc() >=  50 + isBlessed() && enchantment() > 0 ? 10 * e : e)
      destroy();
    return false;
  }
  ++(i->second);
  return true;
}

// repair previous damage (return false only if no effect, eg undamaged)
bool basicItem::repair(const damageType &type) {
  // don't check proof in case a subclass provides for another way of damaging
  auto i = damageTrack_.find(type);
  if (i == damageTrack_.end()) return false;
  if (i->second == 0) return false;
  --(i->second);
  return true;
}
// proof against dagage type  (return false only if no more effect possible, eg already proofed or n/a for material type)
bool basicItem::proof(const damageType &type) {
  auto i = proof_.find(type);
  if (i == proof_.end())
    proof_.insert(type);
  return true;
}
// are we fooproof?
bool basicItem::isProof(const damageType &type) const {
  auto i = proof_.find(type);
  return i != proof_.end();
}

// access flags:
bool basicItem::isBlessed() const {
  return flags_[blessed];
}
void basicItem::bless(bool b) {
  flags_[blessed] = b;
}
bool basicItem::isCursed() const {
  return flags_[cursed];
}
void basicItem::curse(bool c) {
  flags_[cursed] = c;
}
bool basicItem::isSexy() const {
  return flags_[sexy];
}
void basicItem::sexUp(bool s) {
  flags_[sexy] = s;
}
bool basicItem::isUnidentified() const {
  return flags_[unidentified];
}
void basicItem::unidentify(bool forget) {
  flags_[unidentified] = forget;
}

// enchantment is some +/- modifier for the item; adds to attack when wielded
// If positive, may also be the number of charges remaining in a limited-use item
int basicItem::enchantment() const {
  return enchantment_;
}
void basicItem::enchant(int enchantment) {
  ::equippable *owner = nullptr;
  if (!itemHolderMap::instance().beforeFirstAdd(*this))
    owner = dynamic_cast<::equippable*>(&holder());
  std::array<const slot *,2> slots;
  if (owner && !owner->slotsOf(*this).empty()) {
    // temporarily unequip the item; this will force stats to recalculate
    slots = owner->forceUnequip(*this);
  }
  enchantment_ += enchantment;
  if (owner && slots[0]) {
    // signal to recalculate the bonus if this item is equipped
    owner->equip(*this, slots);
  }
}


std::set<slotType> basicItem::slots() {
  std::set<slotType> empty;
  return empty;
}

long basicItem::modDamage(long pc, const damage & type) const {
  return pc;
}

