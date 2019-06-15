/* License and copyright go here*/

// Things and stuff

#ifndef ITEMS_HPP__
#define ITEMS_HPP__

#include "damage.hpp"
#include "itemTypes.hpp"
#include "itemholder.hpp"
#include "slots.hpp"
#include "action.hpp"
#include "hasAdjectives.hpp"
#include "beitude.hpp"
#include <algorithm>
#include <memory>
#include <bitset>
#include <set>

class monster;
class monsterType;
class level;
class coord;

class item;

// when using Multiple Inheritance with shared_from_this, you need to use a common virtual base-class
// for shared_from_this, so that any derived class can create shared pointers in one group.
class shared_item : public std::enable_shared_from_this<item> {
};

/*
 * Class to identify containers of liquids (drinkables).
 * NB: containers of fluids *may* contain other things instead.
 */
class liquidContainer {
public:
  // true if there is at least 1 portion to drink, false if empty or containing non-fluids.
  virtual bool containsLiquid() const = 0;
  // called when the contents are consumed. If the container is single-use, it should destroy itself here.
  // this method is responsible for destroying the contents of the bottle.
  virtual void consumeBy(monster &) = 0;
};

/*
 * Pure-virtual base class to define an in-game object
 */
class item : public renderable, public virtual shared_item, public virtual hasAdjectives {
  friend class itemHolderMap;
public:
  item () = default;
  item (const item &) = delete;
  item (item &&) = delete;
  // delegate to itemType by default
  virtual const wchar_t render() const = 0;
  // built up of itemType and adjectives etc.
  virtual std::wstring name() const = 0;
  // built up of all visible properties including encyclopedium
  virtual std::wstring description() const;
  // per-type description:
  virtual std::wstring typeDescription() const = 0;

  // where is it?
  itemHolder& holder() const;

  // what is the object made of?
  virtual materialType material() const = 0;
  // hom much does it weigh?
  virtual double weight() const = 0;
  // if wielded, what damage does this weapon do?
  // if use == true, consume any weapon charges or any other on-hit actions (called on successful hit)
  virtual damageType weaponDamage(bool use) = 0;
  // how much damage has this item taken?
  virtual int damageOfType(const damageType &type) const = 0; 
  // list of all adjectives applicable to type
  virtual std::vector<std::wstring> adjectives() const = 0;
  // damage the item in some way (return false only if no effect)
  virtual bool strike(const damageType &type) = 0;
  // force damage with no side-effects, eg for polymorphing
  virtual void forceDamageOnly(const damageType &type, unsigned char d) = 0;
  // repair previous damage (return false only if no effect, eg undamaged)
  virtual bool repair(const damageType &type) = 0;
  // proof against dagage type  (return true only if no more effect possible, eg already proofed or n/a for material type)
  virtual bool proof(const damageType &type) = 0;
  virtual bool isProof(const damageType &type) const = 0; // proof accessor

  // access flags (these all start false):
  // blesed items have greater effect
  virtual bool isBlessed() const = 0; virtual void bless(bool blessed) = 0;
  // cursed items can't be unequipped, and have lesser effect
  virtual bool isCursed() const = 0; virtual void curse(bool cursed) = 0;
  ::bcu bcu() { return ::bcu(isBlessed(), isCursed()); }
  // unidentified itemsdon't provide a hint, may show another name

  virtual bool isUnidentified() const = 0; virtual void unidentify(bool forget) = 0;
  // sexy items add a +5 + 5*enchantemnt bonus to appearance, but only in outer layers.
  virtual bool isSexy() const = 0; virtual void sexUp(bool sexy) = 0;


  // enchantment is some +/- modifier for the item; adds to attack when wielded
  // it's also the count of the number of magic uses left.
  virtual int enchantment() const = 0;
  virtual void enchant(int enchantment) = 0; // may be negative

  // destroy an item
  virtual bool destroy();

  enum class useResult {
    SUCCESS, // okay; can use another item
      FAIL, // stop & abort; "that didn't work"
      DONE // okay, but don't allow another use (action took time).
  };

  // try to use the object
  virtual useResult use();

  // try to equip an item. Precondition: item must be available for monster to equip.
  // returns true if successful, false otherwise (ie no suitable slots)
  virtual bool equip(monster &owner);

  // called automatically by monster::unequip when successfully unequipped:
  virtual void onUnequip(monster &monster);

