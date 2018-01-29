/* License and copyright go here*/

// define a monster in the dungeon

#include "monster.hpp"
#include "random.hpp"
#include "items.hpp"
#include "output.hpp"
#include "terrain.hpp"
#include "dungeon.hpp"
#include "religion.hpp"
#include "pathfinder.hpp"
#include <sstream>

//Roll 2D52-2
unsigned char dPc() {
  return static_cast<unsigned char>(d51() + d51() - 2);
}


monster::monster(monsterBuilder & b) :
  equippable(slotsFor(b.type_->category())),
  level_(b.iLevel()),
  highlight_(b.isHighlight()),
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
  intrinsics_(),
  abilities_(intrinsics_),
  onDeath_(b.onDeath()) {}

monster::monster(monsterBuilder & b, std::vector<const slot *>slots) :
  equippable(slots),
  level_(b.iLevel()),
  highlight_(b.isHighlight()),
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
  intrinsics_(),
  abilities_(intrinsics_),
  onDeath_(b.onDeath()) {}

void monster::eachTick(const std::function<void()> &callback) { 
  //  eachTick_.emplace_back(new time::callback(true, callback));
  eachTick_.emplace_back(true, callback);
}

const wchar_t monster::render() const { // delegate to type by default
  return type_.renderChar();
}

std::wstring monster::name() const {
  auto damage = damage_.max();
  return type_.name(damage);
}

bool monster::highlight() const {
  return highlight_;
}

