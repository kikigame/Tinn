/* License and copyright go here*/

// define a monster in the dungeon

#include "monster.hpp"
#include "monsterType.hpp"
#include "random.hpp"
#include "level.hpp"
#include "religion.hpp"
#include "time.hpp"
#include "terrain.hpp"
#include "output.hpp"
#include "slots.hpp"
#include "items.hpp"
#include "graphsearch.hpp"
#include "damage.hpp"
#include "pathfinder.hpp"
#include "action.hpp"
#include <algorithm> // find_if

#include <sstream>

//Roll 2D52-2
unsigned char dPc() {
  return static_cast<unsigned char>(d51() + d51() - 2);
}

monsterBuilder::monsterBuilder(bool allowRandom) : 
  level_(NULL), align_(NULL), 
  strength_(0),
  appearance_(0),
  fighting_(0),
  dodge_(0),  
  damage_(0),
  maxDamage_(0),
  male_(0),
  female_(0),
  type_(NULL),
  allowRandom_(allowRandom) {}

void monsterBuilder::startOn(level &l) { level_ = &l; }
void monsterBuilder::strength(unsigned char s) { strength_ = s; }
void monsterBuilder::appearance(unsigned char s) { appearance_ = s; }
void monsterBuilder::fighting(unsigned char s) { fighting_ = s; }
void monsterBuilder::dodge(unsigned char s) { dodge_   = s; }
void monsterBuilder::damage(unsigned char s) { damage_  = s; }
void monsterBuilder::maxDamage(unsigned char s) { maxDamage_  = s; }
void monsterBuilder::male(unsigned char s) { male_   = s; }
void monsterBuilder::female(unsigned char s) { female_  = s; }
void monsterBuilder::align(const deity &a) { align_  = &a; }
void monsterBuilder::type(const monsterType &t) { 
  // type also sets various fields if not already set
  type_  = &t; 
  if (align_ == NULL) align_ = *rndPick(t.alignment().begin(), t.alignment().end());
  if (strength_ == 0) strength_ = type_->iStrength();
  if (appearance_ == 0) appearance_ = type_->iAppearance();
  if (fighting_ == 0) fighting_ = type_->iFighting();
  if (dodge_ == 0) dodge_ = type_->iDodge();
  if (maxDamage_ == 0) maxDamage_ = type_->iMaxDamage();
}

level * monsterBuilder::iLevel() { calcFinalStats(); return level_; }
unsigned char monsterBuilder::strength() { calcFinalStats(); return strength_; }
unsigned char monsterBuilder::appearance() { calcFinalStats(); return appearance_; }
unsigned char monsterBuilder::fighting() { calcFinalStats(); return fighting_; }
unsigned char monsterBuilder::dodge() { calcFinalStats(); return dodge_; }
unsigned char monsterBuilder::damage() { calcFinalStats(); return damage_; }
unsigned char monsterBuilder::maxDamage() { calcFinalStats(); return maxDamage_; }
unsigned char monsterBuilder::male() { calcFinalStats(); return male_; }
unsigned char monsterBuilder::female() { calcFinalStats(); return female_; }
const deity & monsterBuilder::align() { calcFinalStats(); return *align_; }
const monsterType & monsterBuilder::type() { calcFinalStats(); return *type_; }

/*
 * Currently it should'nt be possible to generate characters with negative stats.
 * This is just a bit of defensive coding in case it happens in future.
 */
const unsigned char safesub(const unsigned char value, const unsigned char amount) {
  unsigned char rtn;
  if (value < amount) rtn = value; // don't apply a penalty if it would make a stat negative
  else rtn = value - amount;
  return rtn;
}

