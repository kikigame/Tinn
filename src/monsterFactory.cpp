/* License and copyright go here*/

// create new monsters in the dungeon: implementation details

#include "monster.hpp"
#include "action.hpp"
#include "terrain.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp"
#include "itemTypes.hpp"
#include "items.hpp"

// defined in monster.cpp
void moveMonster(monster &mon);
void monsterAttacks(monster &mon);


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
  allowRandom_(allowRandom),
  progress_(1) {}

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
void monsterBuilder::progress(const int p) { progress_ = p; }

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

// calculate the progression of a stat.
// "from" is what the value would be on the first level the monster appears.
// "p" is the level on which the monster appears, counting from 1 as the first level on which it appears..
unsigned char progressStat(unsigned char from, int p) {
  int progress = std::min(100, p); // just in case
  progress = std::max(1, progress); // just in case
  return static_cast<unsigned char>(std::min(100.,from * (100. + progress - 1) / 100.));
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

  // progress stats due to the monster's depth within the dungeon:
  strength_ = progressStat(strength_, progress_);
  appearance_ = progressStat(appearance_, progress_);
  fighting_ = progressStat(fighting_, progress_);
  dodge_ = progressStat(dodge_, progress_);
  maxDamage_ = progressStat(maxDamage_, progress_);

  if (male_ == 0 && female_ == 0)
    switch (type_->gen()) {
    case genderAssignType::neuter:
      break;
    case genderAssignType::m:
      male_ = 100;
      break;
    case genderAssignType::f:
      female_ = 100;
      break;
    case genderAssignType::mf:
      {
	std::uniform_int_distribution<int> flip(0,1);
	(flip(generator) ? male_ : female_) = 100;	
      }
      break;
    case genderAssignType::mfn:
      {
	std::uniform_int_distribution<int> flip(0,2);
	const int f = flip(generator);
	if (f) (f == 2 ? male_ : female_) = 100;	
      }
    case genderAssignType::herm:
      male_ = female_ = 50;
      break;
    case genderAssignType::direct:
      male_ = dPc();
      female_ = 100 - male_;
      break;
    case genderAssignType::indirect:
      male_ = dPc();
      female_ = dPc();
      break;
    default:
      throw type_->gen();
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
    actionFactory<monster,monster>::get(actionKey_)(false, false, *this, opponent);
  }
};

// monsters that perform an action when they successfully hit a target in combat (by reference)
class targetActionRefMonster : public monster {
private:
  sharedAction<monster,monster> &act_;
public:
  targetActionRefMonster(monsterBuilder &b, sharedAction<monster,monster> &action) :
    monster(b), act_(action) {}
  virtual void onHit(monster &opponent, int) {
    act_(false, false, *this, opponent);
  }
};

