/* License and copyright go here*/

// types of damage

#ifndef DAMAGE_HPP
#define DAMAGE_HPP

#include "materialType.hpp"
#include <map>
#include <memory>
#include <vector>

enum class damageType {
  /* tearing (cutting skin/armour) */
  edged, 
  /* & earth /falling */
    bashing,
  /* & file (burning skin/armour)*/
    hot, 
    cold, 
  /* water (rotting) */
    wet,
  /* air (damages hearing) */
    sonic, 
  /* time */
    disintegration, 
  /* flora */
    starvation,
  /* lightning */
    electric
};

class damageRepo;

class damage {
  friend class damageRepo;
private:
  // key for mapping
  const damageType type_;
  // name of damage type (adjective eg "burning" for hot)
  const wchar_t* const name_;
  // name of mending type (negative damage)
  const wchar_t* const mendName_;
  // adjective for something being damageproof (or null where not-applicable)
  const std::map<const materialType,const wchar_t* const>  proof_;
  // adjective for something being damaged (or null where not-applicable)
  // may be prefixed by "slightly ", "very ", "thoroughly " as applicable
  const std::map<const materialType,const wchar_t* const>  damaged_;
  damage(const damageType type, const wchar_t* const name, const wchar_t* const mendName,
	 const std::map<const materialType,const wchar_t* const> &proof,
	 const std::map<const materialType,const wchar_t* const> &damaged);
public:
  // can we damage the given material?
  bool canDamage(const materialType type) const;
  damageType type() const;
  // name() + L" damage" or L"%s couldn't handle the " + name() 
  const wchar_t * const name() const;
  const wchar_t * const mendName() const;
  const wchar_t * const proofAdj(const materialType type) const;
  const wchar_t * const damageAdj(const materialType type) const;
};

class damageRepo {
private:
  std::map<damageType, damage> byType_;
  static std::unique_ptr<damageRepo> instance_;
  damageRepo();
  void emplace(const std::vector<damage> &d);
public:
  // access a damage type
  const damage& operator[](const damageType t) const;

  // access singleton
  static damageRepo& instance();

  std::map<damageType, damage>::const_iterator begin();
  std::map<damageType, damage>::const_iterator end();

  static void close();
};


#endif // ndef DAMAGE_HPP
