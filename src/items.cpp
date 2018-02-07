/* License and copyright go here*/

// Things and stuff

#include "action.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp" // for holy books
#include "terrain.hpp"
#include "encyclopedia.hpp" // for hitch-hiker's guide
#include "shop.hpp"
#include "dungeon.hpp"
#include "transport.hpp"
#include "target.hpp"

extern std::vector<damageType> allDamageTypes;

template <itemTypeKey it>
item & createItem();



// mixin class for things that apply  an action when equipped / unequipped
// everything where you can equip an item should extend this
class onEquipMixin : virtual public shared_item {
private:
  std::vector<sharedAction<item, monster> *> onEquip_;
public:
  void equipAction(sharedAction<item, monster> &act) {
    onEquip_.push_back(&act);
  }
  bool hasAction(sharedAction<item, monster> &act) const {
    return std::find(onEquip_.begin(), onEquip_.end(), &act) != onEquip_.end();
  }
protected:
  void onEquip(monster &m) {
    auto t = shared_from_this();
    for (auto i : onEquip_) (*i)(t->isBlessed(), t->isCursed(), *t, m);
  }
  void onUnequipImpl(monster &m) {
    auto t = shared_from_this();
    for (auto i : onEquip_)
      if (!m.isEquipped(*i))
	i->undo(t->isBlessed(), t->isCursed(), *t, m);
  }
  std::wstring ofName() const {
    auto onEq = renderedActions();
    if (onEq.empty()) return L"";
    std::wstring rtn = L" of ";
    auto eq = onEq.begin();
    while (true) {
      rtn += (*eq)->name();
      ++eq;
      if (eq != onEq.end()) rtn += L" and ";
      else return rtn;
    }
  }
  std::wstring ofPowers() const {
    auto onEq = renderedActions();
    if (onEq.empty()) return L"";
    std::wstring rtn = L"\nThis item has magical effects:\n";
    for (auto eq : onEq) 
      rtn += eq->name() + L":-\n " + eq->description() + L"\n\n";
    return rtn;
  }
  std::size_t countEquipActions() const {
    return onEquip_.size();
  }
private:
  std::vector<renderedAction<item, monster> *> renderedActions() const {
    std::vector<renderedAction<item, monster> *> onEq;
    for (auto i : onEquip_) {
      auto e = dynamic_cast<renderedAction<item, monster> *>(i);
      if (e) onEq.push_back(e);
    }
    return onEq;
  }
};

// defined here for access to item classes; simplifies the item interface a bit.
bool equippable::isEquipped(sharedAction<item, monster> &act) const {
  for (auto i : equipment_)
    if (i.second) {
      auto e = dynamic_cast<onEquipMixin*>(&(i.second.value()));
      if (e && e->hasAction(act)) return true;
    }
  return false;
}



// mixin class for things which burn charges; see basicItem
class burnChargeMixin : virtual public shared_item {
protected:
  bool hasCharge() const {
    return shared_from_this()->enchantment() > 0;
  }
  virtual void useCharge() {
    shared_from_this()->enchant(-1);
  }
public:
  virtual std::wstring describeCharges() const {
    const int enchantment = shared_from_this()->enchantment();
    const int enchantmentPc = 5 * enchantment;
    std::wstring buffer;
    if (enchantmentPc < 0) {
      buffer += L"There is a negative enchantment. You cannot use the magic\n"
	"in this item again until you have recharged it. There is a penalty\n"
	"of ";
      buffer += enchantmentPc;
      buffer += L"% when using this item as weapon or armour.\n";
    } else if (enchantmentPc == 0) {
      buffer += L"This needs recharging.\n";
    } else {
      buffer += L"There is an enchantment. You can use the magic in this item\n";
      buffer += enchantment + L" times before it will need recharging.\n"
	"There is a bonus of ";
      buffer += enchantmentPc;
      buffer += L"% when using this item as weapon or armour.\n";
    }
    return buffer;
  }
};

std::wstring basicItem::typeDescription() const {
  std::wstring buffer;
  if (isUnidentified()) {
    buffer += type_.vagueDescription();
    return buffer.c_str();
  }
  buffer += L"\n\nDescription:\n";
  buffer += type_.description();
  buffer += + L"\n\nWeight: ";
  buffer += std::to_wstring(weight());
  buffer += L"N\n";

  const bool blessed = isBlessed(), cursed = isCursed();
  if (blessed && !cursed)
    buffer += L"This is blessed. Most blessed items provide 1.5 times the\n"
      "effect of a similar non-blessed, non-cursed items.\n";
  else if (cursed && !blessed)
    buffer += L"This is cursed. Most cursed items provide half the effect\n"
      "of a similar non-blessed, non-cursed items. They cannot be removed\n"
      "if worn/wielded until the curse is removed.\n";
  else if (blessed && cursed)
    buffer += L"This is both blessed and cursed. The blessing removes some\n"
      "effects of the curse, but does not void it. Cursed items cannot be\n"
      "removed if worn/wielded until the curse is removed.\n";
  else
    buffer += L"This would be more effective if blessed.\n";

  const int enchantmentPc = 5 * enchantment();
  auto *burnCharge = dynamic_cast<const burnChargeMixin*>(this);
  if (!burnCharge) {
    if (enchantmentPc < 0) {
      buffer += L"There is a negative enchantment. There is a penalty of ";
      buffer += enchantmentPc;
      buffer += L"% when\nusing this item as weapon or armour.\n";
    } else if (enchantmentPc == 0) {
      buffer += L"There is no magical enchantment on this item.\n";
    } else {
      buffer += L"There is an enchantment. There is a bonus of ";
      buffer += enchantmentPc;
      buffer += L"% when\nUsing this item as weapon or armour.\n";
    }
  } else {
    buffer += burnCharge->describeCharges();
  }

  return buffer;
}



/*
 * Mixin for items that can be used with other items
 */
class useWithMixin : public virtual shared_item {
public:
  virtual bool use(const std::wstring &withName,
		   const std::wstring &action,
		   const std::wstring &preposition,
		   itemHolder &itemHolder) {
    std::function<void(item&, std::wstring)> f = 
      [this,&withName,&action,&preposition](item &i, std::wstring name){
      if (dynamic_cast<useWithMixin*>(&i) == this) return; // can't use an item with itself
      auto &ios = ioFactory::instance();
      if (ios.ynPrompt(action + name + L" "+ preposition +L" your " + withName + L"?"))
	if (use(i) == item::useResult::FAIL)
	  ios.message(L"That doesn't seem to work.");
    };
    itemHolder.forEachItem(f);
    return true;
  }

  // try to use the object with another (eg put object into container; put candles on candlestick)
  virtual item::useResult use(item &other) {
    return item::useResult::FAIL; // no effect by default
  }
};

/*
 * Mixin for items that have an effect against a monster
 */
class actionMonsterMixin : public virtual shared_item {
private:
  renderedAction<monster, monster> &action_;
public:
  actionMonsterMixin(renderedAction<monster, monster> &action) :
    action_(action) {}
  virtual ~actionMonsterMixin(){}
  virtual std::wstring actionName() const { return action_.name(); }
  virtual bool fire() {
    auto holder = &(shared_from_this()->holder());
    monster *m;
    do {
      m = dynamic_cast<monster *>(holder);
      item * it = dynamic_cast<item *>(holder);
      if (it) holder = &(it->holder());
    } while (m == nullptr);
    optionalRef<monster> target;
    if (m->isPlayer()) {
      wchar_t dir;
      // prompt the user for a monster to fire on
      auto &io = ioFactory::instance();
      dir = io.dirPrompt();
      switch (dir) {
      case L'<': case L'>':
	break; // you will miss
      case L'.':
	target = optionalRef<monster>(*m); // firing at onesself
	break;
      default:
	target = m->curLevel().findMonster(*m, dir);
	break;
      }
      if (!target)
	io.message(L"You don't hit even one monster.");
    } else {
      // select a monster to fire on
      return false; // TODO
    }
    if (!target)
      return false;
    auto pThis = shared_from_this();
      return action_(pThis->isBlessed(), pThis->isCursed(), *m, target.value());
  }
};



/*
 * Objects that can be equipped - worn, wielded, etc.
 * Except for 2-slot items; these use twoEquip
 */
