/* License and copyright go here*/

// genus and species of monsters in Tinn

#ifndef MONSTERTYPE_HPP
#define MONSTERTYPE_HPP

#include "monsterIntrinsics.hpp"
#include "materialType.hpp"
#include <memory>
#include <vector>
#include <string>
#include <bitset>


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
  // birds have wings, head and torso. "digits" are claws. (There's some debate online about whether all birds have talons, or whether claws are something separate. But I think colloquially we would talk about a song-bird's claws or a falcon's talons).
    bird,
  // as bird, but "digits" are talons.
    raptor,
  // merfolk: as biped, but tail instead of legs
    merfolk,
  // snake: basically just a tail.
    snake,
  // blobs have nothing. They are just blobs.
  blob,
  END
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

/*TODO: (chars in Nethack not in TINN:)
 * (e) eyes/spheres
 * (g) gremlins/gargoyles
 * (h) non-human humanoids (we use for kelpie in horse form)
 * (i) minor demons
 * (j) jellies
 * (k) kobolds
 * (l) leprichorns
 * (m) mimics
 * (n) nymphs
 * (p) piercers
 * (q) quadrupeds
 * (r) rodents
 * (t) trappers
 * (u) horses/unicorns (we use h for kelpie in horse form)
 * (v) vorticies
 * (w) worms
 * (x) fantastic insects
 * (y) light
 * (z) zruties
 * (C) centaur
 * (E) elemental/stalker
 * (F) fungii / mould
 * (G) gnomes
 * (H) large humanoids (giant/ettin/titan/minotaur)
 * (I) (unused)
 * (J) Jabberwock
 * (K) Kops
 * (L) Lich
 * (M) Mummies
 * (N) Naga
 * (O) Ogre
 * (Q) Quantum mechanics
 * (R) rust monster/disenchanter
 * (S) snake
 * (U) umber hulk
 * (V) Vampyre
 * (W) Wraith
 * (X) Xorn
 * (Y) Apelikes
 * (Z) Zowbies
 * (') Golums
 * ( ) Ghost
 * (;) Sea monster
 * (:) lizards
 */

enum class monsterTypeKey {
  //bird,
  alien, // 'A'; Nethack uses for Angelic beings
  blob,
  bull,
  raptor,
  dragon,
  dungeoneer, // low-level human, appears on levels 1-3. L3 has cloak. Always 2 random items (limited inventory), plus napsack of consumption & visorless helmet. Ref: Knightmare.
  ferret,
  fox,
  goblin,
  hound,
  human,
  hippalectryon, // greek horse/cock (Nethack already has cockatrice)
  incubus, // male form of succubus
  kelpie, // water spirit. Can shape-shift between horse and human forms (possibly with hooves)
  mokumokuren,
  merfolk,// merman or mermaid; can't move off water. Attacks with charm
  siren, // manevolent half-bird that sits on rocks at sea and lures travellers to their death with its song.
  snake,
  succubus, // female form of incubus
  swarm_butterfly,
  swarm_bees,
  swarm_wasps, // / wasps/ hornets
  swarm_locusts,
  troll, 
  venusTrap,
  zombie,
};

class monster;
class monsterBuilder;
class monsterTypeBuilder;
class deity;
class monsterTypeRepoImpl;
class level;

class monsterTypeName {
private:
  const std::vector<const wchar_t *> adjectives_;
  const wchar_t * name_;
public:
  monsterTypeName(const wchar_t * name);
  monsterTypeName(std::initializer_list<const wchar_t *>, const wchar_t *);
public:
  const wchar_t * name() const;
  std::vector<const wchar_t *>::const_iterator begin() const;
  std::vector<const wchar_t *>::const_iterator end() const;
};

// types of monster
class monsterType {
private:
  const monsterTypeKey key_;
  const monsterCategory category_;
  // TODO: weapon damage types dealt bitset
  // TODO: weapon proofs / resistance
  const std::vector<monsterTypeName> monsterNames_; // by experience
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

  // how much does a corpse of this type weigh?
  const double corpseWeight_;

  // most monsters are fleshy but not all...
  const materialType material_;

  // what materials can this monster eat?
  const std::bitset<materialTypeSize> foodMaterials_;

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

  // monster default flags
  const std::shared_ptr<monsterIntrinsics> intrinsics_;

  enum class flag {
    ALLURING = 0,
    UNDEAD = 1
  };
  const std::bitset<2> flags_;
public:
  monsterType(const monsterTypeBuilder &);
  monsterType(const monsterType &)  = delete;
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
  const std::vector<monsterTypeName> & names() const;
  const monsterTypeName &name(unsigned char maxDamage) const; // look up based on how advanced this creature is
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

  const double corpseWeight() const;
  const bool eats(const materialType foodType) const;

  const genderAssignType gen() const;

  bool operator == (const monsterType & rhs) const;

  // actually defined in monsterFactory.cpp:
  std::unique_ptr<monsterBuilder> builder(bool allowRandom = true) const;
  // spawn the usual monster type:
  std::shared_ptr<monster> spawn(level &, monsterBuilder &b) const;
  std::shared_ptr<monster> spawn(level &) const;

  const std::shared_ptr<monsterIntrinsics> intrinsics() const;

  /////////
  // Non-modifiable traits
  /////////
  
  // is this monster type supernaturally alluring?
  bool alluring() const;
  // undead creatures notably can't become vampires
  bool undead() const;
};

// interface for things that provide a monster type
class monsterTypeProvider {
public:
  virtual const monsterType &getMonsterType() const = 0;
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

/* pick a "solid" monster (not liquid, blob or a zombie) */
const monsterType &rndSolidMonster();

#endif // ndef MONSTERTYPE_HPP
