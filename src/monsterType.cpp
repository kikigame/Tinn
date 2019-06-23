/* License and copyright go here*/

// define characteristics of monster genii and species

// TODO: monster idea: boxen. Like an oxen, but boxier.

#include "monster.hpp"
#include "monsterType.hpp"
#include "random.hpp"
#include "level.hpp"
#include "religion.hpp"
#include "role.hpp"
#include "terrain.hpp"


monsterTypeName::monsterTypeName(const wchar_t * name) :
  adjectives_(),
  name_(name) {};

monsterTypeName::monsterTypeName(std::initializer_list<const wchar_t *> adjectives,
				 const wchar_t * name) :
  adjectives_(adjectives),
  name_(name) {};

const wchar_t * monsterTypeName::name() const {
  return name_;
}
std::vector<const wchar_t *>::const_iterator monsterTypeName::begin() const {
  return adjectives_.begin();
}
std::vector<const wchar_t *>::const_iterator monsterTypeName::end() const {
  return adjectives_.end();
}


std::unique_ptr<monsterTypeRepo> monsterTypeRepo::instance_;

class monsterTypeBuilder {
  //  friend class monsterTypeRepoImpl; // our builder
  friend class monsterType; // our builder
private:
  monsterCategory category_;
  std::vector<monsterTypeName> monsterNames_; // by experience
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
  double corpseWeight_;
  materialType material_;
  std::vector<deity *> alignment_;
  std::wstring encyclopedium_ = L"";
  std::bitset<materialTypeSize> foodMaterials_;
  std::vector<const wchar_t *> sayings_;
  movementType movementType_;
  bonus fearless_;
  monsterIntrinsics intrinsics_;
  bool alluring_;
  bool undead_;
public:
  const monsterTypeKey key_;
  monsterTypeBuilder(monsterTypeKey key) : 
    category_(), monsterNames_(), levelFactor_(0), levelOffset_(0), minSpawn_(0), maxSpawn_(0),
    xpFactor_(0), xpOffset_(0), renderChar_(L'\0'), strength_(0), appearance_(0), fighting_(0),
    dodge_(0), maxDamage_(0), gen_(genderAssignType::neuter), corpseWeight_(0),
    material_(materialType::fleshy), alignment_(),
    foodMaterials_(), sayings_(),
    movementType_ ({ speed::turn2, goTo::player, goBy::smart, 0 }),
    fearless_(), intrinsics_(), alluring_(false), undead_(false),
    key_(key) {}
  monsterTypeBuilder& category(monsterCategory category) { category_ = category; return *this; }
  monsterTypeBuilder& name(const wchar_t * name) { monsterNames_.push_back(name); return *this; }
  monsterTypeBuilder& name(std::initializer_list<const wchar_t *> adjectives, const wchar_t * name) { monsterNames_.emplace_back(adjectives, name); return *this; }
  monsterTypeBuilder& className(const std::wstring & className) { className_ = className; return *this; }
  // how quickly does the monster progress as we get below levelOffset?:
  monsterTypeBuilder& levelFactor(int levelFactor) { levelFactor_ =levelFactor ; return *this; }
  // on which level does this monster start?
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
  monsterTypeBuilder& corpseWeight(double weight) { corpseWeight_ = weight; return *this; }
  monsterTypeBuilder& align(deity & align) { alignment_.push_back(&align); return *this; }
  monsterTypeBuilder& saying(const wchar_t * const saying) { sayings_.push_back(saying); return *this; }
  // if an entry in the "agenda" is an "agendum", then an entry in the "encyclopedia" must be an...
  monsterTypeBuilder& encyclopedium(const std::wstring & encyclopedium) { encyclopedium_ += encyclopedium; return *this; }
  monsterTypeBuilder& eats(materialType t) { foodMaterials_[static_cast<size_t>(t)] = true; return *this; }
  template <typename I>
  monsterTypeBuilder& align(I begin, I end) { 
    for (I i = begin; i != end; ++i)
      alignment_.push_back(&(*i));
    return *this; 
  }
  monsterTypeBuilder& carryWeight(unsigned int carryWeightN) { intrinsics_.carryWeightN(carryWeightN); return *this; }
  monsterTypeBuilder& movement(movementType type) { movementType_ = type; return *this; }
  monsterTypeBuilder& movesThrough(terrainType type) { intrinsics_.move(tFactory.get(type), true); return *this; }
  monsterTypeBuilder& fearless() { fearless_ = bonus(true); return *this; }
  monsterTypeBuilder& scardy() { fearless_ = bonus(false); return *this; }
  monsterTypeBuilder& throws() { intrinsics_.throws(true); return *this; }
  monsterTypeBuilder& zap() { intrinsics_.zap(true); return *this; }
  monsterTypeBuilder& alluring() { alluring_ = true; return *this; }
  monsterTypeBuilder& undead() { undead_ = true; return *this; }
  monsterTypeBuilder& sleeps() { intrinsics_.sleeps(); return *this; }
  monsterTypeBuilder& see() { intrinsics_.see(true); return *this; }
  monsterTypeBuilder& hear() { intrinsics_.hear(true); return *this; }
  monsterTypeBuilder& speedy() { intrinsics_.speedy(true); return *this; }
  monsterTypeBuilder& dblAttack() { intrinsics_.dblAttack(true); return *this; }
  monsterTypeBuilder& swim() { intrinsics_.swim(true); return *this; }
  monsterTypeBuilder& fly() { intrinsics_.fly(true); return *this; }
  monsterTypeBuilder& climb() { intrinsics_.climb(true); return *this; }
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
  corpseWeight_(b.corpseWeight_),
  material_(b.material_),
  foodMaterials_(b.foodMaterials_),
  sayings_(b.sayings_),
  alignment_(b.alignment_),
  movementType_(b.movementType_),
  intrinsics_(std::make_shared<monsterIntrinsics>(b.intrinsics_)),
  flags_(
	 ((b.alluring_? 1 : 0) << static_cast<size_t>(flag::ALLURING)) |
	 ((b.undead_? 1 : 0) << static_cast<size_t>(flag::UNDEAD))
	 ) {
}

const monsterTypeKey monsterType::type() const { return key_; }
const monsterCategory monsterType::category() const { return category_; }
const wchar_t * const monsterType::encyclopedium() const { return encyclopedium_.c_str(); }
const int monsterType::iStrength() const { return strength_; }
const int monsterType::iAppearance() const { return appearance_; }
const int monsterType::iFighting() const { return fighting_; }
const int monsterType::iDodge() const { return dodge_; }
const int monsterType::iMaxDamage() const { return maxDamage_; }
const double monsterType::corpseWeight() const { return corpseWeight_; }
const materialType monsterType::material() const { return material_; }
const wchar_t monsterType::renderChar() const { return renderChar_; }
const std::vector<monsterTypeName>& monsterType::names() const { return monsterNames_; }
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
const bool monsterType::eats(const materialType foodType) const { return foodMaterials_[static_cast<size_t>(foodType)]; }
bool monsterType::operator == (const monsterType & rhs) const {
  return key_ == rhs.key_;
}
const monsterTypeName &monsterType::name(const unsigned char maxDamage) const {
  auto numNames = monsterNames_.size();
  if (numNames == 1) return monsterNames_.at(0); // optimisation
  if (maxDamage < iMaxDamage()) return monsterNames_.at(0); // safety
  // we will take damage.max() as our designated stat.
  // A type-0 monster has iMaxDamage() HP.
  // A type-N monster has up to 100 max HP.
  // We want to divide the levels into N ranges between iMaxDamage() and 100. -1 here results in 0-based index
  auto threshold = (100 - iMaxDamage()) / (numNames - 1);
  // eg if there are 3 names and iMaxDamage() = 40, that gives t=15
  // so we "grow up" at 55, 70, 85, meaning 15 points at each name.
  auto idx = (maxDamage - iMaxDamage()) / threshold;
  return monsterNames_.at(idx);
}
const genderAssignType monsterType::gen() const {
  return gen_;
}
bool monsterType::alluring() const {
  return flags_[static_cast<int>(flag::ALLURING)];
}
bool monsterType::undead() const {
  return flags_[static_cast<int>(flag::UNDEAD)];
}


class monsterTypeRepoImpl {
private:
  friend class monsterTypeRepo;
  std::map<monsterTypeKey,monsterType> r_;
  std::vector<monsterType *> pr_;
public:
  monsterTypeRepoImpl() {
    deityRepo &dr = deityRepo::instance();	    
    
    // unique features: change equipment slots based on size.
    emplace(monsterTypeBuilder (monsterTypeKey::dragon)
	    .category(monsterCategory::dragon) // TODO: Western dragons should also have wings. All should have claws. Not sure if "flank" is right.
	    .name(L"dragonet") // ref: 14th century term for a baby dragon
	    .name(L"dragon")
	    .name(L"dragon")
	    .name(L"dragon")
	    .className(L"Dragon") // Some dragons are serpants, others are lizards. So they have their own class.
	    // alignment of a dragon is overwritten in dragon constructor
	    .align(dr.getExact(Element::none, Domination::none, Outlook::none))
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
	    .corpseWeight(98066.500286389)// 10 metric tonnes for transport; we *can* lift them, but don't want to encourage it...
	    .eats(materialType::fleshy)
	    .eats(materialType::leathery) // definitely carnivores, but I'm guessing they'll eat some of your armour too
	    .eats(materialType::liquid)
	    .saying(L"Behold the Powerful Dragon") // should not actually say this; depends on the monster's specifics
	    // DRAGONS DON'T FLY! It's mythologically inaccurate...
	    .speedy()
	    .dblAttack()
	    .see()
	    .hear()
	    .fearless()
	    .encyclopedium(
L"Dragons are large serpentine creatures; highly intelligent and amongst the\n"
"strongest of all. Even dragonets - the newly hatched young - should only be\n"
"approached withe extreme caution, if at all. Their diet is exclusively meat,\n"
"and preferred diets vary. Catalonians speak of dragons spitting acid, while\n"
"the stories of fire-breathing are older than the written word. Planting a\n"
"dragon's tooth may grow an arm; the blood of a dragon is such vile acid that\n"
"the ground itself will reject it, and it can eat through iron. In Germany it\n"
"is written that dragon's blood can render any skin bathed in it invincible,\n"
"yet in Switzerland, Heinrich von Winkelreid died from the merest touch of\n"
"the dripping blood of a wounded dragon.\n"
"You can tell how powerful a dragon is by counting its claws; around Japan, 3\n"
"claws are common; 4 around Mongolia. Chinese royalty is heralded by 9 great\n"
"dragons, each having 81 yang scales, 36 yin scales, 9 forms and 9 sons.\n"
"The dragon is a rare culinary delicary: eating their skins heal the wound\n"
"of love; eating its head brings courage and bravery; and even planting its\n"
"head can restore the condition of the land.\n"
"They are rarely found: and usually seen in oceans, rivers, caves and tombs."
			   )
	    .throws()
	    .zap()
	    .sleeps() // for centuries
	    .carryWeight(300000) // the strength of 100 humans, seems about right
            .fearless());

    // unique feature: grows continuously
    emplace(monsterTypeBuilder(monsterTypeKey::blob)
	    .category(monsterCategory::blob)
	    .name({L"small"}, L"blob")
	    .name(L"blob")
	    .name({L"big"}, L"blob")
	    .name({L"huge"}, L"blob")
	    .className(L"blobs")
	    .levelFactor(10)
	    .levelOffset(50)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(10)
	    .xpOffset(50)
	    .renderChar(L'P') // grey ooze in Nethack
	    .strength(25)
	    .appearance(0)
	    .fighting(25)
	    .dodge(25)
	    .maxDamage(50)
	    .material(materialType::waxy)
	    .corpseWeight(20000)
	    .align(dr.nonaligned())
	    .saying(L"blob")
	    .eats(materialType::clothy)
	    .eats(materialType::fleshy)
	    .eats(materialType::glassy)
	    .eats(materialType::leathery)
	    .eats(materialType::liquid)
	    .eats(materialType::metallic)
	    .eats(materialType::papery)
	    .eats(materialType::veggy)
	    .eats(materialType::waxy)
	    .eats(materialType::woody) // doesn't eat stone. So there.
	    .carryWeight(0)
	    .movement(stationary) // TODO: grows instead of moving
	    .movesThrough(terrainType::ALTAR)
	    .movesThrough(terrainType::ROCK)
	    .movesThrough(terrainType::GROUND)
	    .movesThrough(terrainType::UP)
	    .movesThrough(terrainType::DOWN)
	    //.movesThrough(terrainType::PIT_HIDDEN) // inately avoids all traps
	    //.movesThrough(terrainType::PIT)
	    //.movesThrough(terrainType::PIANO_HIDDEN)
	    .movesThrough(terrainType::FIRE)
	    .movesThrough(terrainType::WATER)
	    .movesThrough(terrainType::SPACE)
	    .movesThrough(terrainType::KNOTWEED)
	    .movesThrough(terrainType::DECK)
	    .fearless()
	    .dblAttack() // can't move, and deep in the game, so let's give it a chance
	    .encyclopedium(
L"Blobs grow rapidly, consuming most things in their path. On the other hand,\n"
"at least they present a large target."
			   ));

	    

    // unique feature: noxious excrement
    emplace(monsterTypeBuilder(monsterTypeKey::bull)
	    .category(monsterCategory::quadruped)
	    .name(L"bull")
	    .name(L"bull")
	    .name(L"bonnacon")
	    .className(L"large herbivorous mammals")
	    .levelFactor(5)
	    .levelOffset(30)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(10)
	    .xpOffset(30)
	    .renderChar(L'q') // as slash'em
	    .strength(15) // between snake and human
	    .appearance(20)
	    .fighting(75)
	    .dodge(10)
	    .maxDamage(30)
	    .gen(genderAssignType::mf)
	    .corpseWeight(13344.6648847527) // 3000lb, on the heavy side
	    .align(dr.getExact(Element::earth, Domination::aggression, Outlook::cruel))
	    .saying(L"Mooo!")
	    .eats(materialType::veggy)
	    .carryWeight(60000) // 2 humans' worth
	    .movement({speed::slow2, goTo::player, goBy::avoid, 10})
	    .fearless()
	    .sleeps()
	    .see()
	    .hear()
	    .encyclopedium(
L"Bovines are farmed for their meat and milk. One of the largest are the\n"
"bonnacon, a reddish-brown to black creature with ram-like horns, which curl\n"
"back - making them useless for combat. As a consequence, they instead rely\n"
"on a noxious, caustis fecal defence."));
	    
    
    // unique feature: doesn't descend below level 3
    emplace(monsterTypeBuilder(monsterTypeKey::dungeoneer)
	    .category(monsterCategory::biped)
	    .name(L"Dungeoneer")
	    .className(L"humanoid")
	    .levelFactor(1)
	    .levelOffset(0)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(1)
	    .xpOffset(10)
	    .renderChar(L'@')
	    .strength(15)
	    .appearance(60)
	    .fighting(10)
	    .dodge(5) // blind
	    .maxDamage(10) // weak; often need to use brains to solve puzzles instead of fighting.
	    .gen(genderAssignType::mf)
	    .align(dr.getExact(Element::none, Domination::concentration, Outlook::kind))
	    .saying(L"My Quest is for the Cup") // ref:Knightmare (TV Series)
	    .saying(L"My Quest is for the Sword") // ref:Knightmare (TV Series)
	    .saying(L"My Quest is for the Shield") // ref:Knightmare (TV Series)
	    .saying(L"My Quest is for the Crown") // ref:Knightmare (TV Series)
	    .saying(L"My Quest is for the Maiden") // ref:Knightmare (TV Series) (series 1)
	    .corpseWeight(1334.46648459) // 300lb
	     // no eats(); uses a napsack
	    .movement({speed::perturn, goTo::down, goBy::smart, 100})
	    .throws()
	    .zap()
	    .sleeps()
	    .hear() // can't see
	    .carryWeight(1000) // typically children; 1/3 of human
	    .encyclopedium(
L"Sometimes a human ventures into a Dungeon upon a quest. This is usually ill-\n"
"advised."));

    // unique feature: stealing something & running away
    emplace(monsterTypeBuilder (monsterTypeKey::ferret)
	    .category(monsterCategory::quadruped)
	    .name(L"ferret kit")
	    .name(L"ferret")
	    .name(L"hunting ferret")
	    .className(L"small hunting mammels")
	    .levelFactor(1)
	    .levelOffset(-1)
	    .minSpawn(1)
	    .maxSpawn(3)
	    .xpFactor(5)
	    .xpOffset(5)
	    .renderChar(L'f') // felines in nethack
	    .strength(10)
	    .appearance(50) // beautiful but smelly
	    .fighting(30)
	    .dodge(40)
	    .maxDamage(2) // not very vicious, except against rabbits
	    .gen(genderAssignType::mf)
	    .align(dr.getExact(Element::earth, Domination::aggression, Outlook::kind))
	    .saying(L"*poing*") // ref: Sluggy Freelance
	    // ferrets weigh 2-4lb (male) or 1-2lb (female), but we don't support discrimination,
	    // so we'll stick to 2lb.
	    .corpseWeight(8.8964432306) //2lb
	    .eats(materialType::fleshy) // obligate carnivores
	    .eats(materialType::liquid)
	    .movement({speed::turn2, goTo::player, goBy::smart, 75})
	    .zap() // they lack the strength to throw stuff, but are curious enough to use a wand.
	    .carryWeight(20)
	    .encyclopedium(
L"Meaning \"Little Thief\", ferrets are small, hyperflexible elongated mammels\n"
"of the Mustela family. They are domesticated hunting working animals bread for\n"
"hunting rabbit. While they don't burrow, they love running through tunnels,\n"
"playing with whatever comes to hand, and biting - which, with poor eyesight\n"
"and smell, is how they mostly investigate the world.")
	    .see()
	    .hear()
	    .sleeps() // a lot
            .scardy());

    // unique feature: chinese foxes can ignite fires
    // chinese fox can take the form of a young girl, old man, or academic.
    emplace(monsterTypeBuilder(monsterTypeKey::fox)
	    .category(monsterCategory::quadruped)
	    .name(L"fox") // no shapsifting
	    .name(L"kitsuni")
	    .name(L"huli jing")
	    .className(L"small hunting mammels")
	    .levelFactor(1)
	    .levelOffset(-1)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(10)
	    .xpOffset(10)
	    .renderChar(L'f') // felines in nethack
	    .strength(20)
	    .appearance(75)
	    .fighting(40)
	    .dodge(60)
	    .maxDamage(35) // not very vicious, except against rabbits
	    .gen(genderAssignType::mf)
	    .align(dr.getExact(Element::earth, Domination::concentration, Outlook::cruel))
	    .saying(L"GekGekGekGek!")
	    // foxes weigh 6.5 - 24 lbs
	    // so we'll stick to 20lb.
	    .corpseWeight(88.964432306) //20lb
	    .eats(materialType::fleshy) // obligate carnivores
	    .eats(materialType::liquid)
	    .movement({speed::perturn, goTo::player, goBy::avoid, 25})
	    .alluring()
	    .carryWeight(2000) // less than a human
	    .encyclopedium(
L"Vulpine animals get a mixed reaction; these charming, beautiful creatures\n"
"are known for both chicken-rustling and hunting smaller pests. In China,\n"
"when these creatures are not killed, they can live for up to ten centuries,\n"
"learning how to start fires with a strike of their tail, and even to change\n"
"form at will.")
	    .sleeps()
            .scardy()
	    .see()
	    .hear()
	    );
    

    // unique feature: stealing treasure
    emplace(monsterTypeBuilder (monsterTypeKey::goblin)
	    .category(monsterCategory::biped)
	    .name(L"goblin")
	    .name(L"red-cap goblin")
	    .className(L"goblin")
	    .levelFactor(1)
	    .levelOffset(5)
	    .minSpawn(1)
	    .maxSpawn(1) // often alone in folklore
	    .xpFactor(5)
	    .xpOffset(5)
	    .renderChar(L'o') // as nethack
	    .strength(30)
	    .appearance(10)
	    .fighting(30)
	    .dodge(80) // they are tricky, so should be hard to hit
	    .maxDamage(40)
	    .gen(genderAssignType::indirect) //we know there are male & female goblims, and myths that don't spicify or care, but I can't imagine any goblin not messing with any foocubus one way or the other.
	    .align(dr.getExact(Element::fire, Domination::aggression, Outlook::cruel)) // warlike
	    .align(dr.getExact(Element::time, Domination::aggression, Outlook::cruel)) // trickster
	    .align(dr.getExact(Element::none, Domination::aggression, Outlook::cruel)) 
	    .saying(L"Go back to your room. Play with your toys and costumes. Forget about the baby") // ref: Labyrinth
	    .saying(L"I ask for so little. Just fear me, love me, do as I say") // ref: Labyrinth
	    .corpseWeight(889.64432306) //200lb
	    .eats(materialType::papery) // not very fussy
	    .eats(materialType::clothy) // not very fussy
	    .eats(materialType::veggy)
	    .eats(materialType::fleshy)
	    .eats(materialType::leathery)
	    .eats(materialType::liquid)
	    .movement({speed::turn2, goTo::player, goBy::smart, 75})
	    .throws()
	    .zap()
	    .sleeps()
	    .carryWeight(4000)
	    .see()
	    .hear()
	    .encyclopedium(
L"The difference between a goblin and an orc is that orcs don't exist.\n"
"Not all goblins are malevolent; some are merely mischievous. All are obsessed\n"
"with shiny treasure; they will take what they want by force, trickery or even\n"
"magic.")); // not fearless, as they prove quite tough already


    // unique features: change saying based on size.
    emplace(monsterTypeBuilder (monsterTypeKey::hound)
	    .category(monsterCategory::quadruped)
	    //.name(L"puppy") // I don't mind puppies, but I don't want puppy combat on level 1 thanks
	    .name(L"dog")
	    .name(L"coyote") // NB: Nethack announces coyotes with silly latin names, a reference to the Road Runner cartoons
	    .name(L"hound") // bit of an odd track, as "hound" means tamed hunting dog
	    .name(L"wolfhound") // hound for hunting wolves
	    .name(L"wild dog")
	    .name(L"painted dog") // aka african hunting dog or african hunting wolf
	    .name(L"wolf")
	    .name(L"Big Bad Wolf") // ref:fairytales
	    .className(L"Canid") // genus canus
	    .levelFactor(1)
	    .levelOffset(10)
	    .minSpawn(1)
	    .maxSpawn(12) // todo: unsure how big dog packs can be
	    .xpFactor(1)
	    .xpOffset(10)
	    .renderChar(L'd') // as per Nethack
	    .strength(10)
	    .appearance(20)
	    .fighting(40)
	    .dodge(20) // they're frisky, but we don't want a puppy to kill a starting player
	    .maxDamage(10) // not easy to kill even as a baby, although tougher creatures exist
	    .gen(genderAssignType::mf)
	    .align(dr.getExact(Element::earth, Domination::aggression, Outlook::kind))
	    .align(dr.getExact(Element::earth, Domination::aggression, Outlook::cruel))
	    .align(dr.getExact(Element::earth, Domination::aggression, Outlook::none))
	    .corpseWeight(889.64432306) // 200lb. That's scrawny for a wolf, on the big side for a dog
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .saying(L"(howl)") // todo: woof for puppies & dogs
	    .movement({speed::turn2, goTo::unaligned, goBy::smart, 50})
	    .sleeps()
	    .carryWeight(1000)
	    .see()
	    .hear()
	    .encyclopedium(L"Canines are furry, with four legs and a tail. They are easily excited, always\n"
"hungry and pack hunters. They enjoy bones and some are known to bark or howl."));

    // unique feature: uniquely, no unique features
    emplace(monsterTypeBuilder (monsterTypeKey::human)
	    .category(monsterCategory::biped)
	    .name({L"inexperienced"}, L"human")
	    .name(L"human")
	    .name({L"wise"}, L"human")
	    .name({L"elder"}, L"human")
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
	    .gen(genderAssignType::indirect)
	    .align(dr.begin(), dr.end())
	    .saying(L"(mumble mumble) synergise (mumble muble) teamwork.") // clearly a manager...
	    .saying(L"Have you seen my 'phone?") // something people say. They're obsessed with 'phones, but none in this game.
	    .saying(L"Cor; it's like Picadilly Circus 'round 'ere.") // ref: I cannot source this quote, but it's a common saying for "it's busy/crowded/lots of people"
	    .corpseWeight(2713.415185333) // 610lb; average weight of Earth gravity human
	    .eats(materialType::veggy)
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .throws()
	    .sleeps()
	    .zap()
	    .carryWeight(3000) // generous, but encumberance rules always suck anyway
	    .encyclopedium(L"Amongst the smelliest of monsters, these strange creatures have a poor sense\n"
" of smell. They lack in intelligece, cunning, learning, strength, power and\n"
"speed - but are very adept at using tools and being unpredictable."));

    // unique feature: seduction/dream-rape
    emplace(monsterTypeBuilder(monsterTypeKey::incubus)
	    .category(monsterCategory::biped)
	    .name(L"Incubus")
	    .className(L"Lesser demon")
	    .levelFactor(1)
	    .levelOffset(10)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(50)
	    .xpOffset(0)
	    .renderChar(L'&')
	    .strength(20)
	    .appearance(100)
	    .fighting(50)
	    .dodge(5) // why would they dodge?
	    .maxDamage(30)
	    .saying(L"How you doin'?") // ref:Friends (TV Series), Joey
	    .saying(L"Eyy!") // ref: Arthur Herbert Fonzarelli, Happy Days
	    .gen(genderAssignType::m)
	    .material(materialType::liquid) // prevent player from taking corpse easily
	    .align(dr.begin(), dr.end())
	    .movement({speed::turn3, goTo::unaligned, goBy::smart, 10})
	    .corpseWeight(-10) // magical creature
	    .eats(materialType::clothy) // ripping clothes off with its teeth...
    //ref: http://www.chesterfieldparanormalresearch.com/incubus---sucubbus-demon.html; wikipedia; http://mythicalcreatureslist.com/mythical-creature/Succubus; others
	    .alluring()
	    .see()
	    .hear()
	    .swim()
	    .carryWeight(0) // does not carry stuff
	    .encyclopedium(L"The word incubus comes from the Latin /incubāre/ (to lay\n"
      "upon), from Latin /incubō/ (nightmare). Even the most hedonistic\n"
      "of demonologist should think twice before engaging one. They have \n"
      "been known to come to seduce women their dreams with their comely\n"
      "appearance for their own nefarious acts, sometimes in the form of a\n"
      "deceased spouse. They do not feel pleasure themselves, instead driven\n"
      "by the sin of lust. All are indiscriminate and want only to spread\n"
      "their seed for their own ends. "));

    // unique feature: shape-shift between human and horse forms
    // unique feature: makes the sound of thunder as their tail enters the water: ref: http://www.historic-uk.com/CultureUK/The-Kelpie/
    // TODO: An equine kelpie may have a bridle; holding this gives command over Kelpies, but they will resent it.
    emplace(monsterTypeBuilder(monsterTypeKey::kelpie)
	    .category(monsterCategory::biped)
	    .name(L"Kelpie")
	    .name(L"Each-uisge")
	    .className(L"Water Spirits")
	    .levelFactor(1)
	    .levelOffset(20)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(30)
	    .xpOffset(10)
	    .renderChar(L'@') // or L'u' when shape-shifted
	    .strength(30)
	    .appearance(90)
	    .fighting(30)
	    .dodge(20)
	    .maxDamage(50)
	    .saying(L"")
	    .gen(genderAssignType::m) // always male in folklore
	    .material(materialType::liquid) // breaks down into a black slush, according to https://folkrealmstudies.weebly.com/-scottish-legends-the-each-uisge.html
	    .align(dr.getExact(Element::water, Domination::concentration, Outlook::cruel))
	    .align(dr.getExact(Element::water, Domination::aggression, Outlook::cruel))
	    .movement({speed::perturn, goTo::unaligned, goBy::smart, 20})
	    .corpseWeight(4893.04377683) // 1100lb as typical for a horse
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .sleeps()
	    .carryWeight(0) // does not carry stuff
	    .see()
	    .hear()
	    .swim()
	    .movesThrough(terrainType::WATER)
	    .encyclopedium(
L"Kelpies live in rivers and streams, while the stronger Each-uisge prefers\n"
"the sea - although there is some overlap. When in human form, either can be\n"
"easily identified by seaweed in the hair.\n"
"They are often alone and, like most water creatures, are often charming.\n"
"These equine beasts are never far from water, and seem easy to ride, but\n"
"delight in running off with and drowning their - often human - prey.\n"
"Kelpies are often known to enact retribution for bad behaviour conducted\n"
"on a Sunday."));

    // unique feature: charms enemies, watery attacks
    emplace(monsterTypeBuilder(monsterTypeKey::merfolk)
	    .category(monsterCategory::merfolk)
	    .name(L"Merfolk")
	    .className(L"Water Spirits")
	    .levelFactor(1)
	    .levelOffset(30)
	    .minSpawn(1)
	    .maxSpawn(3) // as siren
	    .xpFactor(50)
	    .xpOffset(0)
	    .renderChar(L';') // as SLASHTHEM, for all sea monsters
	    .strength(40)
	    .appearance(80)
	    .fighting(60)
	    .maxDamage(20)
	    .saying(L"La!")
	    .gen(genderAssignType::mf) // males are mermen, which are a different type
	    .material(materialType::fleshy)
	    .align(dr.getExact(Element::water, Domination::concentration, Outlook::kind))
	    .movement({speed::turn3, goTo::none, goBy::smart, 10})
	    .corpseWeight(2713.415185333) // 610lb; average weight of Earth gravity human
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .throws() // trydants traditionally
	    .sleeps()
	    .zap()
	    .carryWeight(3500) // a little stronger than human
	    .see()
	    .hear()
	    .swim()
	    .encyclopedium(
L"Merfolk do not like to stray outside the sea, and mermen less so. Having the\n"
"tail of a fish and the body of the beautiful human, albeit covered in tiny\n"
"scales, the merfolk usually exist peacefully at sea. More than one hapless\n"
"traveller has fallen for their charms, even attempting to save them as if\n"
"drowning, or falling in love with them, and met their end drowned at sea,\n"
"whether by negligence or intent."));
	    

    // unique feature: sits on rocks and lures other monsters to their death with its song
    emplace(monsterTypeBuilder(monsterTypeKey::siren)
	    .category(monsterCategory::bird) // birds with human heads, but we don't differentiate head slots
	    .name(L"Siren")
	    .className(L"birdoid") // NB: Will fight cross-aligned birds of prey
	    .levelFactor(1)
	    .levelOffset(30)
	    .minSpawn(1)
	    .maxSpawn(3) // 3 sirens in the Odyssey
	    .xpFactor(50)
	    .xpOffset(0)
	    .renderChar(L'b')
	    .strength(40)
	    .appearance(80)
	    .fighting(60)
	    .maxDamage(20)
	    .saying(L"La!")
	    .gen(genderAssignType::mf) // sometimes female, but this seems to be cross-contamination with mermaid myths
	    .corpseWeight(49.0333) // as bird of prey
	    .material(materialType::fleshy)
	    .align(dr.getExact(Element::air, Domination::concentration, Outlook::cruel))
	    .align(dr.getExact(Element::water, Domination::concentration, Outlook::cruel))
	    .movement({speed::perturn, goTo::none, goBy::smart, 10}) // slow for a bird, but works better as we are likely to be charmed.
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .carryWeight(0) // does not carry
	    .sleeps()
	    .see()
	    .hear()
	    .swim()
	    .fly()
	    .climb()
	    .alluring()
	    // no throw or zap; they make you go to them
	    .encyclopedium(
L"Of all the birds of the sea, the sirens are the most beautiful and the most\n"
"deadly. With human heads, they sing an irrisistably enchanting song, luring\n"
"unwary tranvellers into the sea, to dash swimmers and bots onto the rocks."));

    // unique feature: length based on hit points (ref: computer game snake; grows at it eats. Also, nethack has some snake characters occuping multiple squares)
    // TODO: Milking venom depending on type?
    emplace(monsterTypeBuilder(monsterTypeKey::snake)
	    .category(monsterCategory::snake)
	    .name(L"grass snake")
	    .name(L"rattle snake")
	    .name(L"adder")
	    .name(L"python")
	    //.name(L"boa constrictor")
	    .name(L"corn snake")
	    .name(L"tree snake")
	    .name(L"viper")
	    .name(L"pit viper")
	    .name(L"cobra")
	    .name(L"king cobra") // TODO: something with lyres and lyre snakes?
	    .className(L"serpant")
	    .levelFactor(1)
	    .levelOffset(5)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(20)
	    .xpOffset(10)
	    .renderChar(L's')
	    .strength(10)
	    .appearance(75)
	    .fighting(40)
	    .dodge(15)
	    .maxDamage(30)
	    .gen(genderAssignType::mf)
	    .material(materialType::fleshy)
	    .corpseWeight(1.9614) // based on 200g for a garter snake
	    .saying(L"Ssssssssssss")
	    .saying(L"Sssssss")
	    .saying(L"SssssSSSssSsSSssss")
	    .eats(materialType::fleshy)
	    .align(dr.begin(), dr.end())
	    .movement({speed::slow3, goTo::wander, goBy::smart})
	    .sleeps() // with their eyes open
	    .carryWeight(0) // does not carry
	    .see()
	    .hear() // no ears; they hear through their jaws
	    .encyclopedium(
L"Be they creatures of immortality, creation, temptation, or hairstyle, snakes\n"
"and other serpants have often found themselves at the centre of mythology.\n"
"Snakes are cold-blooded, so dislike wasting energy. They eat once every few\n"
"weeks, by dislocating their jaw to swallow they prey. Their main form of\n"
"defence is to wrap themselves around branches; although some have venom\n"
"used mostly for hunting, very few are poisonous."
			   ));

    // unique feature: seduction/dream-rape
    emplace(monsterTypeBuilder(monsterTypeKey::succubus)
	    .category(monsterCategory::biped)
	    .name(L"Succuubus")
	    .className(L"Lesser demon")
	    .levelFactor(1)
	    .levelOffset(10)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(50)
	    .xpOffset(0)
	    .renderChar(L'&')
	    .strength(20)
	    .appearance(100)
	    .fighting(50)
	    .dodge(5) // why would they dodge?
	    .maxDamage(30)
	    .saying(L"Boop oop a doop!") // ref:Betty Boop
	    .saying(L"Happy Birthday Mister Prisident") // ref:Marilyn Monroe
	    .gen(genderAssignType::f)
	    .material(materialType::liquid) // prevent player from taking corpse easily
	    .align(dr.begin(), dr.end())
	    .movement({speed::turn3, goTo::player, goBy::smart, 10})
	    .corpseWeight(2713.415185333) // 610lb; average weight of Earth gravity human
	    .eats(materialType::clothy) // ripping clothes off with its teeth...
	    .eats(materialType::liquid)
	    .alluring()
	    .carryWeight(0) // does not carry
	    .see()
	    .hear()
	    .swim()
	    .encyclopedium(
    //ref: http://www.chesterfieldparanormalresearch.com/incubus---sucubbus-demon.html; wikipedia; http://mythicalcreatureslist.com/mythical-creature/Succubus; others
    L"The word succubus comes from the Latin /succubāre/ (to lie\n"
      "under). Succubi are the Harlots of Hell. Even the most hedonistic\n"
      "of demonologist should think twice before engaging one. They have\n"
      "been known to come to seduce men in their dreams with their comely\n"
      "appearance for their own nefarious acts, sometimes in the form of a\n"
      "deceased spouse. They do not feel pleasure themselves, instead driven\n"
      "by the sin of lust. Some say Lilith is a succubus. All are\n"
      "indiscriminate and want only male seed and life-force for their own\n"
    "ends."));

    // unique feature: eats rocks; stony corpses
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
	    .gen(genderAssignType::mfn)
	    .material(materialType::stony)
	    .align(dr.getExact(Element::earth, Domination::none, Outlook::cruel))
	    .movement({speed::slow3, goTo::unaligned, goBy::beeline, 25})
	    .corpseWeight(27134.15185333) // 10 * 610lb; average weight of Earth gravity human
	    .eats(materialType::stony)
	    .eats(materialType::liquid) // I'm sure I've read about them sucking the water of lichenss
	    // Mostly based on http://www.mysticfiles.com/trolls-from-ancient-to-modern/
	    .throws()
	    .sleeps() // by day, usually
	    .carryWeight(6000) // notoriously strong
	    .encyclopedium(L"Dim-witted creatures of Norse and Scandinavian origin. Big and ugly, trolls\n"
"are known for possessing magical objects and treasures - like gold, or\n"
"princesses. They wander little and are fierecely territorial, but have been\n"
"known to attack human shrines. They are known to hate sunlight."));

