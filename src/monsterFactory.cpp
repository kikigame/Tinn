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
#include "role.hpp"
#include "mobile.hpp"
#include "alien.hpp"

#include <cmath> // for std::ceil
#include <ctime> // for dates
#include <bitset>

monsterBuilder::monsterBuilder(bool allowRandom) : 
  level_(NULL),
  highlight_(false),
  align_(NULL), 
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
  progress_(1),
  onDeath_() {}

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
void monsterBuilder::alignAgainst(const deity &a) { align_  = &(deityRepo::instance().getOpposed(a)); }
void monsterBuilder::type(const monsterType &t) { 
  // type also sets various fields if not already 
  type_  = &t; 
  if (align_ == NULL) align_ = *rndPick(t.alignment().begin(), t.alignment().end());
  if (strength_ == 0) strength_ = type_->iStrength();
  if (appearance_ == 0) appearance_ = type_->iAppearance();
  if (fighting_ == 0) fighting_ = type_->iFighting();
  if (dodge_ == 0) dodge_ = type_->iDodge();
  if (maxDamage_ == 0) maxDamage_ = type_->iMaxDamage();
}
void monsterBuilder::progress(const int p) { progress_ = p; }
void monsterBuilder::onDeath(std::function<void()> f) {
  onDeath_.push_back(f);
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
const monsterType & monsterBuilder::type() { return *type_; }
void monsterBuilder::highlight() { highlight_ = true; }
bool monsterBuilder::isHighlight() { return highlight_; }
std::vector<std::function<void()>> &monsterBuilder::onDeath() { return onDeath_; }

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
  // on level 101, all stats would be 100%
  return from + static_cast<unsigned char>(std::min(100.,(100. - from) * (progress -1) / 100));
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
    actionFactory<monster,monster>::get(actionKey_)(bcu(), *this, opponent);
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
    act_(bcu(), *this, opponent);
  }
};

// blobs start on one square and blob outwards {
class blob : public trivialMonster, public bigMonster, private temporal::callback {
private:
  std::vector<coord> pos_;
public:
  blob(monsterBuilder &b) :
    trivialMonster(b),
    bigMonster(),
    temporal::callback(true, [this](){
	dir nsew;
	switch (dPc() % 4) {
	default: nsew = dir(-1,0); break;
	case 1: nsew = dir(1,0); break;
	case 2: nsew = dir(0,-1); break;
	case 3: nsew = dir(0,1); break;
	}
	std::vector<coord> newPos;
	for (coord c : pos_) {
	  coord cc = c.inDir(nsew);
	  if (cc.first < 0 || cc.second < 0 ||
	      cc.first >= level::MAX_WIDTH || cc.second >= level::MAX_HEIGHT)
	    return;
	  if (std::find(pos_.begin(), pos_.end(), cc) == pos_.end() &&
	      abilities()->move(curLevel().terrainAt(cc)))
	    newPos.push_back(cc);
	}
	std::copy(newPos.begin(), newPos.end(), back_inserter(pos_));
	curLevel().bigMonster(*this, pos_);
      }),
    pos_() {
    time::onTick(*this);
  }
  virtual ~blob() {}
  // INTERFACE bigMonster
  // called when position is first set, or on teleport:
  virtual void setPos(const coord &c) {
    pos_.clear();
    pos_.push_back(c);
    curLevel().bigMonster(*this, pos_);
  }
  // return the position of the "main" element:
  virtual coord mainPos() const {
    return pos_.at(0);
  }
  // INTERFACE bigMonster ends
};


// ferrets steal little things then run away
class ferret : public targetActionMonster {
private:
  movementType away_;
public:
  ferret(monsterBuilder &b) : 
    targetActionMonster(b, key::steal_small),
    away_({speed::turn2, goTo::player, goBy::avoid, 25}){
  }
  virtual ~ferret() {}
  const movementType & movement() const {
    if (empty())
      return type().movement(); // go to player; they might have sometihng fun!
    else // I've got it! Run away!
      return away_;
  }
};