template<int equipTyp>
class basicEquip : public basicItem, public onEquipMixin {
private:
  // slots in which this may be equipped, in preference order
  std::set<slotType> supportedSlots_;
public:
  template <typename... S>
  basicEquip(const itemType& type,  S... slots) :
    basicItem(type),
    supportedSlots_({slots...}) {
  }
  virtual ~basicEquip() {}
  virtual bool equip(monster &owner) {
    for (slotType s : supportedSlots_)
      if (owner.equip(*this, s)) {
	onEquipMixin::onEquip(owner);
	return true;
      }
    return false;
  }
  virtual void onUnequip(monster &m) {
    onEquipMixin::onUnequipImpl(m);
  }
  virtual equipType equippable() const {
    return static_cast<equipType>(equipTyp);
  }
  virtual std::wstring name() const {
    return basicItem::name() + ofName();
  }
  virtual std::wstring description() const {
    return basicItem::description() + ofPowers();
  }
};

class basicWeapon : public basicEquip<item::equipType::wielded> {
private:
  damageType damageType_;
public:
  basicWeapon(const itemType & type,  const damageType damage) :
    basicEquip(type, slotType::primary_weapon, slotType::secondary_weapon),
    damageType_(damage) {}
  virtual ~basicWeapon() {}
  virtual damageType weaponDamage(bool) {
    return damageType_;
  }
  virtual equipType equippable() const {
    return equipType::wielded;
  }
};

template <class B>
class chargeWeapon : public B, public burnChargeMixin {
private:
  damageType damageType_;
  const wchar_t * const message_;
public:
  chargeWeapon(const itemType &type, const damageType damage, const wchar_t * const message) :
    B(type, damageType::bashing),
    damageType_(damage),
    message_(message) {
    B::enchant(10);
  }
  virtual ~chargeWeapon() {}
  virtual damageType weaponDamage(bool use) {
    if (!use) return damageType_;
    if (!hasCharge()) return B::weaponDamage(use);
    if (B::isCursed()) {
      useCharge(); if (dPc() < B::isBlessed() ? 75 : 50) return damageType_;
      return B::weaponDamage(use);
    }
    if (!B::isBlessed() || dPc() < 50) useCharge();
    return damageType_;
  }
  virtual void useCharge() {
    burnChargeMixin::useCharge();
    ioFactory::instance().message(message_);
  }
};

template<bool singleShot, bool lineOfSight, unsigned char amount>
class basicThrown : public basicWeapon {
public:
  basicThrown(const itemType & type,  const damageType damage) :
    basicWeapon(type, damage) {}
  virtual ~basicThrown() {};
  virtual item::useResult use() {
    auto source = dynamic_cast<monster *>(&holder());
    if (!source) return item::useResult::FAIL; // must be in main inventory to use
    // 1) pick a monster
    auto target = pickTarget<lineOfSight>(*source);
    if (!target) return item::useResult::FAIL; // can't use missiles without a target
    // 2) get monster's location
    auto tPos = target->curLevel().posOf(*target);
    // 2) damage the monster
    unsigned char dam = amount;
    if (isBlessed()) dam *= 1.5;
    if (isCursed()) dam /= 2;
    // TODO: double damage if sling equipped (rock only)
    auto tName = target->name(); // copy the name in case target is destroyed
    auto &damType = damageRepo::instance()[weaponDamage(true)];
    auto rtn = target->wound(dam, damType);
    if (source->isPlayer()) {
      auto &ios = ioFactory::instance();
      ios.longMsg(L"Your " + name() + (rtn > 0 ? L" hits " + tName
				       : L" misses " + tName));
    } else if (target->isPlayer()) {
      auto &ios = ioFactory::instance();
      ios.longMsg(source->name() + L" " + (rtn > 0 ? L" hits you with a " + name()
					  : L" misses you with its " + name()));
    }
    // 3) relocate item to monster's location, or consume
    if (singleShot)
      holder().destroyItem(*this);
    else
      target->curLevel().holder(tPos).addItem(*this);
    return rtn > 0 ? item::useResult::DONE : item::useResult::FAIL;
  }
};

// as basicEquip, but requiring 2 slots
template<int equipTyp>
class twoEquip : public basicItem, public onEquipMixin {
private:
  // slots in which this may be equipped, in preference order
  std::set<std::pair<slotType, slotType>> supportedSlots_;
public:
  template<typename... S>
  twoEquip(const itemType& type,  S... slots) :
    basicItem(type),
    supportedSlots_({slots...}) {
  }
  virtual ~twoEquip() {}
  virtual bool equip(monster &owner) {
    for (std::pair<slotType, slotType> s : supportedSlots_)
      if (owner.equip(*this, s)) {
	onEquipMixin::onEquip(owner);
	return true;
      }
    return false;
  }
  virtual void onUnequip(monster &m) {
    onEquipMixin::onUnequipImpl(m);
  }
  virtual equipType equippable() const {
    return static_cast<equipType>(equipTyp);
  }
  virtual std::wstring name() const {
    return basicItem::name() + ofName();
  }
  virtual std::wstring description() const {
    return basicItem::description() + ofPowers();
  }
};

class twoHandedWeapon : public twoEquip<item::equipType::wielded> {
private:
  damageType damageType_;
public:
  twoHandedWeapon(const itemType &type,  const damageType &damage) :
    twoEquip(type,
	     std::make_pair(slotType::primary_weapon, slotType::secondary_weapon)
	     ), damageType_(damage) {};
  virtual ~twoHandedWeapon() {}
  virtual damageType weaponDamage(bool) {
    return damageType_;
  }
};

class pie : public basicItem {
private:
  enum class type {
    apple,
      cherry,
      cottage,
      cream,
      lemon_meringue,
      meat,
      pecan,
      pumpkin,
      shephards,
      steak_and_kidney,
      END
      };
  type type_;
public:
  pie(const itemType &typ) : 
    basicItem(typ),
    type_(static_cast<type>(rndPickI(0, static_cast<int>(type::END)))) {}
  virtual ~pie() {}
  const wchar_t *filling() const {
    switch (type_) {
    case type::apple: return L"apple";
    case type::cherry: return L"cherry";
    case type::cottage: return L"cottage";
    case type::cream: return L"cream";
    case type::lemon_meringue: return L"lemon meringue";
    case type::meat: return L"meat";
    case type::pecan: return L"pecan";
    case type::pumpkin: return L"pumpkin";
    case type::shephards: return L"shephard's";
    case type::steak_and_kidney: return L"steak & kidney";
    default: throw type_;
    };
  }
  virtual std::vector<std::wstring> adjectives() const {
    auto rtn = basicItem::adjectives();
    rtn.push_back(filling());
    return rtn;
  };
  virtual materialType material() const {
    switch (type_) {
    case type::cottage:
    case type::shephards:
      return materialType::fleshy;
    default:
      return basicItem::material();
    };
  };
};

class tshirt : public basicEquip<item::equipType::worn> {
public:
  tshirt() :
    basicEquip(itemTypeRepo::instance()[itemTypeKey::tshirt],
	       slotType::shirt) {}
  virtual ~tshirt() {}
  //tshirts are sexy if wet or torn, but this only helps if worn as an outer layer.
  virtual bool isSexy() { // ref: wet tshirt contents I guess
    return basicItem::isSexy() ||
      damageOfType(damageType::edged) > 0 ||
      damageOfType(damageType::wet) > 0;
  }
};

class bottlingKit;

class transmutedWater : public basicItem {
private:
  damageType toRepair_;
  const itemType &type_;
public:
  transmutedWater(const itemType &type,  const damageType &toRepair) :
    basicItem(type), toRepair_(toRepair), type_(type) {}
  virtual bool strike(const damageType &t) {
    if (t != toRepair_)
      return basicItem::strike(t);
    return false;
  }
  virtual bool repair(const damageType &t) {
    if (t == toRepair_)
      transmutate(*this, createItem(itemTypeKey::water));
    else
      return basicItem::repair(t);
    return true;
  }
  const wchar_t * useMessage() const {
    auto &it = itemTypeRepo::instance();
    if (type_ == it[itemTypeKey::water])
      return L"Delicious and refreshing.";
    else if (type_ == it[itemTypeKey::tears])
      return L"This has a poignant taste.";
    else if (type_ == it[itemTypeKey:: heavy_water])
	return L"This tasts serious."; // ref: Back to the Future
    else if (type_ == it[itemTypeKey:: fire_water])
      return L"This has a serious kick to it.";
    else if (type_ == it[itemTypeKey:: pop])
      return L"Pure as sunlight"; // Ref:1927 Coca Cola slogon.
    else if (type_ == it[itemTypeKey:: fizzy_pop])
      return L"*hic!*";
    else if (type_ == it[itemTypeKey:: dehydrated_water])
      return L"You are still thirsty.";
    else if (type_ == it[itemTypeKey:: spring_water])
      return L"Boing!"; // Ref: Same message is used in Nethack for a wand of strking or force bolt spell, if resisted.
    else if (type_ == it[itemTypeKey:: electro_pop])
      return L"You feel musical";
    else
      throw std::wstring(L"Unknown type ") + type_.name();
    }
  optionalRef<sharedAction<item, monster> > useAction() const {
    auto &it = itemTypeRepo::instance();
    if (type_ == it[itemTypeKey::water])
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey::tears])
      // TODO: tradgedy action
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey:: heavy_water])
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey:: fire_water]) {
      if (isCursed())
	return actionFactory<item, monster>::get(sharedAction<item, monster>::key::extra_damage_hot);
      else if (isBlessed())
	return actionFactory<item, monster>::get(sharedAction<item, monster>::key::resist_all_damage_hot);
      else 
	return actionFactory<item, monster>::get(sharedAction<item, monster>::key::resist_damage_hot);
    } else if (type_ == it[itemTypeKey:: pop])
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey:: fizzy_pop])
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey:: dehydrated_water])
      return optionalRef<sharedAction<item, monster> >();
    else if (type_ == it[itemTypeKey:: spring_water])
      // we don't have a jump action, so let's be nice and bestow flight.
      // TODO: this should be blessed only; we should jump otherwise. If we jump.
      return actionFactory<item, monster>::get(sharedAction<item, monster>::key::flight);
    else if (type_ == it[itemTypeKey:: electro_pop])
      return optionalRef<sharedAction<item, monster> >();
    else
      throw std::wstring(L"Unknown type ") + type_.name();
  }
};

