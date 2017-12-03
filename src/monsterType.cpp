/* License and copyright go here*/

// define characteristics of monster genii and species

// TODO: monster idea: boxen. Like an oxen, but boxier.

#include "monster.hpp"
#include "monsterType.hpp"
#include "random.hpp"
#include "level.hpp"
#include "religion.hpp"
#include "role.hpp"



std::unique_ptr<monsterTypeRepo> monsterTypeRepo::instance_;

class monsterTypeBuilder {
  //  friend class monsterTypeRepoImpl; // our builder
  friend class monsterType; // our builder
private:
  monsterCategory category_;
  std::vector<const wchar_t *> monsterNames_; // by experience
  std::wstring className_; // for grouping
  int levelFactor_;
  int levelOffset_;
  int minSpawn_;
  int maxSpawn_;
  int xpFactor_;
  int xpOffset_;
  wchar_t renderChar_;
  int strength_;
  int appearance_;
  int fighting_;
  int dodge_;
  int maxDamage_;
  genderAssignType gen_;
  materialType material_ = materialType::fleshy;
  std::vector<deity *> alignment_;
  std::wstring encyclopedium_ = L"";
  std::vector<const wchar_t *> sayings_;
  movementType movementType_;
public:
  const monsterTypeKey key_;
  monsterTypeBuilder(monsterTypeKey key) : 
    movementType_ ({ speed::turn2, goTo::player, goBy::beeline, 0 }), // smart not done yet
    key_(key) {}					   
  monsterTypeBuilder& category(monsterCategory category) { category_ = category; return *this; }
  monsterTypeBuilder& name(const wchar_t * name) { monsterNames_.push_back(name); return *this; }
  monsterTypeBuilder& className(const std::wstring & className) { className_ = className; return *this; }
  monsterTypeBuilder& levelFactor(int levelFactor) { levelFactor_ =levelFactor ; return *this; }
  monsterTypeBuilder& levelOffset(int levelOffset) { levelOffset_ =levelOffset ; return *this; }
  monsterTypeBuilder& minSpawn(int minSpawn) {       minSpawn_   = minSpawn  ; return *this; }
  monsterTypeBuilder& maxSpawn(int maxSpawn) {       maxSpawn_   = maxSpawn  ; return *this; }
  monsterTypeBuilder& xpFactor(int xpFactor) {       xpFactor_   = xpFactor  ; return *this; }
  monsterTypeBuilder& xpOffset(int xpOffset) {       xpOffset_   = xpOffset  ; return *this; }
  monsterTypeBuilder& renderChar(wchar_t renderChar) {  renderChar_ = renderChar; return *this; }
  monsterTypeBuilder& strength(int strength) { strength_ = strength; return *this; }
  monsterTypeBuilder& appearance(int appearance) { appearance_ = appearance; return *this; }
  monsterTypeBuilder& fighting(int fighting) { fighting_ = fighting; return *this; }
  monsterTypeBuilder& dodge(int dodge) { dodge_ = dodge; return *this; }
  monsterTypeBuilder& maxDamage(int maxDamage) { maxDamage_ = maxDamage; return *this; }
  monsterTypeBuilder& gen(genderAssignType gen) { gen_ = gen; return *this; }
  monsterTypeBuilder& material(materialType mat) { material_ = mat; return *this; }
  monsterTypeBuilder& align(deity & align) { alignment_.push_back(&align); return *this; }
  monsterTypeBuilder& saying(const wchar_t * const saying) { sayings_.push_back(saying); return *this; }
  // if an entry in the "agenda" is an "agendum", then an entry in the "encyclopedia" must be an...
  monsterTypeBuilder& encyclopedium(const std::wstring & encyclopedium) { encyclopedium_ += encyclopedium; return *this; }
  template <typename I>
  monsterTypeBuilder& align(I begin, I end) { 
    for (I i = begin; i != end; ++i)
      alignment_.push_back(&(*i));
    return *this; 
  }
  monsterTypeBuilder& movement(movementType type) { movementType_ = type; return *this; }
};

monsterType::monsterType(const monsterTypeBuilder & b) :
  key_(b.key_),
  category_(b.category_),
  monsterNames_(b.monsterNames_),
  encyclopedium_(b.encyclopedium_),
  className_(b.className_),
  levelFactor_(b.levelFactor_),
  levelOffset_(b.levelOffset_),
  minSpawn_(b.minSpawn_),
  maxSpawn_(b.maxSpawn_),
  xpFactor_(b.xpFactor_),
  xpOffset_(b.xpOffset_),
  renderChar_(b.renderChar_),
  strength_(b.strength_),
  appearance_(b.appearance_),
  fighting_(b.fighting_),
  dodge_(b.dodge_),
  maxDamage_(b.maxDamage_),
  gen_(b.gen_),
  material_(b.material_),
  sayings_(b.sayings_),
  alignment_(b.alignment_),
  movementType_(b.movementType_) {}