class fox : public monster {
private:
  renderedAction<monster, monster> &fireTailAction_;
  bool polymorphed_;
public:
  fox(monsterBuilder &b) :
    monster(b),
    fireTailAction_(actionFactory<monster, monster>::get(sharedAction<monster, monster>::key::fire_tail)),
    polymorphed_(false) {}
  virtual ~fox() {}
  virtual optionalRef<sharedAction<monster, monster>> attackAction() {
    if (name().find(L"huli jing") != std::wstring::npos && dPc() < 10) {
      return fireTailAction_;
    }
    return monster::attackAction();
  }
  virtual void postMove(const coord &, const terrain &) {
    auto roll = dPc();
    if (name().find(L"fox") == std::wstring::npos && roll < 10) {
      polymorphed_ = !polymorphed_;
      const auto &ios = ioFactory::instance();
      polymorphCategory(polymorphed_ ? monsterCategory::quadruped : monsterCategory::biped);
      if (polymorphed_)
	ios.message(L"The " + name() + L" takes on a vulipine visage");
      else {
	ios.message(L"The " + name() + L" takes the shape of " +
	 (roll < 5 ? L"a scholar" : isMale() ? L"an old man" : L"a young girl") );
      }
    }
  }
};

// zombies: instakilled by pit traps
class zombie : public monster {
public:
  zombie(monsterBuilder &b) :
    monster(b) {}
  virtual ~zombie() {}
  virtual void postMove(const coord &pos, const terrain &terrain) {
    monster::postMove(pos, terrain);
    if (terrain.type() == terrainType::PIT)
      death();
  }
};

// a hound changes its saying depending on its level.
// that's probably not a very good superpower really.
class hound : public monster {
public:
  hound(monsterBuilder &b) : 
    monster(b) {}
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
    maxStrength_(strength().max()) {}
  virtual ~kelpie() {}
  virtual void shapeShift() {
    equineForm_ = !equineForm_;
    auto &level = curLevel();
    if (&level == &level.dung().cur_level()) {
      auto &ios = ioFactory::instance();
      if (equineForm_)
	ios.message(L"The " + name() + L" now looks to be a horse.");
      else
	ios.message(L"The " + name() + L" now looks to be a human.");
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
	&& curLevel().dung().pc()->abilities()->hear())
      ioFactory::instance().message(L"You hear the sound of thunder as the " + name() + L"'s tail hits the water");
    if (dPc() <= 11) // ~3%
      shapeShift();
    return true;
  }
  virtual const wchar_t render() const {
    return equineForm_ ? L'u' : L'@';
  }
};

// TODO: spit acid or drain mind in defence
class mokumokuren : public monster {
public:
  mokumokuren(monsterBuilder &b) :
    monster(b) {}
  virtual ~mokumokuren() {}
};

class siren : public monster {
public:
  siren(monsterBuilder &b) :
    monster(b) {
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
    if (l.dung().pc()->abilities()->hear()) {
      ioFactory::instance().message(L"The " + name() + L" sings an attractive song");
    }
    l.forEachMonster([this,&act](monster &t){
	if (t.type().type() != monsterTypeKey::siren &&
	    t.abilities()->hear()) 
	  act(bcu(),*this, t);
      });
  }
};

class merfolk : public monster {
public:
  merfolk(monsterBuilder &b) :
    monster(b) {
    eachTick([this]{
	if (dPc() <= 30) // ~ 1 every 5 moves
	  charm();
      });
    // mermaids have comb & mirror in heraldry, and mermen are often given tridents, but we don't have those types.
    addItem(createItem(itemTypeKey::conch));
  }
  virtual ~merfolk() {}
  virtual std::wstring name() const {
    if (isFemale()) return L"Mermaid";
    if (isMale()) return L"Merman";
    return monster::name();
  }
  void charm() {
    auto &act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::charm);
    auto &l = curLevel();
    if (&(l.dung().cur_level()) != &l) return; // don't waste our voice if no Dungeoneer is around
    if (l.dung().pc()->abilities()->see()) {
      ioFactory::instance().message(L"The " + name() + L" seems very beguiling");
    }
    l.forEachMonster([this,&act](monster &t){
	if (t.type().type() != monsterTypeKey::merfolk) 
	  act(bcu(), *this, t);
      });
  }
  // can't move off water
  virtual bool onMove(const coord &pos, const terrain &terrain) {
    return (terrain.type() == terrainType::WATER);
  }
  // do water damage by default
  virtual damageType unarmedDamageType() const {
    return damageType::wet;
  }
};