class water : public basicItem {
public:
  water() :
    basicItem(itemTypeRepo::instance()[itemTypeKey::water]) {}
  virtual bool strike(const damageType &t) {
    itemTypeKey next;
    switch (t) {
      /* tearing (cutting skin/armour) */
    case damageType::edged: 
      next = itemTypeKey::tears; break;
      /* & earth /falling */
    case damageType::bashing:
      next = itemTypeKey::heavy_water; break;
      /* & file (burning skin/armour)*/
    case damageType::hot: 
      next = itemTypeKey::fire_water; break;
    case damageType::cold: 
      /* water (rotting) */
    case damageType::wet:
      next = itemTypeKey::pop; break;
      /* air (damages hearing) */
    case damageType::sonic: 
      next = itemTypeKey::fizzy_pop; break;
      /* time */
    case damageType::disintegration: 
      next = itemTypeKey::dehydrated_water; break;
      /* flora */
    case damageType::starvation:
      next = itemTypeKey::spring_water; break;
      /* lightning */
    case damageType::electric:
      next = itemTypeKey::electro_pop; break;
    }
    transmutate(*this, createItem(next));
    return true;
  }  
};


// NB: We must be an itemHolder, to meet the itemHolderMap contract.
class bottle : public basicItem, public liquidContainer, private itemHolder {
  friend class bottlingKit;
private:
  optionalRef<item> content() {
    optionalRef<item> rtn;
    forEachItem([&rtn](item &i, std::wstring) {
	rtn = optionalRef<item>(i);
      });
    return rtn;
  }
  // TODO: bottles should use charges for potion sizes.
  optionalRef<const item> content() const {
    optionalRef<const item> rtn;
    forEachItem([&rtn](const item &i, std::wstring) {
	rtn = optionalRef<const item>(i);
      });
    return rtn;
  }
public:
  bottle(const itemType& type) :
    basicItem(type) {}
  bottle(const bottle &) = delete;
  virtual ~bottle() {}
  bool isShipInBottle() const {
    optionalRef<const item> c = content();
    auto cName = c.value().name();
    return (cName == std::wstring(L"ship"));
  }
  virtual std::wstring name() const {
    optionalRef<const item> c = content();
    basicItem::name(); // sets buffer();
    if (c) {
      std::wstring cName = c.value().name();
      if (isShipInBottle()) {
	buffer_ = cName + L" in a " + buffer_;
      } else {
	buffer_ += L" of ";
	buffer_ += cName;
      }
    } else {
      buffer_ = L"empty " + buffer_;
    }
    return buffer_.c_str();
  }
  virtual double weight() const {
    optionalRef<const item> c = content();
    if (c) {
      double baseWeight;
      if (isShipInBottle() && !isCursed())
	baseWeight = basicItem::weight(); // ships in bottles don't use their real weight, unless cursed.
      else
	baseWeight = c.value().weight();
    /* items in bottle are in miniature. So you get an item reduced to 2% of its weight (1:50 scale) for a normal bottle, 1% for blessed, 200% for cursed.*/
      double multiplicand;
      if (isCursed()) multiplicand = 2 * (1 + std::abs(enchantment()));
      else if (isBlessed()) multiplicand = 0.01 - (0.005 * enchantment());
      else multiplicand = 0.02;
      return baseWeight * multiplicand;
    }
    return basicItem::weight();
  }
  virtual bool strike(const damageType &type) {
    bool rtn = basicItem::strike(type);
    //TODO: smash the bottle if broken.
    return rtn;
  }
  // when a bottle is destroyed, its contents are revealed:
  virtual void destroy() {
    optionalRef<item> c = content();
    auto &ios =ioFactory::instance();
    if (!c) {
      ios.message(name() + L" smashes, and the emptiness gets out"); // Hmmm; spawn a vacuum monster?
    } else if (c.value().material() == materialType::liquid) {
      ios.message(name() + L" smashes; there's fluid everywhere");
      itemHolder::destroyItem(c.value());
    } else if (isShipInBottle() && whereToLaunch()) {
      auto &pos = whereToLaunch().value();
      auto cname = c.value().name();
      if (pos.addItem(c.value())) {
	auto &align = dynamic_cast<monster&>(holder()).align();
	if (align.nonaligned()) {
	  ios.message(L"With thanks to the shipwrights.");
	} else {
	  std::wstring deity = align.name();
	  ios.message(deity + L" bless this " + cname + L" and all who sail in her.");
	}
      } else {
	itemHolder::destroyItem(c.value());
	ios.message(name() + L" smashes; you lose the " + cname);
      }
    } else {
      auto cname = c.value().name();
      if (holder().addItem(c.value())) {
	ios.message(name() + L" smashes; you now have a " + cname);
      } else {
	itemHolder::destroyItem(c.value());
	ios.message(name() + L" smashes; you lose the " + cname);
      }
    }
    if (content()) throw L"Destroying bottle without losing its contents!";
    basicItem::destroy();
  }
private:
// if we were a ship in a bottle, on which square must we launch it?
// player must use the bottle while adjacent to water to launch.
  optionalRef<itemHolder> whereToLaunch() {
    optionalRef<itemHolder> rtn;
    auto &h = holder();
    auto m = dynamic_cast<monster*>(&h);
    if (!m) // ship must be in main inventory to 
      return rtn;
    auto &l = m->curLevel();
    coord s = l.posOf(*m);
    for (coord c : coordRectIterator(s.first-1, s.second-1, s.first+1, s.second+1)) {
      if (c == s || c.first < 0 || c.second < 0 || c.first >= level::MAX_WIDTH || c.second >= level::MAX_HEIGHT)
	continue;
      if (l.terrainAt(c).type() == terrainType::WATER)
	return l.holder(c);
    }
    return rtn;
  }
public:
  virtual useResult use() {
    if (content()) {
      auto *pc = dynamic_cast<monster*>(&holder());
      auto &ios = ioFactory::instance();
      bool ship = isShipInBottle();
      if (pc == 0 || !pc->isPlayer() || 
	  (ship && ios.ynPrompt(L"Launch the " + name() + L"?")) ||
	  (!ship && ios.ynPrompt(L"Smash the " + name() + L"?")))
	destroy();
    }
    return item::useResult::SUCCESS;
  }
  // Note that, unlike basicContainer, this isn't use-with-item; the player can't
  // simply create their own bottles:
  virtual bool addItem(item &i) {
    if (content()) return false; // already occupied
    return itemHolder::addItem(i);
  }
  // interface liquidContainer:
  virtual bool containsLiquid() const {
    optionalRef<const item> c = content();
    return c && c.value().material() == materialType::liquid;
  }
  virtual void consumeBy(monster &m) {
    auto &fluid = content().value(); // we don't come in here unless we contain liquid
    const bool isPc = m.isPlayer();
    const wchar_t * msg;
    optionalRef<sharedAction<item, monster> > action;
    if (dynamic_cast<water*>(&fluid)) {
      msg = L"Delicious and refreshing.";
    } else if (dynamic_cast<transmutedWater*>(&fluid)) {
      auto &tw = dynamic_cast<transmutedWater&>(fluid);
      msg = tw.useMessage();
      action = tw.useAction();
    }
    if (isPc) ioFactory::instance().message(msg);
    if (action) action.value()(isBlessed(), isCursed(), *this, m);
    if (enchantment() > 1) enchant(-1);
    else itemHolder::destroyItem(fluid);
  }
};


