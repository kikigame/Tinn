/* License and copyright go here*/

// genus and species of monsters in Tinn

#ifndef MONSTERTYPE_HPP
#define MONSTERTYPE_HPP

#include "materialType.hpp"
#include "movement.hpp"
#include <vector>
#include <string>


// broad categories of monster
// each category has the same wearable slots
enum class monsterCategory {
  // bipeds have two arms, lwo legs, torso and head.
  biped,
  // quadrupeds have four legs, flank, rump, head and tail.
    quadruped,
  // hooved quadrupeds are same as quadrupeds except the feet are called hooves
    hooved_quadruped,
  // centaurs have two arms, four legs, flank, rump, head and tail.
  // NB: We don't model the forelegs.
    centaur,
  // dragons are flightless quadrupeds with wings. flank is callad thorax and rump is called abdomen. Toes are called claws. NB: feet are called paws according to the OED (usage rare usage).
  // NB: We don't model the wings, as they are vestigial and cannot hold thigns.
  // TODO: Is this system breaking down? If you polymorph from a dragon into a bird, your foreleg-equipped weapons will jump to your wings. On the one hand, this makes sense (dragons wouldn't hold things in their wings while birds need to), but on the other hand, how does a cursed item move?
    dragon,
  // birds have wings, head and torso. TODO: claws should become talons perhaps?
    bird,
  // blobs have nothing. They are just blobs.
    blob
};

enum class genderAssignType {
  /* Monster will have no gender */
  neuter,
  /* Always male */
    m,
  /* Always female (nymphs, mermaids) */
    f, 
  /* 100%M or 100%F, equal chance */
    mf, 
  /* neuter, 100%M or 100%F, equal chance */
    mfn,
  /* true hermaphrodite */
    herm,
  /* M+F=100% */
    direct, 
  /* M=d%, F=d% independently */
    indirect
};

enum class monsterTypeKey {
  dragon,
  human, 
  troll, 
  zombie, 
  generalMonster // TODO: This is mostly for debugging
};

class monster;
class monsterTypeBuilder;
class deity;
class monsterTypeRepoImpl;

// types of monster
class monsterType {
private:
  const monsterTypeKey key_;
  const monsterCategory category_;
  // TODO: food types
  // TODO: weapon damage types dealt bitset
  // TODO: weapon proofs / resistance
  // TODO: othir intrinsics
  // TODO: special attacks
  // TODO: special defences
  // TODO: special ad-hocs
  // TODO: starting inventory
  const std::vector<const wchar_t *> monsterNames_; // by experience
  const std::wstring encyclopedium_;
  const std::wstring className_; // for grouping
  const int levelFactor_;
  const int levelOffset_;
  const int minSpawn_;
  const int maxSpawn_;
  const int xpFactor_;
  const int xpOffset_;
  const char renderChar_;

  // max/starting stats for a level 1 monster:
  // (0-5 percent for flavour is added at random)
  const int strength_;
  const int appearance_;
  const int fighting_;
  const int dodge_;
  const int maxDamage_; // hit points

  const genderAssignType gen_;

  // most monsters are fleshy but not all...
  const materialType material_;

  // 1 of more things that monsters of this kind can say.
  // The game will prefix this with "If I could speak, I would say..."
  // in the case where the monster is incapable of speech; thus, all
  // monsters will have at least one saying, even if speechless.
  // Generally starts with a capital letter and includes punctuation.
  // Quotes should be in (") double quotes, and no double-quoting.
  // (that way, we can replace double quotes with singles to quote
  // the whole thing, without worrying about apostrophies).
  // accessed via class monster, to let monsters override sayings.
  const std::vector<const wchar_t *> sayings_;

  // to which deities does this monster align?
  // for some classes, that will be all along one axis, for others individual paths.
  // each individual monster gets one of these options at random.
  const std::vector<deity *> alignment_;

  // how does this monster move?
  const movementType movementType_;
public:
  monsterType(const monsterTypeBuilder &);
  monsterType(const monsterType &) 
    : key_(monsterTypeKey::human), category_(monsterCategory::biped),
      levelFactor_(0), levelOffset_(0), minSpawn_(0), maxSpawn_(0), xpFactor_(0),
      xpOffset_(0), renderChar_('\0'), strength_(0), appearance_(0), fighting_(0),
      dodge_(0), maxDamage_(0), gen_(genderAssignType::f), material_(materialType::fleshy),
      movementType_(stationary) { 
    throw "needed for containers but shouldn't copy monster types!";};

  const monsterTypeKey type() const;
  const materialType material() const;
  const monsterCategory category() const;
  const wchar_t * const encyclopedium() const;

  // "i" is for "initial":
  const int iStrength() const;
  const int iAppearance() const;
  const int iFighting() const;
  const int iDodge() const;
  const int iMaxDamage() const;

  const wchar_t renderChar() const;
  const std::vector<const wchar_t *> & names() const;
  const std::vector<deity *> & alignment() const;
  const movementType & movement() const;

  const int getLevelFactor()const ;
  const int getLevelOffset()const ;
  const int getMinSpawn()const ;
  const int getMaxSpawn()const ;
  const int getXpFactor()const ;
  const int getXpOffset()const ;

  const std::vector<const wchar_t *>::const_iterator sayingsBegin() const;
  const std::vector<const wchar_t *>::const_iterator sayingsEnd() const;

  bool operator == (const monsterType & rhs) const;
};

/* singleton repository for monster types */
class monsterTypeRepo {
private:
  std::unique_ptr<monsterTypeRepoImpl> pImpl_;
  static std::unique_ptr<monsterTypeRepo> instance_;
  monsterTypeRepo();
public:
  const monsterType& operator[](const monsterTypeKey &) const;
  std::vector<monsterType*>::iterator begin();
  std::vector<monsterType*>::iterator end();
  static monsterTypeRepo& instance();
  static void close();
};

#endif // ndef MONSTERTYPE_HPP