class dragon : public monster {
private:
  const bool western_;
public:
  dragon(monsterBuilder &b) : 
    monster(roll(b), equipment(b)),
    western_(align().domination() == Domination::aggression) {}
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

class goblin : public targetActionMonster {
public:
  goblin(monsterBuilder &b) :
    targetActionMonster(b, targetActionMonster::key::steal_shiny) {}
  virtual ~goblin() {};
};

class dungeoneer : public trivialMonster {
public:
  dungeoneer(monsterBuilder &b) :
    trivialMonster(b) {}
  virtual bool highlight() const {
    return curLevel().dung().pc()->job().type() == roleType::shopkeeper;
  }
  virtual const wchar_t * say() const {
    auto &role = curLevel().dung().pc()->job();
    if (role.type() == roleType::shopkeeper &&
	role.questsBegin()->isSuccessful())
      return L"My quest is complete! I'm out of here!";
    else return monster::say();
  }
  virtual ~dungeoneer() {};
};

class bull : public trivialMonster {
public:
  bull(monsterBuilder &b) :
    trivialMonster(b) {}
  virtual ~bull() {};
  virtual std::wstring name() const {
    std::wstring name = trivialMonster::name();
    auto pos = name.find(L"bull");
    if (pos != std::wstring::npos && isFemale())
      name.replace(pos, pos+4, L"cow");
    return name;
  }
  virtual damageType unarmedDamageType() const {
    if (name().find(L"bonnacon") != std::wstring::npos)
      return trivialMonster::unarmedDamageType();
    return damageType::hot; // (fire) noxious anal excretions, according to Pliny the Elder
  }
};

class incubus : public targetActionRefMonster {
public:
  incubus(monsterBuilder &b) :
    targetActionRefMonster(b, incubusAction()) {}
  virtual ~incubus() {};
  virtual optionalRef<sharedAction<monster, monster>> attackAction() {
    return optionalRef<sharedAction<monster, monster>>(incubusAction());
  }
};


// snakes get longer based on their hit points.
class snake : public trivialMonster, public bigMonster {
private:
  std::vector<coord> segments_;
  monsterBuilder builder_;
public:
  snake(monsterBuilder &b) :
    trivialMonster(b),
    segments_(),
    builder_(b) {}
  virtual ~snake() {}
  virtual void death() {
    snakeToLength();
    curLevel().bigMonster(*this, segments_);
    trivialMonster::death();
  }
  // called from level class.
  virtual void setPos(const coord &c) {
    if (segments_.empty()) {
      // creates if needed (on first call; we don't make segments until we move)
      segments_.push_back(c);
      return;
    }
    snakeToLength();
    coord old = segments_[0];
    segments_[0] = c;
    if (segments_.size() == 1) return;
    const coord &towardHead = segments_[1];
    auto distance = c.linearDistance(towardHead);
    switch (distance) {
    case 0: // head moving onto tail; nothing to do
      break;
    case 1: // head moved 1 square
    case 2: // head moved away from tail; move last segment to where the head was
      segments_.pop_back();
      segments_.insert(segments_.begin()+1, old);
      break;
    default: // full-on teleport; curl up snake
      std::fill(segments_.begin(), segments_.end(), c);
    }
    curLevel().bigMonster(*this, segments_);
  }
  virtual coord mainPos() const {
    return segments_.at(0);
  }
  // returns number of segments; 1 for head, +1 for each tail
  unsigned char length() const {
    unsigned char health = injury().max() - injury().cur();
    unsigned char rtn = std::ceil(health / 10.0);
    return std::max(static_cast<unsigned char>(1),rtn);
  }
private:
  void snakeToLength() {
    unsigned char numSegments = length();
    // first handle shortening the snake:
    if (segments_.size() > numSegments) {
      // remove those segments:
      segments_.erase(segments_.begin() + numSegments, segments_.end());
    }
    // lengthen the monster:
    while (segments_.size() < numSegments)
      segments_.push_back(segments_.back());
  }
};

// swarms grow and shrink around the target square
template<int maxDist, bool charms>
class swarm : public trivialMonster, public bigMonster {
private:
  static constexpr int max = maxDist*2+1;
  optionalRef<sharedAction<monster, monster>> attackAction_;
  coord mainPos_;
public:
  swarm(monsterBuilder &b) :
    trivialMonster(b), attackAction_(), mainPos_(0,0) {
    if (charms) {
      attackAction_ = actionFactory<monster,monster>::get(sharedAction<monster, monster>::key::charm);
    }
  }
  virtual ~swarm() {};
  // called from level class.
  virtual void setPos(const coord &c) {
    mainPos_ = c;
    if (mainPos_.first < maxDist) mainPos_.first = maxDist;
    if (mainPos_.second < maxDist) mainPos_.second = maxDist;
    if (mainPos_.first > level::MAX_WIDTH - maxDist - 1) mainPos_.first = level::MAX_WIDTH - maxDist - 1;
    if (mainPos_.second > level::MAX_HEIGHT - maxDist - 1) mainPos_.second = level::MAX_HEIGHT - maxDist - 1;
    // update the rest of the swarm on move
    std::array<std::bitset<max>,max> locs;
    for (int x=0; x < maxDist; ++x)
      for (int y=0; y < maxDist; ++y)
	locs[x][y] = dPc() < 75;
    std::vector<coord> pos;
    if (pos.empty()) {
      for (int x=mainPos_.first-maxDist; x<=mainPos_.first+maxDist; ++x)
	for (int y=mainPos_.second-maxDist; y<=mainPos_.second+maxDist; ++y) {
	  coord next(x,y);
	  if (abilities()->move(curLevel().terrainAt(next)))
	    pos.emplace_back(x,y);
	}
    }
    if (pos.size() > 1) {
      std::vector<coord> pos2(pos);
      pos.clear();
      do {
	auto it = rndPick(pos2.begin(), pos2.end());
	pos.push_back(*it);
	pos2.erase(it);
      } while (pos.size() < pos2.size());
    }
    if (!pos.empty()) // otherwise don't move
      curLevel().bigMonster(*this, pos);
  }
  virtual coord mainPos() const {
    return mainPos_;
  }
  virtual optionalRef<sharedAction<monster, monster>> attackAction() {
    return attackAction_;
  }
};

class succubus : public targetActionRefMonster {
public:
  succubus(monsterBuilder &b) :
    targetActionRefMonster(b, succubusAction()) {}
  virtual ~succubus() {};
  virtual optionalRef<sharedAction<monster, monster>> attackAction() {
    return optionalRef<sharedAction<monster, monster>>(succubusAction());
  }
};

class bird : public trivialMonster {
public:
  bird(monsterBuilder &b) :
    trivialMonster(b) {
  }
  virtual ~bird() {}
};

class hippalectryon : public trivialMonster {
public:
  hippalectryon(monsterBuilder &b) :
    trivialMonster(b) {
  }
  virtual ~hippalectryon() {}
  virtual optionalRef<sharedAction<monster, monster>> attackAction() {
    return actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::comedy);
  }
};