// base class for multi-item containers
class basicContainer : public basicItem, public itemHolder, public useWithMixin {
public:
  basicContainer(const itemType& type) :
    basicItem(type),
    useWithMixin() {}
  virtual ~basicContainer() {}
  virtual double weight() const {
    double rtn = basicItem::weight();
    forEachItem([&rtn](const item &item, std::wstring) {
	rtn += item.weight();
      });
    if (isBlessed()) rtn *= 0.9;
    if (isCursed()) rtn *= 2;
    return rtn;
  }
  virtual bool strike(const damageType &type) {
    bool rtn = basicItem::strike(type);
    int target = 50;
    if (isBlessed()) target /=10;
    if (isCursed()) target *=12;
    forEachItem([&rtn, target, type](item &c, std::wstring) {
	// pass damage to contents
	if (dPc() < target)
	  rtn |= c.strike(type);
      });
    return rtn;
  }

  virtual item::useResult use() {
    const std::wstring name(basicItem::name());
    bool success = true;
    success &=useWithMixin::use(name, L"Put ", L"into", holder()); // calls use(item) as needed
    return success && useWithMixin::use(name, L"Take ", L"from", *this) ? // calls use(item) as needed
      item::useResult::SUCCESS : item::useResult::FAIL;
  }
  // put into or take out of bag:
  virtual item::useResult use(item &other) {
    if (!contains(other)) // not in this container, so put in
      itemHolder::addItem(other);
    else // in this container, so take out (add to *our* holder)
      holder().addItem(other);
    return item::useResult::SUCCESS;
  }
  // put into bag; interface itemHolder
  virtual bool addItem(item & other) {
    if (contains(other)) return false;
    itemHolder::addItem(other);
    return true;
  }

protected:
  // may be called by subclasses, eg prismatic container
  virtual bool repairContents(const damageType &type) {
    bool rtn = basicItem::repair(type);
    int target = 50;
    if (isBlessed()) target *=12;
    if (isCursed()) target /=10;
    forEachItem([&rtn, target, type](item &c, std::wstring) {
	// pass damage to contents
	if (dPc() < target)
	  rtn |= c.repair(type);
      });
    return rtn;
  }
  
};




class readableItem : public basicItem {
public:
  readableItem(const itemType& type) :
    basicItem(type) {}
  virtual ~readableItem() {}
   // TODO: Text, spells etc.
};

class hitchGuide : public readableItem {
public:
  hitchGuide(const itemType& type) :
    readableItem(type) {}
  virtual ~hitchGuide() {}
  virtual item::useResult use() {
    if (isCursed()) return item::useResult::FAIL; // seems a bit harsh, but I don't have a better idea yet
    ::invokeGuide(isBlessed());
    return item::useResult::SUCCESS;
  }
};

class holyBook : public readableItem {
private:
  const deity &align_;
public:
  holyBook() :
    readableItem(itemTypeRepo::instance()[itemTypeKey::holy_book]),
    align_(*rndPick(deityRepo::instance().begin(), deityRepo::instance().end())) {}
  holyBook( const deity &align) :
    readableItem(itemTypeRepo::instance()[itemTypeKey::holy_book]),
    align_(align) {}
  virtual ~holyBook() {}
  virtual std::wstring name() const {
    if (align_.nonaligned())
      // ref: Russell's Teapot, a phylosophical analogy used to place the buden of proof on the preacher.
      return L"Book of the Teapot";
    buffer_ = L"holy book of " + align_.name();
    return buffer_.c_str();
  }
  virtual item::useResult use() {
    auto &ios = ioFactory::instance();
    if (isCursed()) {
      ios.message(L"The pages are stuck together.");
      return item::useResult::FAIL;
    }
    ios.message(L"You turn the pages of the " + name());
    ios.longMsg(align_.description());
    return item::useResult::SUCCESS;
  }
};

class iou : public readableItem {
private:
  const double amount_;
  const std::wstring whom_;
  const std::wstring service_;
public:
  iou(double amount, std::wstring whom, std::wstring service) :
    readableItem(itemTypeRepo::instance()[itemTypeKey::iou]),
    amount_(amount), whom_(whom), service_(service) {
    curse(true);
  }
  virtual ~iou() {};
  virtual std::wstring name() {
    buffer_ = L"I.O.U. (";
    buffer_ += whom_ + L")";
    return buffer_.c_str();
  }
  virtual damageType weaponDamage(bool) {
    return damageType::electric; // ref: nethack has a ball-and-chain punishment, which makes a reasonable weapon...
  }
  virtual item::useResult use() {
    std::wstring msg = name();
    msg += L"\nFor services rendered:\n\n\t";
    msg += service_;
    msg += L"\n\nIf payment is not received within 100 turns of issue, debt\n"
      "collectors will be deployed.\nYou have been warned.\n\n"
      "Payment may be made to any Platinum Express Shop"; // ref: see platinum express card
    auto &ios = ioFactory::instance();
    ios.longMsg(msg);
    return item::useResult::SUCCESS;
  }
  virtual void destroy() {
    holder().addItem(createIou(amount_, whom_, service_));
  }
  virtual bool isBlessed() { return false;}
  virtual bool isCursed() { return true;}
  virtual bool isProof(const damageType &type) { return true; }
  double amount() const { return amount_; }
  const std::wstring & service() const { return service_; }
};

// no need to extend basicContainer, as we always consume our contents.
class napsackOfConsumption : public twoEquip<item::equipType::worn>, public itemHolder, public useWithMixin {
public:
  napsackOfConsumption() :
    twoEquip(itemTypeRepo::instance()[itemTypeKey::napsack_of_consumption],
	     // gameplay reason: this is a utility item that can't be worn along with armour, so use the hauburk & doublet slots where most armour lives.
	     // story reason: In Knightmare, you see napsacks worn under cloaks, but over shirts.
	     std::make_pair(slotType::hauburk, slotType::doublet)) {}
  virtual ~napsackOfConsumption() {};
  virtual item::useResult use() {
    if (!usable()) return item::useResult::FAIL;
    const std::wstring name(basicItem::name());
    return useWithMixin::use(name, L"Put ", L"into", holder()) ? // calls use(item) as needed
      item::useResult::SUCCESS : item::useResult::FAIL;
  }
  virtual item::useResult use(item & other) {
    return addItem(other) ? item::useResult::SUCCESS : item::useResult::FAIL;
  }
  virtual bool addItem(item & other) {
    if (!usable()) return false;
    auto &m = dynamic_cast<monster &>(holder()); // checked by usable()
    auto p = m.isPlayer();
    bool rtn = false;
    try {
      rtn = m.eat(other, false);
      if (rtn && p) ioFactory::instance().message(L"The " + other.name() + L" is consumed.");
      if (!rtn && p) ioFactory::instance().message(L"The " + name() + L" partially consumes the " + other.name());
    } catch (monster::inedibleException e) {
      if (!rtn && p) ioFactory::instance().message(L"The " + name() + L" can't consume the " + other.name());      
    } catch (monster::notHungryException e) {
      rtn = other.holder().destroyItem(other);
      if (rtn && p) ioFactory::instance().message(L"The " + other.name() + L" is consumed.");
      if (!rtn && p) ioFactory::instance().message(L"The " + name() + L" can't consume the " + other.name());      
    }
    return rtn;
  };
private:
  bool usable() {
    // 1) we must be in the main inventory of a monster
    auto m = dynamic_cast<monster *>(&holder());
    if (m == nullptr) return false;
    // 2) we must be equipped to use
    if (m->slotsOf(*this)[0] == nullptr) {
      if (m->isPlayer())
	ioFactory::instance().message(L"You must be wearing the napsack to open it.");
      return false;
    }
    return true;
  }
};


class shopCard : public basicItem {
public:
  shopCard(const itemType& type) :
    basicItem(type) {}  
  virtual ~shopCard(){}
  virtual item::useResult use() {
    auto m = dynamic_cast<monster*>(&holder());
    if (m)
      ::goShopping(*m);
    return m ? item::useResult::SUCCESS : item::useResult::FAIL;
  }
};