void monsterBuilder::calcFinalStats() {
  if (finalStatsDone_) return;
  finalStatsDone_ = true;
  if (allowRandom_) strength_ += dPc()/20;
  if (allowRandom_) appearance_ += dPc()/20;
  if (allowRandom_) fighting_ += dPc()/20;
  if (allowRandom_) dodge_ += dPc()/20;
  if (allowRandom_) maxDamage_ += dPc()/20;

  // alignment stat bonuses always apply at monster creation time:
  switch (align_->element()) {
  case Element::earth:
    strength_ += 5; break;
  default: break;
  }

  switch (align_->domination()) {
  case Domination::concentration:
    dodge_ += 5; break;
  case Domination::aggression:
    fighting_ += 10; strength_ += 5; appearance_ = safesub(appearance_, 10); break;
  default: break;
  }

  switch (align_->outlook()) {
  case Outlook::kind:
    appearance_ += 10; maxDamage_ = safesub(maxDamage_, 10);
    break;
  case Outlook::cruel:
    dodge_ += 10; appearance_ = safesub(appearance_, 10);
    break;
  default: break;
  }
}

monster::monster(monsterBuilder & b) :
  level_(b.iLevel()),
  strength_(b.strength()),
  appearance_(b.appearance()),
  fighting_(b.fighting()),
  dodge_(b.dodge()),
  damage_(b.damage(), b.maxDamage()), //characteristic(0, 20)),
  male_(b.male_),
  female_(b.female_),
  eachTick_(),
  type_(*b.type_),
  align_(b.align_),
  equipment_(),
  intrinsics_() {
  // create all slots as empty initially
  for (auto slot : slotsFor(type_.category()))
    equipment_.emplace(slot, optionalRef<item>());
}

void monster::eachTick(const std::function<void()> &callback) { 
  //  eachTick_.emplace_back(new time::callback(true, callback));
  eachTick_.emplace_back(true, callback);
}

const wchar_t monster::render() const { // delegate to type by default
  return type_.renderChar();
}

const wchar_t * const monster::name() const {
  auto damage = damage_.max();
  return type_.name(damage);
}

const wchar_t * const monster::description() const {
  return type_.encyclopedium();
}

/*
 * Bonus for strength is all wielded weapons' bonuses
 */
const characteristic& monster::strength() const { return strength_; }
const characteristic& monster::appearance() const { return appearance_; }
const characteristic& monster::fighting() const { return fighting_; }
const characteristic& monster::dodge() const { return dodge_; }
const characteristic& monster::injury() const { return damage_; }
const characteristic& monster::male() const { return male_; }
const characteristic& monster::female() const { return female_; }

materialType monster::material() const { return type_.material(); }
const monsterType& monster::type() const { return type_; }

const deity &monster::align() const { return *align_; }

const attackResult monster::attack(monster &target) {
  // We hit (unless dodged) the target if D% < fighting, or always on a roll of 0.
  auto dHit = dPc();
  unsigned char f = fighting().cur();
  
  if (dHit > f) return attackResult(injury(), L"miss");

  // Now to see if the opponent dodged it...
  unsigned char d = target.dodge().cur();
  if (dHit < d) return attackResult(injury(), L"evaded");

  auto weap = equipment_.find(slotBy(slotType::primary_weapon));
  if (weap == equipment_.end()) weap = equipment_.find(slotBy(slotType::secondary_weapon));
  optionalRef<item> weapon;
  if (weap != equipment_.end()) weapon = weap->second;
  damageType type = (weapon) ? weapon.value().weaponDamage() : damageType::bashing;
  auto dt = damageRepo::instance()[type];

  // Now to see how much damage we did...
  auto max = target.injury().max();
  int damage = target.wound(strength_.cur(), dt);
  onHit(target, damage);
  if (damage == 0) return attackResult(injury(), L"ineffectual");
  if (static_cast<unsigned char>(damage) == max) return attackResult(injury(), L"fatal");
  if (static_cast<unsigned char>(damage) >= max/2) return attackResult(injury(), L"good hit");
  return attackResult(injury(), L"hit");
}

// wounding in combat: between 0 and the damage_ stat, averaging 50%, then rounded down:
int monster::wound(unsigned char reductionPc, const damage & type) {
  long damage = dPc();
  damage *= reductionPc;
  damage /= 100;
  damage = modDamage(damage, type);
  damage_ += static_cast<unsigned char>(damage);
  if (damage_.cur() == damage_.max()) death();
  return damage;
}