class alienMonster : public trivialMonster {
private:
  alien::world whence_;
  damageType damageType_;
public:
  alienMonster(monsterBuilder &b) :
    trivialMonster(b),
    whence_(alien::world::spawn()),
    damageType_(rndPick<damageType>()) {
    initIntrinsics();
  }
  alienMonster(monsterBuilder &b, const alien::world &w) :
    trivialMonster(b), whence_(w) {
    initIntrinsics();
  };
  virtual ~alienMonster() {}
  virtual std::wstring name() const {
    return trivialMonster::name() + whence_.name();
  }
  virtual std::wstring description() const {
    return trivialMonster::description() + L"\n\n" + whence_.description();
  }
  virtual const deity& align() const {
    return whence_.align();
  }
  virtual damageType unarmedDamageType() const {
    return damageType_;
  }
private:
  // pointlessly rolling my own pseudorandomnumber generator:
  void rotate(std::bitset<sizeof(wchar_t)> &b,
	      const std::bitset<sizeof(wchar_t)> &c) const {
    b = b << 1 | b >> (sizeof(wchar_t)-1);
    b ^= c;
  }
  bool is(std::bitset<sizeof(wchar_t)> &b,
	  const std::bitset<sizeof(wchar_t)> &c) const {
    bool rtn = b.count() % 2 == 0;
    rotate(b,c);
    return rtn;
  }
  bonus bon(std::bitset<sizeof(wchar_t)> &a,
	    std::bitset<sizeof(wchar_t)> &b) const {
    bool isA = a.count() % 2 == 0;
    bool isB = b.count() % 2 == 0;
    rotate(a,b);
    return isA && isB ? bonus(true) :
      !isA && !isB ? bonus(false) : bonus();
  }
  void initIntrinsics() {
    auto &able = *abilities();
    auto a = std::bitset<sizeof(wchar_t)>(whence_.name()[0]);
    auto b = std::bitset<sizeof(wchar_t)>(whence_.name()[1]);
    auto c = std::bitset<sizeof(wchar_t)>(whence_.name()[2]);
    for (int i=0; i < static_cast<int>(damageType::END); ++i) {
      damageType dt = static_cast<damageType>(i);
      const damage &d = damageRepo::instance()[dt];
      if (is(a,b) && is(b,c) && is(c,a))
	able.proof(d);
      else if (is(a,b))
	able.resist(d);
      else if (is(a,b) && is(b,c) && is(c,a))
	able.extraDamage(d);
    }
    able.eatVeggie(bon(a,b));
    able.dblAttack(bon(a,b));
    able.hear(is(a,b));
    able.see(is(a,b));
    able.fly(is(a,b));
    able.fearless(bon(a,b));
    able.climb(bon(a,b));
    able.speedy(bon(a,b));
    able.throws(is(a,b));
    able.zap(is(a,b));
    able.sleeps(is(a,b));

    if (is(a,b))
      able.move(tFactory.get(terrainType::CRACK));
    if (is(a,b))
      able.move(tFactory.get(terrainType::KNOTWEED));
    if (is(a,b) && is(b,c))
      able.move(tFactory.get(terrainType::FIRE));
    if (is(a,b))
      able.move(tFactory.get(terrainType::SPACE));
    
    // carryWeight; humans are 3000 Newtons
    uint16_t w=0;
    for (int x=0; x < 13; ++x) // up to 8192
      w = w << 1 | (is(a,b) ? 1 : 0);
    able.carryWeightN(w);
  }
};


