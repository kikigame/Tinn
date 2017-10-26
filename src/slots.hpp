/* License and copyright go here*/

// utility class to hord a coordinate pair

#ifndef SLOTS_HPP__
#define SLOTS_HPP__

#include <memory>
#include <vector>
#include <set>

enum class monsterCategory;

// For now, Centaurs don't get forelegs; they get arms instead.

enum class slotType {
  primary_weapon, // Nethack is right-handed, so we are left :)
    secondary_weapon,
    
    headband,
    hat, //Helmet/hat/headdress/shaffron (shaffron is a horse helmet)
    glasses,
    amulet,//Necklace/amulet/talisman/medallian
    ring_left_thumb,
    ring_left_index,
    ring_left_middle,
    ring_left_ring,
    ring_left_little,
    bracelet_left,
    gloves,
    gauntlets,
    ring_right_thumb,
    ring_right_index,
    ring_right_middle,
    ring_right_ring,
    ring_right_little,
    bracelet_right,

    toe_left_thumb,
    toe_left_index,
    toe_left_middle,
    toe_left_fourth,
    toe_left_little,
    anklet_left,

    toe_right_thumb,
    toe_right_index,
    toe_right_middle,
    toe_right_fourth,
    toe_right_little,
    anklet_right,
    socks,
    boots, //boots/shoes

    underwear,
    shorts, ///trousers/skirt/crupper (crupper is horse rump armour)
    belt, //scabbord/belt
    shirt,
    doublet,
    hauburk, //  hauburk (chain portion of plate armour)/hauburgeon (sleeveless hauburk)
    saddle,
    jerkin, // jerkin/flanchard (flanchard is horse armour between saddle and crupper)
    robe, // robe/cloak/peytral (horse chest armour)
    tail, // tail ring/bow
};

class slot {
public:
  virtual const wchar_t * const name(monsterCategory cat) const = 0;
  // returns all slots that can immediately cover this, even if not applicable to a given monster category.
  // caller needs to recurse. There are no loops.
  virtual const std::vector<const slot*> covers() const = 0;
  // returns all slots this immediately covers, even if not applicable to a given monster category.
  // caller needs to recurse. There are no loops.
  virtual const std::vector<const slot*> covered() const = 0;
  //  virtual const std::shared_ptr<slot> pairedWith() const = 0; // optional
protected:
  slot();
private:
  slot(const slot&) = delete;
  slot& operator =(const slot&) = delete;
};

const slot* slotBy(slotType slot);

const std::vector<const slot*>& slotsFor(monsterCategory cat);

const std::set<const slot*> weaponSlots();

#endif // ndef SLOTS
