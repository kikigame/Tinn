/* License and copyright go here*/

// utility class to hord a coordinate pair

#include "slots.hpp"
#include "monsterType.hpp"
#include <map>

slot::slot() {}

class slotImpl : public slot {
private: 
  const wchar_t * const defName_;
  const std::map<monsterCategory, const wchar_t * const> name_;
  std::vector<const slot* > covers_;
  std::vector<const slot* > covered_;
  //  std::weak_ptr<slot> pairedWith_; - was intended for left-right pairs, but no use for it yet
public:
  slotImpl(const wchar_t * const defName, const std::map<monsterCategory, const wchar_t * const> &name) :
    defName_(defName), name_(name) {}
  slotImpl(const slotImpl & other) = delete;
  virtual const wchar_t * const name(monsterCategory cat) const {
    auto i = name_.find(cat);
    if (i == name_.end()) return defName_;
    return i->second;
  }
  virtual const std::vector<const slot*> covers() const {
    return covers_;
  }
  virtual const std::vector<const slot*> covered() const {
    return covered_;
  }
  //  virtual const std::shared_ptr<slot> pairedWith() const {
  //    return pairedWith_;
  //  }
  slotImpl & covers(const slot &c) { covers_.push_back(&c); return *this; }
  slotImpl & covered(const slot &c) { covered_.push_back(&c); return *this; }
  slotImpl & operator =(const slotImpl &) = delete;
  //  slotImpl & pairedWith(std::shared_ptr<slot> c) { pairedWith_ = c; return *this; }
};