    // unique feature: staying put & minding their own business
    emplace(monsterTypeBuilder (monsterTypeKey::venusTrap)
	    .category(monsterCategory::blob)
	    .name(L"venus fly trap")
	    .name(L"venus moth trap")
	    .name(L"venus spider trap")
	    .name(L"venus beetle trap")
	    .name(L"venus cockroach trap")
	    .name(L"venus human trap")
	    .name(L"venus troll trap")
	    .className(L"plants")
	    .levelFactor(1)
	    .levelOffset(-1)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(10)
	    .xpOffset(1)
	    .renderChar(L'#') // trees in Nethack are '#' or '±'
	    .strength(10)
	    .appearance(90) // beautiful but look dangerous
	    .fighting(50)
	    .dodge(0)
	    .maxDamage(10)
	    .gen(genderAssignType::neuter)
	    .align(dr.getExact(Element::plant, Domination::aggression, Outlook::cruel))
	    .corpseWeight(1.9613) // 200g
	    .eats(materialType::fleshy) // obligate carnivores
	    // they may drink water, but they can't open a bottle of it
	    .movement({speed::stop, goTo::none, goBy::avoid, 0})
	    .carryWeight(0)
	    .encyclopedium(
L"There are a great number of creatures in the world, and not all sit neatly\n"
"in their categories. The Venus trap is a deadly flora which allures its\n"
"victims with sweet aromas, until they meet their deadly end within its hairy\n"
"jaw-like leaves...\n"
"The Dionaea muscipula is found mostly in sub-tropical wetlands.")
            .fearless()); // can't move anyway

