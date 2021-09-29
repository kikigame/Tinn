/* License and copyright go here*/

// Things and stuff

#ifndef ITEM_TYPE_HPP__
#define ITEM_TYPE_HPP__

#include "materialType.hpp"
#include "renderable.hpp"
#include <memory>
#include <map>

class itemTypeImpl;

class itemType {
private:
  std::unique_ptr<itemTypeImpl> pImpl_;
public:
  itemType(itemTypeImpl*);
  const wchar_t render() const;
  std::wstring name() const;
  const wchar_t * const description() const;
  // used if unidentified:
  const wchar_t * const vagueDescription() const;
  const double baseWeight() const;
  const materialType material() const;
  bool operator ==(const itemType &other) const;
};



enum class itemTypeKey {
  ////
  // Food types ('%' as per Nethack)
  ////
  apple, // weight exactly 1 Neuton in normal gravity
  bread, // veggy; staple (loaf)
  cake, // veggy; staple, as bread
  corpse, // "a dead <foo>"
  pie, // veggy; basicaly a scaled-up apple
  joint, // as corpse, but a fraction of the size. Small monsters excluded.
  steak, // as joint, but yet smaller fraction.

  ////
  // Weapon types ('!', unlike nethack, because they're inherantly dangerous)
  ////
  // club,
    mace, // a basic bashing weapon, but also the stuff of myths.
    two_sword, // 2-handed sword of cutting type
    flamethrower, // 2-handed heat weapon, runs on charges
    nitrogen_tank, // 2-handed cold weapon, runs on charges
    bubble_gun, //does minor wet damage
    sonic_cannon, //sound damage. Ref: Dr Who (Ice Warriors' weapon)
    maser, // amplifies ambient radiation into a weapon; disintegration/time (bit of artistic license here)
    //starvation/flora - I can't think of any just now
    taser, //electric damage
  // fulgurator - Jules' Verne missile
  // death_ray, // Tesla's unsuccessful invention; electric missile
  //flail, 

  ////
  // Missiles ('¬', unused in nethack. Looks to be flying away...)
  ////
    rock, // 5% bashing; double if sling equipped
    throwstick, // 20% bashing damage; a non-returning boomerang (also a 5% hand-weapon).
    dart, // 5% edged damage
    bolt, // 5% edged damage; 20% if crossbow equipped
    percussion_grenade, // 50% sound damage; single-shot
    slipstone, // as rock, but if charged used to add "sharp" adj
    lodestone, // as rock, but if charged used to add "metallic" adj
  /*
    heat_seaking, // 20% hot damage; seeks hot target
    cold_seaking, // 20% cold damage; seeks cold target
    water_seaking, // 20% wet damage; seeks watery target
  */

  ////
  // Missile launchers (also '¬'; count as missiles in shops)
  ////
  sling, // fires rocks
  crossbow, // fires bolts
  
  ////
  // Clothing types ('[' as nethack armour until we can improve on it)
  // Useful ref: http://medieval.stormthecastle.com/all-about-medieval-armor.htm
  ////
    bow, // worn by horses on their tail
    boots,
    buckler, // secondary weapon that also provider armour
    cloak,
    crupper, // (horse rump armour)
    doublet,
    flanchard, // (horse mid-back armour)
    haubergeon,
    hauberk,
    helmet,
    gloves, // leathery; worn under gauntlets
    boxing_gloves, // leathery, worn in the gauntlet slot as you can put gloves under but not gauntlets over
    gauntlets, //Metallic; Worn over gloves
    jerkin,
    peytral, // (horse chest armour)
    robe,
    saddle, // horses aren't currently rideable, but this is important as armour
    scabbord, // currently behaves the same as a belt
    shirt,
    shorts,
    skirt,
    socks,
    trousers,
    tshirt, // sexy if wet or torn
    underpants,
  //TODO:    goggles,

  ////
  // Stylii ('/' for wand in nethack, cf \ for throne)
  ////
    stick,

  ////
  // Bottles ('8' looks bottle-like to me. Unused in nethack, except for warnings.
  ////
    bottle,

  ////
  // Readables ('¶'; nethack uses '?' for scrolls and '+' for spellbooks (and doors))
  ////
    codex,
    hitch_guide, // access to the in-game encyclopedia of itself
    holy_book, // for a given religion
    iou, // given by a shop when you can't pay for an item. Undroppable