struct slotRepoType {
  std::map<monsterCategory, std::vector<const slot*> > byCat_;
  // NB: if we polymorph between a quadruped and a centaur form, the weapon slot name jumps from forepaws to hands. This does not make sense if weapons are cursed.
  std::unique_ptr<slotImpl> primary_weapon = std::unique_ptr<slotImpl>(new slotImpl(L"wielded (left hand)", {{monsterCategory::quadruped, L"wielded (left forepaw)"}, {monsterCategory::hooved_quadruped, L"wielded (left forehoof)"}, {monsterCategory::dragon, L"wielded (left forepaw)"}, {monsterCategory::bird, L"wielded (left wing)"}, {monsterCategory::birdOfPrey, L"wielded (left wing)"}, {monsterCategory::blob, L"outermost left blobby protrusion"}}));
  std::unique_ptr<slotImpl> secondary_weapon = std::unique_ptr<slotImpl>(new slotImpl(L"wielded (right hand)", {{monsterCategory::quadruped, L"wielded (right forepaw)"}, {monsterCategory::hooved_quadruped, L"wielded (right forehoof)"}, {monsterCategory::hooved_quadruped, L"wielded (right forepaw)"}, {monsterCategory::bird, L"wielded (right wing)"}, {monsterCategory::birdOfPrey, L"wielded (right wing)"}, {monsterCategory::blob, L"outermost right blobby protrusion"}}));
  std::unique_ptr<slotImpl> headband = std::unique_ptr<slotImpl>(new slotImpl(L"headband", {{monsterCategory::blob, L"top blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat = std::unique_ptr<slotImpl>(new slotImpl(L"hat", {{monsterCategory::blob, L"topmost blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses = std::unique_ptr<slotImpl>(new slotImpl(L"eyes", {{monsterCategory::blob, L"top front blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet = std::unique_ptr<slotImpl>(new slotImpl(L"neck",  {{monsterCategory::blob, L"upper blobby protrusion"}}));

  std::unique_ptr<slotImpl> headband_2 = std::unique_ptr<slotImpl>(new slotImpl(L"second headband", {{monsterCategory::blob, L"top second blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat_2 = std::unique_ptr<slotImpl>(new slotImpl(L"second hat", {{monsterCategory::blob, L"topmost second blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses_2 = std::unique_ptr<slotImpl>(new slotImpl(L"second eyes", {{monsterCategory::blob, L"top front second blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet_2 = std::unique_ptr<slotImpl>(new slotImpl(L"second neck",  {{monsterCategory::blob, L"second upper blobby protrusion"}}));

  std::unique_ptr<slotImpl> headband_3 = std::unique_ptr<slotImpl>(new slotImpl(L"third headband", {{monsterCategory::blob, L"top third blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat_3 = std::unique_ptr<slotImpl>(new slotImpl(L"third hat", {{monsterCategory::blob, L"topmost third blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses_3 = std::unique_ptr<slotImpl>(new slotImpl(L"third eyes", {{monsterCategory::blob, L"top front third blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet_3 = std::unique_ptr<slotImpl>(new slotImpl(L"third neck",  {{monsterCategory::blob, L"third upper blobby protrusion"}}));

  std::unique_ptr<slotImpl> headband_4 = std::unique_ptr<slotImpl>(new slotImpl(L"fouth headband", {{monsterCategory::blob, L"top fourth blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat_4 = std::unique_ptr<slotImpl>(new slotImpl(L"fourth hat", {{monsterCategory::blob, L"topmost fourth blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses_4 = std::unique_ptr<slotImpl>(new slotImpl(L"fourth eyes", {{monsterCategory::blob, L"top front fourth blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet_4 = std::unique_ptr<slotImpl>(new slotImpl(L"fourth neck",  {{monsterCategory::blob, L"fourth upper blobby protrusion"}}));

  std::unique_ptr<slotImpl> headband_5 = std::unique_ptr<slotImpl>(new slotImpl(L"fifth headband", {{monsterCategory::blob, L"top fifth blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat_5 = std::unique_ptr<slotImpl>(new slotImpl(L"fifth hat", {{monsterCategory::blob, L"topmost fifth blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses_5 = std::unique_ptr<slotImpl>(new slotImpl(L"fifth eyes", {{monsterCategory::blob, L"top front fifth blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet_5 = std::unique_ptr<slotImpl>(new slotImpl(L"fifth neck",  {{monsterCategory::blob, L"fifth upper blobby protrusion"}}));

  std::unique_ptr<slotImpl> ring_left_index = std::unique_ptr<slotImpl>(new slotImpl(L"left first claw",   {{monsterCategory::biped, L"left index finger"},  {monsterCategory::birdOfPrey, L"left first talon"},  {monsterCategory::merfolk, L"left index finger"},  {monsterCategory::blob, L"first left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_middle = std::unique_ptr<slotImpl>(new slotImpl(L"left second claw", {{monsterCategory::biped, L"left middle finger"}, {monsterCategory::birdOfPrey, L"left second talon"}, {monsterCategory::merfolk, L"left middle finger"}, {monsterCategory::blob, L"second left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_ring = std::unique_ptr<slotImpl>(new slotImpl(L"left third claw",    {{monsterCategory::biped, L"left ring finger"},   {monsterCategory::birdOfPrey, L"left third talon"},  {monsterCategory::merfolk, L"left ring finger"},   {monsterCategory::blob, L"third left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_little = std::unique_ptr<slotImpl>(new slotImpl(L"left fourth claw", {{monsterCategory::biped, L"left little finger"}, {monsterCategory::birdOfPrey, L"left fourth talon"}, {monsterCategory::merfolk, L"left little finger"}, {monsterCategory::blob, L"fourth left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"left fifth claw",   {{monsterCategory::biped, L"left thumb"},         {monsterCategory::birdOfPrey, L"left opposed talon"},{monsterCategory::merfolk, L"left thumb"},          {monsterCategory::blob, L"fifth left blobby protrusion"}}));

  std::unique_ptr<slotImpl> bracelet_left = std::unique_ptr<slotImpl>(new slotImpl(L"left foreleg", {{monsterCategory::biped, L"left wrist"}, {monsterCategory::merfolk, L"left wrist"}, {monsterCategory::blob, L"upper left blobby protrusion"}}));
  std::unique_ptr<slotImpl> gloves = std::unique_ptr<slotImpl>(new slotImpl(L"forepaws",            {{monsterCategory::biped, L"hands"},      {monsterCategory::merfolk, L"hands"},      {monsterCategory::blob, L"outer left blobby protrusion"}}));
  // gauntlets read the same as gloves, but they are different slots; you can wear gauntlets over gloves:
  std::unique_ptr<slotImpl> gauntlets = std::unique_ptr<slotImpl>(new slotImpl(L"forepaws",         {{monsterCategory::biped, L"hands"},      {monsterCategory::merfolk, L"hands"},      {monsterCategory::blob, L"outer left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_index = std::unique_ptr<slotImpl>(new slotImpl(L"right first claw",       {{monsterCategory::biped, L"right index finger"},  {monsterCategory::birdOfPrey, L"right first talon"},  {monsterCategory::merfolk, L"right index finger"}, {monsterCategory::blob, L"first right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_middle = std::unique_ptr<slotImpl>(new slotImpl(L"right second claw",     {{monsterCategory::biped, L"right middle finger"}, {monsterCategory::birdOfPrey, L"right second talon"}, {monsterCategory::merfolk, L"right middle finger"},{monsterCategory::blob, L"second right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_ring = std::unique_ptr<slotImpl>(new slotImpl(L"right third claw",        {{monsterCategory::biped, L"right ring finger"},   {monsterCategory::birdOfPrey, L"right third talon"},  {monsterCategory::merfolk, L"right ring finger"},  {monsterCategory::blob, L"third right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_little = std::unique_ptr<slotImpl>(new slotImpl(L"right fourth claw",     {{monsterCategory::biped, L"right little finger"}, {monsterCategory::birdOfPrey, L"right fourth talon"}, {monsterCategory::merfolk, L"right little finger"},{monsterCategory::blob, L"fourth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"right fifth claw",       {{monsterCategory::biped, L"right thumb"},         {monsterCategory::birdOfPrey, L"right opposed talon"},{monsterCategory::merfolk, L"right thumb"},        {monsterCategory::blob, L"fifth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> bracelet_right = std::unique_ptr<slotImpl>(new slotImpl(L"right foreleg",            {{monsterCategory::biped, L"right wrist"},                                                               {monsterCategory::merfolk, L"right wrist"},        {monsterCategory::blob, L"upper right blobby protrusion"}}));
																											                                                   
  std::unique_ptr<slotImpl> toe_left_index = std::unique_ptr<slotImpl>(new slotImpl(L"left first hind claw",     {{monsterCategory::biped, L"left index toe"},                                                            {monsterCategory::merfolk, L"left first scale"},     {monsterCategory::bird, L"left first claw"}, {monsterCategory::birdOfPrey, L"left first talon"}, {monsterCategory::blob, L"first left blobby protrusion"}})); // wikipedia allows "index toe", so we'll keep it the same as for fingers.
  std::unique_ptr<slotImpl> toe_left_middle = std::unique_ptr<slotImpl>(new slotImpl(L"left second hind claw",   {{monsterCategory::biped, L"left middle toe"},                                                           {monsterCategory::merfolk, L"left second scale"},    {monsterCategory::bird, L"left second claw"}, {monsterCategory::birdOfPrey, L"left second talon"}, {monsterCategory::blob, L"second left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_fourth = std::unique_ptr<slotImpl>(new slotImpl(L"left third hind claw",    {{monsterCategory::biped, L"left ring toe"},                                                             {monsterCategory::merfolk, L"left third scale"},      {monsterCategory::bird, L"Left third claw"}, {monsterCategory::birdOfPrey, L"left third talon"}, {monsterCategory::blob, L"third left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_little = std::unique_ptr<slotImpl>(new slotImpl(L"left fourth hind claw",   {{monsterCategory::biped, L"left little toe"},                                                           {monsterCategory::merfolk, L"left fourth scale"},    {monsterCategory::bird, L"Left fourth claw"}, {monsterCategory::birdOfPrey, L"left fourth talon"}, {monsterCategory::blob, L"fourth left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"left fifth hind claw",     {{monsterCategory::biped, L"left big toe"},                                                             {monsterCategory::merfolk, L"left big scale"},       {monsterCategory::bird, L"Left fifth claw"}, {monsterCategory::birdOfPrey, L"left opposed talon"}, {monsterCategory::blob, L"fifth left blobby protrusion"}})); // the latin name for the big toe is the "hallux"
  std::unique_ptr<slotImpl> anklet_left = std::unique_ptr<slotImpl>(new slotImpl(L"left hind leg",               {{monsterCategory::biped, L"left ankle"},                                                               {monsterCategory::merfolk, L"left tail-finlet"},         {monsterCategory::bird, L"left leg"}, {monsterCategory::birdOfPrey, L"left leg"}, {monsterCategory::blob, L"lower left blobby protrusion"}}));
																											                                                   
  std::unique_ptr<slotImpl> toe_right_index = std::unique_ptr<slotImpl>(new slotImpl(L"right first hind claw",   {{monsterCategory::biped, L"right index toe"},                                                          {monsterCategory::merfolk, L"right first scale"},    {monsterCategory::bird, L"right first claw"}, {monsterCategory::birdOfPrey, L"right first talon"}, {monsterCategory::blob, L"first right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_middle = std::unique_ptr<slotImpl>(new slotImpl(L"right second hind claw", {{monsterCategory::biped, L"right middle toe"},                                                         {monsterCategory::merfolk, L"right second scale"},   {monsterCategory::bird, L"right second claw"}, {monsterCategory::birdOfPrey, L"right second talon"}, {monsterCategory::blob, L"second right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_fourth = std::unique_ptr<slotImpl>(new slotImpl(L"right third hind claw",  {{monsterCategory::biped, L"right ring toe"},                                                           {monsterCategory::merfolk, L"right third scale"},     {monsterCategory::bird, L"right third claw"}, {monsterCategory::birdOfPrey, L"right third talon"}, {monsterCategory::blob, L"third right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_little = std::unique_ptr<slotImpl>(new slotImpl(L"right fourth hind claw", {{monsterCategory::biped, L"right little toe"},                                                         {monsterCategory::merfolk, L"right fourth scale"},   {monsterCategory::bird, L"right forth claw"}, {monsterCategory::birdOfPrey, L"right fourth talon"}, {monsterCategory::blob, L"fourth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"right fifth hind claw",   {{monsterCategory::biped, L"right big toe"},                                                            {monsterCategory::merfolk, L"right big scale"},      {monsterCategory::bird, L"right fifth claw"}, {monsterCategory::birdOfPrey, L"right opposed talon"}, {monsterCategory::blob, L"fifth right blobby protrusion"}}));

  std::unique_ptr<slotImpl> ring_nose = std::unique_ptr<slotImpl>(new slotImpl(L"nasal ring", {{monsterCategory::bird, L"beak ring"}, {monsterCategory::birdOfPrey, L"beak ring"}, {monsterCategory::blob, L"inner top front blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_nose_2 = std::unique_ptr<slotImpl>(new slotImpl(L"second nasal ring", {{monsterCategory::bird, L"second beak ring"}, {monsterCategory::birdOfPrey, L"second beak ring"}, {monsterCategory::blob, L"inner top front second blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_nose_3 = std::unique_ptr<slotImpl>(new slotImpl(L"third nasal ring", {{monsterCategory::bird, L"third beak ring"}, {monsterCategory::birdOfPrey, L"third beak ring"}, {monsterCategory::blob, L"inner top third front blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_nose_4 = std::unique_ptr<slotImpl>(new slotImpl(L"fourth nasal ring", {{monsterCategory::bird, L"fourth beak ring"}, {monsterCategory::birdOfPrey, L"fourth beak ring"}, {monsterCategory::blob, L"inner top front fourth blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_nose_5 = std::unique_ptr<slotImpl>(new slotImpl(L"fifth nasal ring", {{monsterCategory::bird, L"fifth beak ring"}, {monsterCategory::birdOfPrey, L"fifth beak ring"}, {monsterCategory::blob, L"inner top front fifth blobby protrusion"}}));
  std::unique_ptr<slotImpl> anklet_right = std::unique_ptr<slotImpl>(new slotImpl(L"right hind leg", {{monsterCategory::biped, L"right ankle"}, {monsterCategory::bird, L"right leg"}, {monsterCategory::birdOfPrey, L"right leg"}, {monsterCategory::merfolk, L"right tail-finlet"}, {monsterCategory::blob, L"lower right blobby protrusion"}}));

  // like gloves and gauntlets, socks go under shoes but are reported as the same slot:
  std::unique_ptr<slotImpl> socks = std::unique_ptr<slotImpl>(new slotImpl(L"hindpaws", {{monsterCategory::biped, L"feet"}, {monsterCategory::merfolk, L"distal fin"}, {monsterCategory::blob, L"lower blobby protrusions"}}));
  std::unique_ptr<slotImpl> shoes = std::unique_ptr<slotImpl>(new slotImpl(L"hindpaws", {{monsterCategory::biped, L"feet"}, {monsterCategory::merfolk, L"proximal fin"}, {monsterCategory::blob, L"lower blobby protrusions"}}));

  std::unique_ptr<slotImpl> underwear = std::unique_ptr<slotImpl>(new slotImpl(L"pelvis", {{monsterCategory::merfolk, L"adipose fin"}, {monsterCategory::blob, L"lower middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> shorts = std::unique_ptr<slotImpl>(new slotImpl(L"upper legs", {{monsterCategory::quadruped, L"rump"}, {monsterCategory::hooved_quadruped, L"rump"}, {monsterCategory::centaur, L"rump"}, {monsterCategory::dragon, L"abdomen"}, {monsterCategory::merfolk, L"upper tail"}, {monsterCategory::blob, L"top part of lower blobby protrusion"}})); // TODO: insects & lizards should have adbomen
  std::unique_ptr<slotImpl> belt = std::unique_ptr<slotImpl>(new slotImpl(L"belt", {{monsterCategory::blob, L"outer middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> shirt = std::unique_ptr<slotImpl>(new slotImpl(L"torso", {{monsterCategory::blob, L"upper middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> doublet = std::unique_ptr<slotImpl>(new slotImpl(L"torso", {{monsterCategory::blob, L"upper middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> hauburk = std::unique_ptr<slotImpl>(new slotImpl(L"torso", {{monsterCategory::blob, L"upper middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> saddle = std::unique_ptr<slotImpl>(new slotImpl(L"saddle", {}));
  std::unique_ptr<slotImpl> jerkin = std::unique_ptr<slotImpl>(new slotImpl(L"body", {{monsterCategory::quadruped, L"flanks"}, {monsterCategory::hooved_quadruped, L"flanks"}, {monsterCategory::centaur, L"flanks"},{monsterCategory::dragon, L"thorax"}})); // TODO: insects & lizards should have thorax
  std::unique_ptr<slotImpl> robe = std::unique_ptr<slotImpl>(new slotImpl(L"body", {})); // robe/cloak/peytral
  std::unique_ptr<slotImpl> tail = std::unique_ptr<slotImpl>(new slotImpl(L"tail", {{monsterCategory::blob, L"rear blobby protrusion"}}));

  slotRepoType() {
    primary_weapon->covers(*gauntlets);
    secondary_weapon->covers(*gauntlets);
    headband->covered(*hat);
    hat->covers(*headband).covers(*glasses);
    glasses->covered(*hat)
      .covers(*ring_nose);
    headband_2->covered(*hat_2);
    hat_2->covers(*headband_2).covers(*glasses_2);
    glasses_2->covered(*hat_2)
      .covers(*ring_nose_2);
    headband_3->covered(*hat_3);
    hat_3->covers(*headband_3).covers(*glasses_3);
    glasses_3->covered(*hat_3)
      .covers(*ring_nose_3);
    headband_4->covered(*hat_4);
    hat_4->covers(*headband_4).covers(*glasses_4);
    glasses_4->covered(*hat_4)
      .covers(*ring_nose_4);
    headband_5->covered(*hat_5);
    hat_5->covers(*headband_5).covers(*glasses_5);
    glasses_5->covered(*hat_5)
      .covers(*ring_nose_5);
    ring_left_thumb->covered(*gloves);
    ring_left_index->covered(*gloves);
    ring_left_middle->covered(*gloves);
    ring_left_ring->covered(*gloves);
    ring_left_little->covered(*gloves);
    bracelet_left->covered(*gloves);
    gloves->covered(*gauntlets)
      .covers(*ring_left_thumb)
      .covers(*ring_left_index)
      .covers(*ring_left_middle)
      .covers(*ring_left_ring)
      .covers(*ring_left_little)
      .covers(*bracelet_left)
      .covers(*ring_right_thumb)
      .covers(*ring_right_index)
      .covers(*ring_right_middle)
      .covers(*ring_right_ring)
      .covers(*ring_right_little)
      .covers(*bracelet_right);
    gauntlets->covers(*gloves)
      .covered(*primary_weapon)
      .covered(*secondary_weapon);
    ring_right_thumb->covered(*gloves);
    ring_right_index->covered(*gloves);
    ring_right_middle->covered(*gloves);
    ring_right_ring->covered(*gloves);
    ring_right_little->covered(*gloves);
    bracelet_right->covered(*gloves);

    toe_left_thumb->covered(*socks);
    toe_left_index->covered(*socks);
    toe_left_middle->covered(*socks);
    toe_left_fourth->covered(*socks);
    toe_left_little->covered(*socks);
    anklet_left->covered(*socks);
    toe_right_thumb->covered(*socks);
    toe_right_index->covered(*socks);
    toe_right_middle->covered(*socks);
    toe_right_fourth->covered(*socks);
    toe_right_little->covered(*socks);
    anklet_right->covered(*socks);
    socks->covered(*shoes)
      .covers(*toe_left_thumb)
      .covers(*toe_left_index)
      .covers(*toe_left_middle)
      .covers(*toe_left_fourth)
      .covers(*toe_left_little)
      .covers(*bracelet_left)
      .covers(*toe_right_thumb)
      .covers(*toe_right_index)
      .covers(*toe_right_middle)
      .covers(*toe_right_fourth)
      .covers(*toe_right_little)
      .covers(*bracelet_right);
    shoes->covers(*socks);

    underwear->covered(*shorts);
    shorts->covers(*underwear).covered(*belt).covered(*saddle);
    belt->covers(*shorts).covers(*hauburk);
    shirt->covered(*doublet);
    doublet->covered(*hauburk);
    hauburk->covers(*doublet).covered(*belt).covered(*saddle);
    saddle->covers(*shorts)
      .covers(*hauburk).covers(*tail);
    jerkin->covers(*saddle).covered(*robe);
    robe->covers(*jerkin);
    tail->covered(*saddle);

    byCat_.emplace(monsterCategory::biped, std::vector<const slot * >({
	  primary_weapon.get(),
	  secondary_weapon.get(),
	  headband.get(),
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  ring_nose.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get() // robe/cloak/peytral (horse chest armour)
	  }));// biped
    byCat_.emplace(monsterCategory::quadruped,  std::vector<const slot * >({
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  ring_nose.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  saddle.get(),
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get() // tail ring/bow
	  }));//quadruped
    byCat_.emplace(monsterCategory::hooved_quadruped,  std::vector<const slot * >({
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  ring_nose.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  saddle.get(),
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get() // tail ring/bow
	  }));//hooved_quadruped
    byCat_.emplace(monsterCategory::centaur,  std::vector<const slot * >({
	  primary_weapon.get(),
	  secondary_weapon.get(),
	  headband.get(),
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  ring_nose.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  saddle.get(),
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get() // tail ring/bow
	  }));//centaur
    byCat_.emplace(monsterCategory::dragon,  std::vector<const slot * >({
	  primary_weapon.get(),
	  secondary_weapon.get(),
	  headband.get(),
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  headband_2.get(),
	  hat_2.get(),
	  glasses_2.get(),
	  amulet_2.get(),
	  headband_3.get(),
	  hat_3.get(),
	  glasses_3.get(),
	  amulet_3.get(),
	  headband_4.get(),
	  hat_4.get(),
	  glasses_4.get(),
	  amulet_4.get(),
	  headband_5.get(),
	  hat_5.get(),
	  glasses_5.get(),
	  amulet_5.get(),
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  ring_nose.get(),
	  ring_nose_2.get(),
	  ring_nose_3.get(),
	  ring_nose_4.get(),
	  ring_nose_5.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  saddle.get(),
	  tail.get() // tail ring/bow
	  }));//dragon
    byCat_.emplace(monsterCategory::bird,  std::vector<const slot * >({
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(),
	  toe_left_little.get(),
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  toe_right_little.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  saddle.get(),
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get() // tail ring/bow
	  }));//bird
    byCat_.emplace(monsterCategory::birdOfPrey,  std::vector<const slot * >({
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  toe_left_thumb.get(),
	  toe_left_index.get(),
	  toe_left_middle.get(),
	  toe_left_fourth.get(), // owls have 2 talons, 1 opposable and 1 opposed, so skip "little toe"
	  anklet_left.get(),
	  toe_right_thumb.get(),
	  toe_right_index.get(),
	  toe_right_middle.get(),
	  toe_right_fourth.get(),
	  anklet_right.get(),
	  socks.get(),
	  shoes.get(), //boots/shoes
	  underwear.get(),
	  shorts.get(), ///trousers/skirt/crupper (crupper is horse rump armour)
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  saddle.get(),
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get() // tail ring/bow
	  }));//birdOfPrey
    byCat_.emplace(monsterCategory::merfolk, std::vector<const slot * >({
	  primary_weapon.get(),
	  secondary_weapon.get(),
	  headband.get(),
	  hat.get(), //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
	  glasses.get(),
	  amulet.get(),//Necklace/amulet/talisman/medallian
	  ring_left_thumb.get(),
	  ring_left_index.get(),
	  ring_left_middle.get(),
	  ring_left_ring.get(),
	  ring_left_little.get(),
	  bracelet_left.get(),
	  gloves.get(),
	  gauntlets.get(),
	  ring_right_thumb.get(),
	  ring_right_index.get(),
	  ring_right_middle.get(),
	  ring_right_ring.get(),
	  ring_right_little.get(),
	  bracelet_right.get(),
	  ring_nose.get(),
	  belt.get(), //scabbord/belt
	  shirt.get(),
	  doublet.get(),
	  hauburk.get(), //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
	  jerkin.get(), // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
	  robe.get(), // robe/cloak/peytral (horse chest armour)
	  tail.get()
	  }));// biped
    byCat_.emplace(monsterCategory::blob,  std::vector<const slot * >({
	  saddle.get() // think we should be able to saddle most things
	    }));//blob
  }
};

static slotRepoType slotRepo;

const std::vector<const slot*>& slotsFor(monsterCategory cat) {
  return slotRepo.byCat_.at(cat);
}

const slot* slotBy(slotType slot) {
  switch (slot) {
    case slotType::primary_weapon: 	return slotRepo.primary_weapon.get(); // Nethack is right-handed, so we are left :)
    case slotType::secondary_weapon: 	return slotRepo.secondary_weapon .get();
    case slotType::headband: 		return slotRepo.headband.get();
    case slotType::hat: 		return slotRepo.hat.get();
    case slotType::glasses: 		return slotRepo.glasses.get();
    case slotType::amulet: 		return slotRepo.amulet.get();//Necklace/amulet/talisman/medallian
    case slotType::headband_2: 		return slotRepo.headband_2.get();
    case slotType::hat_2: 		return slotRepo.hat_2.get();
    case slotType::glasses_2: 		return slotRepo.glasses_2.get();
    case slotType::amulet_2:		return slotRepo.amulet_2.get();
    case slotType::headband_3: 		return slotRepo.headband_3.get();
    case slotType::hat_3: 		return slotRepo.hat_3.get();
    case slotType::glasses_3: 		return slotRepo.glasses_3.get();
    case slotType::amulet_3: 		return slotRepo.amulet_3.get();
    case slotType::headband_4: 		return slotRepo.headband_4.get();
    case slotType::hat_4: 		return slotRepo.hat_4.get();
    case slotType::glasses_4: 		return slotRepo.glasses_4.get();
    case slotType::amulet_4: 		return slotRepo.amulet_4.get();
    case slotType::headband_5: 		return slotRepo.headband_5.get();
    case slotType::hat_5: 		return slotRepo.hat_5.get();
    case slotType::glasses_5: 		return slotRepo.glasses_5.get();
    case slotType::amulet_5: 		return slotRepo.amulet_5.get();

    case slotType::ring_left_thumb: 	return slotRepo.ring_left_thumb.get();
    case slotType::ring_left_index: 	return slotRepo.ring_left_index.get();
    case slotType::ring_left_middle: 	return slotRepo.ring_left_middle .get();
    case slotType::ring_left_ring: 	return slotRepo.ring_left_ring.get();
    case slotType::ring_left_little: 	return slotRepo.ring_left_little .get();
    case slotType::bracelet_left: 	return slotRepo.bracelet_left.get();
    case slotType::gloves: 		return slotRepo.gloves.get();
    case slotType::gauntlets: 		return slotRepo.gauntlets.get();
    case slotType::ring_right_thumb: 	return slotRepo.ring_right_thumb .get();
    case slotType::ring_right_index: 	return slotRepo.ring_right_index .get();
    case slotType::ring_right_middle: 	return slotRepo.ring_right_middle .get();
    case slotType::ring_right_ring: 	return slotRepo.ring_right_ring.get();
    case slotType::ring_right_little: 	return slotRepo.ring_right_little .get();
    case slotType::bracelet_right: 	return slotRepo.bracelet_right.get();
							                
    case slotType::toe_left_thumb: 	return slotRepo.toe_left_thumb.get();
    case slotType::toe_left_index: 	return slotRepo.toe_left_index.get();
    case slotType::toe_left_middle: 	return slotRepo.toe_left_middle.get();
    case slotType::toe_left_fourth: 	return slotRepo.toe_left_fourth.get();
    case slotType::toe_left_little: 	return slotRepo.toe_left_little.get();
    case slotType::anklet_left: 	return slotRepo.anklet_left.get();
							                
    case slotType::toe_right_thumb: 	return slotRepo.toe_right_thumb.get();
    case slotType::toe_right_index: 	return slotRepo.toe_right_index.get();
    case slotType::toe_right_middle: 	return slotRepo.toe_right_middle .get();
    case slotType::toe_right_fourth: 	return slotRepo.toe_right_fourth .get();
    case slotType::toe_right_little: 	return slotRepo.toe_right_little .get();
    case slotType::ring_nose:	 	return slotRepo.ring_nose.get();
    case slotType::ring_nose_2:	 	return slotRepo.ring_nose_2.get();
    case slotType::ring_nose_3:	 	return slotRepo.ring_nose_3.get();
    case slotType::ring_nose_4:	 	return slotRepo.ring_nose_4.get();
    case slotType::ring_nose_5:	 	return slotRepo.ring_nose_5.get();
    case slotType::anklet_right: 	return slotRepo.anklet_right.get();
    case slotType::socks: 		return slotRepo.socks.get();
    case slotType::shoes: 		return slotRepo.shoes.get(); //boots/shoes
							                
    case slotType::underwear: 		return slotRepo.underwear.get();
    case slotType::shorts: 		return slotRepo.shorts.get(); ///trousers/skirt/crupper (crupper is horse rump armour)
    case slotType::belt: 		return slotRepo.belt.get(); //scabbord/belt
    case slotType::shirt: 		return slotRepo.shirt.get();
    case slotType::doublet: 		return slotRepo.doublet.get();
    case slotType::hauburk: 		return slotRepo.hauburk.get(); //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
    case slotType::saddle: 		return slotRepo.saddle.get();
    case slotType::jerkin: 		return slotRepo.jerkin.get(); // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
    case slotType::robe: 		return slotRepo.robe.get(); // robe/cloak/peytral (horse chest armour)
    case slotType::tail: 		return slotRepo.tail.get(); // tail ring/bow
  default: 
    throw slot;
  }
}
