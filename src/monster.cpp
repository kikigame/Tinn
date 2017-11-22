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

/*
monster::monster(level * level, const monsterType &type) : 
  level_(level),
  strength_(characteristic(20)),
  appearance_(characteristic(20)),
  fighting_(characteristic(20)),
  dodge_(characteristic(20)),
  damage_(characteristic(0, 20)),
  male_(characteristic(0)),
  female_(characteristic(0)),
  type_(type),
  align_(&deityRepo::instance().nonaligned()) {}
*/

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
  align_(b.align_) {
  // create all slots as empty initially
  for (auto slot : slotsFor(type_.category()))
    equipment_.emplace(slot, std::auto_ptr<item>(NULL));
}

void monster::eachTick(const std::function<void()> &callback) { 
  //  eachTick_.emplace_back(new time::callback(true, callback));
  eachTick_.emplace_back(true, callback);
}

const wchar_t monster::render() const { // delegate to type by default
  return type_.renderChar();
}

const wchar_t * const monster::name() const {
  auto numNames = type_.names().size();
  if (numNames == 1) return type_.names().at(0); // optimisation
  auto damage = damage_.max();
  if (damage < type_.iMaxDamage()) return type_.names().at(0); // safety
  // we will take damage.max() as our designated stat.
  // A type-0 monster has type_.iMaxDamage() HP.
  // A type-N monster has up to 100 max HP.
  // We want to divide the levels into N ranges between type_.iMaxDamage() and 100.
  auto threshold = (100 - type_.iMaxDamage()) / (numNames + 1);
  // eg if there are 3 names and iMaxDamage() = 40, that gives t=15
  // so we "grow up" at 55, 70, 85, meaning 15 points at each name.
  auto idx = (damage - type_.iMaxDamage()) / threshold;
  return type_.names().at(idx);
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
  damageType type;
  if (weap != equipment_.end() && weap->second) type = weap->second->weaponDamage();
  else type = damageType::bashing;
  auto dt = damageRepo::instance()[type];

  // Now to see how much damage we did...
  auto max = target.injury().max();
  int damage = target.wound(strength_.cur(), dt);
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
    auto item = i.second;
    if (item) pc = item->modDamage(pc, type);
  }
  return pc;
}


// called upon death...
void monster::death() {
  level_->removeDeadMonster(*this);
}


// calculate the current strength bonus from equipment
int calcStrBonus(const std::map<const slot*, std::shared_ptr<item> > eq) {
  int rtn = 0;
  auto e = eq.end();
  for (auto s : weaponSlots()) {
    auto i = eq.find(s);
    if (i != e && i->second != nullptr) {
      auto it = i->second;
      // +1 so that even an unenchanted weapon does some damage:
      rtn += it->enchantment() + 1;
    }
  }
  return rtn;
}

// TODO: shields. These occupy weapon slots, and allow for deflection rather than armour.
// calculate the current strength bonus from equipment
int calcDodBonus(const std::map<const slot*, std::shared_ptr<item> > eq) {
  int rtn = 0;
  auto wp = weaponSlots();
  auto wpe = wp.end();
  for (auto i : eq)
    if (i.second && wp.find(i.first) == wpe) {
      // considering occupied non-weapon slots
      auto &item = *(i.second);
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

class coverSearch : public graphSearch<const slot*, std::shared_ptr<item> > {
  virtual std::vector<const slot*> deeper(const slot* const & sl) {
    return sl->covered();
  }
};

// calculate the current appearance bonus from equipment
int calcAppBonus(const std::map<const slot*, std::shared_ptr<item> > eq) {
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
	auto &item = *(i.second);
	rtn += item.enchantment();
	if (item.isSexy()) ++rtn;
	if (item.isSexy() && item.isBlessed()) ++rtn;
	// TODO: tshirts should get extra bonuses for being torn or wet (double if blessed).
	// the tshirt bonus is easy to get, but not that useful as it will generally be covered.
      }
    }
  return rtn;
}

bool monster::equip(std::shared_ptr<item> item, const slotType slot) {
  auto s = slotBy(slot);
  auto i = equipment_.find(s);
  if (i == equipment_.end()) return false;
  if (i->second.get() != nullptr) return false;
  // we can equip the item. Calculate any previous bonuses
  const int strBonus = calcStrBonus(equipment_);
  const int appBonus = calcAppBonus(equipment_);
  const int dodBonus = calcDodBonus(equipment_);
  // equip the item
  //  equipment_.insert(std::pair<const slot*, std::shared_ptr<item>>(s, item));
  equipment_[s] = item;
  // calculate any new bonuses and apply adjustment
  strength_ += calcStrBonus(equipment_) - strBonus;
  appearance_ += calcAppBonus(equipment_) - appBonus;
  dodge_ += calcDodBonus(equipment_) - dodBonus;
  return true;
}

