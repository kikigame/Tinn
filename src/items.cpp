/* License and copyright go here*/

// Things and stuff

#include "items.hpp"
#include "damage.hpp"
#include "random.hpp"
#include "monster.hpp"
#include "shop.hpp"
#include "encyclopedia.hpp"
#include "output.hpp"
#include <set>
#include <bitset>
#include <algorithm>


// when using Multiple Inheritance with shared_from_this, you need to use a common virtual base-class
// for shared_from_this, so that any derived class can create shared pointers in one group.
class shared_item : public std::enable_shared_from_this<item> {
};


extern std::vector<damageType> allDamageTypes;

// class for items with no especial behaviour:
class basicItem : public item, virtual public shared_item {
private:
  enum { blessed, cursed, unidentified, sexy, NUM_FLAGS } flags;
  const itemType& type_;
  const io& io_;
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
  itemHolder *holder_; // where this item is
public:
  basicItem(const itemType& type, const io &ios) :
    item(),
    type_(type),
    io_(ios),
    enchantment_(0),
    holder_(0) {
    const damageRepo &dr = damageRepo::instance();
    for (auto dt : allDamageTypes)
      if (dr[dt].canDamage(type_.material()))
	damageTrack_.emplace(dt, 0);
  }
  virtual ~basicItem() {};
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
      buffer_ += L"This would be more effective if blessed.";

    const int enchantmentPc = 5 * enchantment();
    if (!type_.burnsCharges()) {
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
    if (enchantmentPc < 0) {
      buffer_ += L"There is a negative enchantment. You cannot use the magic\n"
	"in this item again until you have recharged it. There is a penalty\n"
	"of ";
      buffer_ += enchantmentPc;
      buffer_ += L"% when using this item as weapon or armour.\n";
    } else if (enchantmentPc == 0) {
      buffer_ += L"This needs recharging.\n";
    } else {
      buffer_ += L"There is an enchantment. You can use the magic in this item\n";
      buffer_ += enchantment() + L" times before it will need recharging.\n"
	"There is a bonus of ";
      buffer_ += enchantmentPc;
      buffer_ += L"% when using this item as weapon or armour.\n";
    }
    }

