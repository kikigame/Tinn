/* License and copyright go here*/

// Things and stuff

#include "itemTypes.hpp"
#include "random.hpp"
#include "damage.hpp"
#include <set>
#include <bitset>
#include <algorithm>


class itemTypeImpl {
private:
  const wchar_t renderChar_;
  const wchar_t * const name_;
  const wchar_t * const description_;
  const wchar_t * const vagueDescription_;
  const double baseWeight_;
  const materialType materialType_;
public:
  itemTypeImpl(const double baseWeight,
	       const materialType materialType,
	       const wchar_t renderChar,
	       const wchar_t * const name,
	       const wchar_t * const description,
	       const wchar_t * const vagueDescription) :
    renderChar_(renderChar),
    name_(name),
    description_(description),
    vagueDescription_(vagueDescription),
    baseWeight_(baseWeight),
    materialType_(materialType) {}
  const wchar_t render() const { return renderChar_; }
  const wchar_t * const name() const { return name_; }
  const wchar_t * const description() const { return description_; }
  const wchar_t * const vagueDescription() const { return vagueDescription_; }
  const double baseWeight() const { return baseWeight_; }
  const materialType material() const { return materialType_; }
};

itemType::itemType(itemTypeImpl *impl) :
  pImpl_(impl) {}

const wchar_t itemType::render() const {
  return pImpl_->render();
}
const wchar_t * const itemType::name() const {
  return pImpl_->name();
}
const wchar_t * const itemType::description() const {
  return pImpl_->description();
}
const wchar_t * const itemType::vagueDescription() const {
  return pImpl_->vagueDescription();
}
const double itemType::baseWeight() const { 
  return pImpl_->baseWeight(); 
}
const materialType itemType::material() const { 
  return pImpl_->material(); 
}

bool itemType::operator ==(const itemType &other) const {
  return pImpl_ == other.pImpl_;
}

std::vector<damageType> allDamageTypes = {
  damageType::edged, 
  damageType::bashing,
  damageType::hot, 
  damageType::cold, 
  damageType::wet,
  damageType::sonic, 
  damageType::disintegration, 
  damageType::starvation,
  damageType::electric
};


std::unique_ptr<itemTypeRepo> itemTypeRepo::instance_;

