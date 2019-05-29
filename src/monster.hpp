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
#include "itemholder.hpp"
#include "bonus.hpp"
#include "damage.hpp"
#include "equippable.hpp"
#include "hasAdjectives.hpp"
#include "monstermutation.hpp"

#include <memory> // shared_ptr
#include <list>

class monsterImpl;
class deity;
class monsterBuilder;
class levelImpl;
class slot;
class damage;
template <class ...T> class sharedAction;

// attack() can remove a monster from the game, so we copy
// the data to a struct to return it:
struct attackResult {
  attackResult(const characteristic &damage,
	       const wchar_t * const text) :
    injury_(damage), text_(text) {};
  characteristic injury_;
  const wchar_t *const text_;
};

class monsterAlarm;

// I'm not using pImpl here, because pain.
// Sutter (in Effective C++) reminds me that nothing virtual should go in the pImpl, as the pImpl can't be
// effectively used as a base or derrived class.
class monster : public renderable, public itemHolder, public equippable, public virtual hasAdjectives {
  friend class monsterAlarm;
private:
  level *level_; // raw pointer to avoid cyclic reference; level owns its monsters
  std::bitset<2> flags_;
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
  std::shared_ptr<monsterIntrinsics> intrinsics_;
  std::shared_ptr<monsterAbilityMods> abilities_;
  // which monsters have charmed this one?
  std::list<monster*> charmedBy_;
  std::vector<std::function<void()>> onDeath_;
  std::vector<std::wstring> extraAdjectives_;//usually empty.
  std::set<std::reference_wrapper<const mutation>, std::less<const mutation> > mutations_;
protected:
  // create monster by builder, with specific weapon slots (used internally by dragons)
  monster(monsterBuilder & b, std::vector<const slot *>slots);
public:
  /*
   * Passed a callback, which will be invoked each tick and discarded
   * when this monster is
   */
  void eachTick(const std::function<void()> &callback);
  virtual const wchar_t render() const; // delegate to type by default
  virtual std::wstring name() const; // delegate to type depending on level by default;
  virtual bool highlight() const;
  virtual std::wstring description() const; // delegate to type by default
  // create monster by type
  monster(level * onLvl, const monsterType &type);
  // create monster by builder
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
  characteristic& strength(); // affect how hard we hit, and how much we can carry
  characteristic& appearance(); // affects prices in shops; foocubi
  characteristic& fighting(); // chance to hit during combat
  characteristic& dodge(); // chance to avoid being hit during combat (affected by armour)
  characteristic& injury(); // damage track

  // I've decided to make these characteristics, so that they can be lost and regained during the game.
  const characteristic& male() const;
  const characteristic& female() const;

  // what about the monster's intrinsics?
  // NB: This is only the intrinsic properties; a monster may also gain these powers from extrinsics (equipped items)
  std::shared_ptr<monsterIntrinsics> intrinsics();
  const std::shared_ptr<monsterIntrinsics> intrinsics() const;

  // monster abilities. Based on intrinsics, but modified by items to add, remove or change things.
  std::shared_ptr<monsterAbilities> abilities();
  const std::shared_ptr<monsterAbilities> abilities() const;

  // retrieve the current level; needed by moveMobile() function as monsters can switch levels.
  level & curLevel(); 
  const level & curLevel() const; 

  // Spiritual alignment
  virtual const deity& align() const;

  // fighting...
  const attackResult attack(monster &target);

  // falling...
  void fall(unsigned char reductionPc);

  // ... and other injuries... (returns the lossed score)
  int wound(const monster &by, unsigned char reductionPc, const damage & dt);

  // damage armour (called automatically by wound())
  bool damageArmour(const damage &d);
  
  // what kind of damage does this monster do?
  virtual damageType unarmedDamageType() const;

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
  // May return false to prevent movement
  virtual bool onMove(const coord &pos, const terrain &terrain);
  // called after moving. Fatal traps applied here to get the corpse in the right square.
  virtual void postMove(const coord &pos, const terrain &terrain);