// ferrets steal little things then run away
class ferret : public targetActionMonster {
private:
  movementType away_;
public:
  ferret(monsterBuilder &b) : 
    targetActionMonster(b, key::steal_small),
    away_({speed::turn2, goTo::player, goBy::avoid, 25}){
    intrinsics().see(true);
    intrinsics().hear(true);
  }
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
  bool onMove(const coord &pos, const terrain &terrain) {
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
    monster(b) {
    intrinsics().see(true);
    intrinsics().hear(true);
  }
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

class kelpie : public monster {
private:
  bool equineForm_;
  unsigned char maxStrength_;
public:
  kelpie(monsterBuilder &b) :
    monster(b),
    equineForm_(false),
    maxStrength_(strength().max()) {
    intrinsics().see(true);
    intrinsics().hear(true);
    intrinsics().swim(true);
    intrinsics().move(*(tFactory.get(terrainType::WATER).get()), true);
  }
  virtual ~kelpie() {}
  virtual void shapeShift() {
    equineForm_ = !equineForm_;
    auto &level = curLevel();
    if (&level == &level.dung().cur_level()) {
      auto &ios = ioFactory::instance();
      if (equineForm_)
	ios.message(L"The " + std::wstring(name()) + L" now looks to be a horse.");
      else
	ios.message(L"The " + std::wstring(name()) + L" now looks to be a human.");
    }
    if (equineForm_) {
      strength().bonus(20);
      polymorphCategory(monsterCategory::hooved_quadruped);
    } else {
      strength().cripple(strength().max() - maxStrength_);
      polymorphCategory(monsterCategory::biped);
    }
  }
  virtual bool onMove(const coord &pos, const terrain &terrain) {
    if (!monster::onMove(pos, terrain)) return false;
    if (terrain.type() == terrainType::WATER && equineForm_ && (&curLevel() == &curLevel().dung().cur_level())
	&& curLevel().dung().pc()->abilities().hear())
      ioFactory::instance().message(L"You hear the sound of thunder as the " + std::wstring(name()) + L"'s tail hits the water");
    if (dPc() <= 11) // ~3%
      shapeShift();
    return true;
  }
  virtual const wchar_t render() const {
    return equineForm_ ? L'u' : L'@';
  }
};

class siren : public monster {
public:
  siren(monsterBuilder &b) :
    monster(b) {
    intrinsics().see(true);
    intrinsics().hear(true);
    intrinsics().swim(true);
    intrinsics().fly(true);
    intrinsics().climb(true);
    eachTick([this]{
	if (dPc() <= 30) // ~ 1 every 5 moves
	  sirenCall();
      });
  }
  virtual ~siren() {}
  void sirenCall() {
    auto &act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::attract);
    auto &l = curLevel();
    if (&(l.dung().cur_level()) != &l) return; // don't waste our voice if no Dungeoneer is around
    if (l.dung().pc()->abilities().hear()) {
      ioFactory::instance().message(L"The " + std::wstring(name()) + L" sings an attractive song");
    }
    l.forEachMonster([this,&act](monster &t){
	if (t.type().type() != monsterTypeKey::siren) 
	  act(false,false,*this, t);
      });
  }
};


class dragon : public monster {
private:
  const bool western_;
public:
  dragon(monsterBuilder &b) : 
    monster(roll(b), equipment(b)),
    western_(align().domination() == Domination::aggression) {
    // DRAGONS DON'T FLY! It's mythologically inaccurate...
    intrinsics().speedy(true);
    intrinsics().dblAttack(true);
    intrinsics().see(true);
    intrinsics().hear(true);
    intrinsics().fearless(true);
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
    return b;
  }

  static std::vector<const slot*> equipment(monsterBuilder &b) {
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
    // remove any kit slots for claws we don't have:
    std::vector<const slot*> rtn = slotsFor(monsterCategory::dragon);
    auto e = rtn.end();
    switch (claws) {
    case 0:
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_left_index));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_right_index));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_left_index));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_right_index));
      // no break
    case 1:
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_left_middle));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_right_middle));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_left_middle));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_right_middle));
      // no break
    case 2:
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_left_ring));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_right_ring));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_left_fourth));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_right_fourth));
      // no break
    case 3:
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_left_little));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_right_little));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_left_little));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_right_little));
      // no break
    case 4:
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_left_thumb));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_right_thumb));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_left_thumb));
      e = std::remove(rtn.begin(), e, slotBy(slotType::toe_right_thumb));
      // no break
    }
    switch (heads) {
    case 0:
    case 1:
      e = std::remove(rtn.begin(), e, slotBy(slotType::headband_2));
      e = std::remove(rtn.begin(), e, slotBy(slotType::hat_2));
      e = std::remove(rtn.begin(), e, slotBy(slotType::glasses_2));
      e = std::remove(rtn.begin(), e, slotBy(slotType::amulet_2));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_nose_2));
      // no break
    case 2:
      e = std::remove(rtn.begin(), e, slotBy(slotType::headband_3));
      e = std::remove(rtn.begin(), e, slotBy(slotType::hat_3));
      e = std::remove(rtn.begin(), e, slotBy(slotType::glasses_3));
      e = std::remove(rtn.begin(), e, slotBy(slotType::amulet_3));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_nose_3));
      // no break
    case 3:
      e = std::remove(rtn.begin(), e, slotBy(slotType::headband_4));
      e = std::remove(rtn.begin(), e, slotBy(slotType::hat_4));
      e = std::remove(rtn.begin(), e, slotBy(slotType::glasses_4));
      e = std::remove(rtn.begin(), e, slotBy(slotType::amulet_4));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_nose_4));
      // no break
    case 4:
      e = std::remove(rtn.begin(), e, slotBy(slotType::headband_5));
      e = std::remove(rtn.begin(), e, slotBy(slotType::hat_5));
      e = std::remove(rtn.begin(), e, slotBy(slotType::glasses_5));
      e = std::remove(rtn.begin(), e, slotBy(slotType::amulet_5));
      e = std::remove(rtn.begin(), e, slotBy(slotType::ring_nose_5));
      // no break
    }
    rtn.erase(e, rtn.end());

    return rtn;
  }
};

