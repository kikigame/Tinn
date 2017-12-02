/* License and copyright go here*/

// Things and stuff

#ifndef ITEMS_HPP__
#define ITEMS_HPP__

#include "damage.hpp"
#include "itemTypes.hpp"
#include "itemholder.hpp"
#include "slots.hpp"
#include <memory>
#include <string>
#include <vector>
#include <set>

class monster;
class level;
class coord;
class io;

/*
 * Pure-virtual base class to define an in-game object
 */
class item : public renderable {
public:
  // delegate to itemType by default
  virtual const wchar_t render() const = 0;
  // built up of itemType and adjectives etc.
  virtual const wchar_t * const name() const = 0;
  // built up of all visible properties.
  virtual const wchar_t * const description() const = 0;

  // where shall it be?
  virtual void move(itemHolder &holder) = 0;
  virtual const itemHolder& holder() const = 0;

  // what is the object made of?
  virtual materialType material() const = 0;
  // hom much does it weigh?
  virtual double weight() const = 0;
  // if wielded, what damage does this weapon do?
  virtual damageType weaponDamage() const = 0;
  // how much damage has this item taken?
  virtual int damageOfType(const damageType &type) const = 0; 
  // list of all adjectives applicable to type
  virtual iterable<std::wstring, std::vector<std::wstring>,true > adjectives() const = 0;
  // damage the item in some way (return false only if no effect)
  virtual bool strike(const damageType &type) = 0;
  // repair previous damage (return false only if no effect, eg undamaged)
  virtual bool repair(const damageType &type) = 0;
  // proof against dagage type  (return false only if no effect, eg already proofed or n/a for material type)
  virtual bool proof(const damageType &type) = 0;
  virtual bool isProof(const damageType &type) const = 0; // proof accessor

  // access flags (these all start false):
  // blesed items have greater effect
  virtual bool isBlessed() const = 0; virtual void bless(bool blessed) = 0;
  // cursed items can't be unequipped, and have lesser effect
  virtual bool isCursed() const = 0; virtual void curse(bool cursed) = 0;
  // unidentified itemsdon't provide a hint, may show another name

  virtual bool isUnidentified() const = 0; virtual void unidentify(bool forget) = 0;
  // sexy items add a +5 + 5*enchantemnt bonus to appearance, but only in outer layers.
  virtual bool isSexy() const = 0; virtual void sexUp(bool sexy) = 0;


  // enchantment is some +/- modifier for the item; adds to attack when wielded
  // it's also the count of the number of magic uses left.
  virtual int enchantment() const = 0;
  virtual void enchant(int enchantment) = 0; // may be negative

  // destroy an item
  virtual void destroy() = 0;

  // try to use the object
  virtual bool use() = 0;

  // try to use the object with another (eg put object into container; put candles on candlestick)
  //virtual bool use(std::shared_ptr<item> other) = 0;

  // try to equip an item. Precondition: item must be available for monster to equip.
  // returns true if successful, false otherwise (ie no suitable slots)
  virtual bool equip(std::shared_ptr<monster> owner) = 0;

  // in which slots can this item be equipped?
  // empty set if none
  virtual std::set<slotType> slots() = 0;

  // given a damage figue of damage of a given type,
  // return the total damage taken (which may be negative!)
  // NB: Units are percentage points by which to affect the damage stat.
  // this considers intrinsics first then recurses over equipped items.
  // this base implementation simply returns the supplied value.
  virtual long modDamage(long pc, const damage & type) const = 0;


protected:
  const io & ios();
};

// create an item of the given type. io may be used later by that item, eg for prompts when using.
std::shared_ptr<item> createItem(const itemTypeKey & it, const io & ios);

// create a random item suitable for the given level depth
std::shared_ptr<item> createRndItem(const int depth, const io & ios);

#endif // ndef ITEMS_HPP__