template <monsterTypeKey T>
struct monsterTypeTraits {
  typedef bird type; // default
};
template<> struct monsterTypeTraits<monsterTypeKey::alien> { typedef alienMonster type; };
template<> struct monsterTypeTraits<monsterTypeKey::blob> { typedef blob type; };
template<> struct monsterTypeTraits<monsterTypeKey::bull> { typedef bull type; };
template<> struct monsterTypeTraits<monsterTypeKey::dungeoneer> { typedef dungeoneer type; };
template<> struct monsterTypeTraits<monsterTypeKey::ferret> { typedef ferret type; };
template<> struct monsterTypeTraits<monsterTypeKey::fox> { typedef fox type; };
template<> struct monsterTypeTraits<monsterTypeKey::goblin> { typedef goblin type; };
template<> struct monsterTypeTraits<monsterTypeKey::hound> { typedef hound type; };
template<> struct monsterTypeTraits<monsterTypeKey::hippalectryon> { typedef hippalectryon type; };
template<> struct monsterTypeTraits<monsterTypeKey::incubus> { typedef incubus type; };
template<> struct monsterTypeTraits<monsterTypeKey::kelpie> { typedef kelpie type; };
template<> struct monsterTypeTraits<monsterTypeKey::mokumokuren> { typedef mokumokuren type; };
template<> struct monsterTypeTraits<monsterTypeKey::merfolk> { typedef merfolk type; };
template<> struct monsterTypeTraits<monsterTypeKey::siren> { typedef siren type; };
template<> struct monsterTypeTraits<monsterTypeKey::snake> { typedef snake type; };
template<> struct monsterTypeTraits<monsterTypeKey::succubus> { typedef succubus type; };
template<> struct monsterTypeTraits<monsterTypeKey::zombie> { typedef zombie type; };
template<> struct monsterTypeTraits<monsterTypeKey::dragon> { typedef dragon type; };
//template<> struct monsterTypeTraits<monsterTypeKey::bird> { typedef bird type; };
template<> struct monsterTypeTraits<monsterTypeKey::raptor> { typedef bird type; };
template<> struct monsterTypeTraits<monsterTypeKey::swarm_butterfly> { typedef swarm<3,true> type; };
template<> struct monsterTypeTraits<monsterTypeKey::swarm_bees> { typedef swarm<2,false> type; };
template<> struct monsterTypeTraits<monsterTypeKey::swarm_wasps> { typedef swarm<3,false> type; };
template<> struct monsterTypeTraits<monsterTypeKey::swarm_locusts> { typedef swarm<4,false> type; };