  ////
  // Containers ('=' as bounding something; nethack uses '(' as for all tools; cf weapons are ')')
  ////
    poke, // old word for large pocket, especially not sewn in. Also, a bag.
    napsack_of_consumption, // ref:Knightmare (TV show)
    chaos_container, // does things to stuff put in it
    //prism_box, // any enchantemnt/flag changes on the box propagate to the contents; splitting the waveform of magic
    oil_can, // if charged, adds "slippery" adjective
    salt_cellar, // if charged, adds "salted" or "cured" adjective
    pepper_cellar,  // if charged, adds "peppered" adjective

  ////
  // Liquids ('~', the tail of a long worm in Nethack; don't know when we'd render this)
  ////
    water, // plain water
    tears, // (water + tear damage) // potion of sorrow
    heavy_water, // (water + bashing/earth damage)
    fire_water, // (water + fire)
    pop, // (water + water)
    fizzy_pop, // (water + air)
    dehydrated_water, // (water + time)
    spring_water, // (water + flora damage)
    electro_pop, // (water + lightning damage)
    ectoplasm, // from ghosts
    kelpie_juice, // ⁓ (similar to tilde to avoid autogen); dead kelpie slush.
    demon_essence,  // ⁓ (similar to tilde to avoid autogen); dead demon.

  ////
  // Jewellery ('*' ; nethack uses * for gems, " for amulets/webs and = for rings
  ////
    wooden_ring,
    amulet, // stony; worn on neck. Called a talisman when has an effect.
    necklace, // metallic, worn on neck
    tikka, // pendant worn on short chain down the hair parting (india); headband slot
    spectacles, // glassy, worn on eyes slot
    bracelet, // metallic arm jewellery
    anklet, // metallic leg jewellery
    cloth_gloves, // clothy form of gloves
    armband, // black band worn for mourning; singular

  //// 
  // Valuables ('$'; nethack uses this for coins (denominated in zorkmids) and '*' for gems.
  //// // NB: We can have *different types* of coins, with different currency descriptions and value by weight...
    kalganid, // from Asimov's Foundation series
    gpl_slip, // Gold Pressed Latinum; ref: Star Trek: TNG & later. 
    gpl_strip, // = 100 slips
    gpl_bar, // = 20 strips
    gpl_brick, // = 50 bars

  ////
  // Tools ('(' as Nethack until we can think of something better
  ////
    shop_card, // activate to go shopping.
    bottling_kit, // for putting things into bottles
    portable_hole, // for moving through walls/floors or carrying things
    fruit_machine, // for gambling. Summons monsters. Dangerous.
    repair_kit, // if charged, adds "repaired" adjective, and removes some damage
    rag, // adds "polished" adjective
    tuning_fork, // add "tuned" adjective (usable only if you can hear)

  ////
  // Instruments ('♪'; Nethack uses '(' for all tools)
  ////

    theremin,  // attack_ray_med_electric
    visi_sonor, // charm
    baliset, // enchant_item
    drum, // nudity
    bagpipes, // petrify
    conch, // disarm
    harmonica, // tremolo-tuned; attack_ray_med_sonic
    pan_flute, // attraction
    lyre, // teleport_away
    pianoforte, // attack_ray_med_bashing


  ////
  // Vehicle (generally unique symbols)
  ////

    bridge, // immobile tranport, allows ground creatures on water
    ship, // mobile transport, allows ground creatures on water. L'⍽' (shouldered open box), which I think looks better than the more usual L'␣' (&blank;/open box)

  ////
  // Flowers & plants '{' as per dnethack (plant monster)
  ////
  bouquet_mixed,
  lily,
  lotus,
  sunflower,

};


/*
 * These are special items that can only be created by the createQuestItem() method.
 * They do not have types in the itemtype repository.
 */
enum class questItemType {
  grail, // shop quest item
  diamond, // thief quest item
  icon, // crusader quest item
};


class itemTypeRepo {
private:
  std::map<itemTypeKey, std::unique_ptr<itemType> > byType_;
  static std::unique_ptr<itemTypeRepo> instance_;
  itemTypeRepo();
public:
  const itemType &operator[](itemTypeKey k) const;
  static itemTypeRepo &instance();
  std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator begin();
  std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator end();
  static void close();
private:
  void emplace(itemTypeKey k, itemTypeImpl* impl);
};

#endif // ndef ITEM_TYPE_HPP__