std::wstring monster::description() const {
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
characteristic& monster::strength() { return strength_; }
characteristic& monster::appearance() { return appearance_; }
characteristic& monster::fighting() { return fighting_; }
characteristic& monster::dodge() { return dodge_; }
characteristic& monster::injury() { return damage_; }
const characteristic& monster::male() const { return male_; }
const characteristic& monster::female() const { return female_; }

materialType monster::material() const { return type_.material(); }
const monsterType& monster::type() const { return type_; }

const deity &monster::align() const { return *align_; }

const attackResult monster::attack(monster &target) {
  if (std::find(charmedBegin(), charmedEnd(), &target) != charmedEnd()
      && dPc() < target.appearance().cur())
    return attackResult(0, L"seems to be under a charm");

  // We hit (unless dodged) the target if D% < fighting, or always on a roll of 0.
  auto dHit = dPc();
  unsigned char f = fighting().cur();
  
  if (dHit > f) return attackResult(injury(), L"miss");

  // find the weapon:
  auto weapon = findWeapon();
  damageType type = (weapon) ? weapon.value().weaponDamage(true) : unarmedDamageType();
  auto dt = damageRepo::instance()[type];

  // Now to see if the opponent dodged it...
  unsigned char d = target.dodge().cur();
  if (dHit < d) {
    target.damageArmour(dt); // always damage armour on unsuccessful attack, unless proofed.
    return attackResult(injury(), L"evaded");
  }

  // Now to see how much damage we did...
  auto max = target.injury().max();
  int damage = target.wound(strength_.cur(), dt);
  bool fatal = static_cast<unsigned char>(damage) >= max;
  if (!fatal) onHit(target, damage);
  if (damage == 0) return attackResult(injury(), L"ineffectual");
  if (fatal) return attackResult(injury(), L"fatal");
  if (static_cast<unsigned char>(damage) >= max/2) return attackResult(injury(), L"good hit");
  return attackResult(injury(), L"hit");
}

damageType monster::unarmedDamageType() const {
  return damageType::bashing;
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

bool monster::damageArmour(const damage &d) {
  auto w = findWeapon();
  if (!w) return false; // nothing to damage, so fail
  auto &item = w.value();
  bool rtn = item.strike(d.type());
  if (rtn && item.damageOfType(d.type()) > 4) {
    if (isPlayer()) ioFactory::instance().message(L"Your " + item.name() + L" couldn't survive the " + d.name() );
    destroyItem(item);
  }
  return rtn;
}

long monster::modDamage(/*by value*/long pc, const damage & type) const {
  char delta = abilities().resist(type);
  pc -= delta * 5; // NB: Can go positive, such is the goodness of magic
  return equippable::modDamage(pc, type);
}


// called upon death...
void monster::death() {
  for (auto f : onDeath_) f();
  level_->removeDeadMonster(*this);
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

const level & monster::curLevel() const {
  return *level_;
}

bool monster::onMove(const coord &pos, const terrain &terrain) {
  if (intrinsics_.entrapped()) {
    intrinsics_.entrap(-1);
    if (isPlayer()) {
      auto &ios = ioFactory::instance();
      if (!intrinsics_.entrapped())
	ios.message(L"You can move again now."); // next turn
      else 
	ios.message(L"You try, but you can't move yet!");
      return false;
    }
  }
  switch (terrain.type()) {
  case terrainType::PIT_HIDDEN:
    if (isPlayer()) // TODO: Pit traps should not be revealed by flying monsters, or should they?
      ioFactory::instance().message(L"It's a (pit) trap!"); // ref: Admiral Ackbar, Star Wars film Episode VI: Return of the Jedi.
    return true; // you can move into a hidden pit
  case terrainType::PIT: {
    fall(dPc() / 10);
    const auto climb = abilities().climb();
    const int count=climb == bonus(false) ? 6 : climb == bonus() ? 4 : 2;
    auto rtn = !intrinsics_.entrapped();
    intrinsics_.entrap(count);
    return rtn;
  }
  default:
    return true;
  }
}
void monster::postMove(const coord &pos, const terrain &terrain) {
  switch (terrain.type()) {
  case terrainType::PIT: {
    bool flying(abilities().fly());
    if (flying) {
      if (isPlayer()) ioFactory::instance().message(L"You are now over a pit.");
    } else {
      if (isPlayer()) ioFactory::instance().message(L"You are now in a pit.");
    }
  }
  default:
    return;
  }
  
}

void monster::fall(unsigned char reductionPc) {
  auto damage = wound(dPc() / 10, damageRepo::instance()[damageType::bashing]);
  if (isPlayer()) ioFactory::instance().message
		    (damage == 0 ?
		     L"You adjust to the sudden change in altitude." :
		     L"Gravity hurts sometimes.");
  return;
}

void monster::onLevel(level * lvl) {
  level_ = lvl;
}

const wchar_t * monster::say() const {
  return *rndPick(type().sayingsBegin(), type().sayingsEnd());
}

monsterAbilities& monster::abilities() {
  return abilities_;
}
const monsterAbilities& monster::abilities() const {
  return abilities_;
}

monster::~monster() {}

bool monster::operator == (const monster &rhs) { return this == &rhs; }

/*
 * Algorithm to find next available moves within a level.
 */
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
	if (level_.movable(pos, pos, mon_, avoidTraps, avoidHiddenTraps)) 
	  rtn.emplace(pos);
      }	
    return rtn;
  }
};

/*
 * Algorithm to find next available moves within a level.
 * Returns only cardinal moves; faster for longer distances but may miss some monster-only paths.
 */