template<monsterTypeKey T>
std::shared_ptr<monster> ofTypeImpl(monsterBuilder &b) {
  return ofType<T,typename monsterTypeTraits<T>::type>(b);
}

std::unique_ptr<monsterBuilder> monsterType::builder(bool allowRandom) const {
  auto rtn = std::unique_ptr<monsterBuilder>(new monsterBuilder(allowRandom));
  rtn->type(*this);
  return rtn;
}

std::shared_ptr<monster> monsterType::spawn(level & level) const {
  monsterBuilder b(true);
  b.startOn(level);
  return spawn(b);
}
// these lookup makes allocating dynamic monsters of randomly chosen type posible.
// while also ensuring all implementations of ofType<> actually get compiled in.
std::shared_ptr<monster> monsterType::spawn(monsterBuilder &b) const {
  switch (type()) {
    //  case monsterTypeKey::bird: return ofTypeImpl<monsterTypeKey::bird>(b);
  case monsterTypeKey::alien: return ofTypeImpl<monsterTypeKey::alien>(b);
  case monsterTypeKey::blob: return ofTypeImpl<monsterTypeKey::blob>(b);
  case monsterTypeKey::bull: return ofTypeImpl<monsterTypeKey::bull>(b);
  case monsterTypeKey::raptor: return ofTypeImpl<monsterTypeKey::raptor>(b);
  case monsterTypeKey::dragon: return ofTypeImpl<monsterTypeKey::dragon>(b);
  case monsterTypeKey::dungeoneer: return ofTypeImpl<monsterTypeKey::dungeoneer>(b); 
  case monsterTypeKey::ferret: return ofTypeImpl<monsterTypeKey::ferret>(b);
  case monsterTypeKey::fox: return ofTypeImpl<monsterTypeKey::fox>(b);
  case monsterTypeKey::goblin: return ofTypeImpl<monsterTypeKey::goblin>(b);
  case monsterTypeKey::hound: return ofTypeImpl<monsterTypeKey::hound>(b);
  case monsterTypeKey::hippalectryon: return ofTypeImpl<monsterTypeKey::hippalectryon>(b);
  case monsterTypeKey::human: return ofTypeImpl<monsterTypeKey::human>(b); 
  case monsterTypeKey::incubus: return ofTypeImpl<monsterTypeKey::incubus>(b); 
  case monsterTypeKey::kelpie: return ofTypeImpl<monsterTypeKey::kelpie>(b); 
  case monsterTypeKey::mokumokuren: return ofTypeImpl<monsterTypeKey::mokumokuren>(b); 
  case monsterTypeKey::merfolk: return ofTypeImpl<monsterTypeKey::merfolk>(b);
  case monsterTypeKey::siren: return ofTypeImpl<monsterTypeKey::siren>(b); 
  case monsterTypeKey::snake: return ofTypeImpl<monsterTypeKey::snake>(b); 
  case monsterTypeKey::succubus: return ofTypeImpl<monsterTypeKey::succubus>(b); 
  case monsterTypeKey::swarm_butterfly: return ofTypeImpl<monsterTypeKey::swarm_butterfly>(b); 
  case monsterTypeKey::swarm_bees: return ofTypeImpl<monsterTypeKey::swarm_bees>(b); 
  case monsterTypeKey::swarm_wasps: return ofTypeImpl<monsterTypeKey::swarm_wasps>(b); 
  case monsterTypeKey::swarm_locusts: return ofTypeImpl<monsterTypeKey::swarm_locusts>(b); 
  case monsterTypeKey::troll: return ofTypeImpl<monsterTypeKey::troll>(b); 
  case monsterTypeKey::venusTrap: return ofTypeImpl<monsterTypeKey::venusTrap>(b);
  case monsterTypeKey::zombie: return ofTypeImpl<monsterTypeKey::zombie>(b);
  default: throw type();
  }
}
const std::shared_ptr<monsterIntrinsics> monsterType::intrinsics() const {
  return intrinsics_;
}

