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
  // are enchantment charges used up when using the item magically?
  const bool burnsCharges() const;
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
  //flail, 

  ////
  // Missiles ('¬', unused in nethack. Looks to be flying away...)
  ////
    rock,

  ////
  // Clothing types ('[' as nethack armour until we can improve on it)
  // Useful ref: http://medieval.stormthecastle.com/all-about-medieval-armor.htm
  ////
    helmet,

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

  ////
  // Containers ('=' as bounding something; nethack uses '(' as for all tools; cf weapons are ')')
  ////
    poke, // old word for large pocket, especially not sewn in. Also, a bag.
  //prism_box, // any enchantemnt/flag changes on the box propagate to the contents; splitting the waveform of magic

  ////
  // Liquids ('~', the tail of a long worm in Nethack; don't know when we'd render this)
  ////
    water,

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
