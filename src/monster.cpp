/* License and copyright go here*/

// define a monster in the dungeon

#include "monster.hpp"
#include "random.hpp"
#include "items.hpp"
#include "output.hpp"
#include "terrain.hpp"
#include "monstermutation.hpp"

//Roll 2D52-2
unsigned char dPc() {
  return static_cast<unsigned char>(d51() + d51() - 2);
}

// store the charmed monsters outside of the object, otherwise we get
// into all sorts of shenanigans trying to clear up pointers when the
// monsters die.
std::multimap<const monster*, const monster*> charmedMonsters;
std::multimap<const monster*, const monster*> charmedMonstersReverse;


monster::monster(monsterBuilder & b) :
  equippable(slotsFor(b.type_->category())),
  level_(b.iLevel()),
  flags_(b.isHighlight() ? 1 : 0),
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
  intrinsics_(b.type_->intrinsics()),
  abilities_(std::make_shared<monsterAbilityMods>(*this, intrinsics_)),
  onDeath_(b.onDeath()),
  mutations_() {}

monster::monster(monsterBuilder & b, std::vector<const slot *>slots) :
  equippable(slots),
  level_(b.iLevel()),
  flags_(b.isHighlight() ? 1 : 0),
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
  intrinsics_(b.type_->intrinsics()),
  abilities_(std::make_shared<monsterAbilityMods>(*this, intrinsics_)),
  onDeath_(b.onDeath()),
  mutations_() {}

void monster::eachTick(const std::function<void()> &callback) { 
  //  eachTick_.emplace_back(new time::callback(true, callback));
  eachTick_.emplace_back(true, callback);
}

const wchar_t monster::render() const { // delegate to type by default
  if (isMutated(mutationType::GHOST)) return L' ';
  return type_.renderChar();
}

std::wstring monster::name() const {
  std::wstring buffer;
  auto damage = damage_.max();
  auto &name = type_.name(damage);
  for (auto a : adjectives())
    buffer += a + L" ";
  for (auto a : name) // monster type adjectives
    buffer += std::wstring(a) + L" ";
  for (auto &m : mutations_)
    if (m.get().appliesTo(type()))
      buffer += m.get().prefix();
  buffer += name.name();
  return buffer;
}