template<bool avoidTraps, bool avoidHiddenTraps>
class nextLevelCardMoves {
private:
  const monster &mon_;
  const level &level_;
public:
  nextLevelCardMoves(monster &mon) :
    mon_(mon), level_(mon.curLevel()) {};
  std::set<coord> operator()(const coord &c) const {
    static std::set<dir> all({{dir(-1,0), dir(0,-1), dir(1,0), dir(0,1)}});
    std::set<coord> rtn;
    for (auto dir : all) {
      auto pos = c.inDir(dir);
      if (pos.first < 0 || pos.second < 0 ||
	  pos.first >= level::MAX_WIDTH || pos.second >= level::MAX_HEIGHT)
	continue; // off the map; here be monsters...
      if (level_.movable(pos, pos, mon_, avoidTraps, avoidHiddenTraps)) 
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
  auto pcPos = level.pcPos();
  if (pcPos.first < 0) return; // only bother moving if player is on the level.
  const movementType &type = mon.movement();

  auto fastness = mon.intrinsics().adjust(type.speed_);
  auto myPos = level.posOf(mon);

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
    ::dir dir(0,0);
    coord targetPos;

    bool charmed = false;
    if (mon.charmedBegin() != mon.charmedEnd()) {
      auto pM = rndPick(mon.charmedBegin(), mon.charmedEnd());
      if (dPc() < (*pM)->appearance().cur()) {
	targetPos = level.posOf(**pM);
	dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	charmed = true;
      }
    }

    if (!charmed) switch (type.goTo_) {
    case goTo::none: 
      return;  // this does not move
    case goTo::wander:
      {
	const std::vector<char> dirs({-1, 0, +1 }); // could use boost::counting_iterator here, but I don't want the dependency
	dir.first = *rndPick(dirs.begin(), dirs.end());
	dir.second = *rndPick(dirs.begin(), dirs.end());
      }
      break;
    case goTo::coaligned:
      if (pcPos.first < 0 ||
	  level.dung().pc()->align().coalignment(mon.align()) >= 3) {
	mon.curLevel().forEachMonster([&mon, &dir, &level, &targetPos, &pcPos, &myPos](monster &m){
	    if (m.align().coalignment(mon.align()) >= 3) {
	      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
	      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	    }
	  });
	// TODO: Would be nice to move to coaligned zone if no monster exists
	break;
      } // else fall right through to player
    case goTo::unaligned:
      if (type.goTo_ == goTo::unaligned && (
	  pcPos.first < 0 ||
	  level.dung().pc()->align().coalignment(mon.align()) < 3)) {
	mon.curLevel().forEachMonster([&mon, &dir, &level, &targetPos, &pcPos, &myPos](monster &m){
	    if (m.align().coalignment(mon.align()) >= 3) {
	      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
	      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
	      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
	    }
	  });
      break;
      } // else fall through to goTo::player
    case goTo::player:
      targetPos = pcPos; if (targetPos.first < 0) return; // player is not on this level; skip
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::up:
      targetPos = level.findTerrain(terrainType::UP); 
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;
      }
      break;
    case goTo::down:
      targetPos = level.findTerrain(terrainType::DOWN);
      {
      dir.first = myPos.first < targetPos.first ? 1 : myPos.first == targetPos.first ? 0 : -1;
      dir.second = myPos.second < targetPos.second ? 1 : myPos.second == targetPos.second ? 0 : -1;     
      }
      break;
    default:
      throw type.goTo_;
    }

    // special case: work out best direction before applying jitter; ignore dir worked out above
    if (type.goBy_ == goBy::smart) {
      if (pathfinder<2>::absdistance(myPos, pcPos) < 4) {
	// very close; use a more accurate pathfinder
	auto nlm = nextLevelMoves<true, true>(mon); // up to 8 options
	std::function<std::set<coord >(const coord &)> nextMoves = nlm;
	dir = pathfinder<2>(nextMoves).find(myPos, targetPos); // order: 8 ^ 2 = 64
      } else {
	// only consider cardinal moves. Jitter may still help.
	auto nlm = nextLevelCardMoves<true, true>(mon); // up to 4 options
	std::function<std::set<coord >(const coord &)> nextMoves = nlm;
	dir = pathfinder<8>(nextMoves).find(myPos, targetPos); // order: 4 ^ 6 = 4096
      }
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

  // we don't allow attacking until the player reaches us
  if (&(level.dung().cur_level()) != &level) return;

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
      std::vector<ref<monster> > monstersAt = m;
      for (auto ren : monstersAt) {
	auto &en = ren.value();
	if (&en == &mon) continue; // monsters don't usually fight themselves
	if (en.align().coalignment(malign) >= 3) continue; // monsters don't usually fight other creautures of the same alignment
	if (en.type() == mon.type()) continue; // monsters don't usually fight other creatures of the same class
	std::wstringstream msg;
	msg << (mon.isPlayer() ? L"You" : mon.name())
	    << myPos
	    << L" attacks "
	    << (en.isPlayer() ? L"you" : en.name())
	    << pos
	    << L": ";
	auto result = mon.attack(en); // may invalidate refernec mon.
	msg << result.text_;

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


bool monster::destroyItem(item &item) {
  if (slotsOf(item)[0] != nullptr && !unequip(item)) return false; // must unequip before move
  return itemHolder::destroyItem(item);
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

bool monster::eat(item &item, bool force) {
  double weight = std::ceil(item.weight()); // round up
  if (!type().eats(item.material())) {
    if (force) weight *= -1;
    else throw inedibleException();
  }
  // some things can affect consumption:
  bonus bonus;
  if (item.material() == materialType::veggy) 
    bonus = abilities().eatVeggie();
  // TODO: Penalties for eating corpses?
  if (damage_.cur() == 0) {
    if (force && weight > 0) weight *= -1;
    else throw notHungryException();
  }
  if (bonus == ::bonus(true)) { weight *= 2; }
  if (bonus == ::bonus(false)) { weight *= 0.5; }
  auto rtn = item.holder().destroyItem(item);
  // subtract from damage; can't go below 0: 
  damage_ -= (weight > 254.5) ? 255 : static_cast<unsigned char>(weight);
  return rtn;
}

void monster::eat() {
  auto isEdible = [this](item &i) {
    if (!type().eats(i.material())) return false;
    if (isPlayer())
      return ioFactory::instance().ynPrompt(L"Eat " + i.name() + L"?");
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



bool monster::setCharmedBy(monster & mon) {
  auto m = &mon;
  if (m == this) return false;
  if (std::find(charmedBy_.begin(), charmedBy_.end(), m) != charmedBy_.end()) return false;
  charmedBy_.push_back(m);
  return true;
}

std::list<monster*>::const_iterator monster::charmedBegin() const {
  return charmedBy_.begin();
}
std::list<monster*>::const_iterator monster::charmedEnd() const {
  return charmedBy_.end();
}

void monster::onEquip(item &item, const slot *s1, const slot *s2) {
  // we can equip the item. Calculate any previous bonuses
  const int strBonus = equippable::strBonus();
  const int appBonus = equippable::appBonus();
  const int dodBonus = equippable::dodBonus();
  // equip the item
  equippable::onEquip(item, s1, s2);  
  // calculate any new bonuses and apply adjustment
  strength_.adjustBy(equippable::strBonus() - strBonus);
  appearance_.adjustBy(equippable::appBonus() - appBonus);
  dodge_.adjustBy(equippable::dodBonus() - dodBonus);
}

bool monster::unequip(item &item) {
  if (item.isCursed()) return false;
  // Calculate any previous bonuses
  const int strBonus = equippable::strBonus();
  const int appBonus = equippable::appBonus();
  const int dodBonus = equippable::dodBonus();
  bool rtn = equippable::unequip(item);
  if (rtn) {
    // calculate any new bonuses and apply adjustment
    strength_ += equippable::strBonus() - strBonus;
    appearance_ += equippable::appBonus() - appBonus;
    dodge_ += equippable::dodBonus() - dodBonus;
    item.onUnequip(*this);
  }
  return rtn;
}

std::array<const slot *, 2> monster::forceUnequip(item &item) {
  // Calculate any previous bonuses
  const int strBonus = equippable::strBonus();
  const int appBonus = equippable::appBonus();
  const int dodBonus = equippable::dodBonus();
  auto rtn = equippable::forceUnequip(item);
  // calculate any new bonuses and apply adjustment
  strength_ += equippable::strBonus() - strBonus;
  appearance_ += equippable::appBonus() - appBonus;
  dodge_ += equippable::dodBonus() - dodBonus;
  item.onUnequip(*this);
  return rtn;
}