  // in which slots can this item be equipped?
  // empty set if none
  //virtual std::set<slotType> slots() = 0;
  enum equipType { wielded, worn, none } type;
  virtual equipType equippable() const;

  // given a damage figue of damage of a given type,
  // return the total damage taken (which may be negative!)
  // NB: Units are percentage points by which to affect the damage stat.
  // this considers intrinsics first then recurses over equipped items.
  // this base implementation simply returns the supplied value.
  virtual long modDamage(long pc, const damage & type) const = 0;

  // callback to identify when an item has been added to a holder.
  virtual void onAdd(itemHolder &holder) const {}
};

// class for items with no especial behaviour:
class basicItem : public item {
private:
  enum { blessed, cursed, unidentified, sexy, NUM_FLAGS } flags;
  std::map<damageType, int> damageTrack_;
  // what are we *explicitly* proof against?
  // something may be proof against a material even with no damage track; this may
  // become useful in later development (eg transferring proofs between objects or
  // polymorphing the type and/or material of the object). Generally the existance
  // of a damage track should be checked first.
  std::set<damageType> proof_;
  std::bitset<NUM_FLAGS> flags_;
  int enchantment_;
protected:
  mutable std::wstring buffer_; // for transient returns.
  const itemType& type_;
public:
  basicItem(const itemType& type);
  basicItem(const basicItem &other) = delete;
  virtual ~basicItem();
  virtual const wchar_t render() const;
  virtual std::wstring simpleName() const;
  virtual std::wstring name() const;
  virtual std::wstring typeDescription() const;

  virtual materialType material() const;
  virtual double weight() const;
  // use - if true, causes charges to be consumed if needed.
  virtual damageType weaponDamage(bool use);
  virtual int damageOfType(const damageType &type) const;
  virtual std::vector<std::wstring> adjectives() const;
  virtual bool strike(const damageType &type);
  virtual void forceDamageOnly(const damageType &type, unsigned char d);
  virtual bool repair(const damageType &type);
  virtual bool proof(const damageType &type);
  virtual bool isProof(const damageType &type) const;
  virtual bool isBlessed() const;
  virtual void bless(bool b);
  virtual bool isCursed() const;
  virtual void curse(bool c);
  virtual bool isSexy() const;
  virtual void sexUp(bool s);
  virtual bool isUnidentified() const;
  virtual void unidentify(bool forget);
  virtual int enchantment() const;
  virtual void enchant(int enchantment);
  virtual std::set<slotType> slots();
  virtual long modDamage(long pc, const damage & type) const;
  const itemType &getType() const { return type_; }
};

// create an item of the given type. io may be used later by that item, eg for prompts when using.
// NB: if "it" is a water type, then return value should normally only be placed in a fluid container (eg bottle).
item & createItem(const itemTypeKey & key);

// craete a quest item:
template<questItemType it>
item & createQuestItem();
template<questItemType it>
item & createQuestItem(std::function<void(const itemHolder&)> &);

class deity;
// create a holy book with specific alignment
item & createHolyBook(const deity &align);

// corpses need especial handling:
item & createCorpse(const monster &m);
item & createCorpse(const monsterType &mt, const unsigned char maxDamage); //  as we shouldn't need a monster to create an item.

// create a random item suitable for the given level depth
item & createRndItem(const int depth, bool allowLiquids = false);
template<typename... T> // wchar_t *
item & createRndItem(const int depth, T... filter) {
  std::vector<wchar_t> f{filter...};
  while (true) {
    item &rtn = createRndItem(depth, true);
    wchar_t c = rtn.render();
    if (std::find(f.begin(), f.end(), c) != f.end())
      return rtn;
  }
}

// create a bottled item
template <itemTypeKey type>
item & createBottledItem();

// create a bottle with random liquid contents:
item & createRndBottledItem(const int depth);

// create a wand
item & createWand(sharedAction<monster,monster>::key of);

// create an equippable item with a power
item & createEquippable(const itemTypeKey &, sharedAction<item,monster>::key of);

// create an IOU card
item & createIou(const double amount, const std::wstring &whom, const std::wstring &service);

// replace an item with another of a different type
// "from" must be a fully created object in a container.
// "to" must be an enrolled object not yet in a container.
void transmutate(item & from, item &to);

optionalRef<monster> whoHolds(const item &i);

// destroy the item and create another of a new type
bool polymorph(item &from, const itemTypeKey &to);

#endif // ndef ITEMS_HPP__