    // unique feature: beeline approach ignoring all traps. Instant death on pin traps
    emplace(monsterTypeBuilder (monsterTypeKey::zombie)
	    .category(monsterCategory::biped)
	    .name({L"rotting"}, L"zombie")
	    .name({L"elderly"}, L"zombie")
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
	    .corpseWeight(2713.415185333) // 610lb; average weight of Earth gravity human
	    .material(materialType::fleshy)
	    .align(dr.getExact(Element::time, Domination::aggression, Outlook::none))
	    .movement({speed::slow3, goTo::player, goBy::zombeeline, 0})
	    .carryWeight(0)
	    .undead()
	    // do not eat
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
      "effective brute-force warriors, although they can be lured into traps."
			   )
      .fearless()); // brainless implies fearless

    // unique features: flying
    emplace(monsterTypeBuilder(monsterTypeKey::raptor)
	    .category(monsterCategory::raptor)
	    .name(L"falcon")
	    .name(L"eagle")
	    .name(L"harpy eagle")
	    .className(L"raptor")
	    .levelFactor(4)
	    .levelOffset(3)
	    .minSpawn(1)
	    .maxSpawn(1)
	    .xpFactor(30)
	    .xpOffset(20)
	    .renderChar(L'b')
	    .strength(20)
	    .appearance(5)
	    .fighting(10)
	    .dodge(5)
	    .maxDamage(70)
	    .saying(L"Squawk!")
	    .gen(genderAssignType::mf)
	    .material(materialType::fleshy)
	    .align(dr.getExact(Element::air, Domination::concentration, Outlook::kind))
	    .align(dr.getExact(Element::air, Domination::concentration, Outlook::none))
	    .align(dr.getExact(Element::air, Domination::concentration, Outlook::cruel))
	    .corpseWeight(49.0333) // this should vary wildly, but set it at 5kg for a roughly typical weight
	    .movement({speed::turn2, goTo::wander, goBy::avoid, 0})
	    .eats(materialType::fleshy)
	    .eats(materialType::liquid)
	    .carryWeight(40) // a little heavier than a big rabbit
	    .sleeps()
	    // ref: https://skeptoid.com/episodes/4262
	    .encyclopedium(
     L"Most flying creatures will leave you alone unless disturbed, but\n"
      "beware that they also effortlessly evade most traps."
			   ));

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

// pick a random solid monster, eg for culinary purposes:
const monsterType &rndSolidMonster() {
  auto &repo = monsterTypeRepo::instance();
  const monsterType *rtn;
  do {
    // do not return zombie, because no-one wants a zombie steak.
    auto key = static_cast<monsterTypeKey>(rndPickI(0, static_cast<int>(monsterTypeKey::zombie)));
    rtn = &(repo[key]);
  } while (rtn->material() == materialType::liquid);
  return *rtn;
}