bool monster::highlight() const {
  return flags_[0];
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
bool monster::align(const deity &d) {
  align_ = &d;
  return &d == &align();
}

/*
 * This is the public method called whenever one monster attacks another.
 * For the player, we always want this to be a plain attack or a monster attack, if any.
 * For other monsters, we should also consider any applicable item attacks.
 */
const attackResult monster::attack(monster &target) {
  if (std::find_if(charmedBegin(), charmedEnd(), [&target](const std::pair<const monster*, const monster*> &m){return m.second == &target;}) != charmedEnd()
      && dPc() < target.appearance().cur())
    return attackResult(0, L"seems to be under a charm");

  auto cur = target.injury().cur();
  auto max = target.injury().max();
  int damage;

  // non-player monsters may choose to use a monster ability instead
  auto attack = attackAction();
  if (attack) {
    if (!attack.value()(bcu(), *this, target))
      return attackResult(0, L"failed");
    damage = target.injury().cur() - cur;
  } else {
  
    // We hit (unless dodged) the target if D% < fighting, or always on a roll of 0.
    auto dHit = dPc();
    unsigned char f = fighting().cur();
  
    if (dHit > f) return attackResult(injury(), L"miss");

    // find the weapon:
    auto weapon = findWeapon();
    damageType type = (weapon) ? weapon.value().weaponDamage(true) : unarmedDamageType();
    auto dt = damageRepo::instance()[type];

    // Now to see if the opponent dodged it...
    if (!isMutated(mutationType::GHOST) &&
	target.isMutated(mutationType::GHOST) &&
	(!weapon || !weapon.value().isBlessed()))
      return attackResult(injury(), L"unaffected");
    unsigned char d = target.dodge().cur();
    if (dHit < d/2) {
      target.damageArmour(dt); // always damage armour on unsuccessful attack, unless proofed.
      return attackResult(injury(), L"evaded");
    }

    // Now to see how much damage we did...
    damage = target.wound(*this, strength_.cur(), dt);
  }
  bool fatal = static_cast<unsigned char>(cur + damage) >= max;
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
int monster::wound(const monster &by, unsigned char reductionPc, const damage & type) {
  long damage = dPc();
  damage *= reductionPc;
  damage /= 100;
  damage += 5 * by.abilities()->extraDamage(type);
  damage = modDamage(damage, type);
  damage_ += static_cast<unsigned char>(damage);
  if (damage_.cur() == damage_.max()) {
    if (!isPlayer())
      for (auto &mut : by.mutations_)
	if (mut.get().spreadsOnKill(type_)) {
	  mutate(mut.get().type());
	  damage_ = static_cast<unsigned int>(0);
	  return damage;
	}
    auto w = by.findWeapon();
    bool allowCorpse = true;
    if (w) {
      auto weapon = dynamic_cast<weaponMixin*>(&w.value());
      if (weapon && weapon->onKill(*this)) allowCorpse = false;
    }
    death(allowCorpse);
  } else if (!isPlayer() && damage > 0)
    for (auto &mut : by.mutations_)
      if (mut.get().spreadsOnAttack(type_)) {
	mutate(mut.get().type());
	damage_ -= static_cast<unsigned int>(damage / 2);
	break;
      }
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
  char delta = abilities()->resist(type);
  pc -= delta * 5; // NB: Can go positive, such is the goodness of magic
  return equippable::modDamage(pc, type);
}


// when a monster dies, clear its charms:
void clearMonsterCharms(monster *mon) {
  std::list<const monster*> charmees; // everyone mon charms
  auto iCharmees = charmedMonsters.equal_range(mon);
  for (auto i = iCharmees.first; i != iCharmees.second; ++i)
    charmees.emplace_back(i->second);
  for (auto m : charmees) {
    auto c = charmedMonstersReverse.equal_range(m);
    auto iter = c.first;
    while (iter != c.second) {
      if (iter->second == mon) iter = charmedMonstersReverse.erase(iter);
      else ++iter;
    }
  }
  charmedMonsters.erase(iCharmees.first, iCharmees.second);
  std::list<const monster*> charmers; // everyone charming mon...
  auto iCharmers = charmedMonstersReverse.equal_range(mon);
  for (auto i = iCharmers.first; i != iCharmers.second; ++i)
    charmers.emplace_back(i->second);
  for (auto m : charmers) {
    auto c = charmedMonsters.equal_range(m);
    auto iter = c.first;
    while (iter != c.second) {
      if (iter->second == mon) iter = charmedMonsters.erase(iter);
      else ++iter;
    }
  }
  charmedMonstersReverse.erase(iCharmers.first, iCharmers.second);
}

// called upon death...
void monster::death(bool allowCorpse) {
  for (auto f : onDeath_) f();
  clearMonsterCharms(this);
  level_->removeDeadMonster(*this, allowCorpse);
}

// can this monster capture (move onto & instakill) all monsters on pos?
bool monster::capture(const coord &pos) const {
  return false;
}
// called when capturing other monsters (monsters removed from "prey" will not be captured):
void monster::captured(std::vector<std::shared_ptr<monster>> &prey) {}


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

std::shared_ptr<monsterIntrinsics> monster::intrinsics() {
  return intrinsics_;
}

const std::shared_ptr<monsterIntrinsics> monster::intrinsics() const {
  return intrinsics_;
}

level & monster::curLevel() {
  return *level_;
}

const level & monster::curLevel() const {
  return *level_;
}

bool monster::sleeping() const {
  return flags_[1];
}

class monsterAlarm {
private:
  monster &m_;
  std::vector<std::function<void()>> &onDeath_;
  int ticks_;
  time::callback fn_;
  std::vector<std::function<void()>>::iterator od_;
public:
  monsterAlarm(monster &m,
	       std::vector<std::function<void()>> &onDeath,
	       int ticks) :
    m_(m),
    onDeath_(onDeath),
    ticks_(ticks), fn_(true, std::function<void()>([this]() {
	  if (--ticks_ == 0) {
	    m_.awaken();
	    time::offTick(fn());
	    m_.onDeath_.erase(od_);
	    delete this;
	  }    
	})) {};
  monsterAlarm(const monsterAlarm &) = delete;
public:
  time::callback &fn() { return fn_; }
  void od(std::vector<std::function<void()>>::iterator &od) {
    od_ = od;
  }
};

bool monster::sleep(int ticks) {
  if (!abilities()->sleeps()) return false;
  flags_[1] = 1;
  //  time::onTick([th
  monsterAlarm *ticker = new monsterAlarm(*this, onDeath_, ticks);
  auto &fn = ticker->fn();
  time::onTick(fn);
  onDeath_.push_back([&fn,ticker]() {
      time::offTick(fn);
      delete ticker;
    });
  auto ode = onDeath_.end();
  std::advance(ode, -1);
  ticker->od(ode);
  return true;
}
bool monster::awaken() {
  bool rtn = flags_[1];
  flags_[1] = 0;
  return rtn;
}


bool monster::onMove(const coord &pos, const terrain &terrain, const dir &d) {
  if (intrinsics_->entrapped()) {
    intrinsics_->entrap(-1);
    if (isPlayer()) {
      auto &ios = ioFactory::instance();
      if (!intrinsics_->entrapped())
	ios.message(L"You can move again now."); // next turn
      else 
	ios.message(L"You try, but you can't move yet!");
      return false;
    }
  }
  switch (terrain.type()) {
  case terrainType::PIT_HIDDEN:
    fall(dPc() / 10);
    if (isPlayer() && !abilities()->fly())
      ioFactory::instance().message(L"It's a (pit) trap!"); // ref: Admiral Ackbar, Star Wars film Episode VI: Return of the Jedi.
    return true; // you can move into a hidden pit
  case terrainType::PIT: {
    fall(dPc() / 10);
    const auto climb = abilities()->climb();
    const int count=climb == bonus(false) ? 6 : climb == bonus() ? 4 : 2;
    auto rtn = !intrinsics_->entrapped();
    intrinsics_->entrap(count);
    return rtn;
  }
  case terrainType::SPRINGBOARD_HIDDEN:
    if (abilities()->fly())
     return true;
    // else fall through to jump
  case terrainType::SPRINGBOARD: {
    if (abilities()->fly())
      return true;
    if (isPlayer())
      ioFactory::instance().message(L"You are flung through the air!");
    std::vector<coord> targetOpts;
    int dist; // length we have momentum to move
    if (d == dir(0,0)) {
      dist=1;
      for (coord c : coordRectIterator(pos.first-1,pos.second-1,
				       pos.first+1,pos.second+1))
	targetOpts.push_back(c);
    } else {
      // fling distance = 2+depth/12. level 0 goes 2 squares, 100 goes 10
      dist = 2 + curLevel().depth()/12;
      targetOpts.emplace_back(pos.first + dist * d.first,
			      pos.second + dist * d.second);
    }
    const coord &target = *rndPick(targetOpts.begin(), targetOpts.end());
    coord c = pos;
    int len=0; // length actually moved
    std::vector<ref<monster>> hit;
    while (c != target) {
      coord cc = c.towards(target);
      hit = curLevel().monstersAt(cc);
      // in case of big monsters, remove any *this
      hit.erase(std::remove(hit.begin(), hit.end(), ref<monster>(*this)), hit.end());
      if (!hit.empty()) break;
      if (!abilities()->move(curLevel().terrainAt(cc))) break;
      c = cc;
      ++len;
    }
    if (c != pos) // avoid infinite loop
      curLevel().moveTo(*this, c);
    if (len < dist) {
      if (isPlayer())
	ioFactory::instance().message(L"Thump!");
      auto bashing = damageRepo::instance()[damageType::bashing];
      for (auto m : hit) {
	if (m.value().isPlayer())
	  ioFactory::instance().message(L"Thump!");
	m.value().wound(*this, dist - len, bashing);
      }
      wound(*this, dist - len, bashing);
    }
    return len == 0;// we've handled the move
  }
  case terrainType::WEB: {
    auto rtn = !intrinsics_->entrapped();
    intrinsics_->entrap(10);
    return rtn;
  }
  case terrainType::PIANO_HIDDEN:
    // always take 5% damage exactly, plus bash headgear
    // NB: This is unlike nethack, where wearing metal headgear protects from falling rocks.
    damage_ += 5;
    {
      std::set<slotType> hatSlots({{slotType::hat, slotType::hat_2, slotType::hat_3, 
	      slotType::hat_4, slotType::hat_5}});
      for (auto slot : hatSlots) {
	auto it = inSlot(slot);
	if (it) it.value().strike(damageType::bashing);
      }
    }
    curLevel().holder(pos).addItem(createItem(itemTypeKey::pianoforte));
    if (damage_.cur() == damage_.max()) death(true);
    if (isPlayer())
      ioFactory::instance().message(L"Sudenly, a piano falls on you.");
    return true; // you can move into a hidden piano trap
  default:
    return true;
  }
}

bool monster::raiseDead(const mutation &m, itemHolder &items, const coord &pos) {
  auto oI = items.firstItem([this, &m](item &i){
      if (i.highlight()) return false;
      auto pI = dynamic_cast<basicItem*>(&i);
      if (pI && pI->getType() == itemTypeRepo::instance()[itemTypeKey::corpse]) {
	auto pMt = dynamic_cast<monsterTypeProvider*>(&i);
	return (pMt && m.spreadsOnCorpse(pMt->getMonsterType()));
      }
      return false;
    });
  if (oI) {
    auto pMt = dynamic_cast<monsterTypeProvider*>(&(oI.value()));
    std::shared_ptr<monster> t = pMt->getMonsterType().spawn(curLevel());
    auto name = t->name();
    t->mutate(m.type());
    if (isMutated(m.type())) {
      curLevel().addMonster(t, pos);
      oI.value().destroy();
    }
    return true;
  }
  return false;
}

void monster::postMove(const coord &pos, const terrain &terrain) {
  // do we raise the dead?
  if (!mutations_.empty()) {
    auto &items = curLevel().holder(pos);
    for (const mutation &m : mutations_)
      if (raiseDead(m, items, pos)) break;
  }
    
  if (!isPlayer()) {
    // if we're injured and there's food, eat it.
    auto &onFloor = curLevel().holder(pos);
    bool eaten = false;
    if (injury().cur() > 0) {
      optionalRef<item> fud = onFloor.firstItem([this](item &i){
	  return type().eats(i.material());
	});
      if (fud) {
	eat(fud.value());
	eaten = true;
      }
    }
    // olse if we're bolew encumbrance weight, collect an item withitn weight limit
    if (!eaten) {
      int w; optionalRef<item> i;
      do {
	w = (totalWeight() < abilities()->carryWeightN());
	i = onFloor.firstItem([w](item &i) {
	    return i.weight() < w;
	  });
	if (i) {
	  auto &it = i.value();
	  if (!addItem(it)) break;
	  // if item is equippable, equip it.
	  it.equip(*this); // returns false if fails
	}
      } while (w > 0 && i);
    }
  }
  switch (terrain.type()) {
  case terrainType::WEB: {
    if (isPlayer()) ioFactory::instance().message(L"You are entangled in a web.");
    break;
  }
  case terrainType::PIT: {
    bool flying(abilities()->fly());
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
  // NB: Dealing extra bashing damage means you take extra damage from falling...
  auto damage = wound(*this, reductionPc, damageRepo::instance()[damageType::bashing]);
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

std::shared_ptr<monsterAbilities> monster::abilities() {
  std::shared_ptr<monsterAbilities> rtn = abilities_;
  for (const mutation &m : mutations_)
    if (m.appliesTo(type()))
      rtn = m.wrap(rtn);
  return rtn;
}

const std::shared_ptr<monsterAbilities> monster::abilities() const {
  std::shared_ptr<monsterAbilities> rtn = abilities_;
  for (const mutation &m : mutations_)
    if (m.appliesTo(type()))
      rtn = m.wrap(rtn);
  return rtn;
}

monster::~monster() {}

bool monster::operator == (const monster &rhs) { return this == &rhs; }


// delegate to type by default, but overridden for special behiour
const movementType & monster::movement() const {
  return sleeping() ? stationary : type_.movement();
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
    bonus = abilities()->eatVeggie();
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
  bool drunk = false;
  auto isEdible = [this, &drunk](item &i) {
    auto bottle = dynamic_cast<liquidContainer*>(&i);
    if (bottle && type().eats(materialType::liquid) && bottle->containsLiquid()) {
      drunk = !isPlayer() || 
	ioFactory::instance().ynPrompt(L"Drink " + i.name() + L"?");
      if (drunk) bottle->consumeBy(*this);
      return drunk;
    }
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
    if (isPlayer()) ioFactory::instance().message(L"There's nothing to consume here.");
    return;
  }

  if (drunk) return; // we can only drink items in inventory, and the consumeBy() method already handled it.
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
  auto c = charmedMonsters.equal_range(&mon); // what mon charms
  if (std::find_if(c.first, c.second, [&m](const std::pair<const monster*, const monster*> &p){return p.second == m;}) == c.second) return false; // mon charms us already
  charmedMonsters.insert(std::pair<const monster*, const monster*>(this, &mon));
  charmedMonstersReverse.emplace(&mon, this);
  // if mon monster dies, we must no longer be charmed by it, as the pointer becomes invalid.
  // if *this dies, we must clear the callback from mon, as the "this" pointer will become invalid.
  return true;
}

std::multimap<const monster*, const monster*>::const_iterator
monster::charmedBegin() const {
  return charmedMonstersReverse.equal_range(this).first;
}
std::multimap<const monster*, const monster*>::const_iterator
monster::charmedEnd() const {
  return charmedMonstersReverse.equal_range(this).second;
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

extern std::vector<damageType> allDamageTypes;

std::vector<std::wstring> monster::adjectives() const {
  std::vector<std::wstring> rtn;
  // naked - humanoid without clothing
  // sexy - high appearance
  // hungry/charmed etc aren't included because they would be copied to the corpse.
  
  // injury types - similar to item
  auto &dr = damageRepo::instance();
  const materialType &m = material();
  for (auto dt : allDamageTypes) {
    if (intrinsics()->proof(dr[dt])) { // don't consider proof based on equipment, as they'll lose this as a corpse.
      // adjective for being (this material) being proof to this damage type:
      auto ptr = dr[dt].proofAdj(m);
      if (ptr != nullptr)
	rtn.push_back(std::wstring(ptr));
      continue;
    }
  }

  auto &inj = injury();
  if (!inj.isFull()) {
    std::wstring adj(L"injured");
    auto injPc = inj.pc(); // 0-100
    if (injPc < 20 || inj.cur() == inj.max()) {} 
    else if (injPc > 80) rtn.push_back(std::wstring(L"thoroughly ") + adj);
    else if (injPc > 60) rtn.push_back(std::wstring(L"very ") + adj);
    else if (injPc > 40) rtn.push_back(std::wstring(adj));
    else if (injPc > 20) rtn.push_back(std::wstring(L"barely ") + adj);
  }

  rtn.insert(rtn.end(), extraAdjectives_.begin(), extraAdjectives_.end());
  return rtn;
}

void monster::addDescriptor(std::wstring desc) {
  if (std::find(extraAdjectives_.begin(), extraAdjectives_.end(), desc) != extraAdjectives_.end())
    extraAdjectives_.push_back(desc);
}

optionalRef<sharedAction<monster, monster>> monster::attackAction() {
  // default: no action
  return optionalRef<sharedAction<monster, monster>>();
}

bool monster::isMutated(const mutationType &key) const {
  auto &m = mutationFactory::instance()[key];
  return m.appliesTo(type_) && mutations_.count(m) > 0;
}
void monster::mutate(const mutationType &key) {
  mutations_.emplace(mutationFactory::instance()[key]);
}
void monster::deMutate(const mutationType &key) {
  mutations_.erase(mutationFactory::instance()[key]);  
}

movementType stationary = {speed::stop, goTo::none, goBy::avoid, 0 };
