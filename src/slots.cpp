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
  std::unique_ptr<slotImpl> primary_weapon = std::unique_ptr<slotImpl>(new slotImpl(L"wielded (left hand)", {{monsterCategory::quadruped, L"wielded (left forepaw)"}, {monsterCategory::hooved_quadruped, L"wielded (left forehoof)"}, {monsterCategory::dragon, L"wielded (left forepaw)"}, {monsterCategory::bird, L"wielded (left wing)"}, {monsterCategory::blob, L"outermost left blobby protrusion"}}));
  std::unique_ptr<slotImpl> secondary_weapon = std::unique_ptr<slotImpl>(new slotImpl(L"wielded (right hand)", {{monsterCategory::quadruped, L"wielded (right forepaw)"}, {monsterCategory::hooved_quadruped, L"wielded (right forehoof)"}, {monsterCategory::hooved_quadruped, L"wielded (right forepaw)"}, {monsterCategory::bird, L"wielded (right wing)"}, {monsterCategory::blob, L"outermost right blobby protrusion"}}));
  std::unique_ptr<slotImpl> headband = std::unique_ptr<slotImpl>(new slotImpl(L"headband", {{monsterCategory::blob, L"top blobby protrusion"}}));
  std::unique_ptr<slotImpl> hat = std::unique_ptr<slotImpl>(new slotImpl(L"hat", {{monsterCategory::blob, L"topmost blobby protrusion"}}));
  std::unique_ptr<slotImpl> glasses = std::unique_ptr<slotImpl>(new slotImpl(L"eyes", {{monsterCategory::blob, L"top front blobby protrusion"}}));
  std::unique_ptr<slotImpl> amulet = std::unique_ptr<slotImpl>(new slotImpl(L"neck",  {{monsterCategory::blob, L"upper blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_index = std::unique_ptr<slotImpl>(new slotImpl(L"left first claw",   {{monsterCategory::biped, L"left index finger"}, {monsterCategory::blob, L"first left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_middle = std::unique_ptr<slotImpl>(new slotImpl(L"left second claw", {{monsterCategory::biped, L"left middle finger"}, {monsterCategory::blob, L"second left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_ring = std::unique_ptr<slotImpl>(new slotImpl(L"left third claw",    {{monsterCategory::biped, L"left ring finger"}, {monsterCategory::blob, L"third left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_little = std::unique_ptr<slotImpl>(new slotImpl(L"left fourth claw", {{monsterCategory::biped, L"left little finger"}, {monsterCategory::blob, L"fourth left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_left_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"left fifth claw",   {{monsterCategory::biped, L"left thumb"}, {monsterCategory::blob, L"fifth left blobby protrusion"}}));
  std::unique_ptr<slotImpl> bracelet_left = std::unique_ptr<slotImpl>(new slotImpl(L"left foreleg", {{monsterCategory::biped, L"left wrist"}, {monsterCategory::blob, L"upper left blobby protrusion"}}));
  std::unique_ptr<slotImpl> gloves = std::unique_ptr<slotImpl>(new slotImpl(L"forepaws", {{monsterCategory::biped, L"hands"}, {monsterCategory::blob, L"outer left blobby protrusion"}}));
  // gauntlets read the same as gloves, but they are different slots; you can wear gauntlets over gloves:
  std::unique_ptr<slotImpl> gauntlets = std::unique_ptr<slotImpl>(new slotImpl(L"forepaws", {{monsterCategory::biped, L"hands"}, {monsterCategory::blob, L"outer left blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_index = std::unique_ptr<slotImpl>(new slotImpl(L"right first claw", {{monsterCategory::biped, L"right index finger"}, {monsterCategory::blob, L"first right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_middle = std::unique_ptr<slotImpl>(new slotImpl(L"right second claw", {{monsterCategory::biped, L"right middle finger"}, {monsterCategory::blob, L"second right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_ring = std::unique_ptr<slotImpl>(new slotImpl(L"right third claw", {{monsterCategory::biped, L"right ring finger"}, {monsterCategory::blob, L"third right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_little = std::unique_ptr<slotImpl>(new slotImpl(L"right fourth claw", {{monsterCategory::biped, L"right little finger"}, {monsterCategory::blob, L"fourth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> ring_right_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"right fifth claw", {{monsterCategory::biped, L"right thumb"}, {monsterCategory::blob, L"fifth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> bracelet_right = std::unique_ptr<slotImpl>(new slotImpl(L"right foreleg", {{monsterCategory::biped, L"right wrist"}, {monsterCategory::blob, L"upper right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_index = std::unique_ptr<slotImpl>(new slotImpl(L"left first claw", {{monsterCategory::biped, L"left index toe"}, {monsterCategory::blob, L"first left blobby protrusion"}})); // wikipedia allows "index toe", so we'll keep it the same as for fingers.
  std::unique_ptr<slotImpl> toe_left_middle = std::unique_ptr<slotImpl>(new slotImpl(L"left second claw", {{monsterCategory::biped, L"left middle toe"}, {monsterCategory::blob, L"second left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_fourth = std::unique_ptr<slotImpl>(new slotImpl(L"left third claw", {{monsterCategory::biped, L"left ring toe"}, {monsterCategory::blob, L"third left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_little = std::unique_ptr<slotImpl>(new slotImpl(L"left fourth claw", {{monsterCategory::biped, L"left little toe"}, {monsterCategory::blob, L"fourth left blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_left_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"left fifth claw", {{monsterCategory::biped, L"left big toe"}, {monsterCategory::blob, L"fifth left blobby protrusion"}})); // the latin name for the big toe is the "hallux"
  std::unique_ptr<slotImpl> anklet_left = std::unique_ptr<slotImpl>(new slotImpl(L"left hindleg", {{monsterCategory::biped, L"left ankle"}, {monsterCategory::bird, L"left leg"}, {monsterCategory::blob, L"lower left blobby protrusion"}}));

  std::unique_ptr<slotImpl> toe_right_index = std::unique_ptr<slotImpl>(new slotImpl(L"right first claw", {{monsterCategory::biped, L"right index toe"}, {monsterCategory::blob, L"first right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_middle = std::unique_ptr<slotImpl>(new slotImpl(L"right second claw", {{monsterCategory::biped, L"right middle toe"}, {monsterCategory::blob, L"second right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_fourth = std::unique_ptr<slotImpl>(new slotImpl(L"right third claw", {{monsterCategory::biped, L"right ring toe"}, {monsterCategory::blob, L"third right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_little = std::unique_ptr<slotImpl>(new slotImpl(L"right fourth claw", {{monsterCategory::biped, L"right little toe"}, {monsterCategory::blob, L"fourth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> toe_right_thumb = std::unique_ptr<slotImpl>(new slotImpl(L"right fifth claw", {{monsterCategory::biped, L"right big toe"}, {monsterCategory::blob, L"fifth right blobby protrusion"}}));
  std::unique_ptr<slotImpl> anklet_right = std::unique_ptr<slotImpl>(new slotImpl(L"right hindleg", {{monsterCategory::biped, L"right ankle"}, {monsterCategory::bird, L"right leg"}, {monsterCategory::blob, L"lower right blobby protrusion"}}));

  // like gloves and gauntlets, socks go under boots but are reported as the same slot:
  std::unique_ptr<slotImpl> socks = std::unique_ptr<slotImpl>(new slotImpl(L"hindpaws", {{monsterCategory::biped, L"feet"}, {monsterCategory::blob, L"lower blobby protrusions"}}));
  std::unique_ptr<slotImpl> boots = std::unique_ptr<slotImpl>(new slotImpl(L"hindpaws", {{monsterCategory::biped, L"feet"}, {monsterCategory::blob, L"lower blobby protrusions"}}));

  std::unique_ptr<slotImpl> underwear = std::unique_ptr<slotImpl>(new slotImpl(L"pelvis", {{monsterCategory::blob, L"lower middle blobby protrusion"}}));
  std::unique_ptr<slotImpl> shorts = std::unique_ptr<slotImpl>(new slotImpl(L"upper legs", {{monsterCategory::quadruped, L"rump"}, {monsterCategory::hooved_quadruped, L"rump"}, {monsterCategory::centaur, L"rump"}, {monsterCategory::dragon, L"abdomen"}, {monsterCategory::blob, L"top part of lower blobby protrusion"}})); // TODO: insects & lizards should have adbomen
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
    glasses->covered(*hat);
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
    socks->covered(*boots)
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
    boots->covers(*socks);

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
	  anklet_right.get(),
	  socks.get(),
	  boots.get(), //boots/shoes
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
	  anklet_right.get(),
	  socks.get(),
	  boots.get(), //boots/shoes
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
	  anklet_right.get(),
	  socks.get(),
	  boots.get(), //boots/shoes
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
	  anklet_right.get(),
	  socks.get(),
	  boots.get(), //boots/shoes
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
	  boots.get(), //boots/shoes
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
    case slotType::hat: 		return slotRepo.hat.get(); //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
    case slotType::glasses: 		return slotRepo.glasses.get();
    case slotType::amulet: 		return slotRepo.amulet.get();//Necklace/amulet/talisman/medallian
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
    case slotType::anklet_right: 	return slotRepo.anklet_right.get();
    case slotType::socks: 		return slotRepo.socks.get();
    case slotType::boots: 		return slotRepo.boots.get(); //boots/shoes
							                
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

const std::set<const slot*> weaponSlots() {
  std::set<const slot*> weaponSlots = { slotRepo.primary_weapon.get(), slotRepo.secondary_weapon.get() };
  return weaponSlots;
}