long monster::modDamage(/*by value*/long pc, const damage & type) const {
  char delta = intrinsics_.resist(type);
  // TODO: extrinsics
  pc -= delta * 5; // NB: Can go positive, such is the goodness of magic
  for (auto i : equipment_) {
    auto &item = i.second;
    if (item) pc = item.value().modDamage(pc, type);
  }
  return pc;
}


// called upon death...
void monster::death() {
  level_->removeDeadMonster(*this);
}


// calculate the current strength bonus from equipment
int calcStrBonus(const std::map<const slot*, optionalRef<item> > eq) {
  int rtn = 0;
  auto e = eq.end();
  for (auto s : weaponSlots()) {
    auto i = eq.find(s);
    if (i != e && i->second) {
      auto it = i->second;
      // +1 so that even an unenchanted weapon does some damage:
      rtn += it.value().enchantment() + 1;
    }
  }
  return rtn;
}

// TODO: shields. These occupy weapon slots, and allow for deflection rather than armour.
// calculate the current strength bonus from equipment
int calcDodBonus(const std::map<const slot*, optionalRef<item> > eq) {
  int rtn = 0;
  auto wp = weaponSlots();
  auto wpe = wp.end();
  for (auto i : eq)
    if (i.second && wp.find(i.first) == wpe) {
      // considering occupied non-weapon slots
      auto &item = i.second.value();
      double defence;
      // base armour = (weight) * (material armour multiplicand):
      switch (item.material()) {
      case materialType::glassy: defence = (1/3.); break; // basically untoughened leather
      case materialType::woody: defence = 0.5; break; // basically untoughened leather
      case materialType::fleshy: defence = (2/3.); break; // basically untoughened leather
      case materialType::leathery: defence = 1; break; // standard armour
      case materialType::stony: defence = 1.5; break; // very strong if very heavy...
      case materialType::metallic: defence = 3; break; // best armour is metal
      default: defence = 0;
      }
      defence *= item.weight();
      defence += item.enchantment(); // each bonus adds +1 (=> +5%). Note that you can wear a *lot*, so enchantment adds up quite fast.
      if (item.isBlessed()) defence *= 1.5;
      if (item.isCursed()) defence *= 0.5;
      rtn += static_cast<int>(defence);
    }
  return rtn;
}

class coverSearch : public graphSearch<const slot*, optionalRef<item> > {
  virtual std::vector<const slot*> deeper(const slot* const & sl) {
    return sl->covered();
  }
};

// calculate the current appearance bonus from equipment
int calcAppBonus(const std::map<const slot*, optionalRef<item> > eq) {
  int rtn = 0;
  auto wp = weaponSlots();
  auto wpe = wp.end();
  coverSearch cs;
  for (auto i : eq)
    if (wp.find(i.first) == wpe) {
      // considering non-weapon slots
      // non-weapon item in slot i.first is i.second
      // we don't consider enchantments of armour in slots that are covered
      
      if (i.second // is it occupied?
	  && !cs.isCovered(eq, i.first)) {// is it covered?
	auto &item = i.second;
	rtn += item.value().enchantment();
	if (item.value().isSexy()) ++rtn;
	if (item.value().isSexy() && item.value().isBlessed()) ++rtn;
	// TODO: tshirts should get extra bonuses for being torn or wet (double if blessed).
	// the tshirt bonus is easy to get, but not that useful as it will generally be covered.
      }
    }
  return rtn;
}

bool monster::equip(item &item, const std::pair<slotType, slotType> slots) {
  std::array<const slot *, 2> sl{ slotBy(slots.first), slotBy(slots.second) };
  return equip(item, sl);
}

bool monster::equip(item &item, const std::array<const slot *,2> slots) {

  for (auto s : slots) {
    auto i = equipment_.find(s);
    if (i == equipment_.end()) return false; // monster doesn't have this slot
    if (i->second) return false; // already occupied
  }

  onEquip(item, slots[0], slots[1]);
  return true;
}