// bottling kits can be wielded as a bashing weapon (ref:tinopener in nethack)
// TODO: this will bottle liquid monsters when killed, instead of losing their liquid
// you can use it with at item only when wielded, as bottling takes time (TODO)
// TODO: starting charges; check number of charges
class bottlingKit : public basicWeapon, public burnChargeMixin {
public:
  bottlingKit(const itemType & type) :
    basicWeapon(type, damageType::bashing) {
    enchant(dPc());
  }
  virtual ~bottlingKit() {}
  virtual item::useResult use() {
    auto &ios = ioFactory::instance();
    if (!hasCharge()) {
      ios.message(L"You are out of bottle caps.");
      return item::useResult::FAIL;
    }
    auto &pc = dynamic_cast<monster&>(holder());
    optionalRef<item> bot = pc.firstItem([](item &i) {
	auto *bot = dynamic_cast<bottle*>(&i);
	return bot != 0 && bot->content();
      });
    if (!bot) {
      ios.message(L"You'll need a bottle in your inventory to do that.");
      return item::useResult::FAIL;
    }
    ios.message(L"You try the " + bot.value().name());
    bottle &b = dynamic_cast<bottle&>(bot.value());
    auto found = pc.firstItem([&b, this, &ios](item &i) {
	if (&i == this) return false; // can't bottle a bottling kit with itself
	if (&i == &b) return false; // can't bottle a bottle into itself
	if (ios.ynPrompt(L"Bottle " + i.name() + L"?")) {
	  if (b.addItem(i))
	    return true;
	  else ios.message(L"It didn't fit."); // just in case
	}
	return false;
      });
    if (found) useCharge();
    return found ? item::useResult::SUCCESS : item::useResult::FAIL;
  }
  virtual std::wstring describeCharges() const {
    auto rtn = std::wstring(L"This kit includes ");
    rtn+= enchantment();
    return rtn + L"bottle caps. Use them wisely.";
  }
};


class corpse : public basicItem {
private:
  // what was this in life?
  const monsterType &type_;
  // how advanced was this in life?
  const unsigned char maxDamage_;
public:
  corpse( const monsterType &of, const unsigned char maxDamage) :
    basicItem(itemTypeRepo::instance()[itemTypeKey::corpse]),
    type_(of),
    maxDamage_(maxDamage) {
    // TODO: should any monster types be proof against damage? Already (eg) wet?
    // TODO: incubus & succubus should be sexy. After all, they are confident...
    // if (monsterType == monsterType::incubus || monsterType == monsterType::succubus) sexUp(true);
  }
  virtual ~corpse() {};
  virtual materialType material() const {
    // trolls are stony, bats are leathery, plants are veggy, etc.
    return type_.material();
  }
  virtual double weight() const {
    return type_.corpseWeight(); // weight of average human in N Earth gravity (610lb). TODO: per type somehow
  }
  // if wielded, what damage does this weapon do?
  // list of all adjectives applicable to type
  virtual std::vector<std::wstring> adjectives() const {
    auto baseAdjectives = basicItem::adjectives();
    std::vector<std::wstring> adjectives;
    adjectives.push_back(L"dead");
    adjectives.insert(adjectives.end(), baseAdjectives.begin(), baseAdjectives.end());
    return adjectives;
  }
  virtual std::wstring simpleName() const {
    return type_.name(maxDamage_); // overridden to change type_ from itemType to monsterType.
  }
  virtual item::useResult use() {
    auto &ios = ioFactory::instance();
    ios.message(L"Please don't; it's not that kind of game.");
    return item::useResult::FAIL;
  }
};

// joints or steaks of an animal/monster
class joint : public basicItem {
private:
  const monsterType &type_;
public:
  joint(const itemType &it, const monsterType &of) :
    basicItem(it),
    type_(of) {}
  virtual ~joint() {}
  virtual materialType material() const {
    // trolls are stony, bats are leathery, plants are veggy, etc.
    return type_.material();
  }
  const wchar_t * const cut() const {
    switch (enchantment()) {
    case 0: return L"rump";
    case 1: return L"top-rump";
    case 2: return L"leg";
    case 3: return L"skirt";
    case 4: return L"brisket";
    case 5: return L"chuck";
    case 6: return L"blade";
    case 7: return L"neck";
    case 8: return L"flank";
    case 9: return L"shank";
    case 10: return L"rib-eye";
    case 11: return L"silverside";
    case 12: return L"topside";
    case 13: return L"fillet";
    default: return L"sirloin";
    }
  }
  virtual std::vector<std::wstring> adjectives() const {
    auto adjectives = basicItem::adjectives();
    if (enchantment() > 14 || isBlessed()) adjectives.push_back(L"finest");
    adjectives.push_back(type_.names().at(0));
    adjectives.push_back(cut());
    return adjectives;    
  }
};

// TODO: steak fails the ISA test for joint, but we can't make it a private baseclass without
// hiding "item". Should we rethink items in terms of composition?
class steak : public joint {
private:
  enum class prep {
    bleu,
      rare,
      medrare,
      medium,
      welldone,
      END
      };
  prep prep_;
public:
  steak(const itemType &it, const monsterType &of) :
    joint(it, of),
    prep_(static_cast<prep>(rndPickI(0,static_cast<int>(prep::END)))) {}
  virtual ~steak() {}
  const wchar_t * const prepName() const {
    switch (prep_) { // TODO: should this be based on fire damage?
    case prep::bleu: return L"bleu";
    case prep::rare: return L"rare";
    case prep::medrare: return L"medium-rare";
    case prep::medium: return L"medium";
    case prep::welldone: return L"well-done";
    default: throw prep_;
    }
  }
  virtual std::vector<std::wstring> adjectives() const {
    auto baseAdjectives = joint::adjectives();
    std::vector<std::wstring> adjectives;
    adjectives.push_back(prepName());
    adjectives.insert(adjectives.end(), baseAdjectives.begin(), baseAdjectives.end());
    return adjectives;
  }
};


void transmutate(item &from, item &to) {
  std::array<const slot *,2> slots = {nullptr, nullptr};
  monster *m = dynamic_cast<monster *>(&from);
  auto h = from.holder();
  if (m) {
    auto slots = m->slotsOf(from); // may be nullptr, nullptr
    if (slots[0] != nullptr) m->unequip(from);
  }
  auto &map = itemHolderMap::instance();
  auto &holder = map.forItem(from);
  holder.replaceItem(from, to);
  for (auto &dt : allDamageTypes) {
    // ignore return value; if to ignores the damage type there's no need to tell user it's fixed.
    const auto amount = from.damageOfType(dt);
    if (amount != 0)
      for (int i=0; i < amount; ++i) 
	to.strike(dt);
    if (from.isProof(dt))
      to.proof(dt);
  }
  if (from.isBlessed())
    to.bless(true);
  if (from.isCursed())
    to.curse(true);
  if (from.isUnidentified())
    to.unidentify(true);
  if (from.isSexy())
    to.sexUp(true);
  const auto charm = from.enchantment();
  if (charm != 0)
    to.enchant(charm);
  if (slots[0] != nullptr) {
    m->equip(to, slots);
  }
}


// wands: sticks that carry charges in their enchantment and some sort of action.
// TODO: if a wand is blessed by a deity, it could be come a holy rod of that deity, preventing access from other temples and providing extra effects in the right shrine or against the right monster?
class wand : public basicItem, public burnChargeMixin, public actionMonsterMixin {
public:
  wand(unsigned char numCharges, renderedAction<monster, monster> &action) :
    basicItem(itemTypeRepo::instance()[itemTypeKey::stick]),
    burnChargeMixin(),
    actionMonsterMixin(action) {
    enchant(numCharges);
  }
  virtual ~wand() {};
  virtual std::wstring simpleName() const {
    return hasCharge() ? L"wand" : basicItem::simpleName();
  }
  virtual std::wstring name() const {
    basicItem::name(); // sets buffer_
    if (hasCharge()) {
      buffer_ += L" of ";
      buffer_ += actionMonsterMixin::actionName();
    }
    return buffer_.c_str();
  }
  virtual item::useResult use() {
    if (!hasCharge()) return item::useResult::FAIL;
    if (isCursed()) {
      // fail to work about half the time:
      if (dPc() < 50) {
	if (isBlessed()) return item::useResult::FAIL; // blessed cursed wands don't lose charges when they fail to work
	useCharge();
	return item::useResult::FAIL; // plain cursed wands do lose charges if they fail to work
      }
    }
    if (!isBlessed() || dPc() < 50) // blessed wands use a charge only 50% of the time, normal wands 100%
      useCharge();
    return fire() ? item::useResult::SUCCESS : item::useResult::FAIL;
  }
};

