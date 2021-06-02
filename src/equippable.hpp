/* License and copyright go here*/

// things that can equip items

#ifndef EQUIPPABLE_HPP
#define EQUIPPABLE_HPP

#include "optionalRef.hpp"
#include "slots.hpp"
#include "graphsearch.hpp"

class item;
class slot;
template<class ...T> class sharedAction;
class monster;
class monsterType;
class damage;

#include <map>
#include <functional>

enum equipResult { SUCCESS, NO_SLOT, SLOT_FULL };

class equippable {
private:
  // carried items are inventory. Worn/wielded items are equipment.
  // an item can only be equipment if it is also inventory.
  // slots may be empty; an individual monster may have equipped items that are in
  // slots unavailable to its monster type (eg if polymorphed; you won't lose your
  // cursed tail bow).
  std::map<const slot*, optionalRef<item> > equipment_;
public:
  // try and wield/wear etc. the given item in the specified slot. Returns true if successful, false
  // if the slot was full or n/a for this monster type. Precodition: slot must be available for type.
  equipResult equip(item &item, const slot *slot);
  equipResult equip(item &item, const slotType slot);
  // equip a 2-handed weapon, or a pair of something:
  equipResult equip(item &item, const std::pair<slotType, slotType> &slots);
  // NB: The following method should always work if immediately after forceUnequip: (for recalculation of stats):
  equipResult equip(item &item, const std::array<const slot *, 2> &slots);
  // try to unequip an item. Returns true on success, false if not equipped or cursed
  virtual bool unequip(item &item);
  bool isEquipped(sharedAction<item, monster> &act) const;
  // returns true if this monster has this equipment slot and it is empty
  // returns false if this monster does not have this slot, or it is occupied.
  bool slotAvail(const slot * slot) const;
  optionalRef<item> inSlot(slotType slot);
  // returns the slot if equipped, nullptr otherwise:
  const std::array<const slot *, 2> slotsOf(const item &item) const;
  // forcably unequip an item, bypassing normal rules (eg for stealing a cursed item):
  // NB: An immediate call to equip(item, slots) should always work after this:
  virtual std::array<const slot *, 2> forceUnequip(item &);

  // call on all wielded items
  void forEachWeapon(std::function<void(const item &)> per) const;
protected:
  equippable(std::vector<const slot *>slots);
  virtual ~equippable();
  /*
   * return the first weapon
   */
  optionalRef<item> findWeapon() const;
  /*
   * return the first outer armour
   */
  optionalRef<item> findArmour() const;
  /*
   * Modify damage for armour
   */
  virtual long modDamage(long pc, const damage &type) const;
  // used to switch monster category
  void polymorphCategory(monsterCategory c);
  // called after item is equipped to calculate bonuses
  // for single-slot items, pass s1 == s2
  virtual void onEquip(item &item, const slot *s1, const slot *s2);
  /*
   * Calculate the bonuses from the equipped items
   */
  int strBonus() const; // strength
  int dodBonus() const; // dodge
  int appBonus() const; // appearance
};

class coverSearch : public graphSearch<const slot*, optionalRef<item> > {
  virtual std::vector<const slot*> deeper(const slot* const & sl);
};

#endif //ndef EQUIPPABLE_HPP