bool monster::equip(item &item, const slotType slot) {
  auto s = slotBy(slot);
  return equip(item, s);
}

bool monster::equip(item &item, const slot *s) {
  auto i = equipment_.find(s);
  if (i == equipment_.end()) return false; // monster doesn't have this slot
  if (i->second) return false; // already occupied
  onEquip(item, s, s);
  return true;
}

void monster::onEquip(item &item, const slot *s1, const slot *s2) {
  // we can equip the item. Calculate any previous bonuses
  const int strBonus = calcStrBonus(equipment_);
  const int appBonus = calcAppBonus(equipment_);
  const int dodBonus = calcDodBonus(equipment_);
  // equip the item
  //  equipment_.insert(std::pair<const slot*, std::shared_ptr<item>>(s, item));
  equipment_[s1] = item;
  equipment_[s2] = item;
  // calculate any new bonuses and apply adjustment
  strength_ += calcStrBonus(equipment_) - strBonus;
  appearance_ += calcAppBonus(equipment_) - appBonus;
  dodge_ += calcDodBonus(equipment_) - dodBonus;
}

bool monster::unequip(item &item) {
  if (item.isCursed()) return false;
  // Calculate any previous bonuses
  const int strBonus = calcStrBonus(equipment_);
  const int appBonus = calcAppBonus(equipment_);
  const int dodBonus = calcDodBonus(equipment_);
  bool rtn = false;
  auto eend = equipment_.end();
  for (auto e = equipment_.begin(); e != eend; ++e)
    if (e->second && &(e->second.value()) == &item) {
      // we can equip the item. 
      // unequip the item
      optionalRef<::item> nullItem;
      e->second = nullItem;
      rtn = true;
    }
  if (rtn) {
    // calculate any new bonuses and apply adjustment
    strength_ += calcStrBonus(equipment_) - strBonus;
    appearance_ += calcAppBonus(equipment_) - appBonus;
    dodge_ += calcDodBonus(equipment_) - dodBonus;
  }
  return rtn;
}