  // called when the monster moves level
  virtual void onLevel(level * lvl);

  // called whenever opponent is hit
  virtual void onHit(monster &opponent, int damage) {};

  // retrieve a saying
  virtual const wchar_t * say() const;

  // delegate to type by default, but overridden for special behiour
  virtual const movementType & movement() const;

  // overridden to include slot names in the prompt.
  virtual void forEachItem(std::function<void(const item&, std::wstring)>) const;
  virtual void forEachItem(std::function<void(item&, std::wstring)>);

  // overridden to disable dropping of cursed items
  // and ensure items unequipped on move
  virtual bool removeItemForMove(item &item, itemHolder &next);

  // overridden to ensure items unequipped on destroy
  virtual bool destroyItem(item &item);

  // consume the given item.
  // Returns true if item is fully consumed.
  // Returns false if item is partially consumed
  // Throws inedibleException if item is inedible.
  // Throws notHungryException if monster is not hungry
  // if "force" is true, exceptions are not thrown, but damage is taken instead.
  virtual bool eat(item &item, bool force = false);

  // try to eat something, either from current location or inventory:
  virtual void eat();

  class inedibleException : public std::exception{};
  class notHungryException : public std::exception{};

  // this will move towards m (dPc() < m.appearance().cur())
  // this will be less aggressive towards m (dPc() < m.appearance().cur() to skip attack)
  bool setCharmedBy(monster &m);

  std::list<monster *>::const_iterator charmedBegin() const;
  std::list<monster *>::const_iterator charmedEnd() const;

  // overridden to recalculate stats
  virtual bool unequip(item &item);

  // adjectives
  std::vector<std::wstring> adjectives() const;

  // adds an adjective
  void addDescriptor(std::wstring);

  // pick an action to attack with; called during combat.
  // return empty reference for conventional melee attack.
  virtual optionalRef<sharedAction<monster, monster>> attackAction();

  bool sleeping() const;
  bool sleep(int ticks);
  bool awaken();

  // test if a monster has a mutation; NB if the mutation doesn't apply to this monster it'll have no effect, and this will return false.
  bool isMutated(const mutationType &m) const;
  void mutate(const mutationType &key);
  void deMutate(const mutationType &key);
protected:

  // given a damage figue of damage of a given type,
  // return the total damage taken (which may be negative!)
  // NB: Units are percentage points by which to affect the damage stat.
  // this considers intrinsics first then recurses over equipped items.
  virtual long modDamage(long pc, const damage & type) const;

  // called when a monster dies
  virtual void death();

  // overridden to recalculate stats
  virtual void onEquip(item &item, const slot *s1, const slot *s2);
  virtual std::array<const slot *, 2> forceUnequip(item &);
private:
  bool raiseDead(const mutation &m, itemHolder &holder, const coord &pos);
};

// big monsters maintain their own position:
class bigMonster {
public:
  // called when position is first set, or on teleport:
  virtual void setPos(const coord &c) = 0;
  // return the position of the "main" element:
  virtual coord mainPos() const = 0;
  virtual ~bigMonster() {};
};

class monsterBuilder {
  friend class monster;
  level *level_;
  bool highlight_;
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
  int progress_; // >=1, based on depth of monster
  std::vector<std::function<void()>> onDeath_;
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
  void alignAgainst(const deity &d); // align to opposite of d
  void type(const monsterType & t);
  void highlight();
  void onDeath(std::function<void()>);
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
  void progress(int progress);
  bool isHighlight();
  std::vector<std::function<void()>> &onDeath();
};

// create a roaming monster initially on the given level:
template<monsterTypeKey T>
std::shared_ptr<monster> ofType(level & level, monsterBuilder &b = monsterBuilder(true));

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms);

#endif // ndef MONSTER_HPP
