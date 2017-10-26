/* License and copyright go here*/

#include <map>
#include "terrain.hpp"

// terrain bits

const wchar_t * const to_string(const terrainType & t) {
  switch (t) {
  case terrainType::ROCK: return L"ROCK";
  case terrainType::GROUND: return L"GROUND";
  case terrainType::UP: return L"UP";
  case terrainType::DOWN: return L"DOWN";
  case terrainType::PIT_HIDDEN: return L"PIT_HIDDEN";
  case terrainType::PIT: return L"PIT";
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
  case terrainType::ROCK:
    return false;
  case terrainType::GROUND:
  case terrainType::UP:
  case terrainType::DOWN:
  case terrainType::PIT:
  case terrainType::PIT_HIDDEN:
    return true;
  default:
    throw type_; // missing type from enum
  }
}


class terrainFactoryImpl {
private:
  ::std::map<terrainType, ::std::shared_ptr<terrain>> store_;
public:
  terrainFactoryImpl() {
    store(new terrain(L' ', L"Rock", L"Solid rock; too hard to dig through without tools", terrainType::ROCK));
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::GROUND));
    store(new terrain(L'<', L"Ramp up", L"Accessible ramp up to the previous level.", terrainType::UP));
    store(new terrain(L'>', L"Ramp down", L"Accessible ramp down to the next level.", terrainType::DOWN));
    store(new terrain(L'.', L"Ground", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::PIT_HIDDEN));
    // this symbol should be unicode (U+22BB), propositional/Boolean logical xor.
    // Nethack uses ^ for all traps, but that doesn't fit with pits at all.
    store(new terrain(L'⊻', L"Pit", L"Subterrainian earth; looks like a solid floor, but be careful or it may give way.", terrainType::PIT));
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