bool monster::slotAvail(const slot *s) const {
  return equipment_.find(s) != equipment_.end();
}
const std::array<const slot *,2> monster::slotsOf(const item &item) const {
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

bool monster::drop(item &ite, const coord &c) {
  if (ite.isCursed()) return false;
  // unequip before dropping:
  auto sl = slotsOf(ite);
  if (sl[0] != nullptr && !unequip(ite)) return false;
  auto i = firstItem([&ite](item &it) { return &it == &ite; });
  if (!i) return false; // can't drop what we don't have
  // drop
  return level_->holder(c).addItem(ite);
}


// for game reasons only; we need yes/no responses to choose how monsters react to each other
bool monster::isMale() const {
  unsigned char target = male_.cur();
  return target == characteristic::MAX_MAX || dPc() < target;
}
// for game reasons only; we need yes/no responses to choose how monsters react to each other
bool monster::isFemale() const {
  unsigned char target = female_.cur();
  return target == characteristic::MAX_MAX || dPc() < target;
}

monsterIntrinsics & monster::intrinsics() {
  return intrinsics_;
}

level & monster::curLevel() {
  return *level_;
}


std::wstring monster::onMove(const coord &pos, const terrain &terrain) {
  if (intrinsics_.entrapped())
    return L"You try, but you're still escaping from the trap!";
  if (terrain.type() == terrainType::PIT) {
    bool flying(abilities().fly());
    if (flying) return L"You are now over a pit.";
    const auto message = fall(dPc() / 10);
    const auto climb = abilities().climb();
    const int count=climb == bonus(false) ? 6 : climb == bonus() ? 4 : 2;
    intrinsics_.entrap(count);
    eachTick([this](){
	intrinsics_.entrap(-1);
	if (!intrinsics_.entrapped()) {
	  // TODO: tell player they've escaped
	  time::offPlayerMove(*(eachTick_.rbegin())); // traps must be in filo stack order
	  eachTick_.pop_back();
	}
      });
    return message;
  }
  return L"";
}

const wchar_t * const monster::fall(unsigned char reductionPc) {
  auto damage = wound(dPc() / 10, damageRepo::instance()[damageType::bashing]);
  return (damage == 0 ?
	  L"You adjust to the sudden change in altitude." L" You are now in a pit." :
	  L"Gravity hurts sometimes." L" You are now in a pit.");
}

void monster::onLevel(level * lvl) {
  level_ = lvl;
}

const wchar_t * monster::say() const {
  return *rndPick(type().sayingsBegin(), type().sayingsEnd());
}

monsterAbilities& monster::abilities() {
  return intrinsics_; // TODO: extrinsics
}
const monsterAbilities& monster::abilities() const {
  return intrinsics_; // TODO: extrinsics
}

monster::~monster() {}

bool monster::operator == (const monster &rhs) { return this == &rhs; }


template<bool avoidTraps, bool avoidHiddenTraps>
class nextLevelMoves {
private:
  const monster &mon_;
  const level &level_;
public:
  nextLevelMoves(monster &mon) :
    mon_(mon), level_(mon.curLevel()) {};
  std::set<coord> operator()(const coord &c) const {
    std::set<coord> rtn;
    for (int x=c.first-1; x<=c.first+1; ++x)
      for (int y=c.second-1; y<=c.second+1; ++y) {
	coord pos(x,y);
	if (pos == c) continue; // same square
	if (pos.first < 0 || pos.second < 0 ||
	    pos.first >= level::MAX_WIDTH || pos.second >= level::MAX_HEIGHT)
	  continue; // off the map; here be monsters...
	if (level_.movable(pos, mon_, avoidTraps, avoidHiddenTraps)) 
	  rtn.emplace(pos);
      }	
    return rtn;
  }
};

// delegate to type by default, but overridden for special behiour
const movementType & monster::movement() const {
  return type_.movement();
}


void moveMonster(monster &mon) {
  level & level = mon.curLevel();
  const movementType &type = mon.movement();

  auto fastness = mon.intrinsics().adjust(type.speed_);

  // do we move and - if so - how many times?
  int loopMax=0;
  switch (fastness) {
  case speed::slow3: if ((time::moveCount()) % 3 == 0) ++loopMax; break;
  case speed::slow2: if ((time::moveCount()) % 2 == 0) ++loopMax; break;
  case speed::perturn: ++loopMax; break;
  case speed::turn2: loopMax = 2; break;
  case speed::turn3: loopMax = 3; break;
  default: throw type.speed_;
  }
  for (int counter = 0; counter < loopMax; ++counter) {
    std::pair<char,char> dir(0,0);
    coord targetPos;

    switch (type.goTo_) {
    case goTo::none: 
      return;  // this does not move
    case goTo::wander:
      {
	const std::vector<char> dirs({-1, 0, +1 }); // could use boost::counting_iterator here, but I don't want the dependency
	dir.first = *rndPick(dirs.begin(), dirs.end());
	dir.first = *rndPick(dirs.begin(), dirs.end());
      }
      break;
    case goTo::player:
      targetPos = level.pcPos(); if (targetPos.first < 0) return; // player is not on this level; skip
      {auto myPos = level.posOf(mon);
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::up:
      targetPos = level.findTerrain(terrainType::UP); 
      {auto myPos = level.posOf(mon);
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::down:
      targetPos = level.findTerrain(terrainType::DOWN);
      {auto myPos = level.posOf(mon);
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;     
      }
      break;
    default:
      throw type.goTo_;
    }

    // special case: work out best direction before applying jitter; ignore dir worked out above
    if (type.goBy_ == goBy::smart) {
      auto nlm = nextLevelMoves<true, true>(mon);
      std::function<std::set<coord >(const coord &)> nextMoves = nlm;
      dir = pathfinder<2>(nextMoves).find(level.posOf(mon), targetPos);
    }


    // apply jitter:
    if (type.jitterPc > 0) {
      auto jit = dPc();
      if (jit < type.jitterPc)
	switch (jit % 8) {
	case 0: --dir.first; --dir.second; break;
	case 1: --dir.first; break;
	case 2: --dir.first; ++dir.second; break;
	case 3: --dir.second; break;
	case 4: ++dir.second; break;
	case 5: ++dir.first; --dir.second; break;
	case 6: ++dir.first; break;
	case 7: ++dir.first; ++dir.second; break;
	}
    }

    // jitter should not affect movement speed:
    if (dir.first > 1) dir.first=1;
    if (dir.second > 1) dir.second=1;
    if (dir.first < -1) dir.first=-1;
    if (dir.second < -1) dir.second=-1;

    // now move the monster:
    switch (type.goBy_) {
    case goBy::avoid:
      dir.first =- dir.first; dir.second =- dir.second;
      // no break
    case goBy::beeline:
      level.move(mon, dir, true);
      break;
    case goBy::zomavoid:
      dir.first =- dir.first; dir.second =- dir.second;
      // no break
    case goBy::zombeeline:
      level.move(mon, dir, false);
      break;
    case goBy::smart:
      level.move(mon, dir, true);
      break;
    case goBy::teleport:
      level.moveTo(mon, targetPos);
      break;
    default:
      throw type.goBy_;
    }
  }
}

void monsterAttacks(monster &mon) {
  level & level = mon.curLevel();
  // currently, all monsters will attack anything that
  // a) they are adjacent to, and
  // b) they are a different alignment to, on at least one axis OR both unaligned.
  // This needs improving, with multiple monster attacks.

  auto myPos = level.posOf(mon);
  auto &dam = mon.injury();
  auto &malign = mon.align();
  for (int dx=-1; dx <= +1; ++dx)
    for (int dy=-1; dy <= +1; ++dy) {
      coord pos(myPos.first + dx, myPos.second + dy);
      auto m = level.monstersAt(pos);
      // take a copy, in case (eg a monster dies) the collection changes
      std::vector<std::shared_ptr<monster>> monstersAt;
      for (auto i = m.first; i != m.second; ++i) monstersAt.emplace_back(i->second);
      for (auto en : monstersAt) {
	if (en.get() == &mon) continue; // monsters don't usually fight themselves
	if (en->align().coalignment(malign) >= 3) continue; // monsters don't usually fight other creautures of the same alignment
	if (en->type() == mon.type()) continue; // monsters don't usually fight other creatures of the same class
	auto result = mon.attack(*en);

	std::wstringstream msg;
	msg << (mon.isPlayer() ? L"You" : mon.name())
	    << myPos
	    << L" attacks "
	    << (en->isPlayer() ? L"you" : en->name())
	    << pos
	    << L": "
	    << result.text_;

	auto m = msg.str();

	auto &ios = ioFactory::instance();
	if (m.find(L"\n") != m.npos)
	  ios.longMsg(m);
	else
	  ios.message(m);

	if (dam.cur() == dam.max())
	  return; // defensive coding against resistive attacks from other monsters
      }
    }
}


// a boring monster with no special effects
class trivialMonster : public monster {
public:
  trivialMonster(monsterBuilder &b) : 
    monster(b) {} //
  virtual ~trivialMonster() {}
};

// monsters that perform an action when they successfully hit a target in combat
class targetActionMonster : public monster {
public:
  typedef sharedAction<monster,monster>::key key;
private:
  const key actionKey_;
public:
  targetActionMonster(monsterBuilder &b, key actionKey) :
    monster(b), actionKey_(actionKey) {}
  virtual void onHit(monster &opponent, int) {
    actionFactory<monster,monster>::get(actionKey_)(*this, opponent);
  }
};

// ferrets steal little things then run away
class ferret : public targetActionMonster {
private:
  movementType away_;
public:
  ferret(monsterBuilder &b) : 
    targetActionMonster(b, key::steal_small),
    away_({speed::turn2, goTo::player, goBy::avoid, 25}){}
  virtual ~ferret() {}
  const movementType & movement() const {
    if (empty())
      return type().movement(); // go to player; they might have sometihng fun!
    else // I've got it! Run away!
      return away_;
  }
};

// zombies: instakilled by pit traps
class zombie : public monster {
public:
  zombie(monsterBuilder &b) :
    monster(b) {}
  virtual ~zombie() {}
  std::wstring onMove(const coord &pos, const terrain &terrain) {
    auto rtn = monster::onMove(pos, terrain);
    if (terrain.type() == terrainType::PIT)
      death();
    return rtn;
  }
};

// a hound changes its saying depending on its level.
// that's probably not a very good superpower really.
class hound : public monster {
public:
  hound(monsterBuilder &b) : 
    monster(b) {} //
  virtual ~hound() {}
  virtual const wchar_t *say() const {
    const std::wstring nm = name();
    if (nm.compare(L"puppy") == 0)
      return L"Yip";
    else if (nm.compare(L"puppy") == 0)
      return L"wuff wuff"; // ref:default bell in Unix "Screen" in Nethack mode
    else if (nm.compare(L"Big Bad Wolf") == 0)
      return L"Who's afraid?"; //ref:Red Riding Hood (folktale)
    else
      return monster::say();
  }
};


class dragon : public monster {
private:
  const bool western_;
public:
  dragon(monsterBuilder &b) : 
    monster(roll(b)),
    western_(align().domination() == Domination::aggression) {
    // DRAGONS DON'T FLY! It's mythologically inaccurate...
    intrinsics().speedy(true);
    intrinsics().dblAttack(true);
  }
  virtual ~dragon() {}
  // overridden to return a type-dependant saying:
  virtual const wchar_t * say() const {
    return western_ ?
      L"You look delicious." : // ref: Tolkien, "do not meddle in the affairs of dragons, for you are crunchy and go well with ketchup" (Bilbo Baggins, The Hobbit).
      L"Do you know Cantonese?"; // ref: Chinese legend of dragons teaching humans language.
  }
private:
  static monsterBuilder& roll(monsterBuilder &b) {
    // Apply the random dragon rules
    const int roll(dPc());
    const Element element = 
      (roll < 25) ? Element::air :
      (roll <= 75) ? Element::water :
      (roll <= 80) ? Element::earth :
      Element::fire;

    bool western = element != Element::air;
    if (element == Element::water) western = (0 == coinFlip());

    const Domination dominion = western ? Domination::aggression : Domination::concentration;

    const int rollOut(dPc());
    Outlook outlook = 
      (rollOut < (western ? 25 : 75)) ? Outlook::kind :
      (rollOut < (western ? 26 : 76)) ? Outlook::none :
      Outlook::cruel;

    b.align(deityRepo::instance().getExact(element, dominion, outlook));

    // number of claws depends on the level
    // In Chinese mythology, more claws mean more power.
    const int depth = b.iLevel()->depth(); // 0 - 100 inclusive
    const int claws = std::max(1, depth / 20); // (0-39)=1, 40-59=2, 60-79=3,80-99=4, 100=5
    
    // TODO: number of heads. The hydra had multiple heads, but was not a dragon.
    // There are multiple references to dragons with 2, 3, 9 or multiples of 9 heads.
    const int val(dPc() / 10 - 5);
    const int heads = 
      (val <= 1) ? 1 :
      (val > claws) ? claws : // never more heads than claws.
      val;

    return b;
  }
};

std::shared_ptr<monster> ofType(const monsterType &type, level & level) {
  monsterBuilder b(true);
  b.startOn(level);
  // stats, alignment etc are also set when type is set:
  //  b.type(monsterTypeRepo::instance()[type]);
  b.type(type);

  // invoke move() every move, even when the player is helpless:
  std::shared_ptr<monster> ptr;
  switch (type.type()) {
  case monsterTypeKey::ferret:
    ptr = std::make_shared<ferret>(b);
    break;
  case monsterTypeKey::goblin:
    ptr = std::make_shared<targetActionMonster>(b, targetActionMonster::key::steal_shiny);
    break;
  case monsterTypeKey::hound:
    ptr = std::make_shared<hound>(b);
    break;
  case monsterTypeKey::zombie:
    ptr = std::make_shared<zombie>(b);
    break;
  case monsterTypeKey::dragon:
    ptr = std::make_shared<dragon>(b);
    break;
  case monsterTypeKey::bird:
  case monsterTypeKey::birdOfPrey:
  default:
    ptr = std::make_shared<trivialMonster>(b);
  }
  // pass by value works, but creates an extra 2 persistent refs, causing a memory leak as the ref-count never hits 0.
  // pass by reference causes a SIGSEGV; not sure why.
  // you can't create a second shared_ptr on the same pointer.
  auto &m = *ptr;
  ptr->eachTick([&m]() {moveMonster(m);} );
  ptr->eachTick([&m]() {monsterAttacks(m);} );
  return ptr;
}

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms) {
  auto from = monsterTypeRepo::instance().begin();
  auto to = monsterTypeRepo::instance().end();
  return rndGen<monsterType*, std::vector<monsterType*>::iterator>(from, to, depth, rooms);
}

bool monster::removeItemForMove(item &item, itemHolder &next) {
  if (item.isCursed()) return false; // cursed items can't be dropped.
  if (slotsOf(item)[0] != nullptr && !unequip(item)) return false; // must unequip before move
  return itemHolder::removeItemForMove(item, next);
}

void monster::forEachItem(const std::function<void(item&, std::wstring)> f) {
  itemHolder::forEachItem([this, f](item& i, std::wstring msg) {
    auto slots = slotsOf(i);
    if (slots[0] != nullptr) {
      msg += L": ";
      msg += slots[0]->name(monster::type().category());
      msg += L" ";
      if (slots[1] != slots[0]) {
	msg += slots[1]->name(monster::type().category());
	msg += L" ";
      }
    }
    f(i, msg);
    });
}

void monster::forEachItem(const std::function<void(const item&, std::wstring)> f) const {
  itemHolder::forEachItem([this, f](const item& i, std::wstring msg) {
    auto slots = slotsOf(i);
    if (slots[0] != nullptr) {
      msg += L": ";
      msg += slots[0]->name(monster::type().category());
      msg += L" ";
      if (slots[1] != slots[0]) {
	msg += slots[1]->name(monster::type().category());
	msg += L" ";
      }
    }
    f(i, msg);
    });
}

class corpse;

bool monster::eat(item &item) {
  if (!type().eats(item.material())) throw inedibleException();
  // TODO: Penalties for eating corpses?
  if (damage_.cur() == 0)
    throw notHungryException();
  double weight = std::ceil(item.weight()); // round up
  // subtract from damage; can't go below 0: 
  damage_ -= (weight > 254.5) ? 255 : static_cast<unsigned char>(weight);
  return item.holder().destroyItem(item);
}

void monster::eat() {
  auto isEdible = [this](item &i) {
    if (!type().eats(i.material())) return false;
    if (isPlayer())
      return ioFactory::instance().ynPrompt(std::wstring(L"Eat ") + i.name() + L"?");
    return true; // monsters eat whatever
  };
  // is there anything to eat in the current location?
  auto &holder = level_->holder(level_->posOf(*this));
  optionalRef<item> it = holder.firstItem(isEdible);
  // if not, try our own inventory:
  if (!it) it = firstItem(isEdible);
  if (!it) {
    if (isPlayer()) ioFactory::instance().message(L"There's nothing to eat here.");
    return;
  }

  try {
    // keep eating until the 
    std::weak_ptr<item> pItem = it.value().shared_from_this();
    while (pItem.lock() && eat(it.value()));
  } catch (notHungryException e) {
    if (isPlayer()) ioFactory::instance().message(L"You don't feel hungry right now.");
  } catch (inedibleException e) {
    if (isPlayer()) ioFactory::instance().message(L"This doesn't look tasty.");
  }
}