bool monster::unequip(std::shared_ptr<item> item) {
  if (item->isCursed()) return false;
  auto eend = equipment_.end();
  for (auto e = equipment_.begin(); e != eend; ++e)
    if (e->second == item) {
      // we can equip the item. Calculate any previous bonuses
      const int strBonus = calcStrBonus(equipment_);
      const int appBonus = calcAppBonus(equipment_);
      const int dodBonus = calcDodBonus(equipment_);
      // unequip the item
      equipment_.erase(e); 
      // calculate any new bonuses and apply adjustment
      strength_ += calcStrBonus(equipment_) - strBonus;
      appearance_ += calcAppBonus(equipment_) - appBonus;
      dodge_ += calcDodBonus(equipment_) - dodBonus;
      return true;
    }
  return false;
}

bool monster::slotAvail(const slot *s) const {
  return equipment_.find(s) != equipment_.end();
}
const slot * monster::slotOf(std::shared_ptr<item> item) const {
  for (auto i : equipment_) {
    if (i.second != nullptr && i.second.get() == item.get())
      return i.first;
  }
  return nullptr;
}

typedef std::shared_ptr<item> pItem;

bool monster::drop(pItem ite, const coord &c) {
  if (ite->isCursed()) return false;
  // unequip before dropping:
  const slot * sl = slotOf(ite);
  if (sl != nullptr && !unequip(ite)) return false;
  auto i = std::find_if(inventory_.begin(), inventory_.end(), 
			[ite](const pItem &it) {return it.get() == ite.get();});
  if (i == inventory_.end()) return false; // can't drop what we don't have
  // drop
  inventory_.erase(i);
  level_->addItem(ite, c);
  return true;
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

bool monster::addItem(std::shared_ptr<item> item) {
  inventory_.emplace_back(item);
  return true;
}
iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > >  monster::contents() {
  return iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > >(inventory_);
}

void monster::onMove(const coord &pos, const terrain &terrain) {
  if (terrain.type() == terrainType::PIT) {
    bool flying(false); // TODO: flying monsters
    const wchar_t * const message =
      flying ? L"You are now over a pit." :
      fall(dPc() / 10);
    // TODO: show message
  }
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

monster::~monster() {}

bool monster::operator == (const monster &rhs) { return this == &rhs; }


void moveMonster(monster &mon) {
  level & level = mon.curLevel();
  const movementType &type = mon.type().movement();

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
      dir.first =- dir.first; dir.first =- dir.first;
      // no break
    case goBy::beeline:
      level.move(mon, dir, true);
      break;
    case goBy::zomavoid:
      dir.first =- dir.first; dir.first =- dir.first;
      // no break
    case goBy::zombeeline:
      level.move(mon, dir, false);
      break;
    case goBy::smart:
      throw L"Not yet implemented: path finding";
    case goBy::teleport:
      level.moveTo(mon, targetPos);
      break;
    default:
      throw type.goBy_;
    }
  }
}

void monsterAttacks(monster &mon, const std::shared_ptr<io> ios) {
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

	if (m.find(L"\n") != m.npos)
	  ios->longMsg(m);
	else
	  ios->message(m);

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

// zombies: instakilled by pit traps
class zombie : public monster {
public:
  zombie(monsterBuilder &b) :
    monster(b) {}
  virtual ~zombie() {}
  virtual void onMove(const coord &pos, const terrain &terrain) {
    monster::onMove(pos, terrain);
    if (terrain.type() == terrainType::PIT)
      death();
  }
};

class dragon : public monster {
private:
  const bool western_;
public:
  dragon(monsterBuilder &b) : 
    monster(roll(b)),
    western_(align().domination() == Domination::aggression) {
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

    // TODO: number of claws depends on the level
    // In Chinese mythology, more claws mean more power.
    const int claws = 5;
    
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

std::shared_ptr<monster> ofType(const monsterType &type, level & level, const std::shared_ptr<io> ios ) {
  monsterBuilder b(true);
  b.startOn(level);
  // stats, alignment etc are also set when type is set:
  //  b.type(monsterTypeRepo::instance()[type]);
  b.type(type);

  // invoke move() every move, even when the player is helpless:
  std::shared_ptr<monster> ptr;
  switch (type.type()) {
  case monsterTypeKey::zombie:
    ptr = std::make_shared<zombie>(b);
    break;
  case monsterTypeKey::dragon:
    ptr = std::make_shared<dragon>(b);
    break;
  default:
    ptr = std::make_shared<trivialMonster>(b);
  }
  // pass by value works, but creates an extra 2 persistent refs, causing a memory leak as the ref-count never hits 0.
  // pass by reference causes a SIGSEGV; not sure why.
  // you can't create a second shared_ptr on the same pointer.
  auto &m = *ptr;
  ptr->eachTick([&m]() {moveMonster(m);} );
  ptr->eachTick([&m, ios]() {monsterAttacks(m, ios);} );
  return ptr;
}

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms) {
  auto from = monsterTypeRepo::instance().begin();
  auto to = monsterTypeRepo::instance().end();
  return rndGen<monsterType*, std::vector<monsterType*>::iterator>(from, to, depth, rooms);
}
