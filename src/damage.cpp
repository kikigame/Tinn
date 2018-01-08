/* License and copyright go here*/

// damage types

#include "damage.hpp"

damage::damage(const damageType type, const wchar_t* const name, const wchar_t* const mendName,
	       const std::map<const materialType,const wchar_t* const> &proof,
	       const std::map<const materialType,const wchar_t* const> &damaged) :
  type_(type), name_(name), mendName_(mendName), proof_(proof), damaged_(damaged) {}

  // can we damage the given material?
bool damage::canDamage(const materialType type) const {
  return proof_.find(type) != proof_.end();
}

damageType damage::type() const { return type_; }
const wchar_t * const damage::name() const { return name_; }
const wchar_t * const damage::mendName() const { return mendName_; }
const wchar_t * const damage::proofAdj(const materialType type) const { 
  auto rtn = proof_.find(type);
  return rtn == proof_.end() ? nullptr : proof_.at(type); 
}
const wchar_t * const damage::damageAdj(const materialType type) const {
  auto rtn = damaged_.find(type);
  return rtn == damaged_.end() ? nullptr : damaged_.at(type);
}

std::unique_ptr<damageRepo> damageRepo::instance_;

damageRepo & damageRepo::instance() { 
  if (!instance_) {
    instance_.reset(new damageRepo());
  }
  return *instance_; 
}

std::map<damageType, damage>::const_iterator damageRepo::begin() {
  return byType_.begin();
}
std::map<damageType, damage>::const_iterator damageRepo::end() {
  return byType_.end();
}


void damageRepo::close() {
  instance_.reset(NULL);
}

void damageRepo::emplace(const std::vector<damage> &damages) {
  for (auto &d : damages)
    byType_.emplace(d.type(), d);
}

const damage& damageRepo::operator[](const damageType t) const {
  return byType_.at(t);
}

damageRepo::damageRepo() {
  emplace({{
      damageType::edged, L"cutting", L"sewn", 
	{
	  { materialType::clothy, L"sturdy" },
	  { materialType::glassy, L"fracture-resistant" },
	  { materialType::leathery, L"sturdy" },
	  { materialType::papery, L"sturdy" },
	  { materialType::woody, L"sturdy" },
	},{
	  { materialType::clothy, L"torn" },
	  { materialType::glassy, L"crazed" },
	  { materialType::leathery, L"torn" },
	  { materialType::papery, L"torn" },
	  { materialType::woody, L"splintered" }
	}
    },{
      damageType::bashing, L"bashing", L"spa",
	{
	  { materialType::fleshy, L"tender" },
	  { materialType::veggy, L"tender" },
	  { materialType::glassy, L"fracture-resistant" },
	  { materialType::metallic, L"tempered" },
	  { materialType::stony, L"shatterproof" },
	  { materialType::waxy, L"hard-wearing" },
	  { materialType::woody, L"sturdy" }
	},{
	  { materialType::fleshy, L"tenderised" },
	  { materialType::veggy, L"tenderised" },
	  { materialType::glassy, L"crazed" },
	  { materialType::metallic, L"hammered" },
	  { materialType::stony, L"crazed" },
	  { materialType::waxy, L"misshapen" },
	  { materialType::woody, L"splintered" }
	}
    },{
      damageType::hot, L"hot", L"cool",
	{
	  { materialType::clothy, L"flameproof" },
	  { materialType::fleshy, L"tender" },
	  { materialType::veggy, L"tender" },
	  { materialType::glassy, L"fracure-resistant" },
	  { materialType::leathery, L"flameproof" },
	  { materialType::liquid, L"anti-boiled" },
	  { materialType::papery, L"flameproof" },
	  { materialType::waxy, L"flameproof" },
	  { materialType::woody, L"flameproof" }
	},{
	  { materialType::clothy, L"singed" },
	  { materialType::fleshy, L"tenderised" },
	  { materialType::veggy, L"tenderised" },
	  { materialType::glassy, L"crazed" },
	  { materialType::leathery, L"singed" },
	  { materialType::liquid, L"simmering" },
	  { materialType::papery, L"singed" },
	  { materialType::waxy, L"singed" },
	  { materialType::woody, L"singed" }
	}
    },{
      damageType::cold, L"cold", L"warm",
	{
	  { materialType::fleshy, L"prefrozen" },
	  { materialType::veggy, L"prefrozen" },
	  { materialType::liquid, L"anti-freezed" },
	},{
	  { materialType::fleshy, L"chilled" },
	  { materialType::veggy, L"chilled" },
	  { materialType::liquid, L"icy" },
	}
    },{
      damageType::wet, L"wetting", L"reviving",
	{
	  { materialType::clothy, L"antiseptic" },
	  { materialType::fleshy, L"sterilised" },
	  { materialType::veggy, L"sterilised" },
	  { materialType::leathery, L"antiseptic" },
	  { materialType::liquid, L"undilutable" }, // TODO: if we make this L"concentrated" then it could also affect the weight?
	  { materialType::metallic, L"rustproof" },
	  { materialType::papery, L"antiseptic" },
	  { materialType::stony, L"antiseptic" },
	  { materialType::woody, L"antiseptic" }
	},{
	  { materialType::clothy, L"mouldy" },
	  { materialType::fleshy, L"mouldy" },
	  { materialType::veggy, L"mouldy" },
	  { materialType::leathery, L"mouldy" },
	  { materialType::liquid, L"diluted" },
	  { materialType::metallic, L"rusty" },
	  { materialType::papery, L"mouldy" },
	  { materialType::stony, L"algae-covered" },
	  { materialType::woody, L"squisty" }
	}
    },{
      damageType::sonic, L"noise", L"hearing",
	{
	  { materialType::glassy, L"fracture-resistant" },
	},{
	// sonic weapots don't work on wood (ref Dr Who)
	  { materialType::glassy, L"crazed" }
	}
    },{
      damageType::disintegration, L"disintegration", L"reintegration",
	{
	  { materialType::clothy, L"well-made" },
	  { materialType::fleshy, L"sterilized" },
	  { materialType::veggy, L"sterilized" },
	  { materialType::glassy, L"fracture-resistant" },
	  { materialType::leathery, L"well-made" },
	  { materialType::metallic, L"well-made" },
	  { materialType::papery, L"well-made" },
	  { materialType::stony, L"shatterproof" },
	  { materialType::waxy, L"hard-wearing" },
	  { materialType::woody, L"sturdy" }
	},{
	  { materialType::clothy, L"worn" },
	  { materialType::fleshy, L"mouldy" },
	  { materialType::veggy, L"mouldy" },
	  { materialType::glassy, L"crazed" },
	  { materialType::leathery, L"worn" },
	  { materialType::metallic, L"worn" },
	  { materialType::papery, L"wmorn" },
	  { materialType::stony, L"crazed" },
	  { materialType::waxy, L"misshapen" },
	  { materialType::woody, L"splintered" }
	}
      },{
	// starvation doesn't apply to material objects
	damageType::starvation, L"hunger", L"feeding",
	  {},{}
      },{
      damageType::electric, L"electricity", L"chakra-alignment",
	{
	  { materialType::fleshy, L"precooked" },
	  { materialType::veggy, L"precooked" },
	  { materialType::metallic, L"insulated" },
	},{
	  { materialType::fleshy, L"burnt" },
	  { materialType::veggy, L"burnt" },
	  { materialType::metallic, L"sparky" },
	}

      }});
}