class instrument : public basicItem, burnChargeMixin, actionMonsterMixin {
public:
  instrument(renderedAction<monster, monster> &action, const itemType & type) :
    basicItem(type),
    burnChargeMixin(),
    actionMonsterMixin(action) {
    /*
    theremin,  // attack_ray_med_electric
    visi_sonar, // charm
    baliset, // enchant_item
    drum, // nudity
    bagpipes, // petrify
    conch, // disarm
    harmonica, // tremolo-tuned; attack_ray_med_sonic
    pan_flute, // attraction
    lyre, // teleport_away
    pianoforte, // attack_ray_med_bashing
    */
  }
  virtual ~instrument() {}
  virtual std::wstring name() const {
    basicItem::name(); // sets buffer_
    if (hasCharge()) {
      buffer_ += L" of ";
      buffer_ += actionMonsterMixin::actionName();
    }
    return buffer_.c_str();
  }
  virtual std::vector<std::wstring> adjectives() const {
    auto rtn = basicItem::adjectives();
    if (type_ == itemTypeRepo::instance()[itemTypeKey::harmonica])
      rtn.push_back(L"tremolo-tuned");
    return rtn;
  }
  virtual item::useResult use() {
    if (!dynamic_cast<monster*>(&holder()))
      return item::useResult::FAIL; // can only be used in main inventory
    if (!hasCharge()) {
      if (dynamic_cast<monster&>(holder()).curLevel().dung().pc()->abilities().hear()) {
	if (type_ == itemTypeRepo::instance()[itemTypeKey::bagpipes])
	  ioFactory::instance().message(L"The beautiful sound of " + name() + L" fills the air");
	else
	  ioFactory::instance().message(L"The beautiful sound of a " + name() + L" fills the air");
      }
      return item::useResult::SUCCESS;
    }
    if (isCursed()) {
      // fail to work about half the time:
      if (dPc() < 50) {
	if (isBlessed()) return item::useResult::FAIL; // blessed cursed wands don't lose charges when they fail to work
	useCharge();
	return item::useResult::FAIL; // plain cursed wands do lose charges if they fail to work
      }
    }
    if (!isBlessed() || dPc() < 50) // blessed wands use a charge only 50% of the time, normal wands 100%
      useCharge();
    return fire() ? item::useResult::SUCCESS : item::useResult::FAIL;
  }
};

class basicTransport : public basicItem, public transport {
public:
  basicTransport(const itemType &type, 
		 const terrainType &activate, 
		 const terrainType &allow, 
		 const movementType &movement) :
    basicItem(type),
    transport(activate, allow, movement) {}
  virtual ~basicTransport() {}
};

class necklace : public basicEquip<item::equipType::worn> {
public:
  necklace(const itemType &type) :
    basicEquip<item::equipType::worn>(type, slotType::amulet) {}
  virtual ~necklace() {};
  virtual std::wstring simpleName() const {
    if (countEquipActions() > 0) {
      return L"talisman"; // meaning an amulet with magical effects
    } else
      return basicItem::simpleName();
  }
};

// traits class holds:
// - type: default class type to use
// - make(): static template function, typed on concrete class type, taking itemtype parameter, producing new object on the heap and returning.
// Verbose, but we can use wrapper classes to add mixin/subclass functionality to an object in future.
template <itemTypeKey key> struct itemTypeTraits {};


template <> struct itemTypeTraits<itemTypeKey::apple> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t);} };
template <> struct itemTypeTraits<itemTypeKey::bread> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t);} };
template <> struct itemTypeTraits<itemTypeKey::cake> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t);} };
template <> struct itemTypeTraits<itemTypeKey::pie> {
  typedef pie type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t);} };
template <> struct itemTypeTraits<itemTypeKey::joint> {
  typedef joint type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t, rndSolidMonster());} };
template <> struct itemTypeTraits<itemTypeKey::steak> {
  typedef steak type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t, rndSolidMonster());} };
template <> struct itemTypeTraits<itemTypeKey::mace> {
  typedef basicWeapon type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t, damageType::bashing);} };
template <> struct itemTypeTraits<itemTypeKey::two_sword> {
  typedef twoHandedWeapon type;
  // we specifically chose a historical 2-handed cutting sword to annoy the reenactment purists :)
  template<typename type>
  static item *make(const itemType &t) {return new type(t, damageType::edged);} };
template <> struct itemTypeTraits<itemTypeKey::flamethrower> {
  typedef chargeWeapon<twoHandedWeapon> type;
  template<typename type>
  static item *make(const itemType &t) {return new type(t, damageType::hot, L"A burst of flames leaps forth"); }
  };
template <> struct itemTypeTraits<itemTypeKey::nitrogen_tank> {
  typedef chargeWeapon<twoHandedWeapon> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::cold, L"A jet of super-cooled liquid fires forth"); }
};
template <> struct itemTypeTraits<itemTypeKey::bubble_gun> {
  typedef basicWeapon type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::wet);}
};
template <> struct itemTypeTraits<itemTypeKey::sonic_cannon> {
  typedef chargeWeapon<basicWeapon> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::sonic, L"Air vibrations shudder forth"); }
};
template <> struct itemTypeTraits<itemTypeKey::maser> {
  typedef chargeWeapon<basicWeapon> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::disintegration, L"Radiation fires forth"); }
};
template <> struct itemTypeTraits<itemTypeKey::taser> {
  typedef chargeWeapon<basicWeapon> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::electric, L"Spark fly forth");}
};
template <> struct itemTypeTraits<itemTypeKey::rock> {
  typedef basicThrown<false, true, 5> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, damageType::bashing); }
};
template <> struct itemTypeTraits<itemTypeKey::bow> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::hat, slotType::tail); }
};
template <> struct itemTypeTraits<itemTypeKey::boots> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shoes); }
};
template <> struct itemTypeTraits<itemTypeKey::cloak> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::robe); }
};
template <> struct itemTypeTraits<itemTypeKey::crupper> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shorts); }
};
template <> struct itemTypeTraits<itemTypeKey::doublet> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::doublet); }
};
template <> struct itemTypeTraits<itemTypeKey::flanchard> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::jerkin); }
};
template <> struct itemTypeTraits<itemTypeKey::haubergeon> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::hauburk); }
};
template <> struct itemTypeTraits<itemTypeKey::hauberk> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::hauburk); }
};
template <> struct itemTypeTraits<itemTypeKey::helmet> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::hat); }
};
template <> struct itemTypeTraits<itemTypeKey::jerkin> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::jerkin); }
};
template <> struct itemTypeTraits<itemTypeKey::peytral> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::robe); }
};
template <> struct itemTypeTraits<itemTypeKey::robe> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::robe); }
};
template <> struct itemTypeTraits<itemTypeKey::saddle> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::saddle); }
};
template <> struct itemTypeTraits<itemTypeKey::scabbord> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::belt); }
};
template <> struct itemTypeTraits<itemTypeKey::shirt> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shirt); }
};
template <> struct itemTypeTraits<itemTypeKey::shorts> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shorts); }
};
template <> struct itemTypeTraits<itemTypeKey::skirt> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shorts); }
};
template <> struct itemTypeTraits<itemTypeKey::socks> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::socks); }
};
template <> struct itemTypeTraits<itemTypeKey::trousers> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::shorts); }
};
template <> struct itemTypeTraits<itemTypeKey::tshirt> {
  typedef tshirt type;
  template<typename type>
  static item *make(const itemType &t) { return new type(); }
};
template <> struct itemTypeTraits<itemTypeKey::underpants> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::underwear); } 
};
template <> struct itemTypeTraits<itemTypeKey::stick> {
  typedef wand type;
  template<typename type>
  static item *make(const itemType &t) { 
    auto &action = actionFactory<monster, monster>::get(static_cast<sharedAction<monster,monster>::key>(rndPickI(0, static_cast<int>(sharedAction<monster,monster>::key::END))));
    // random wands are initially created as sticks, must be enchanted to use:
    return new type(0,  action); }
};
template <> struct itemTypeTraits<itemTypeKey::bottle> {
  typedef bottle type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::codex> {
  typedef readableItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::hitch_guide> {
  typedef hitchGuide type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::holy_book> {
  typedef holyBook type;
  template<typename type>
  static item *make(const itemType &t) { return new type(); } 
};
// corpse not handled by traits
// IOU not handled by traits
template <> struct itemTypeTraits<itemTypeKey::poke> {
  typedef basicContainer type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::napsack_of_consumption> {
  typedef napsackOfConsumption type;
  template<typename type>
  static item *make(const itemType &t) { return new type(); }
};
template <> struct itemTypeTraits<itemTypeKey::water> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::tears> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::edged); }
};
template <> struct itemTypeTraits<itemTypeKey::heavy_water> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::bashing); }
};
template <> struct itemTypeTraits<itemTypeKey::fire_water> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::hot); }
};
 template <> struct itemTypeTraits<itemTypeKey::pop> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::wet); }
};
template <> struct itemTypeTraits<itemTypeKey::fizzy_pop> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::sonic); }
};
template <> struct itemTypeTraits<itemTypeKey::dehydrated_water> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::disintegration); }
};
template <> struct itemTypeTraits<itemTypeKey::spring_water> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::starvation); }
};
template <> struct itemTypeTraits<itemTypeKey::electro_pop> {
  typedef transmutedWater type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  damageType::electric); }
};
template <> struct itemTypeTraits<itemTypeKey::wooden_ring> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::ring_left_thumb, slotType::ring_left_index, slotType::ring_left_middle, slotType::ring_left_ring, slotType::ring_left_little, slotType::ring_right_thumb, slotType::ring_right_index, slotType::ring_right_middle, slotType::ring_right_ring, slotType::ring_right_little, slotType::toe_left_thumb, slotType::toe_left_index, slotType::toe_left_middle, slotType::toe_left_fourth, slotType::toe_left_little, slotType::toe_right_thumb, slotType::toe_right_index, slotType::toe_right_middle, slotType::toe_right_fourth, slotType::toe_right_little); }
};
template <> struct itemTypeTraits<itemTypeKey::amulet> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::amulet); }
};
template <> struct itemTypeTraits<itemTypeKey::necklace> {
  typedef necklace type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::tikka> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::headband); }
};
template <> struct itemTypeTraits<itemTypeKey::spectacles> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t,  slotType::glasses); }
};
template <> struct itemTypeTraits<itemTypeKey::bracelet> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::bracelet_left, slotType::bracelet_right); }
};
template <> struct itemTypeTraits<itemTypeKey::anklet> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::anklet_left, slotType::anklet_right); }
};
template <> struct itemTypeTraits<itemTypeKey::cloth_gloves> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::gloves); }
};
template <> struct itemTypeTraits<itemTypeKey::armband> {
  typedef basicEquip<item::equipType::worn> type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, slotType::bracelet_left, slotType::bracelet_right); }
};
template <> struct itemTypeTraits<itemTypeKey::kalganid> {
  typedef basicItem type; // TODO: should we be able to equip coins on our eyes?
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::gpl_slip> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::gpl_strip> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::gpl_bar> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::gpl_brick> {
  typedef basicItem type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::shop_card> {
  typedef shopCard type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::bottling_kit> {
  typedef bottlingKit type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t); }
};
template <> struct itemTypeTraits<itemTypeKey::theremin> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) {  // attack_ray_med_electric
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::attack_ray_med_electric);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::visi_sonor> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // charm
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::charm);
    return new type(action, t);
  }    
};
template <> struct itemTypeTraits<itemTypeKey::baliset> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // enchant_item
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::enchant_item);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::drum> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // nudity
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::nudity);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::bagpipes> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // petrify
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::petrify);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::conch> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // disarm
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::disarm);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::harmonica> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // tremolo-tuned; attack_ray_med_sonic
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::attack_ray_med_sonic);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::pan_flute> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // attraction
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::attract);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::lyre> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // teleport_away
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::teleport_away);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::pianoforte> {
  typedef instrument type;
  template<typename type>
  static item *make(const itemType &t) { // attack_ray_med_bashing
    auto &action = actionFactory<monster, monster>::get(sharedAction<monster,monster>::key::attack_ray_med_bashing);
    return new type(action, t);
  }
};
template <> struct itemTypeTraits<itemTypeKey::bridge> {
  typedef basicTransport type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, terrainType::WATER, terrainType::GROUND,
					    movementType({speed::stop, goTo::none, goBy::avoid, 0}));}
};
template <> struct itemTypeTraits<itemTypeKey::ship> {
  typedef basicTransport type;
  template<typename type>
  static item *make(const itemType &t) { return new type(t, terrainType::WATER, terrainType::GROUND, 
					    movementType({speed::slow3, goTo::player, goBy::avoid, 0})); }
};