void equipMonster(const monsterTypeKey &type, level &level, monster &m) {
  switch (type) {
  case monsterTypeKey::dungeoneer: {
    auto &helmet = createItem(itemTypeKey::helmet); // todo: visorless
    m.addItem(helmet);
    m.equip(helmet, slotType::hat);
    auto &napsack = createItem(itemTypeKey::napsack_of_consumption);
    m.addItem(napsack);
    m.equip(napsack, slotType::hauburk);
    m.addItem(createRndItem(10 * level.depth(), L'$', L'('));
    m.addItem(createRndBottledItem(10 * level.depth()));
    return;
  }
  default:
    return;
  }
}

std::shared_ptr<monster> ofType(const monsterType &type, level & level) {
  monsterBuilder b(true);
  b.startOn(level);
  // stats, alignment etc are also set when type is set:
  //  b.type(monsterTypeRepo::instance()[type]);
  b.type(type);

  const int levelFactor = type.getLevelFactor();
  const int levelOffset = type.getLevelOffset();
  b.progress(std::min(1, level.depth() - levelOffset) * levelFactor);

  // invoke move() every move, even when the player is helpless:
  std::shared_ptr<monster> ptr;
  switch (type.type()) {
  case monsterTypeKey::dungeoneer:
    ptr = std::make_shared<trivialMonster>(b);
    ptr->intrinsics().hear(true); // can't see
    break;
  case monsterTypeKey::ferret:
    ptr = std::make_shared<ferret>(b);
    break;
  case monsterTypeKey::goblin:
    ptr = std::make_shared<targetActionMonster>(b, targetActionMonster::key::steal_shiny);
    ptr->intrinsics().see(true);
    ptr->intrinsics().hear(true);
    break;
  case monsterTypeKey::hound:
    ptr = std::make_shared<hound>(b);
    break;
  case monsterTypeKey::incubus:
    ptr = std::make_shared<targetActionRefMonster>(b, incubusAction());
    ptr->intrinsics().see(true);
    ptr->intrinsics().hear(true);
    ptr->intrinsics().swim(true);
    break;
  case monsterTypeKey::kelpie:
    ptr = std::make_shared<kelpie>(b);
    break;
  case monsterTypeKey::siren:
    ptr = std::make_shared<siren>(b);
    break;
  case monsterTypeKey::succubus:
    ptr = std::make_shared<targetActionRefMonster>(b, succubusAction());
    ptr->intrinsics().see(true);
    ptr->intrinsics().hear(true);
    ptr->intrinsics().swim(true);
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
    ptr->intrinsics().see(true);
    ptr->intrinsics().hear(true);
    ptr->intrinsics().fly(true);
  }
  // pass by value works, but creates an extra 2 persistent refs, causing a memory leak as the ref-count never hits 0.
  // pass by reference causes a SIGSEGV; not sure why.
  // you can't create a second shared_ptr on the same pointer.
  auto &m = *ptr;
  ptr->eachTick([&m]() {moveMonster(m);} );
  ptr->eachTick([&m]() {monsterAttacks(m);} );
  equipMonster(type.type(), level, *ptr);
  return ptr;
}

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms) {
  auto from = monsterTypeRepo::instance().begin();
  auto to = monsterTypeRepo::instance().end();
  auto filter = [depth](monsterType* mt) {
    // Dungeoneers aren't found below level 3 (Ref: Knightmare, which had only 3 levels)
      return (depth <= 3 || mt->type() != monsterTypeKey::dungeoneer) &&
      // don't randomnly generate sirens; they must be on watery levels, on the rocks:
      mt->type() != monsterTypeKey::siren;
  };
  auto ffrom = make_filtered_iterator(filter, from, to);
  auto tto = make_filtered_iterator(filter, to, to);
  return rndGen<monsterType*, filtered_iterator<std::vector<monsterType*>::iterator > >(ffrom, tto, depth, rooms);
}
