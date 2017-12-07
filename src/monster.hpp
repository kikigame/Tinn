/* License and copyright go here*/

// define a monster in the dungeon

#ifndef MONSTER_HPP
#define MONSTER_HPP

#include "renderable.hpp"
#include "characteristic.hpp"
#include "level.hpp"
#include "monsterType.hpp"
#include "monsterIntrinsics.hpp"
#include "materialType.hpp"
#include "time.hpp"
#include "slots.hpp"
#include "itemholder.hpp"
#include "bonus.hpp"

#include <memory> // shared_ptr
#include <string>
#include <list>

class monsterImpl;
class deity;
class monsterBuilder;
class levelImpl;
class role;
class slot;
class damage;

// attack() can remove a monster from the game, so we copy
// the data to a struct to return it:
struct attackResult {
  attackResult(const characteristic &damage,
	       const wchar_t * const text) :
    injury_(damage), text_(text) {};
  characteristic injury_;
  const wchar_t *const text_;
};

// I'm not using pImpl here, because pain.
// Sutter (in Effective C++) reminds me that nothing virtual should go in the pImpl, as the pImpl can't be
// effectively used as a base or derrived class.
class monster : public renderable, public itemHolder {
private:
  level *level_; // raw pointer to avoid cyclic reference; level owns its monsters
  characteristic strength_;
  characteristic appearance_;
  characteristic fighting_;
  characteristic dodge_;
  characteristic damage_;
  characteristic male_;
  characteristic female_;
  std::list<time::callback> eachTick_;
  const monsterType & type_;
  // align stored as a non-null pointer, not a reference, so we can always reassign it (permanent alignment change)
  // Smart pointer is not needed as gods are effectively a bunch of create-on-demand singletons
  // (just like in real life?)
  deity const  *align_;
  // carried items are inventory. Worn/wielded items are equipment.
  // an item can only be equipment if it is also inventory.
  // slots may be empty; an individual monster may have equipped items that are in
  // slots unavailable to its monster type (eg if polymorphed; you won't lose your
  // cursed tail bow).
  std::map<const slot*, optionalRef<item> > equipment_;
  monsterIntrinsics intrinsics_;
public:
  /*
   * Passed a callback, which will be invoked each tick and discarded
   * when this monster is
   */
  void eachTick(const std::function<void()> &callback);
  virtual const wchar_t render() const; // delegate to type by default
  virtual const wchar_t * const name() const; // delegate to type depending on level by default;
  virtual const wchar_t * const description() const; // delegate to type by default
  monster(level * onLvl, const monsterType &type);
  monster(monsterBuilder & builder);
  virtual ~monster();
  bool operator == (const monster &rhs);
  bool operator != (const monster &rhs) { return !(*this == rhs); }

  // is this monster computer-controlled?
  virtual bool isPlayer() const { return false; }

  const characteristic& strength() const; // affect how hard we hit, and how much we can carry
  const characteristic& appearance() const; // affects prices in shops; foocubi
  const characteristic& fighting() const; // chance to hit during combat
  const characteristic& dodge() const; // chance to avoid being hit during combat (affected by armour)
  const characteristic& injury() const; // damage track

  // I've decided to make these characteristics, so that they can be lost and regained during the game.
  const characteristic& male() const;
  const characteristic& female() const;

  // what about the monster's intrinsics?
  // NB: This is only the intrinsic properties; a monster may also gain these powers from extrinsics (equipped items)
  monsterIntrinsics & intrinsics();

  // monster abilities. Based on intrinsics, but modified by items to add, remove or change things.
  monsterAbilities & abilities();
  const monsterAbilities & abilities() const;

  // retrieve the current level; needed by moveMonster() function as monsters can switch levels.
  level & curLevel(); 

  // Spiritual alignment
  const deity& align() const;

  // fighting...
  const attackResult attack(monster &target);

  // falling...
  virtual const wchar_t* const fall(unsigned char reductionPc);

  // ... and other injuries... (returns the lossed score)
  int wound(unsigned char reductionPc, const damage & dt);

  // try and wield/wear etc. the given item in the specified slot. Returns true if successful, false
  // if the slot was full or n/a for this monster type. Precodition: slot must be available for type.
  bool equip(item &item, const slotType slot);
  // try to unequip an item. Returns true on success, false if not equipped or cursed
  bool unequip(item &item);
  // returns true if this monster has this equipment slot and it is empty
  // returns false if this monster does not have this slot, or it is occupied.
  bool slotAvail(const slot * slot) const;
  // returns the slot if equipped, nullptr otherwise:
  const slot * slotOf(const item &item) const;
  // drop an item. Returns true on success, false on failure (eg cursed)
  bool drop(item &item) { return drop(item, level_->posOf(*this)); }
  // drop an item. Returns true on success, false on failure (eg cursed)
  bool drop(item &item, const coord &c);

  // evaluated as D% <= (stat). NON-DETERMINISTIC!
  bool isMale() const;
  // evaluated as D% <= (stat). NON-DETERMINISTIC!
  bool isFemale() const;

  materialType material() const;

  const monsterType& type() const;

  // called when the monster moves within a level
  // NB: At this stage, traps have already been revealed. Trap effects generally applied here.
  virtual void onMove(const coord &pos, const terrain &terrain);

  // called when the monster moves level
  virtual void onLevel(level * lvl);

  // retrieve a saying
  virtual const wchar_t * say() const;

  // overridden to include slot names in the prompt.
  virtual void forEachItem(std::function<void(const item&, std::wstring)>) const;
  virtual void forEachItem(std::function<void(item&, std::wstring)>);

  // TODO: overridden to disable dropping of cursed items
  // - or is this too low-level? allow stealing etc?
  // virtual bool removeItemForMove(item &item);

protected:

  // given a damage figue of damage of a given type,
  // return the total damage taken (which may be negative!)
  // NB: Units are percentage points by which to affect the damage stat.
  // this considers intrinsics first then recurses over equipped items.
  virtual long modDamage(long pc, const damage & type) const;

  // called when a monster dies
  virtual void death();

};

class monsterBuilder {
  friend class monster;
  level *level_;
  const deity *align_;
  unsigned char strength_;
  unsigned char appearance_;
  unsigned char fighting_;
  unsigned char dodge_;
  unsigned char damage_;
  unsigned char maxDamage_;
  unsigned char male_;
  unsigned char female_;   
  const monsterType * type_;
  bool allowRandom_;
  bool finalStatsDone_ = false;
  void calcFinalStats(); // TODO: Initial XP for monsters
public:
  monsterBuilder(bool allowRandom = true);
  void startOn(level &l);
  // the folowing override the monster type:
  void strength(unsigned char s);
  void appearance(unsigned char a);
  void fighting(unsigned char f);
  void dodge(unsigned char d);
  void maxDamage(unsigned char d);
  void damage(unsigned char d); // in case we want to create wounded monsters
  void male(unsigned char m);
  void female(unsigned char f);
  void align(const deity & d);
  void type(const monsterType & t);
  level * iLevel();
  const deity & align();
  unsigned char strength();
  unsigned char appearance();
  unsigned char fighting();
  unsigned char dodge();
  unsigned char damage();
  unsigned char maxDamage();
  unsigned char male();
  unsigned char female();
  const monsterType & type();
};

// create a reaming monster initially on the given level:
std::shared_ptr<monster> ofType(const monsterType &t, level &onLvl, const std::shared_ptr<io> ios);

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms);

#endif // ndef MONSTER_HPP