template <itemTypeKey t, class T>
item &createItem();

// create an item of the given type. io may be used later by that item, eg for prompts when using.
// TODO: Randomness for flavour: enchantment, flags, etc.
template <itemTypeKey t>
item &createItem() {
  return createItem<t, itemTypeTraits<t> >();
}

template <itemTypeKey t, class T>
item &createItem() {
  //item& createItem(const itemTypeKey & t) {
  auto &r = itemTypeRepo::instance();
  typedef typename T::type concreteType;
  item *rtn = T::template make<concreteType>(r[t]);
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn; // now safe to take reference
}

item & createEquippable(const itemTypeKey &t, sharedAction<item,monster>::key of) {
  auto &action = actionFactory<item, monster>::get(of);
  //  auto &item = createItem<t, equippableTypeTraits<t> >();
  auto &item = createItem(t);
  dynamic_cast<onEquipMixin&>(item).equipAction(action);
  return item;
}


template<questItemType it>
struct questItemTypeTraits{};

template<questItemType it>
class basicQuestItem : public item, virtual public shared_item {
private:
  typedef questItemTypeTraits<it> traits;
public:
  basicQuestItem() {}
  basicQuestItem(const basicQuestItem &) = delete;
  basicQuestItem(basicQuestItem &&) = delete;
  //quest items should be highlighted:
  virtual bool highlight() const { return true; }
  virtual const wchar_t render() const {
    return traits::render_;
  }
  virtual std::wstring name() const {
    return traits::name_;
  }
  virtual std::wstring typeDescription() const {
    return traits::desc_;
  }
  virtual materialType material() const {
    return traits::mat_;
  }
  virtual double weight() const {
    return traits::weight_;
  }
  virtual damageType weaponDamage(bool use) {
    return traits::weaponDamage_;
  }
  virtual std::vector<std::wstring> adjectives() const {
    return std::vector<std::wstring>(); // by default
  }
  // indestructibility:
  virtual bool strike(const damageType &) { return false; }
  virtual bool repair(const damageType &) { return true; }
  virtual bool proof(const damageType &) { return true; }
  virtual bool isProof(const damageType &) const { return true; }
  virtual bool isBlessed() const { return true; }
  virtual bool isCursed() const { return false; }
  virtual void bless(bool b) { }
  virtual void curse(bool b) { }
  virtual bool isUnidentified() const { return false; }
  virtual void unidentify(bool b) { }
  virtual bool isSexy() const { return true; }
  virtual void sexUp(bool b) { }
  virtual int enchantment() const { return 5; };
  virtual void enchant(int) { }
  virtual int damageOfType(const damageType &) const { return 0; }
  virtual long modDamage(long pc, const damage &) const {
    return pc; // per default
  }
};

template<>
struct questItemTypeTraits<questItemType::grail>{
  typedef basicQuestItem<questItemType::grail> type;
  static constexpr const wchar_t render_ = L'('; // utility
  static constexpr const wchar_t * const name_ = L"Holy Grail"; // must contain "Grail" (shop quest)
  static constexpr const wchar_t * const desc_ = L"";
  static constexpr const materialType mat_ = materialType::stony;
  static constexpr const double weight_ = 100; // TODO
  static constexpr const damageType weaponDamage_ = damageType::bashing;
};

template<questItemType it>
item & createQuestItem() {
  auto rtn = new typename questItemTypeTraits<it>::type;
  itemHolderMap::instance().enroll(*rtn);
  return *rtn;
}

// ensure object templates are created:
void dummy() {
  createQuestItem<questItemType::grail>();
}

item & createHolyBook(const deity &align) {
  auto rtn = new holyBook(align);
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn;
}

item &createCorpse(const monsterType &mt, const unsigned char maxDamage) {
  auto rtn = new corpse(mt, maxDamage);
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn;
}

template <itemTypeKey type>
item & createBottledItem() {
  auto &rtn = createItem<itemTypeKey::bottle>();
  auto &toBottle = createItem<type>();
  dynamic_cast<bottle &>(rtn).addItem(toBottle);
  return rtn;
}
item & createRndBottledItem(const int depth) {
  auto &rtn = createItem<itemTypeKey::bottle>();
  auto &toBottle = createRndItem(depth, L'~');
  dynamic_cast<bottle &>(rtn).addItem(toBottle);
  return rtn;
}

item & createWand(sharedAction<monster,monster>::key of) {
  auto &action = actionFactory<monster, monster>::get(of);
  // non-random wands are initially created with 1-5 charges:
  auto rtn = new wand(dPc() / 20, action);
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn;
}

item & createIou(const double amount, const std::wstring &whom, const std::wstring &service) {
  auto rtn = new iou(amount, whom, service);
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn;
}

double forIou(const item &i, double d, std::wstring &buf) {// used in shop.cpp
  const iou *ii = dynamic_cast<const iou*>(&i);
  if (ii) {
    buf = buf + L"\t" + ii->service() + L"\n";
    return ii->amount() + d;
  } else
    return d;
}

