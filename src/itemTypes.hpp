/* License and copyright go here*/

// Things and stuff

#ifndef ITEM_TYPE_HPP__
#define ITEM_TYPE_HPP__

#include "materialType.hpp"
#include "renderable.hpp"
#include <memory>
#include <map>

class itemTypeImpl;

class itemType : public renderable {
private:
  std::unique_ptr<itemTypeImpl> pImpl_;
public:
  itemType(itemTypeImpl*);
  const wchar_t render() const;
  const wchar_t * const name() const;
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
  corpse, // "a dead <foo>"

  ////
  // Weapon types ('!', unlike nethack, because they're inherantly dangerous)
  ////
  // club,
    mace, // a basic bashing weapon, but also the stuff of myths.
    two_sword, // 2-handed sword of cutting type
  //flail, 

  ////
  // Missiles ('¬', unused in nethack. Looks to be flying away...)
  ////
    rock,

  ////
  // Clothing types ('[' as nethack armour until we can improve on it)
  // Useful ref: http://medieval.stormthecastle.com/all-about-medieval-armor.htm
  ////
    bow, // worn by horses on their tail
    boots,
    cloak,
    crupper, // (horse rump armour)
    doublet,
    flanchard, // (horse mid-back armour)
    haubergeon,
    hauberk,
    helmet,
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
    

  ////
  // Stylii ('/' for wand in nethack, cf \ for throne)
  ////
    stick,

  ////
  // Bottles ('8' looks bottle-like to me. Unused in nethack.
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
    napsack_of_consumption, // ref:Knightmare
  //prism_box, // any enchantemnt/flag changes on the box propagate to the contents; splitting the waveform of magic

  ////
  // Liquids ('~', the tail of a long worm in Nethack; don't know when we'd render this)
  ////
    water, // plain water
    tears, // (water + tear damage) // potion of sorrow (TODO)
    heavy_water, // (water + bashing/earth damage)
    fire_water, // (water + fire)
    pop, // (water + water)
    fizzy_pop, // (water + air)
    dehydrated_water, // (water + time)
    spring_water, // (water + flora damage)
    electro_pop, // (water + lightning damage) - TODO: synthesis

  ////
  // Jewellery ('*' ; nethack uses * for gems, " for amulets/webs and = for rings
  ////
    wooden_ring,

  //// 
  // Valuables ('$'; nethack uses this for coins (denominated in zorkmids) and '$' for gems.
  //// // NB: We can have *different types* of coins, with different currency descriptions and value by weight...
    kalganid, // from Asimov's Foundation series

  ////
  // Tools ('(' as Nethack until we can think of something better
  ////
    shop_card, // activate to go shopping.
    bottling_kit, // for putting things into bottles
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