    return buffer_.c_str();
  }

  virtual void move(itemHolder &holderTo) {
    // do nothing if we're already moved (or in the process of moving!)
    if (holder_ == &holderTo) return;
    // take a shared pointer, so we don't expire when removing from old container
    auto pThis = shared_from_this();
    // remove from the old container first
    if (holder_){ // new items have a null holder
      auto c = holder_->contents();
      for (auto i = c.begin(); i != c.end(); ++i)
	if (&(**i) == this) {
	  c.erase(i);
	  break;
	}
    }
    // note our new container; now safe to call ourselves recursively
    holder_ = &holderTo;
    // now add to the new container; may call move() recursively as it can't set holder_
    holderTo.addItem(pThis);
  }

  virtual itemHolder& holder() const {
    return *holder_;
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
  virtual iterable<std::wstring, std::vector<std::wstring>,true > adjectives() const {
    std::vector<std::wstring> rtn;
    if (isUnidentified())
      return iterable<std::wstring, std::vector<std::wstring>, true>(rtn);
    if (isBlessed()) rtn.push_back(L"blessed");
    if (isCursed()) rtn.push_back(L"cursed");
    auto &dr = damageRepo::instance();
    auto m = type_.material();
    for (auto dt : allDamageTypes) {
      if (isProof(dt)) {
	// adjective for being (this material) being proof to this damage type:
	rtn.push_back(std::wstring(dr[dt].proofAdj(m)));
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
    return iterable<std::wstring, std::vector<std::wstring>, true>(rtn);
  }

  // damage the item in some way (return false only if no effect)
  virtual bool strike(const damageType &type) {
    if (isProof(type)) return false;
    auto i = damageTrack_.find(type);
    if (i == damageTrack_.end()) return false;
    ++(i->second); // TODO: many items should be destroyed if they get too damaged. Here or in caller?
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
  // proof against dagage type  (return false only if no effect, eg already proofed or n/a for material type)
  virtual bool proof(const damageType &type) {
    auto i = proof_.find(type);
    bool rtn = i != proof_.end();
    proof_.insert(type);
    return rtn;
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

  virtual bool equip(std::shared_ptr<monster> owner) {
    return false; // can't equip this item type by default
  }

  // TODO: destroy items
  // destroy an item in inventory
  virtual void destroy() {
    auto c = holder_->contents();
    for (auto i = c.begin(); i != c.end(); ++i)
      if (&(**i) == this) {
	c.erase(i);
	return;
      }
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

  const io & ios() {
    return io_;
  }

  virtual long modDamage(long pc, const damage & type) const {
    return pc;
  }

};

/*
 * Mixin for items that can be used with other items
 */
class useWithMixin : public virtual shared_item {
private:
  const io &io_;
public:
  useWithMixin(const io &io) : 
    io_(io) {};
  virtual bool use(const std::wstring &withName,
		   const std::wstring &action,
		   const std::wstring &preposition,
		   itemHolder &itemHolder) {
    std::function<void(std::shared_ptr<item>, std::wstring)> f = 
      [this,&withName,&action,&preposition](std::shared_ptr<item> i, std::wstring name){
      if (i == shared_from_this()) return; // can't use an item with itself
      if (io_.ynPrompt(action + name + L" "+ preposition +L" your " + withName + L"?"))
	if (!use(i))
	  io_.message(L"That doesn't seem to work.");
    };
    itemHolder.forEachItem(f);
    return true;
  }

  // try to use the object with another (eg put object into container; put candles on candlestick)
  virtual bool use(std::shared_ptr<item> other) {
    return false; // no effect by default
  }
};


/*
 * Objects that can be equipped - worn, wielded, etc.
 */
class basicEquip : public basicItem, public virtual shared_item{
private:
  // slots in which this may be equipped, in preference order
  std::set<slotType> supportedSlots_;
public:
  template <typename... S>
  basicEquip(const itemType& type, const io &ios, S... slots) :
    basicItem(type, ios),
    supportedSlots_({slots...}) {
  }
  virtual ~basicEquip() {}
  virtual bool equip(std::shared_ptr<monster> owner) {
    for (slotType s : supportedSlots_)
      if (owner->equip(shared_from_this(), s))
	return true;
    return false;
  }
  virtual std::set<slotType>slots() {
    return supportedSlots_;
  }
};

class basicWeapon : public basicEquip {
private:
  damageType damageType_;
public:
  basicWeapon(const itemType & type, const io &ios, const damageType damage) :
    basicEquip(type, ios, slotType::primary_weapon, slotType::secondary_weapon),
    damageType_(damage) {}
  virtual ~basicWeapon() {}
  damageType weaponDamage() const {
    return damageType_;
  }
};

class basicThrown : public basicWeapon {
  // TODO: Throwability. For now, we just wield them menacingly...
public:
  basicThrown(const itemType & type, const io &ios, const damageType damage) :
    basicWeapon(type, ios, damage) {}
  virtual ~basicThrown() {};
};

class bottle : public basicItem, itemHolder {
private:
  std::shared_ptr<item> content_;
public:
  bottle(const itemType& type, const io &ios) :
    basicItem(type, ios) {}
  virtual ~bottle() {}
  virtual const wchar_t * const name() const {
    basicItem::name(); // sets buffer();
    if (content_) {
      auto cName = content_->name();
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
    basicItem::destroy();
    if (!content_) {
      ios().message(std::wstring(name()) + L" smashes, and the emptiness gets out"); // Hmmm; spawn a vacuum monster?
    } else if (content_->material() == materialType::liquid) {
      content_->destroy();
    } else {
      holder_->addItem(content_);
    }
  }
  virtual bool use() {
    // TODO: if we are in a player's inventory, get a confirm prompt 
    if (content_) {
      destroy();
    }
    return true;
  }
  // interface itemHolder
  // should this be empty? bottles protect/freeze their contents utterly except through damaging the bottle.
  virtual iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > contents() {
    std::vector<std::shared_ptr<item>> contents;
    if (content_) contents.push_back(content_);
    return iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > (contents);
  }
  // Note that, unlike basicContainer, this isn't use-with-item; the player can't
  // simply create their own bottles:
  virtual bool addItem(std::shared_ptr<item> item) {
    if (content_) return false; // already occupied
    content_= item;
    return true;
  }
};


// base class for multi-item containers
class basicContainer : public basicItem, public itemOwner, public useWithMixin {
public:
  basicContainer(const itemType& type, const io &ios) :
    basicItem(type, ios),
    useWithMixin(ios) {}
  virtual ~basicContainer() {}
  virtual double weight() const {
    double rtn = basicItem::weight();
    for (auto i : ccontents())
      rtn += i->weight();
    if (isBlessed()) rtn *= 0.9;
    if (isCursed()) rtn *= 2;
    return rtn;
  }
  virtual bool strike(const damageType &type) {
    bool rtn = basicItem::strike(type);
    int target = 50;
    if (isBlessed()) target /=10;
    if (isCursed()) target *=12;
    for (auto c : contents()) // pass damage to contents
      if (dPc() < target)
	rtn |= c->strike(type);
    return rtn;
  }
  // TODO: when we destroy a container, what happens to its contents?

  virtual bool use() {
    const std::wstring name(basicItem::name());
    bool success = true;
    success &=useWithMixin::use(name, L"Put ", L"into", *holder_); // calls use(item) as needed
    return success && useWithMixin::use(name, L"Take ", L"from", *this); // calls use(item) as needed
  }
  // put into or take out of bag:
  virtual bool use(std::shared_ptr<item> other) {
    auto contents = ccontents();
    auto end = contents.end();
    auto p = std::find(contents.begin(), end, other);
    if (p == end) // not in this container, so put in
      other->move(*this);
    else // in this container, so take out
      other->move(*holder_);
    return true;
  }
  // put into bag; interface itemHolder
  virtual bool addItem(std::shared_ptr<item> other) {
    auto contents = ccontents();
    auto end = contents.end();
    auto p = std::find(contents.begin(), end, other);
    if (p != end) return false;
    other->move(*this);
    itemOwner::addItem(other);
    return true;
  }

protected:
  // may be called by subclasses, eg prismatic container
  virtual bool repairContents(const damageType &type) {
    bool rtn = basicItem::repair(type);
    int target = 50;
    if (isBlessed()) target *=12;
    if (isCursed()) target /=10;
    for (auto c : contents()) // pass damage to contents
      if (dPc() < target)
	rtn |= c->repair(type);
    return rtn;
  }
  
};




class readableItem : public basicItem {
public:
  readableItem(const itemType& type, const io &ios) :
    basicItem(type, ios) {}
  virtual ~readableItem() {}
   // TODO: Text, spells etc.
};

class hitchGuide : public readableItem {
public:
  hitchGuide(const itemType& type, const io &ios) :
    readableItem(type, ios) {}
  virtual ~hitchGuide() {}
  virtual bool use() {
    if (isCursed()) return false; // seems a bit harsh, but I don't have a better idea yet
    ::invokeGuide(ios(), isBlessed());
    return true;
  }
  /*
  virtual bool use(std::shared_ptr<item> other) {
    if (isCursed()) return false; // seems a bit harsh, but I don't have a better idea yet
    ::invokeGuide(ios(), isBlessed(), other);
    return true;
    }*/
};

class shopCard : public basicItem {
public:
  shopCard(const itemType& type, const io &ios) :
    basicItem(type, ios) {}  
  virtual ~shopCard(){}
  virtual bool use() {
    ::goShopping(ios(), *holder_);
    return true;
  }
};


class corpse : public basicItem {
private:
  // what was this in life?
  const monsterType &type_;
  // how advanced was this in life?
  const unsigned char maxDamage_;
public:
  corpse(const io &ios, const monsterType &of, const unsigned char maxDamage) :
    basicItem(itemTypeRepo::instance()[itemTypeKey::corpse], ios),
    type_(of),
    maxDamage_(maxDamage) {
    // TODO: should any monster types be proof against damage? Already (eg) wet?
    // TODO: incubus & succubus should be sexy. After all, they are confident...
    // if (monsterType == monsterType::incubus || monsterType == monsterType::succubus) sexUp(true);
  }
  virtual ~corpse() {};
  virtual materialType material() const {
    // trolls are stony, bats are leathery, plats are veggy, etc.
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
  virtual iterable<std::wstring, std::vector<std::wstring>,true > adjectives() const {
    auto baseAdjectives = basicItem::adjectives();
    std::vector<std::wstring> adjectives;
    adjectives.push_back(L"dead");
    adjectives.insert(adjectives.end(), baseAdjectives.begin(), baseAdjectives.end());
    return iterable<std::wstring, std::vector<std::wstring>, true>(adjectives);
  }
  virtual const wchar_t * const simpleName() const {
    return type_.name(maxDamage_); // overridden to change type_ from itemType to monsterType.
  }
};

// create an item of the given type. io may be used later by that item, eg for prompts when using.
// TODO: Randomness for flavour: enchantment, flags, etc.
// TODO: Wands will need starting enchantment.
std::shared_ptr<item> createItem(const itemTypeKey & t, const io & ios) {
  auto &r = itemTypeRepo::instance();
  switch(t) {
  case itemTypeKey::apple:
    return std::shared_ptr<item>(new basicItem(r[t], ios));
    //case itemTypeKey::corpse: // not handled here; we do this when a c
  case itemTypeKey::mace:
    return std::shared_ptr<item>(new basicWeapon(r[t], ios, damageType::bashing));
  case itemTypeKey::rock:
    return std::shared_ptr<item>(new basicThrown(r[t], ios, damageType::bashing));
  case itemTypeKey::helmet:
    return std::shared_ptr<item>(new basicEquip(r[t], ios, slotType::hat));
  case itemTypeKey::stick:
    return std::shared_ptr<item>(new basicItem(r[t], ios)); // TODO: wands & charges
  case itemTypeKey::bottle:
    return std::shared_ptr<item>(new bottle(r[t], ios));
  case itemTypeKey::codex:
    return std::shared_ptr<item>(new readableItem(r[t], ios));
  case itemTypeKey::hitch_guide:
    return std::shared_ptr<item>(new hitchGuide(r[t], ios));
  case itemTypeKey::poke:
    return std::shared_ptr<item>(new basicContainer(r[t], ios));
  case itemTypeKey::water:
    return std::shared_ptr<item>(new basicItem(r[t], ios)); // TODO: undropability
  case itemTypeKey::wooden_ring:
    return std::shared_ptr<item>(new basicEquip(r[t], ios, slotType::ring_left_thumb, slotType::ring_left_index, slotType::ring_left_middle, slotType::ring_left_ring, slotType::ring_left_little, slotType::ring_right_thumb, slotType::ring_right_index, slotType::ring_right_middle, slotType::ring_right_ring, slotType::ring_right_little, slotType::toe_left_thumb, slotType::toe_left_index, slotType::toe_left_middle, slotType::toe_left_fourth, slotType::toe_left_little, slotType::toe_right_thumb, slotType::toe_right_index, slotType::toe_right_middle, slotType::toe_right_fourth, slotType::toe_right_little));
  case itemTypeKey::kalganid:
    return std::shared_ptr<item>(new basicItem(r[t], ios)); // TODO: should we be able to equip coins on our eyes?
  case itemTypeKey::shop_card:
    return std::shared_ptr<item>(new shopCard(r[t], ios));
  default:
    throw t; // unknown type
  }
}

std::shared_ptr<item> createCorpse(const io &ios, const monsterType &mt, const unsigned char maxDamage) {
  return std::shared_ptr<item>(new corpse(ios, mt, maxDamage));
}

// create a random item suitable for the given level depth
// TODO: depth limitations
std::shared_ptr<item> createRndItem(const int depth, const io & ios) {
  auto &r = itemTypeRepo::instance();
  while (true) {
    auto type = rndPick(r.begin(), r.end());
    if (type->first == itemTypeKey::water) continue;
    if (type->first == itemTypeKey::corpse) continue;
    return createItem(type->first, ios);
  }
}