const monsterTypeKey monsterType::type() const { return key_; }
const monsterCategory monsterType::category() const { return category_; }
const wchar_t * const monsterType::encyclopedium() const { return encyclopedium_.c_str(); }
const int monsterType::iStrength() const { return strength_; }
const int monsterType::iAppearance() const { return appearance_; }
const int monsterType::iFighting() const { return fighting_; }
const int monsterType::iDodge() const { return dodge_; }
const int monsterType::iMaxDamage() const { return maxDamage_; }
const materialType monsterType::material() const { return material_; }
const wchar_t monsterType::renderChar() const { return renderChar_; }
const std::vector<const wchar_t*>& monsterType::names() const { return monsterNames_; }
const std::vector<deity*>& monsterType::alignment() const { return alignment_; }
const movementType& monsterType::movement() const { return movementType_; }
const int monsterType::getLevelFactor() const { return levelFactor_; }
const int monsterType::getLevelOffset() const { return levelOffset_; }
const int monsterType::getMinSpawn() const { return minSpawn_; }
const int monsterType::getMaxSpawn() const { return maxSpawn_; }
const int monsterType::getXpFactor() const { return xpFactor_; }
const int monsterType::getXpOffset() const { return xpOffset_; }
const std::vector<const wchar_t *>::const_iterator monsterType::sayingsBegin() const {
  return sayings_.begin();
}
const std::vector<const wchar_t *>::const_iterator monsterType::sayingsEnd() const {
  return sayings_.end();
}
bool monsterType::operator == (const monsterType & rhs) const {
  return key_ == rhs.key_;
}
const wchar_t * monsterType::name(const unsigned char maxDamage) const {
  auto numNames = monsterNames_.size();
  if (numNames == 1) return monsterNames_.at(0); // optimisation
  if (maxDamage < iMaxDamage()) return monsterNames_.at(0); // safety
  // we will take damage.max() as our designated stat.
  // A type-0 monster has iMaxDamage() HP.
  // A type-N monster has up to 100 max HP.
  // We want to divide the levels into N ranges between iMaxDamage() and 100.
  auto threshold = (100 - iMaxDamage()) / (numNames + 1);
  // eg if there are 3 names and iMaxDamage() = 40, that gives t=15
  // so we "grow up" at 55, 70, 85, meaning 15 points at each name.
  auto idx = (maxDamage - iMaxDamage()) / threshold;
  return monsterNames_.at(idx);
}