item &createRndEquippable(const itemTypeKey &type) {
  typedef sharedAction<item,monster>::key key;
  key of = static_cast<key>(rndPickI(0, static_cast<int>(key::END)));
  return createEquippable(type, of);
}


// create a random item suitable for the given level depth
// TODO: depth limitations
item &createRndItem(const int depth, bool allowLiquids) {
  auto &r = itemTypeRepo::instance();
  while (true) {
    auto type = rndPick(r.begin(), r.end());
    // we can produce water, but we must bottle it:
    if (type->first == itemTypeKey::water && !allowLiquids)
      return createBottledItem<itemTypeKey::water>();
    // other more exotic liquids are usually ignored:
    if (!allowLiquids && r[type->first].material() == materialType::liquid) continue;
    // we never autogenerate a corpse because they always need a monster first:
    if (type->first == itemTypeKey::corpse) continue;
    // we never autogenerate an IOU; they're only created by shops
    if (type->first == itemTypeKey::iou) continue;
    // don't autogenerate napsacks of consumption; they're for Dungeoneers:
    if (type->first == itemTypeKey::napsack_of_consumption) continue;
    // don't autogenerate water transport:
    if (type->first == itemTypeKey::bridge) continue;
    if (type->first == itemTypeKey::ship) continue;
    // some jewellery is magic:
    if (type->second->render() == L'*' && dPc() < depth)
      return createRndEquippable(type->first);
    // general case: call createItem():
    return createItem(type->first); // already enrolled
  }
}


// giant switch ensures every case is compiled in, and means we can create items
// of dynamic type:
item &createItem(const itemTypeKey &key) {
  switch (key) {
  case itemTypeKey::apple: return createItem<itemTypeKey::apple>();
  case itemTypeKey::bread: return createItem<itemTypeKey::bread>();
  case itemTypeKey::cake: return createItem<itemTypeKey::cake>();
    //  case itemTypeKey::corpse: return createItem<itemTypeKey::corpse>();
  case itemTypeKey::pie: return createItem<itemTypeKey::pie>();
  case itemTypeKey::joint: return createItem<itemTypeKey::joint>();
  case itemTypeKey::steak: return createItem<itemTypeKey::steak>();
  case itemTypeKey::mace: return createItem<itemTypeKey::mace>();
  case itemTypeKey::two_sword: return createItem<itemTypeKey::two_sword>();
  case itemTypeKey::flamethrower: return createItem<itemTypeKey::flamethrower>();
  case itemTypeKey::nitrogen_tank: return createItem<itemTypeKey::nitrogen_tank>();
  case itemTypeKey::bubble_gun: return createItem<itemTypeKey::bubble_gun>();
  case itemTypeKey::sonic_cannon: return createItem<itemTypeKey::sonic_cannon>();
  case itemTypeKey::maser: return createItem<itemTypeKey::maser>();
  case itemTypeKey::taser: return createItem<itemTypeKey::taser>();
  case itemTypeKey::rock: return createItem<itemTypeKey::rock>();
  case itemTypeKey::bow: return createItem<itemTypeKey::bow>();
  case itemTypeKey::boots: return createItem<itemTypeKey::boots>();
  case itemTypeKey::cloak: return createItem<itemTypeKey::cloak>();
  case itemTypeKey::crupper: return createItem<itemTypeKey::crupper>();
  case itemTypeKey::doublet: return createItem<itemTypeKey::doublet>();
  case itemTypeKey::flanchard: return createItem<itemTypeKey::flanchard>();
  case itemTypeKey::haubergeon: return createItem<itemTypeKey::haubergeon>();
  case itemTypeKey::hauberk: return createItem<itemTypeKey::hauberk>();
  case itemTypeKey::helmet: return createItem<itemTypeKey::helmet>();
  case itemTypeKey::jerkin: return createItem<itemTypeKey::jerkin>();
  case itemTypeKey::peytral: return createItem<itemTypeKey::peytral>();
  case itemTypeKey::robe: return createItem<itemTypeKey::robe>();
  case itemTypeKey::saddle: return createItem<itemTypeKey::saddle>();
  case itemTypeKey::scabbord: return createItem<itemTypeKey::scabbord>();
  case itemTypeKey::shirt: return createItem<itemTypeKey::shirt>();
  case itemTypeKey::shorts: return createItem<itemTypeKey::shorts>();
  case itemTypeKey::skirt: return createItem<itemTypeKey::skirt>();
  case itemTypeKey::socks: return createItem<itemTypeKey::socks>();
  case itemTypeKey::trousers: return createItem<itemTypeKey::trousers>();
  case itemTypeKey::tshirt: return createItem<itemTypeKey::tshirt>();
  case itemTypeKey::underpants: return createItem<itemTypeKey::underpants>();
  case itemTypeKey::stick: return createItem<itemTypeKey::stick>();
  case itemTypeKey::bottle: return createItem<itemTypeKey::bottle>();
  case itemTypeKey::codex: return createItem<itemTypeKey::codex>();
  case itemTypeKey::hitch_guide: return createItem<itemTypeKey::hitch_guide>();
  case itemTypeKey::holy_book: return createItem<itemTypeKey::holy_book>();
    //  case itemTypeKey::iou: return createItem<itemTypeKey::iou>();
  case itemTypeKey::poke: return createItem<itemTypeKey::poke>();
  case itemTypeKey::napsack_of_consumption: return createItem<itemTypeKey::napsack_of_consumption>();
  case itemTypeKey::water: return createItem<itemTypeKey::water>();
  case itemTypeKey::tears: return createItem<itemTypeKey::tears>();
  case itemTypeKey::heavy_water: return createItem<itemTypeKey::heavy_water>();
  case itemTypeKey::fire_water: return createItem<itemTypeKey::fire_water>();
  case itemTypeKey::pop: return createItem<itemTypeKey::pop>();
  case itemTypeKey::fizzy_pop: return createItem<itemTypeKey::fizzy_pop>();
  case itemTypeKey::dehydrated_water: return createItem<itemTypeKey::dehydrated_water>();
  case itemTypeKey::spring_water: return createItem<itemTypeKey::spring_water>();
  case itemTypeKey::electro_pop: return createItem<itemTypeKey::electro_pop>();
  case itemTypeKey::wooden_ring: return createItem<itemTypeKey::wooden_ring>();
  case itemTypeKey::amulet: return createItem<itemTypeKey::amulet>();
  case itemTypeKey::necklace: return createItem<itemTypeKey::necklace>();
  case itemTypeKey::tikka: return createItem<itemTypeKey::tikka>();
  case itemTypeKey::spectacles: return createItem<itemTypeKey::spectacles>();
  case itemTypeKey::bracelet: return createItem<itemTypeKey::bracelet>();
  case itemTypeKey::anklet: return createItem<itemTypeKey::anklet>();
  case itemTypeKey::cloth_gloves: return createItem<itemTypeKey::cloth_gloves>();
  case itemTypeKey::armband: return createItem<itemTypeKey::armband>();
  case itemTypeKey::kalganid: return createItem<itemTypeKey::kalganid>();
  case itemTypeKey::gpl_slip: return createItem<itemTypeKey::gpl_slip>();
  case itemTypeKey::gpl_strip: return createItem<itemTypeKey::gpl_strip>();
  case itemTypeKey::gpl_bar: return createItem<itemTypeKey::gpl_bar>();
  case itemTypeKey::gpl_brick: return createItem<itemTypeKey::gpl_brick>();
  case itemTypeKey::shop_card: return createItem<itemTypeKey::shop_card>();
  case itemTypeKey::bottling_kit: return createItem<itemTypeKey::bottling_kit>();
  case itemTypeKey::theremin: return createItem<itemTypeKey::theremin>();
  case itemTypeKey::visi_sonor: return createItem<itemTypeKey::visi_sonor>();
  case itemTypeKey::baliset: return createItem<itemTypeKey::baliset>();
  case itemTypeKey::drum: return createItem<itemTypeKey::drum>();
  case itemTypeKey::bagpipes: return createItem<itemTypeKey::bagpipes>();
  case itemTypeKey::conch: return createItem<itemTypeKey::conch>();
  case itemTypeKey::harmonica: return createItem<itemTypeKey::harmonica>();
  case itemTypeKey::pan_flute: return createItem<itemTypeKey::pan_flute>();
  case itemTypeKey::lyre: return createItem<itemTypeKey::lyre>();
  case itemTypeKey::pianoforte: return createItem<itemTypeKey::pianoforte>();
  case itemTypeKey::bridge: return createItem<itemTypeKey::bridge>();
  case itemTypeKey::ship: return createItem<itemTypeKey::ship>();
  default: throw key;
  }
}
