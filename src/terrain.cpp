/* License and copyright go here*/

#include <map>
#include "terrain.hpp"
#include "monster.hpp"

// terrain bits

const wchar_t * const to_string(const terrainType & t) {
  switch (t) {
  case terrainType::ROCK: return L"ROCK";
  case terrainType::GROUND: return L"GROUND";
  case terrainType::UP: return L"UP";
  case terrainType::DOWN: return L"DOWN";
  case terrainType::PIT_HIDDEN: return L"PIT_HIDDEN";
  case terrainType::PIT: return L"PIT";
  case terrainType::FIRE: return L"FIRE";
  case terrainType::WATER: return L"WATER";
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
  return render_;
}

const wchar_t * const terrain::name() const {
  return name_;
}

const wchar_t * const terrain::description() const {
  return description_;
}

terrainType terrain::type() const {
  return type_;
}

bool terrain::movable(const monster &m) const {
  switch (type_) {
  case terrainType::ALTAR:
  case terrainType::ROCK:
    return false;
  case terrainType::GROUND:
  case terrainType::UP:
  case terrainType::DOWN:
  case terrainType::PIT:
  case terrainType::PIT_HIDDEN:
    return true;
  case terrainType::FIRE:
    return false;
  case terrainType::WATER:
    return m.abilities().swim();
  default:
    throw type_; // missing type from enum
  }
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
    return !(m.abilities().fly());
  case terrainType::PIT_HIDDEN:
    return includeHidden && !(m.abilities().fly());
  case terrainType::FIRE:
  case terrainType::WATER:
    return false;
  default:
    throw type_; // missing type from enum
  }
}


class terrainFactoryImpl {
private:
  ::std::map<terrainType, ::std::shared_ptr<terrain>> store_;
public:
  terrainFactoryImpl() {
    // TODO: Yes I know altars and idols aren't the same thing; I want to keep it vague until I've figured out what, if anything, they do.
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
    store(new terrain(L'≈', L"Water", L"Water; most creatures avoid this due to the risk of drowning.", terrainType::WATER));
    // Nethack uses '~' rather than U+2240 (wreath product).
    store(new terrain(L'≀', L"Fire", L"Fire; most creatures avoid this due to the risk of burning.", terrainType::FIRE));
  }
  ::std::shared_ptr<terrain> get(terrainType type) const {
    return store_.at(type);
  }
private:
  void store(terrain * t) {
    store_.insert(::std::pair<terrainType, ::std::shared_ptr<terrain>>(t->type(), ::std::shared_ptr<terrain>(t)));
  }
};

terrainFactory::terrainFactory() : 
  pImpl_(new terrainFactoryImpl()) {
}

terrainFactory::~terrainFactory() {
}

std::shared_ptr<terrain> terrainFactory::get(terrainType type) const {
  return pImpl_->get(type);
}