// is it the Monday after American fThinkgiving, which is itself the fourth Thursday in November?
bool isItCyberMonday() {
    time_t tm = time(0);
    auto pT = localtime(&tm);
    auto &t = *pT;
    if (t.tm_wday != 1) return false;
    // it's Monday; what was last Thursday?
    if (t.tm_mon < 10) return false; // < 10 not yet November,
    t.tm_mday -= 4; if (t.tm_mday <0) {
      t.tm_mon--;
      t.tm_mday += 30; // 30 is correct only if November!
    }
    if (t.tm_mon != 10) return false;
    // it is November!

    // if the 1st is a Thursday, the 4th Thursday is 1,8,15,22
    // if the 1st is a Friday, the 4th Thursday is 7,14,21,28
    return (t.tm_mday >= 22 && t.tm_mday <= 28);
}

template<monsterTypeKey T, class M>
std::shared_ptr<monster> ofType(monsterBuilder &b) {
  //  monsterBuilder b(true);
  // stats, alignment etc are also set when type is set:
  auto &type = monsterTypeRepo::instance()[T];
  b.type(type);

  auto iLevel = b.iLevel();
  if (!iLevel) {// bit hacky, but allow non-level monsters for transient use.
      std::shared_ptr<monster> ptr = std::make_shared<M>(b);
      return ptr;
  }
  level &level = *iLevel;
  const int levelFactor = type.getLevelFactor();
  const int levelOffset = type.getLevelOffset();
  b.progress(std::max(1, level.depth() - levelOffset) * levelFactor);

  // invoke move() every move, even when the player is helpless:
  std::shared_ptr<monster> ptr = std::make_shared<M>(b);
  // pass by value works, but creates an extra 2 persistent refs, causing a memory leak as the ref-count never hits 0.
  // pass by reference causes a SIGSEGV; not sure why.
  // you can't create a second shared_ptr on the same pointer.
  auto &m = *ptr;
  ptr->eachTick([&m]() {moveMobile<monster>(m);} );
  ptr->eachTick([&m]() {monsterAttacks(m);} );
  equipMonster(type.type(), level, *ptr);

  if (!b.isHighlight() && level.depth() > 30 && dPc() < level.depth()) {
    // dpc / 26 => 0..25 => 0; 26..51 => 1; 52..77 => 2; 78..100 => 3
    constexpr int maxMut = (1+static_cast<int>(mutationType::END));
    int val = (dPc() / ((80 / maxMut) + 1));
    if (val < static_cast<int>(mutationType::END))
      ptr->mutate(static_cast<mutationType>(val));
  } else {
    // more Cybers on Cyber Monday
    static bool i(isItCyberMonday());
    if (i && !b.isHighlight() && level.depth() > 15 && dPc() < 50) {
      ptr->mutate(mutationType::CYBER);
    }
  }

  return ptr;
}

std::vector<std::pair<unsigned int, monsterType*>> spawnMonsters(int depth, int rooms,
   std::function<bool(const monsterType*)> f) {
  auto from = monsterTypeRepo::instance().begin();
  auto to = monsterTypeRepo::instance().end();
  auto filter = [depth,f](monsterType* mt) {
    // Dungeoneers aren't found below level 3 (Ref: Knightmare, which had only 3 levels)
      return (depth <= 3 || mt->type() != monsterTypeKey::dungeoneer) &&
      // don't randomnly generate sirens; they must be on watery levels, on the rocks:
      mt->type() != monsterTypeKey::siren &&
      // likewise, don't generate merfolk outside the water:
      mt->type() != monsterTypeKey::merfolk &&
      // likewise, don't generate mokumokuren; they come from cracked levels
      mt->type() != monsterTypeKey::mokumokuren &&
      f(mt);
  };
  auto ffrom = make_filtered_iterator(filter, from, to);
  auto tto = make_filtered_iterator(filter, to, to);
  return rndGen<monsterType*, filtered_iterator<std::vector<monsterType*>::iterator > >(ffrom, tto, depth, rooms);
}