class monsterTypeRepoImpl {
private:
  friend class monsterTypeRepo;
  std::map<monsterTypeKey,monsterType> r_;
  std::vector<monsterType *> pr_;
public:
  monsterTypeRepoImpl() {
    deityRepo &dr = deityRepo::instance();

    emplace(monsterTypeBuilder (monsterTypeKey::dragon)
	    .category(monsterCategory::dragon) // TODO: Western dragons should also have wings. All should have claws. Not sure if "flank" is right.
	    .name(L"Dragonet") // ref: 14th century term for a baby dragon
	    .name(L"Dragon")
	    .name(L"Dragon")
	    .name(L"Dragon")
	    .className(L"Dragon") // TODO: Some dragons are serpants, others are lizards.
	    .levelFactor(10)
	    .levelOffset(10)
	    .minSpawn(1)
	    .maxSpawn(1) // they're pretty much always solitary
	    .xpFactor(20)
	    .xpOffset(50)
	    .renderChar(L'D') // as per Nethack
	    .strength(50)
	    .appearance(90) // after all, they're used decoratively all over the place
	    .fighting(50)
	    .dodge(5) // they're big
	    .maxDamage(80) // not easy to kill even as a baby, although tougher creatures exist
	    .gen(genderAssignType::indirect)
	    .saying(L"Behold the Powerful Dragon") // should not actually say this; depends on the monster's specifics
	    .encyclopedium(L"The dragon is a powerful creature shrouded in mystery.")); // TODO: Better this

    emplace(monsterTypeBuilder (monsterTypeKey::human)
	    .category(monsterCategory::biped)
	    .name(L"inexperienced human")
	    .name(L"human")
	    .name(L"wise human")
	    .name(L"elder human")
	    .className(L"humanoid")
	    .levelFactor(1)
	    .levelOffset(-1)
	    .minSpawn(1)
	    .maxSpawn(10)
	    .xpFactor(5)
	    .xpOffset(5)
	    .renderChar(L'@')
	    .strength(20)
	    .appearance(50)
	    .fighting(30)
	    .dodge(10)
	    .maxDamage(30)
	    .gen(genderAssignType::mf) //TODO
	    .align(dr.begin(), dr.end())
	    .saying(L"(mumble mumble) synergise (mumble muble) teamwork.") // clearly a manager...
	    .saying(L"Have you seen my 'phone?") // something people say. They're obsessed with 'phones, but none in this game.
	    .saying(L"Cor; it's like Picadilly Circus 'round 'ere.") // ref: I cannot source this quote, but it's a common saying for "it's busy/crowded/lots of people"
	    .encyclopedium(L"Amongst the smelliest of monsters, these strange creatures have a poor sense\n"
" of smell. They lack in intelligece, cunning, learning, strength, power and\n"
"speed - but are very adept at using tools and being unpredictable."));

    emplace(monsterTypeBuilder (monsterTypeKey::troll)
	    .category(monsterCategory::biped)
	    .name(L"pebble troll")
	    .name(L"rock troll")
	    .name(L"boulder troll")
	    .name(L"bridge troll")
	    .name(L"mountain troll")
	    .className(L"humanoid")
	    .levelFactor(1)
	    .levelOffset(1)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(10)
	    .xpOffset(10)
	    .renderChar(L'T')
	    .strength(40)
	    .appearance(10)
	    .fighting(60)
	    .dodge(5)
	    .maxDamage(50)
	    .saying(L"Troll smash!") // ref:Hulk smash (Marvel comics)
	    .saying(L"Mmmm rocks...")
	    .gen(genderAssignType::mfn) //TODO
	    .material(materialType::stony)
	    .align(dr.getExact(Element::earth, Domination::none, Outlook::cruel))
	    .movement({speed::slow3, goTo::player, goBy::beeline, 25})
	    // Mostly based on http://www.mysticfiles.com/trolls-from-ancient-to-modern/
	    .encyclopedium(L"Dim-witted creatures of Norse and Scandinavian origin. Big and ugly, trolls\n"
"are known for possessing magical objects and treasures - like gold, or\n"
"princesses. They wander little and are fierecely territorial, but have been\n"
"known to attack human shrines. They are known to hate sunlight."));

    emplace(monsterTypeBuilder (monsterTypeKey::zombie)
	    .category(monsterCategory::biped)
	    .name(L"rotting zombie")
	    .name(L"elderly zombie")
	    .name(L"human zombie")
	    .name(L"warrior zombie")
	    .name(L"giant zombie")
	    .className(L"zombie")
	    .levelFactor(2)
	    .levelOffset(1)
	    .minSpawn(1)
	    .maxSpawn(5)
	    .xpFactor(20)
	    .xpOffset(10)
	    .renderChar(L'Z')
	    .strength(50)
	    .appearance(1)
	    .fighting(10)
	    .dodge(1)
	    .maxDamage(70)
	    .saying(L"Brains...")
	    .gen(genderAssignType::mfn)
	    .material(materialType::fleshy)
	    .align(dr.getExact(Element::time, Domination::aggression, Outlook::none))
	    .movement({speed::slow3, goTo::player, goBy::zombeeline, 0})
	    // ref: https://skeptoid.com/episodes/4262
	    .encyclopedium(
     L"Zombie powder is derived from the pufferfish, human bones and various\n"
      "plants with stinging spines. It is reportedly used by Vodou Bokors to\n"
      "induce a state of physical paralysis. After burying, resting and\n"
      "exhuming the body, it can be revived to a mindless state, devoid of\n"
      "freewill and able to follow basic instructions. Entire workforces can\n"
      "be made of such slaves.\n"
      // ref: Zombie Island (the old 1980s console game, possibly CP/M DOS. Possibly called something like "Escape from Zombie Island" or just "Zombie". You were on an island with zombies that always moved towards you, and had to lure them into pits to defeat them. You died if you jumped into a pit or contacted a zombie.)
      "Having no sense of self-preservation, their hypnotic state makes them\n"
      "effective brute-force warriors, although they can be lured into traps."));

    //ref: http://www.chesterfieldparanormalresearch.com/incubus---sucubbus-demon.html; wikipedia; http://mythicalcreatureslist.com/mythical-creature/Succubus; others
    auto todo = L"The word succubus comes from the Latin /succubāre/ (to lie\n"
      "under). Succubi are the Harlots of Hell. Even the most hedonistic\n"
      "of demonologist should think twice before engaging one. They have\n"
      "been known to come to seduce men in their dreams with their comely\n"
      "appearance for their own nefarious acts, sometimes in the form of a\n"
      "deceased spouse. They do not feel pleasure themselves, instead driven\n"
      "by the sin of lust. Some say Lilith is a succubus. All are\n"
      "indiscriminate and want only male seed and life-force for their own\n"
      "ends.";
    auto todo1 = L"The word incubus comes from the Latin /incubāre/ (to lay\n"
      "upon), from Latin /incubō/ (nightmare). Even the most hedonistic\n"
      "of demonologist should think twice before engaging one. They have \n"
      "been known to come to seduce women their dreams with their comely\n"
      "appearance for their own nefarious acts, sometimes in the form of a\n"
      "deceased spouse. They do not feel pleasure themselves, instead driven\n"
      "by the sin of lust. All are indiscriminate and want only to spread\n"
      "their seed for their own ends. ";

    /* TODO: each monster needs something unique. What can this have? Should we add mat-type gold & instakill?
     * how about making it more powerful or more common on Cyber Monday? - Monday after Thinkgiving, which is itself the fourth Thursday in November
    emplace(monsterTypeBuilder (monsterTypeKey::cyber)
	    .category(monsterCategory::biped)
	    .name(L"Cybermite") // TODO: cybermites should be category blob and mute
	    .name(L"Cybermat")  // TODO: cybermatss should be category blob and mute
	    .name(L"Cyberman")
	    .name(L"Cyberleader")
	    .name(L"Cybercontroller")
	    .name(L"Mr Clever") // TODO: this shold be a named monster if we ever get them
	    .className("Cyber")
	    .levelFactor(10)
	    .levelOffset(20)
	    .minSpwn(1)
	    .maxSpawn(20)
	    .xpFactor(10)
	    .xpOffset(20)
	    .renderChar(L'C') // Hmmm; we're taking the Cockatrice symbol. That'll scare some Nethackers.
	    .strength(20)
	    .appearance(50)
	    .fighting(60)
	    .dodge(10)
	    .maxDamage(75)
	    .saying(L"Delete!")
	    .saying(L"You will be Converted!")
	    .saying(L"Resistance is useless!")
	    .gen(genderAssignType::neuter) // gender has been removed along with the brain, and we'll assume sex has too. Mats & Mites get none anyway.
	    .align(dr.getExact(Element::time, Domination::aggression, Outlook::neutral))
	    .movement({speed::slow2, goTo::player, goBy::beeline, 10})
	    .encyclopedium(L"Mondas calls whence they hail; the twin planet of Earth,\n"
      "driven hence to travel the Universe. The cyber folk are unique for\n"
      "their method of reproduction, in which they take an otherwise\n"
      "unremarkable living human and augment it with various technological\n"
      "improvements until it becomes like them. Their lack of emotion seems\n"
      "not to soften their militant silvery nature. Gold is known to affect\n"
      "the respiratory system of older models.";
    */
  }
  const monsterType& get(const monsterTypeKey & key) const {
    return r_.at(key);
  }
private:
  void emplace(const monsterTypeBuilder &b) {
    r_.emplace(b.key_, b);
    pr_.push_back(&(r_.at(b.key_)));
  }
  std::vector<monsterType*>::iterator begin() {
    return pr_.begin();
  }
  std::vector<monsterType*>::iterator end() {
    return pr_.end();
  }
    
};

monsterTypeRepo::monsterTypeRepo()
  : pImpl_(new monsterTypeRepoImpl()) {}

monsterTypeRepo & monsterTypeRepo::instance() {
  if (!instance_) instance_.reset(new monsterTypeRepo());
  return *instance_;
}

const monsterType& monsterTypeRepo::operator[](const monsterTypeKey & key) const {
  return pImpl_->get(key);
}

std::vector<monsterType*>::iterator monsterTypeRepo::begin() {
  return pImpl_->begin();
}
std::vector<monsterType*>::iterator monsterTypeRepo::end() {
  return pImpl_->end();
}

void monsterTypeRepo::close() {
  instance_.reset(NULL);
}

movementType stationary { speed::slow3, goTo::none, goBy::avoid, 0 };
