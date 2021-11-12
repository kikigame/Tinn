/* License and copyright go here*/

#include <map>
#include "terrain.hpp"
#include "monster.hpp"
#include "random.hpp"

// terrain bits

const wchar_t * const to_string(const terrainType & t) {
  switch (t) {
  case terrainType::ROCK: return L"ROCK";
  case terrainType::GROUND: return L"GROUND";
  case terrainType::UP: return L"UP";
  case terrainType::DOWN: return L"DOWN";
  case terrainType::PIT_HIDDEN: return L"PIT_HIDDEN";
  case terrainType::PIT: return L"PIT";
  case terrainType::PIANO_HIDDEN: return L"PIANO_HIDDEN";
  case terrainType::FIRE: return L"FIRE";
  case terrainType::WATER: return L"WATER";
  case terrainType::SPACE: return L"SPACE";
  case terrainType::BULKHEAD: return L"BULKHEAD";
  case terrainType::DECK: return L"DECK";
  case terrainType::KNOTWEED: return L"KNOTWEED";
  case terrainType::CRACK: return L"CRACK";
  case terrainType::WEB: return L"WEB";
  case terrainType::SPRINGBOARD: return L"SPRINGBOARD";
  case terrainType::SPRINGBOARD_HIDDEN: return L"SPRINGBOARD_HIDDEN";
  case terrainType::WELL: return L"WELL";
  case terrainType::WISHING_WELL: return L"WISHING_WELL";
  default: throw t;
  }
}


terrain::terrain(const wchar_t render, const wchar_t* const name,const wchar_t * const description, 
		 const terrainType type) :
  render_(render),
  name_(name),
  description_(description),
  type_(type) {}

terrain::~terrain() {}

const wchar_t terrain::render() const {
  if (type_ == terrainType::SPACE) {
    static std::wstring spacechars(L"∙*·o★☆");
    if (dPc() < 5) // 0.81%
      return *rndPick(spacechars.begin(), spacechars.end());
  }
  return render_;
}

std::wstring terrain::name() const {
  return name_;
}

std::wstring terrain::description() const {
  return description_;
}

terrainType terrain::type() const {
  return type_;
}

bool terrain::groundLike() const {
  return type_ == terrainType::GROUND || type_ == terrainType::DECK ||
    type_ == terrainType::WELL || type_ == terrainType::WISHING_WELL;
}

bool terrain::entraps(const monster &m, bool includeHidden) const {
  switch (type_) {
  case terrainType::ALTAR:
  case terrainType::ROCK:
  case terrainType::GROUND:
  case terrainType::UP:
  case terrainType::DOWN:
    return false;
  case terrainType::PIT:
    return !(m.abilities()->fly());
  case terrainType::PIT_HIDDEN:
    return includeHidden && !(m.abilities()->fly());
  case terrainType::PIANO_HIDDEN:
    return includeHidden;
  case terrainType::FIRE:
  case terrainType::WATER:
  case terrainType::SPACE:
  case terrainType::BULKHEAD:
  case terrainType::DECK:
  case terrainType::KNOTWEED:
  case terrainType::CRACK:
  case terrainType::WELL:
  case terrainType::WISHING_WELL:
    return false;
  case terrainType::WEB:
    return m.type().type() != monsterTypeKey::spider;
  case terrainType::SPRINGBOARD:
    return !(m.abilities()->fly());
  case terrainType::SPRINGBOARD_HIDDEN:
    return includeHidden && !(m.abilities()->fly());
  default:
    throw type_; // missing type from enum
  }
}

bool terrain::shouldSupportItems() const {
  return type_ != terrainType::FIRE && type_ != terrainType::WATER && type_ != terrainType::WELL && type_ != terrainType::WISHING_WELL;
}

bool terrain::operator ==(const terrain &other) const {
  return type_ == other.type();
}
bool terrain::operator !=(const terrain &other) const {
  return !(*this == other);
}


class terrainFactoryImpl {
private:
  ::std::map<terrainType, ::std::unique_ptr<terrain>> store_;
public:
  terrainFactoryImpl() {
    store(new terrain(L'Π', L"Altar", L"Idol of worship; placed in shrines for vereration of the gods", terrainType::ALTAR));
    store(new terrain(L' ', L"Rock", L"Solid rock; too hard to dig through without tools", terrainType::ROCK));
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::GROUND));
    store(new terrain(L'<', L"Ramp up", L"Accessible ramp up to the previous level.", terrainType::UP));
    store(new terrain(L'>', L"Ramp down", L"Accessible ramp down to the next level.", terrainType::DOWN));
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::PIT_HIDDEN));
    // this symbol should be unicode (U+22BB), propositional/Boolean logical xor.
    // Nethack uses ^ for all traps, but that doesn't fit with pits at all.
    store(new terrain(L'⊻', L"Pit", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::PIT));
    // Nethack uses '~' rather than U+2248 (approximately equal). I prefer the latter as it suggests more movement
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::PIANO_HIDDEN));
    store(new terrain(L'≈', L"Water", L"Water; most creatures avoid this due to the risk of drowning.", terrainType::WATER));
    // Nethack uses '~' rather than U+2240 (wreath product).
    store(new terrain(L'≀', L"Fire", L"Fire; most creatures avoid this due to the risk of burning.", terrainType::FIRE));
    store(new terrain(L' ', L"Deep Space", L"Distance can be an illusion. Bring your own oxygen and propulsion.", terrainType::SPACE));
    store(new terrain(L'□', L"Bulkhead", L"Bulkheads are partitions or walls forming the structural integrity of an\nembankment, ship, aircraft or spacecraft.", terrainType::BULKHEAD));
    store(new terrain(L'.', L"Deck", L"Artificial platform for standing upon.", terrainType::DECK));
    store(new terrain(L'{', L"Knotweed", L"This rhizomatous plant is native to Japan, but amongst the most invasive\n"
"and destructive of all. It takes 2-3 years to kill a knotweed plant using\n"
"conventional herbicide; growing up to 2-3m deep or more. It establishes for\n"
"decades below ground before been seen in significant quantity in a matter of\n"
"weeks. It can grow through the smallest cracks in concrete and regrow from\n"
"the tiniest remaining fragment.", terrainType::KNOTWEED));
    store(new terrain(L'¨',L"Cracked wall", L"This wall is displaying signs of subsidance.", terrainType::CRACK));
    store(new terrain(L'"',L"Web", L"Used by spiders to ensnare prey.\n"
"Warning: spiders will come back to eat their webs. If you're entangled\n"
"when they do, you're lunch.", terrainType::WEB));
    store(new terrain(L'^',L"Spring", L"Allows movement over increased distance.\n"
"Don't blame me if you fling yourself into a wall.\n"
"Balistic monsters may also be used as crude missile weapons.", terrainType::SPRINGBOARD));
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::SPRINGBOARD_HIDDEN));
    store(new terrain(L'ᐁ', L"Well", L"Throw in a coin; if you like. Supplies fresh drinking water.", terrainType::WELL));
    store(new terrain(L'⍌', L"Wishing Well", L"Throw in a coin; used to make a wish. Supplies fresh drinking water.", terrainType::WISHING_WELL));
  }
  const terrain &get(terrainType type) const {
    return *(store_.at(type).get());
  }
private:
  void store(terrain * t) {
    store_[t->type()] = std::unique_ptr<terrain>(t);
  }
};

terrainFactory::terrainFactory() : 
  pImpl_(new terrainFactoryImpl()) {
}

terrainFactory::~terrainFactory() {
}

const terrain &terrainFactory::get(terrainType type) const {
  return pImpl_->get(type);
}