itemTypeRepo::itemTypeRepo() {
    emplace(itemTypeKey::apple,
	  new itemTypeImpl(1.0, materialType::veggy, L'%', L"apple", 
L"The Apple is rumoured to provide knowledge, music, well-built hardware, and\n"
"original sin. It is a fruit witn a fresh, crisp taste of gentle sweetness,\n"
"to compliments cold white wine, pork, cinnamon or hot crumble.\n"
"When struck on the head by an Apple, consider the gravity of the situation.",
L"This is a greenish redish seed-bearing structure of a flowering plant. It is\n"
"notable only for its exact weight of 1N."
			   ));
    emplace(itemTypeKey::corpse, // 610N is arerage weight for a human; actual mass will vary
	    new itemTypeImpl(610, materialType::fleshy, L'%', L"corpse",
L"After vital functions have ceased, creatures become a little less active\n"
"and are ready to enter the food chain.\n",
L"A lifelike reproduction, albeit smellier."));
    emplace(itemTypeKey::mace,
	    new itemTypeImpl(11, materialType::metallic, L'!', L"mace",
L"This bashing weapon sits neatly between a club and a flail. Surprisingly\n"
"light, these come into their own against metallic plate armour. It has a\n"
"brazed flange and metal shaft and is a little over half a metre long. To\n"
"use effectively, swing from the shoulder and use your strength to smash the\n"
"mace into your opponent.\n",
L"This cold, hard stick has a bobble on the end. Looks good for hitting with."
			     ));
    emplace(itemTypeKey::two_sword,
	    // 3lb 11oz based on http://myarmoury.com/feature_oakeshott.html (Baron)
	    // 1.673kg, but I've rounded up because I'm guessing reproductions are lighter and I want a heavy sword
	    new itemTypeImpl(16.5, materialType::metallic, L'!', L"2-handed sword",
L"A type XIIa sword in the Oakeshott classification. Unlike many swords, this\n"
"one has a sharpened heavy blade to rip through the chain-mail of one's\n"
"opponent. Its massive weight requires both hands to wield.",
L"Heavy long bit of metal with a big handle."
			     ));
    emplace(itemTypeKey::rock,
	    new itemTypeImpl(8, materialType::stony, L'¬', L"rock",
L"Not the fundamental building blocks, but the building blocks of the\n"
"fundament. Aside from propping up a table leg or growing alpine plants, the\n"
"humble stone has many uses. This one looks like it would do some damage if\n"
"thrown hard at an enemy. Some creatures have even been known to eat rocks\n"
"like people eat bread rolls. Rock is one half of an important musical\n"
"genre.",
L"Like, totally stoned man." // just a pun, but it could reference nethack's hallucination
			     ));
    emplace(itemTypeKey::bow, // 25momme = mediumweight silk
	    // http://lovasilk.com/guide-to-silk-bedding/momme-weight-explained/ - 1momme = lb weight of 100yds@45"
	    // we want 8"x1" for a decemnt bow, so 25*8/(36*100*45) oz = 0.0012345679 oz
	    new itemTypeImpl(0.00034322697748849383, materialType::clothy, L'[', L"bow",
L"Worn on the left of a hatters' hatband, the right of a milliner's fascinator\n"
"or tied around the tail of fully-dressed warhorse, one's public appearance\n"
"simply doesn't look finished without a length of ribbon in a neatly-tied bow.",
L"Someone stole the present!"
			     ));
    emplace(itemTypeKey::boots, // answers.com says average ~48oz
	    new itemTypeImpl(13.34466488475264, materialType::leathery, L'[', L"pair of boots",
L"A good pair of boots will protect your feet and ankles when walking.",
L"Tall shoes"
			     ));
    emplace(itemTypeKey::cloak, // http://www.cloakmaker.com/cloakfaq.html suggests 14-20lb for a lined cloak; we'll take 17lb
	    new itemTypeImpl(75.6197676802653, materialType::clothy, L'[', L"cloak",
L"Cloaks are one of the oldest forms of clothing, and likely began as a form\f"
"of artificial skins to wrap around onesself for warmth. This one is a semi-\n"
"circle of fabric with a simple closing at the neck. Cloaks are excellent for\n"
"keeping warm while travelling.",
L"Some sort of roundded blanket."
			     ));
    emplace(itemTypeKey::crupper, // https://en.wikipedia.org/wiki/Horses_in_the_Middle_Ages gives 70lbs total for horse armour; we'll split that equally between crupper, flanchard, peytral & saddle for now => 17.5 lb each
	    new itemTypeImpl(77.8438784943908, materialType::metallic, L'[', L"crupper",
L"The simplest cruppers are mere straps intended to prevent the saddle from\n"
"slipping forward. This is the armoured kind, also called a croupiere or\n"
"crupiere bacul, and protects a steed's hindquarters during combat.",
L"Metal bum"
			     ));
    emplace(itemTypeKey::doublet, // https://www.quora.com/How-much-did-the-armor-of-an-English-Knight-of-the-middle-16th-century-weigh?share=1 says "all weights listed included that of a 6.3kg arming doublet, ", so 6.3kg
	    new itemTypeImpl(61.7818951804251, materialType::clothy, L'[', L"doublet",
L"This buttoned jacket reaches down to the waist, is worn over a shirt, and\n"
"as well as looking smart, it protects the wearer from any metallic armour\n"
"worn over the top",
L"Flouncy shirty thing"
			     ));
    emplace(itemTypeKey::flanchard,
	    new itemTypeImpl(77.8438784943908, materialType::metallic, L'[', L"pair of flanchards",
L"The flanchards armour the flanks of a steed during combat.",
L"Metal plates of organic shape"
			     ));
    emplace(itemTypeKey::haubergeon, // https://answers.yahoo.com/question/index?qid=20110825080532AAhi650 gives 11kg for a Haubergeon
	    new itemTypeImpl(107.873150315028, materialType::metallic, L'[', L"haubergeon",
L"This smaller, sleeveless version of the hauberk is a metallic shirt\n"
"offering reasonable protection at a relatively small weight.\n"
"A shirt of chain-mail provides good protection from most kinds of weapon.\n"
"If worn under plate mail, it covers the gaps between each plate, which would\n"
"otherwise become leathal gaps in one's armour.\n",
L"Shiny tshirt"
			     ));
    emplace(itemTypeKey::hauberk, // https://answers.yahoo.com/question/index?qid=20110825080532AAhi650 gives 55lb for a "full mail covering". I think that's probably the upper end, but as weight is the only advantage for a haubergeon, I'll take it at 55lb.
	    new itemTypeImpl(244.6521895538, materialType::metallic, L'[', L"hauberk",
L"A shirt of chain-mail provides good protection from most kinds of weapon.\n"
"If worn under plate mail, it covers the gaps between each plate, which would\n"
"otherwise become leathal gaps in one's armour.\n",
L"Shiny shirt"
			     ));
    emplace(itemTypeKey::helmet, // https://www.medievalarmour.com/c-790-medieval-helmets.aspx
	    new itemTypeImpl(10, materialType::metallic, L'[', L"helmet",
L"The diminutive form of /helm/; a helmet covers only the head. Originally\n"
"metal or bronze, this steel protective headgear will protect from various\n"
"forms of cranial impact. This one looks like a basic bascinet, sans aventail.",
L"A decorative pointy metal hat."
			     ));
    emplace(itemTypeKey::jerkin, // I can't find a good reference, so I'm guessing 36oz for now (4 square feet; see scabbord)
	    new itemTypeImpl(10.0084986635645, materialType::leathery, L'[', L"jerkin",
L"Not a male gyrfalcon, but a close-fitting sleveless over-jacket worn over a\n"
"doublet. This protects the doublet from the rain and helps to keep the core\n"
"of the body warm, as well as providing some additional armour against\n"
"glancing blows.\n",
L"Some kind of waistcoat"
			     ));
    emplace(itemTypeKey::peytral,
	    new itemTypeImpl(77.8438784943908, materialType::metallic, L'[', L"peytral",
L"The peytral is one of the most important pieces of armour for any rider who\n"
"charges their enemy in combat. It protects the chest of the horse, reaching\n"
"down to the upper legs and as far back as the saddle.",
L"Some kind of giant's necklace"
			     ));
    emplace(itemTypeKey::robe, // I found a bathrobe at "2-3lb". I imagine terrycotton would be slightly lighter than lightweight wool, so let's say 5lb https://www.sierratradingpost.com/turkish-cotton-terry-robe-closeouts-for-men~p~16453/questions/2026528~how-much-does-this-robe-weigh/
	    new itemTypeImpl(22.2411081412545, materialType::clothy, L'[', L"robe",
L"A simple robe protects the modesty of the wearer and keeps out most of the\n"
"cold.",
L"Jedi costume"
			     ));
    emplace(itemTypeKey::saddle,
	    new itemTypeImpl(77.8438784943908, materialType::leathery, L'[', L"saddle",
L"A saddle is attached to the back of the steed, and provides a platform for\n"
"the rider to sit on. It is the attachment point for saddle-bags, peytrals\n"
"and flanchards and so is indespensible for an armoured steed.\n",
L"Leather step"
			     ));
    emplace(itemTypeKey::scabbord,  // http://www.waterhouseleather.com/leather_buying_guide.htm suggests ~9oz per square foot roughly for leather. Let's say a belt is 6'x1" = 1sq ft, but doubled leather = 2sq ft => 18oz. Times 1.5 to allow for sword loop & metal fixings => 24oz
	    new itemTypeImpl(6.67233244237632, materialType::leathery, L'[', L"scabbord",
L"It is said that Excalibur's scabbord is worth ten of the sword, but this is\n"
"merely a convenient way to carry one's sword without having to wield it all\n"
"the time.",
L"Belt with decorative dangly bits"
			     ));
    emplace(itemTypeKey::shirt, // https://www.gentelle.com/blog/what-is-gsm-in-fabrics/ gives 130gsm for summer and 210 gsm for luxuary linen. I'm going with 210gsm, on the grounds that modern "luxuary" was probably more like (what would have passed for what we'd now call) year-round weight. Let's guess at 2m for a shirt (roughtly the area of human skin on the body, but shirts fit more loosely and that quickly adds up). =  420g
	    new itemTypeImpl(4.11879301202834, materialType::clothy, L'[', L"shirt",
L"The shirt is the most timeless wardrobe staple, and finds its place on all\n"
"warriors, peasants and nobles alike. Only the material differs; this is a\n"
"very servicable linen.",
L"Cloth chainmail"
			     ));
    emplace(itemTypeKey::shorts, // http://www.cockeyed.com/science/weight/weight_menu.html gives 22oz for US "pants" and 16oz for Jeans. Let's take 20oz for trousers and 16oz for shorts.
	    new itemTypeImpl(4.44822162825088, materialType::clothy, L'[', L"pair of short trousers",
L"These trousers are cut short to the middle of the lower leg, protecting the\n"
"modesty of the weather while guarding against hot weather.",
L"Shirt for a short headless person"
			     ));
    emplace(itemTypeKey::skirt, // guessing same as shorts
	    new itemTypeImpl(4.44822162825088, materialType::clothy, L'[', L"skirt",
L"In tailoring or seamstering, the \"skirt\" is simply the bottom edge of any\n"
"garment. Garments are often made with a long, straight section ending at a\n"
"finished, mostly straight, edge. A garment worn between the waist and legs,\n"
"consisting of a mostly straight section ending at a finished, mostly\n"
"straight, edge, is often known simply as a skirt. This sikrt is cut short to\n"
"the middle of the lower leg, protecting the modesty of the weather while\n"
"guarding against hot weather.",
L"small blanket"
			     ));
    emplace(itemTypeKey::socks, // http://www.cockeyed.com/science/weight/weight_menu.html gives 2oz
	    new itemTypeImpl(0.55602770353136, materialType::clothy, L'[', L"pair of socks",
L"Worn between feet and shoes to protect the wearer from the effects of the\n"
"leather and to provide warmnth.",
L"Angled mittens"
			     ));
    emplace(itemTypeKey::trousers,
	    new itemTypeImpl(5.5602770353136, materialType::clothy, L'[', L"pair of trousers",
L"A trouser is a tubular garmnt with two seams worn along the leg, as distict\n"
"from a trew, which has only one seam. This garment consists of two trousers\n"
"joined by a crotch and seat.",
L"Shirt for a tall headless person"
			     ));
    emplace(itemTypeKey::tshirt, // http://www.cockeyed.com/science/weight/weight_menu.html 5oz for undershirt
	    new itemTypeImpl(1.3900692588284, materialType::clothy, L'[', L"tshirt",
L"A mass-produced cotton undergarment. These are most notorious for political\n"
"slogons being drawn or printed on them at political protests, but have\n"
"become a mainstream staple garment, even worn as outerwear during hot weather.",
L"Holy Y-shaped cloth tube"
			     ));
    emplace(itemTypeKey::underpants, // http://www.cockeyed.com/science/weight/weight_menu.html 1--2oz for underwear; taking 1.5
	    new itemTypeImpl(0.41702077764852, materialType::clothy, L'[', L"pair of underpants",
L"Worn between one's neither-regions and the rest of the world; extends the\n"
"wear time of outer garments and are easier to clean.",
L"Novelty hat"
			     ));
    emplace(itemTypeKey::stick,
	    new itemTypeImpl(0.5, materialType::woody, L'/', L"stick",
L"In its most basic form, a wand is a device with which you may inscribe on a\n"
"wax tablet, also known as a /stylus/. Many wands are imbued with charges of\n"
"magical energy, often associated with druidic of fairy magic, to provide\n"
"an enchantment associated with the material of the wand. It is not usually\n"
"possible to derive the nature of a stick other than by enchanting it.",
L"With enough sticks and a little patience, you too can make a camp fire."
			     ));
    emplace(itemTypeKey::bottle, // 2N based on a 20oz Coke bottle (old style)
	    new itemTypeImpl(2, materialType::glassy, L'8', L"bottle",
L"A humble glass bottle. This container can hold or mix a liquid charge, or\n"
"contain almost anything along with a bottling kit. Their contents will be\n"
"protected from many forms of damage, including spillage, so long as the\n"
"bottle holds up.",
L"Soft drinks rot your teeth. At least remember to recycle your glassware."
			     ));
    emplace(itemTypeKey::codex,
	    // based on 0.9g/cm3 for good quality paper; a paper copy of a big codex on Amazon
	    // measures 39x13x46cm, giving a mass of ~21kg
	    new itemTypeImpl(205, materialType::papery, L'¶', L"codex",  // http://www.antithetical.org/restlesswind/plinth/bookbind2.html
L"Unlike folios, which are mere sheets of vellum folded into verso and recto\n"
" pages, the codex is bound on raised cords, breaking a scroll of text into\n"
"multiple sectionts, allowing instant access to any part of the work.\n"
"The codex is a great method to convey complex knowledge in a lasting form.\n",
L"Book."
			     ));
    emplace(itemTypeKey::hitch_guide,
	    // based on 0.9g/cm3 for good quality paper; a paper copy of a big codex on Amazon
	    // measures 39x13x46cm, giving a mass of ~21kg
	    new itemTypeImpl(205, materialType::metallic, L'¶', L"Hitch-Hiker's Guide",  //ref:Hitchhiker's Guide to the Galaxy
L"\"Kindle\" is the collective noun for kittens. I don't know why I told you\n" //ref: Amazon's electronic book is called a "Kindle".
"that, because this is an electronic book. Various flashy lights appear on it,\n"
"and friendly life advice is displayed prominantly on the cover.\n",
L"Book."
			     ));
    emplace(itemTypeKey::holy_book, // cheating and just copying the codex stats
	    // based on 0.9g/cm3 for good quality paper; a paper copy of a big codex on Amazon
	    // measures 39x13x46cm, giving a mass of ~21kg
	    new itemTypeImpl(205, materialType::papery, L'¶', L"Holy text",  // http://www.antithetical.org/restlesswind/plinth/bookbind2.html
L"Unlike folios, which are mere sheets of vellum folded into verso and recto\n"
" pages, the codex is bound on raised cords, breaking a scroll of text into\n"
"multiple sectionts, allowing instant access to any part of the work.\n"
"The codex is a great method to convey complex knowledge in a lasting form.\n",
L"Book."
			     ));
    emplace(itemTypeKey::iou, // assume 600gsm ID-1 card @ 85.60mm by 53.98mm
	    // not ¶ to avoid generating in shops:
	    new itemTypeImpl(0.0271696454, materialType::waxy, L'✎', L"I.O.U.",
L"If you run up a debt you cannot pay, you will not be allowed to forget it.",
L"Uh-oh..."
			     ));
    emplace(itemTypeKey::poke,
	    new itemTypeImpl(0.5, materialType::clothy, L'=', L"poke",
L"A large canvas bag, a little smaller than a sack. Useful for carrying items\n"
"around. Some people sew small versions into their clothes.\n"
"When ye proffer the pigge, open the poke.", // http://www.worldwidewords.org/qa/qa-pig2.htm
L"One side of this pillow remains unsewn."
			     ));
    emplace(itemTypeKey::water,
	    new itemTypeImpl(2.31585538520766, materialType::liquid, L'~', L"water",  // half a pint
L"Above the ignition point, hydrogen explodes in the precence of oxygen; four\n"
"times the mass of oxygen is consumed to the mass of hydrogen, and the\n"
"resuting liquid has some fascinating properties.\n"
"Water is the only known liquid to expand as it freezes; it forms long spikes\n"
"in its crystals (the root of snowflakes). It is tasteless, odourless and\n"
"vital for all known organic life. It is an excellent solvent, especially\n"
"when under pressure - but is virtually incompressible. Being a liquid, it's\n"
"best kept in a bottle.",
L"Looks kind-of wet. It doesn't *smell* like gin..."
			     ));
    emplace(itemTypeKey::tears, // (water + tear damage) // potion of sorrow (TODO)
	    // http://www.answers.com/Q/What_is_the_density_of_saliva puts the density of saliva at 0.002.
	    // I haven't yet found the density of tears, and haven't been happy/sad enough to measury my own.
	    // So I'm just going to multiply half a pint of water by 0.002 and hope that's realistic.
	    new itemTypeImpl(2.32048709597807532, materialType::liquid, L'~', L"tears",  // half a pint
L"Canst thou bottle ones' sorrow? Canst thou drown it in tears?\n"
"Tears are mostly water, with a few other salts and trace proteins, including\n"
" hormones like  prolactin, adrenocorticotropic hormone, and Leu-enkephalin.",
L"Looks all wet, sadly."
			     ));
    emplace(itemTypeKey::heavy_water, // (water + bashing/earth damage)
	    new itemTypeImpl(2.561104527040676154, materialType::liquid, L'~', L"heavy water",  // half a pint
			     // https://www.aqua-calc.com/page/density-table lists Tritium Oxide as 1.25g/l
			     // and D2O as 1.1059000244141 g/l
			     //http://www.differencebetween.net/science/difference-between-water-and-heavy-water/
L"With a pH of 7.41, heavy water is mildly alkaline. It has an identical\n"
"chemical structure to water, with duterium isotopes of hydrogen atoms\n"
"with a high occurance, having 2 neutrons in the nucleus instead of one.\n"
"The increased density is particularly useful in absorbing radioactive\n"
"particles, making D₂O the fluid of choice for generating heat from nuclear\n"
"reactors.",
L"This water is weighty."
			     ));
    emplace(itemTypeKey::fire_water, // (water + fire)
	    new itemTypeImpl(2.1730597421557556844, materialType::liquid, L'~', L"fire water",  // half a pint
			     // 0.93834g/l as for whiskey (http://www.answers.com/Q/What_is_the_density_of_whiskey)
L"A strong, alcoholic liquor. Advise caution when drinking. An inflammible,\n"
"liquid good for sterilizing wounds.",
L"Smells boozy." // ref:Nethack - potion of booze
			     ));
    emplace(itemTypeKey::pop,
	    new itemTypeImpl(2.31585538520766, materialType::liquid, L'~', L"water",  // half a pint
L"This looks like water and has a mild taste, but the container suggests it\n"
"contains an assortment of healthy options in miniscule amounts.",
L"If spilled, wash with copeous amounts of water." // ref: COSHH sheet safety guide for water
			     ));
    emplace(itemTypeKey::fizzy_pop, // (water + air)
	    new itemTypeImpl(2.2926968313555834, materialType::liquid, L'~', L"fizzy pop",  // half a pint as water
			     // based on 0.99g/l; about 1% carbonation. See
			     // https://sciencing.com/density-carbonated-water-6925741.html
L"The neck of the bottle is crimped to accommodate a free-floating marble,\n"
"held firmly against the upper portion of the neck by the pressure of the\n"
"carbon dioxide dissolved in the delicious flavoured fluid. To release the\n"
"fluid, hold the bottle to your mouth and push the marble down with your\n"
"tongue. May contain sugar or artificial sweeteners.",
L"There's a marble in this glass! Let's get it out and play a game?"
			     ));
    emplace(itemTypeKey::dehydrated_water, // (water + time)
	    new itemTypeImpl(1.15792769260383, materialType::liquid, L'~', L"dehydrated water",  // quater of a pint as water
			     // http://www.instructables.com/id/Make-Your-Own-Dehydrated-Water/
L"The process of dehydration, while it can be speeded up with heat, will\n"
"occur naturally over time, if the air humidity is low. It's best to keep\n"
"drinking water in sealed containers, such as bottles, and keep them away from"
"heat to prevent loss due to evaporation.\n"
"The process of dehydration can be used to good effect when preserving solid\n"
"items, as it prevents the growth of bacteria.",
L"Just add water."
			     ));
    emplace(itemTypeKey::spring_water, // (water + flora damage)
	    new itemTypeImpl(2.31585538520766, materialType::liquid, L'~', L"spring water",  // half a pint as water
L"Natural sources of water, such as lakes and streams, provide a habitat for\n"
"all sorts of microscopic acquatic life, which can alter the balance of the\n"
"water and introduce impurities. Spring water is filtered through many layers\n"
"of rock, reducing impurities of this sort. The main impurity in spring water\n"
"is trace amounts of rock salts, which are good for hydration and an\n"
"important dietary mineral.",
L"Looks kind-of wet. It doesn't *smell* like gin..." // as water
			     ));
    emplace(itemTypeKey::electro_pop, // (water + lightning damage) - TODO: synthesis
	    new itemTypeImpl(2.31585538520766, materialType::liquid, L'~', L"electro pop",  // half a pint as water
L"When exposed to large tension (voltage) of electricity, it degrades into\n"
"hydrogen and oxygen molecules, in a process known as cracking.\n"
"Electro-pop is similar to normal carbonated pop, but is particularly useful\n"
"in synthesis.",
L"Popular music can be loosely destribed as modern music, usually with one\n"
"instrument and one or two vocals. The simplicity of the musical composition\n"
"allows a speedy production cycle, so allowing a great quanity of output and\n"
"so a large variety of subgenres. Synth-pop is a subgenre of pop music using\n"
"electronic synthesisers as instruments. Electropop is a subgenre of synth-\n"
"pop in which the synthesisers are deliberately configured to produce an\n"
"artificial electronic sound."
			     ));

    emplace(itemTypeKey::wooden_ring,
	    new itemTypeImpl(0.05, materialType::woody, L'*', L"wooden ring",
L"Often used by gymnasts to demonstrate the graceful application of upper-body\n"
"strength. This simple ring can still be worn, is extremely lightweight and\n"
"inexpensive, and can be imbued with charges of magical energy.\n",
L"It don\'t mean a thing if you ain\'t got that swing..." // ref: Gladiators (TV series)/Hang Tough (which probably referenced something else)
			     ));
    // all I know is a 20-Kalganid note was a lot for a taxi ride to the nearest spaceship...
    emplace(itemTypeKey::kalganid,
	    new itemTypeImpl(0.001, materialType::metallic, L'$', L"coin (kalganid)",
L"The acquisition of money is a greedy goal, but you can swap coins like this\n"
"for more useful items, should you find a shop, market stall, tradesman or\n"
"indeed anyone more greedy than you. This coin is made of a light and\n"
"unfamiliar metal. Kalganids are also available as notes, but paper money is\n"
"not used much around here.\n",
L"A strangly irridessent disc of metal with a worn-out inscription.\n"
			     ));
    emplace(itemTypeKey::shop_card,
	    // ISO/IEC 7810:2003 says that ID-1 cards measure 85.60mm by 53.98mm, at 0.76mm thick,
	    // with rounded corners of 3.18mm radius. But it doesn't say how much they weigh...
	    // one forum post suggests 4--12.5g, so we'll go with 0.1N (~10.2g).
	    // http://ficoforums.myfico.com/t5/Credit-Cards/Credit-Card-weights/td-p/2881505
	    // NB: Prerunners of charge cards were metal according to Wikipedia.
	    new itemTypeImpl(0.1, materialType::metallic, L'(', L"platinum express card",
			     // ref: "Platinum Yendorian Express Card " is the Tourist quest artifact in Nethack
			     // ... and is itself a reference to the C64 game Wizardry.
L"ID-1 standard cards are accepted by many shops as a form of payment,\n"
"known as \"credit\" cards, or \"debit\" cards in the case of prepayment.\n"
"This one in particular is an older form called a \"charge\" card.\n",
L"This looks like someone's ID, but there's no photo, just a line of numbers."
			     ));
    emplace(itemTypeKey::bottling_kit,
	    // TODO: weight. I can't find a shipping weight online yet
	    // charges are bottle caps
	    new itemTypeImpl(10, materialType::metallic, L'(', L"bottling kit",
L"Bottles not included",
L"These look like parts of some larger purpose."
			     ));
}
void itemTypeRepo::emplace(itemTypeKey k, itemTypeImpl* impl) {
  byType_.emplace(k, std::unique_ptr<itemType>());
  byType_[k].reset(new itemType(impl));
}

const itemType& itemTypeRepo::operator[](itemTypeKey k) const {
    return *(byType_.at(k));
  }
itemTypeRepo& itemTypeRepo::instance() {
  if (!instance_) instance_.reset(new itemTypeRepo()); // was empty
  return *instance_;
}

std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator itemTypeRepo::begin() { return byType_.begin(); }

std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator itemTypeRepo::end() { return byType_.end(); }
  
void itemTypeRepo::close() {
  instance_.reset(NULL);
}
