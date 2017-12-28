/* License and copyright go here*/

// Things and stuff

#include "items.hpp"
#include "damage.hpp"
#include "random.hpp"
#include "monster.hpp"
#include "shop.hpp"
#include "encyclopedia.hpp" // for hitch-hiker's guide
#include "religion.hpp" // for holy books
#include "output.hpp"
#include "action.hpp"
#include <set>
#include <bitset>
#include <algorithm>




extern std::vector<damageType> allDamageTypes;

// mixin class for things which burn charges; see basicItem below.
class burnChargeMixin : virtual public shared_item {
protected:
  bool hasCharge() const {
    return shared_from_this()->enchantment() > 0;
  }
  void useCharge() {
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

// class for items with no especial behaviour:
class basicItem : public item {
private:
  enum { blessed, cursed, unidentified, sexy, NUM_FLAGS } flags;
  const itemType& type_;
  std::map<damageType, int> damageTrack_;
  // what are we *explicitly* proof against?
  // something may be proof against a material even with no damage track; this may
  // become useful in later development (eg transferring proofs between objects or
  // polymorping the type and/or material of the object). Generally the existance
  // of a damage track should be checked first.
  std::set<damageType> proof_;
  std::bitset<NUM_FLAGS> flags_;
  int enchantment_;
protected:
  mutable std::wstring buffer_; // for transient returns.
public:
  basicItem(const itemType& type) :
    item(),
    type_(type),
    enchantment_(0) {
    const damageRepo &dr = damageRepo::instance();
    for (auto dt : allDamageTypes)
      if (dr[dt].canDamage(type_.material()))
	damageTrack_.emplace(dt, 0);
  }
  basicItem(const basicItem &other) = delete;
  virtual ~basicItem() {
  };
  // delegate to itemType by default
  virtual const wchar_t render() const {
    return type_.render();
  }
  // return the simple name for this item type; overridden in corpse.
  virtual const wchar_t * const simpleName() const {
    return type_.name();
  }
  // built up of itemType and adjectives etc.
  virtual const wchar_t * const name() const {
    buffer_ = L"";
    if (enchantment_ < 0) buffer_ += std::to_wstring(enchantment_) + L' ';
    if (enchantment_ > 0) buffer_ += L'+' + std::to_wstring(enchantment_) + L' ';
    for (auto a : adjectives())
      buffer_ += a + L" ";
    buffer_ += simpleName();
    return buffer_.c_str();
  }
  // built up of all visible properties.
  virtual const wchar_t * const description() const {
    name(); // sets buffer_
    const std::size_t len = buffer_.length();
    buffer_ += L"\n";
    buffer_ += std::wstring(len, L'=');
    if (isUnidentified()) {
      buffer_ += type_.vagueDescription();
      return buffer_.c_str();
    }
    buffer_ += L"\n\nDescription:\n";
    buffer_ += type_.description();
    buffer_ += + L"\n\nWeight: ";
    buffer_ += std::to_wstring(weight());
    buffer_ += L"N\n";

    const bool blessed = isBlessed(), cursed = isCursed();
    if (blessed && !cursed)
      buffer_ += L"This is blessed. Most blessed items provide 1.5 times the\n"
	"effect of a similar non-blessed, non-cursed items.\n";
    else if (cursed && !blessed)
      buffer_ += L"This is cursed. Most cursed items provide half the effect\n"
	"of a similar non-blessed, non-cursed items. They cannot be removed\n"
	"if worn/wielded until the curse is removed.\n";
    else if (blessed && cursed)
      buffer_ += L"This is both blessed and cursed. The blessing removes some\n"
	"effects of the curse, but does not void it. Cursed items cannot be\n"
	"removed if worn/wielded until the curse is removed.\n";
    else
      buffer_ += L"This would be more effective if blessed.\n";

    const int enchantmentPc = 5 * enchantment();
    auto *burnCharge = dynamic_cast<const burnChargeMixin*>(this);
    if (!burnCharge) {
      if (enchantmentPc < 0) {
	buffer_ += L"There is a negative enchantment. There is a penalty of ";
	buffer_ += enchantmentPc;
	buffer_ += L"% when\nusing this item as weapon or armour.\n";
      } else if (enchantmentPc == 0) {
	buffer_ += L"There is no magical enchantment on this item.\n";
      } else {
	buffer_ += L"There is an enchantment. There is a bonus of ";
	buffer_ += enchantmentPc;
	buffer_ += L"% when\nUsing this item as weapon or armour.\n";
      }
    } else {
      buffer_ += burnCharge->describeCharges();
    }

    return buffer_.c_str();
  }

  virtual itemHolder& holder() const {
    auto &map = itemHolderMap::instance();
    // sanity check to avoid null reference:
    if (map.beforeFirstAdd(*this))
      throw std::wstring(name()) + L" not yet in any holder";
    return map.forItem(*this);
  }

  // what is the object made of?
  virtual materialType material() const {
    return type_.material();
  }
  // hom much does it weigh?
  virtual double weight() const {
    // basic: just return the base weight
    return type_.baseWeight();
  }
  virtual damageType weaponDamage() const {
    return damageType::bashing;
  }
  virtual int damageOfType(const damageType &type) const {
    auto i = damageTrack_.find(type);
    if (i == damageTrack_.end()) return 0;
    return i->second;
  }
  // list of all adjectives applicable to type
  virtual std::vector<std::wstring> adjectives() const {
    std::vector<std::wstring> rtn;
    if (isUnidentified())
      return rtn;
    if (type_ == itemTypeRepo::instance()[itemTypeKey::spring_water])
      rtn.push_back(L"natural");
    if (isBlessed()) rtn.push_back(L"blessed");
    if (isCursed()) rtn.push_back(L"cursed");
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
      if (d == 1) rtn.push_back(std::wstring(L"a little ") + adj);
      if (d == 2) rtn.push_back(std::wstring(adj));
      if (d == 3) rtn.push_back(std::wstring(L"very ") + adj);
      if (d >= 4) rtn.push_back(std::wstring(L"thoroughly ") + adj);
    }
    return rtn;
  }

  // damage the item in some way (return false only if no effect)
  virtual bool strike(const damageType &type) {
    if (isProof(type)) return false;
    auto i = damageTrack_.find(type);
    if (i == damageTrack_.end()) return false;
    ++(i->second); // TODO: many items should be destroyed if they get too damaged. Do it here or transmute won't work.
    return true;
  }
  // repair previous damage (return false only if no effect, eg undamaged)
  virtual bool repair(const damageType &type) {
    // don't check proof in case a subclass provides for another way of damaging
    auto i = damageTrack_.find(type);
    if (i == damageTrack_.end()) return false;
    if (i->second == 0) return false;
    --(i->second);
    return true;
  }
  // proof against dagage type  (return false only if no more effect possible, eg already proofed or n/a for material type)
  virtual bool proof(const damageType &type) {
    auto i = proof_.find(type);
    if (i == proof_.end())
	proof_.insert(type);
    return true;
  }
  // are we fooproof?
  virtual bool isProof(const damageType &type) const {
    auto i = proof_.find(type);
    return i != proof_.end();
  }

  // access flags:
  virtual bool isBlessed() const {
    return flags_[blessed];
  }
  virtual void bless(bool b) {
    flags_[blessed] = b;
  }
  virtual bool isCursed() const {
    return flags_[cursed];
  }
  virtual void curse(bool c) {
    flags_[cursed] = c;
  }
  virtual bool isSexy() const {
    return flags_[sexy];
  }
  virtual void sexUp(bool s) {
    flags_[sexy] = s;
  }
  virtual bool isUnidentified() const {
    return flags_[unidentified];
  }
  virtual void unidentify(bool forget) {
    flags_[unidentified] = forget;
  }

  // enchantment is some +/- modifier for the item; adds to attack when wielded
  // If positive, may also be the number of charges remaining in a limited-use item
  virtual int enchantment() const {
    return enchantment_;
  }
  virtual void enchant(int enchantment) {
    // TODO: We need to signal to recalculate the bonus if this item is equipped
    enchantment_ += enchantment;
  }

  virtual bool equip(monster &owner) {
    return false; // can't equip this item type by default
  }
  virtual equipType equippable() const { 
    return equipType::none; 
  }
  // destroy an item in inventory
  virtual void destroy() {
    if (holder().contains(*this) )
      holder().destroyItem(*this);
    else
      throw L"Item not in supplied container"; // should not fall through if procondition met
  }

  // try to use the object
  virtual bool use () {
    return false; // no effect by default
  }

  virtual std::set<slotType>slots() {
    std::set<slotType> empty;
    return empty;
  }

  virtual long modDamage(long pc, const damage & type) const {
    return pc;
  }

};


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
	if (!use(i))
	  ios.message(L"That doesn't seem to work.");
    };
    itemHolder.forEachItem(f);
    return true;
  }

  // try to use the object with another (eg put object into container; put candles on candlestick)
  virtual bool use(item &other) {
    return false; // no effect by default
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
  virtual const wchar_t *const actionName() const { return action_.name(); }
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
class basicEquip : public basicItem {
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
      if (owner.equip(*this, s))
	return true;
    return false;
  }
  virtual equipType equippable() const {
    return static_cast<equipType>(equipTyp);
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
  virtual damageType weaponDamage() const {
    return damageType_;
  }
  virtual equipType equippable() const {
    return equipType::wielded;
  }
};

class basicThrown : public basicWeapon {
  // TODO: Throwability. For now, we just wield them menacingly...
public:
  basicThrown(const itemType & type,  const damageType damage) :
    basicWeapon(type, damage) {}
  virtual ~basicThrown() {};
};

// as basicEquip, but requiring 2 slots
template<int equipTyp>
class twoEquip : public basicItem {
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
      if (owner.equip(*this, s))
	return true;
    return false;
  }
  virtual equipType equippable() const {
    return static_cast<equipType>(equipTyp);
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
  virtual damageType weaponDamage() const {
    return damageType_;
  }
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
public:
  transmutedWater(const itemType &type,  const damageType &toRepair)
    : basicItem(type), toRepair_(toRepair) {}
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
class bottle : public basicItem, private itemHolder {
  friend class bottlingKit;
private:
  optionalRef<item> content() {
    optionalRef<item> rtn;
    forEachItem([&rtn](item &i, std::wstring) {
	rtn = optionalRef<item>(i);
      });
    return rtn;
  }
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
  virtual const wchar_t * const name() const {
    optionalRef<const item> c = content();
    basicItem::name(); // sets buffer();
    if (c) {
      auto cName = c.value().name();
      if (cName == std::wstring(L"ship")) {
	buffer_ = cName + std::wstring(L" in a ") + buffer_;
      } else {
	buffer_ += L" of ";
	buffer_ += cName;
      }
    } else {
      buffer_ = std::wstring(L"empty ") + buffer_;
    }
    return buffer_.c_str();
  }
  virtual double weight() const {
    double baseWeight = basicItem::weight();
    /* items in bottle are in miniature. So you get an item reduced to 2% of its weight (1:50 scale) for a normal bottle, 1% for blessed, 200% for cursed.*/
    double multiplicand;
    if (isCursed()) multiplicand = 2 * (1 + std::abs(enchantment()));
    else if (isBlessed()) multiplicand = 0.01 - (0.005 * enchantment());
    else multiplicand = 0.02;
    return baseWeight * multiplicand;
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
      ios.message(std::wstring(name()) + L" smashes, and the emptiness gets out"); // Hmmm; spawn a vacuum monster?
    } else if (c.value().material() == materialType::liquid) {
      ios.message(std::wstring(name()) + L" smashes; there's fluid everywhere");
      itemHolder::destroyItem(c.value());
    } else {
      auto cname = c.value().name();
      // TODO: if it's a ship, and if we're at a dock, we should launch it itstead
      if (holder().addItem(c.value())) {
	ios.message(std::wstring(name()) + L" smashes; you now have a " + cname);
      } else {
      itemHolder::destroyItem(c.value());
	ios.message(std::wstring(name()) + L" smashes; you lose the " + cname);
      }
    }
    if (content()) throw L"Destroying bottle without losing its contents!";
    basicItem::destroy();
  }
  virtual bool use() {
    if (content()) {
      auto *pc = dynamic_cast<monster*>(&holder());
      auto &ios = ioFactory::instance();
      if (pc == 0 || !pc->isPlayer() || ios.ynPrompt(L"Smash the " + std::wstring(name()) + L"?"))
	destroy();
    }
    return true;
  }
  // Note that, unlike basicContainer, this isn't use-with-item; the player can't
  // simply create their own bottles:
  virtual bool addItem(item &i) {
    if (content()) return false; // already occupied
    return itemHolder::addItem(i);
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

  virtual bool use() {
    const std::wstring name(basicItem::name());
    bool success = true;
    success &=useWithMixin::use(name, L"Put ", L"into", holder()); // calls use(item) as needed
    return success && useWithMixin::use(name, L"Take ", L"from", *this); // calls use(item) as needed
  }
  // put into or take out of bag:
  virtual bool use(item &other) {
    if (!contains(other)) // not in this container, so put in
      itemHolder::addItem(other);
    else // in this container, so take out (add to *our* holder)
      holder().addItem(other);
    return true;
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
  virtual bool use() {
    if (isCursed()) return false; // seems a bit harsh, but I don't have a better idea yet
    ::invokeGuide(isBlessed());
    return true;
  }
  /*
  virtual bool use(std::shared_ptr<item> other) {
    if (isCursed()) return false; // seems a bit harsh, but I don't have a better idea yet
    ::invokeGuide(ios(), isBlessed(), other);
    return true;
    }*/
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
  virtual const wchar_t * const name() const {
    if (align_.nonaligned())
      // ref: Russell's Teapot, a phylosophical analogy used to place the buden of proof on the preacher.
      return L"Book of the Teapot";
    buffer_ = L"holy book of " + std::wstring(align_.name());
    return buffer_.c_str();
  }
  virtual bool use() {
    auto &ios = ioFactory::instance();
    if (isCursed()) {
      ios.message(L"The pages are stuck together.");
      return false;
    }
    ios.message(L"You turn the pages of the " + std::wstring(name()));
    ios.longMsg(align_.description());
    return true;
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
  virtual const wchar_t * const name() {
    buffer_ = L"I.O.U. (";
    buffer_ += whom_ + L")";
    return buffer_.c_str();
  }
  virtual damageType weaponDamage() {
    return damageType::electric; // ref: nethack has a ball-and-chain punishment, which makes a reasonable weapon...
  }
  virtual bool use() {
    std::wstring msg = name();
    msg += L"\nFor services rendered:\n\n\t";
    msg += service_;
    msg += L"\n\nIf payment is not received within 100 turns of issue, debt\n"
      "collectors will be deployed.\nYou have been warned.\n\n"
      "Payment may be made to any Platinum Express Shop"; // ref: see platinum express card
    auto &ios = ioFactory::instance();
    ios.longMsg(msg);
    return true;
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

class shopCard : public basicItem {
public:
  shopCard(const itemType& type) :
    basicItem(type) {}  
  virtual ~shopCard(){}
  virtual bool use() {
    auto m = dynamic_cast<monster*>(&holder());
    if (m)
      ::goShopping(*m);
    return m;
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
  virtual bool use() {
    auto &ios = ioFactory::instance();
    if (!hasCharge()) {
      ios.message(L"You are out of bottle caps.");
      return false;
    }
    auto &pc = dynamic_cast<monster&>(holder());
    optionalRef<item> bot = pc.firstItem([](item &i) {
	auto *bot = dynamic_cast<bottle*>(&i);
	return bot != 0 && bot->content();
      });
    if (!bot) {
      ios.message(L"You'll need a bottle in your inventory to do that.");
      return false;
    }
    ios.message(std::wstring(L"You try the ") + bot.value().name());
    bottle &b = dynamic_cast<bottle&>(bot.value());
    auto found = pc.firstItem([&b, this, &ios](item &i) {
	if (&i == this) return false; // can't bottle a bottling kit with itself
	if (&i == &b) return false; // can't bottle a bottle into itself
	if (ios.ynPrompt(std::wstring(L"Bottle ") + i.name() + L"?")) {
	  if (b.addItem(i))
	    return true;
	  else ios.message(L"It didn't fit."); // just in case
	}
	return false;
      });
    if (found) useCharge();
    return found;
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
    // TODO: delegate to monsterType? More code but better encapsulated
    if (type_.type() == monsterTypeKey::troll) return materialType::stony;
    return materialType::fleshy;
  }
  //virtual double weight() {
  //return 610; // weight of average human in N Earth gravity (610lb). TODO: per type somehow
  //}
  // if wielded, what damage does this weapon do?
  //virtual damageType weaponDamage() const = 0; // TODO: sharp monsters? electric?
  // list of all adjectives applicable to type
  virtual std::vector<std::wstring> adjectives() const {
    auto baseAdjectives = basicItem::adjectives();
    std::vector<std::wstring> adjectives;
    adjectives.push_back(L"dead");
    adjectives.insert(adjectives.end(), baseAdjectives.begin(), baseAdjectives.end());
    return adjectives;
  }
  virtual const wchar_t * const simpleName() const {
    return type_.name(maxDamage_); // overridden to change type_ from itemType to monsterType.
  }
  virtual bool use() {
    auto &ios = ioFactory::instance();
    ios.message(L"Please don't; it's not that kind of game.");
    return false;
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
  virtual const wchar_t *const simpleName() const {
    return hasCharge() ? L"wand" : basicItem::simpleName();
  }
  virtual const wchar_t *const name() const {
    basicItem::name(); // sets buffer_
    if (hasCharge()) {
      buffer_ += L" of ";
      buffer_ += actionMonsterMixin::actionName();
    }
    return buffer_.c_str();
  }
  virtual bool use() {
    if (!hasCharge()) return false;
    if (isCursed()) {
      // fail to work about half the time:
      if (dPc() < 50) {
	if (isBlessed()) return false; // blessed cursed wands don't lose charges when they fail to work
	useCharge();
	return false; // plain cursed wands do lose charges if they fail to work
      }
    }
    if (!isBlessed() || dPc() < 50) // blessed wands use a charge only 50% of the time, normal wands 100%
      useCharge();
    return fire();
  }
};


// create an item of the given type. io may be used later by that item, eg for prompts when using.
// TODO: Randomness for flavour: enchantment, flags, etc.
item& createItem(const itemTypeKey & t) {
  auto &r = itemTypeRepo::instance();
  item *rtn;
  switch(t) {
  case itemTypeKey::apple:
    rtn = new basicItem(r[t]);
    break;
    //case itemTypeKey::corpse: // not handled here; we do this when a monster dies
  case itemTypeKey::mace:
    rtn = new basicWeapon(r[t], damageType::bashing);
    break;
  case itemTypeKey::two_sword:
    rtn = new twoHandedWeapon(r[t], damageType::edged); // we specifically chose a historical 2-handed cutting sword to annoy the reenactment purists :)
    break;
  case itemTypeKey::rock:
    rtn = new basicThrown(r[t], damageType::bashing);
    break;
  case itemTypeKey::bow:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::hat, slotType::tail);
    break;
  case itemTypeKey::boots:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shoes);
    break;
  case itemTypeKey::cloak:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::robe);
    break;
  case itemTypeKey::crupper:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shorts);
    break;
  case itemTypeKey::doublet:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::doublet);
    break;
  case itemTypeKey::flanchard:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::jerkin);
    break;
  case itemTypeKey::haubergeon:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::hauburk);
    break;
  case itemTypeKey::hauberk:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::hauburk);
    break;
  case itemTypeKey::helmet:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::hat);
    break;
  case itemTypeKey::jerkin:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::jerkin);
    break;
  case itemTypeKey::peytral:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::robe);
    break;
  case itemTypeKey::robe:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::robe);
    break;
  case itemTypeKey::saddle:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::saddle);
    break;
  case itemTypeKey::scabbord:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::belt);
    break;
  case itemTypeKey::shirt:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shirt);
    break;
  case itemTypeKey::shorts:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shorts);
    break;
  case itemTypeKey::skirt:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shorts);
    break;
  case itemTypeKey::socks:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::socks);
    break;
  case itemTypeKey::trousers:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::shorts);
    break;
  case itemTypeKey::tshirt:
    rtn = new tshirt();
    break;
  case itemTypeKey::underpants:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::underwear);
    break;
  case itemTypeKey::stick: {
    auto &action = actionFactory<monster, monster>::get(static_cast<sharedAction<monster,monster>::key>(rndPickI(0, static_cast<int>(sharedAction<monster,monster>::key::END))));
    // random wands are initially created as sticks, must be enchanted to use:
    rtn = new wand(0, action);
    break;
  }
  case itemTypeKey::bottle:
    rtn = new bottle(r[t]);
    break;
  case itemTypeKey::codex:
    rtn = new readableItem(r[t]);
    break;
  case itemTypeKey::hitch_guide:
    rtn = new hitchGuide(r[t]);
    break;
  case itemTypeKey::holy_book:
    rtn = new holyBook();
    break;
    //   case itemTypeKey::iou: // not handled here
  case itemTypeKey::poke:
    rtn = new basicContainer(r[t]);
    break;
  case itemTypeKey::water:
    rtn = new basicItem(r[t]);
    break;
  case itemTypeKey::tears:
    rtn = new transmutedWater(r[t], damageType::edged);
    break;
  case itemTypeKey::heavy_water:
    rtn = new transmutedWater(r[t], damageType::bashing);
    break;
  case itemTypeKey::fire_water:
    rtn = new transmutedWater(r[t], damageType::hot);
    break;
  case itemTypeKey::pop:
    rtn = new transmutedWater(r[t], damageType::wet);
    break;
  case itemTypeKey::fizzy_pop:
    rtn = new transmutedWater(r[t], damageType::sonic);
    break;
  case itemTypeKey::dehydrated_water:
    rtn = new transmutedWater(r[t], damageType::disintegration);
    break;
  case itemTypeKey::spring_water:
    rtn = new transmutedWater(r[t], damageType::starvation);
    break;
  case itemTypeKey::electro_pop:
    rtn = new transmutedWater(r[t], damageType::electric);
    break;
  case itemTypeKey::wooden_ring:
    rtn = new basicEquip<item::equipType::worn>(r[t], slotType::ring_left_thumb, slotType::ring_left_index, slotType::ring_left_middle, slotType::ring_left_ring, slotType::ring_left_little, slotType::ring_right_thumb, slotType::ring_right_index, slotType::ring_right_middle, slotType::ring_right_ring, slotType::ring_right_little, slotType::toe_left_thumb, slotType::toe_left_index, slotType::toe_left_middle, slotType::toe_left_fourth, slotType::toe_left_little, slotType::toe_right_thumb, slotType::toe_right_index, slotType::toe_right_middle, slotType::toe_right_fourth, slotType::toe_right_little);
    break;
  case itemTypeKey::kalganid:
    rtn = new basicItem(r[t]); // TODO: should we be able to equip coins on our eyes?
    break;
  case itemTypeKey::shop_card:
    rtn = new shopCard(r[t]);
    break;
  case itemTypeKey::bottling_kit:
    rtn = new bottlingKit(r[t]);
    break;
  default:
    throw t; // unknown type
  }
  itemHolderMap::instance().enroll(*rtn); // takes ownership
  return *rtn; // now safe to take reference
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

item & createBottledItem(const itemTypeKey &type) {
  auto &rtn = createItem(itemTypeKey::bottle);
  auto &toBottle = createItem(type);
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

// create a random item suitable for the given level depth
// TODO: depth limitations
item &createRndItem(const int depth) {
  auto &r = itemTypeRepo::instance();
  while (true) {
    auto type = rndPick(r.begin(), r.end());
    // we can produce water, but we must bottle it:
    if (type->first == itemTypeKey::water)
      return createBottledItem(itemTypeKey::water);
    // other more exotic liquids are ignored for now:
    if (r[type->first].material() == materialType::liquid) continue;
    // we never autogenerate a corpse because they always need a monster first:
    if (type->first == itemTypeKey::corpse) continue;
    // we never autogenerate an IOU; they're only created by shops
    if (type->first == itemTypeKey::iou) continue;
    // general case: call createItem():
    return createItem(type->first); // already enrolled
  }
}
