/* License and copyright go here*/

// implement the behaviour of garden tiles

#include "gardenZone.hpp"
#include "level.hpp"
#include "terrain.hpp"
#include "monster.hpp"
#include "items.hpp"
#include "religion.hpp"
#include "random.hpp"
#include "itemTypes.hpp"

// TODO: use this somewhere
gardenZone::gardenZone(std::unique_ptr<geometry> &&geometry,
		       level &lvl, bool hostile) :
  geometry_(), hostile_(hostile), lvl_(lvl), callbacks_() {
  geometry_.swap(geometry);
}

gardenZone::~gardenZone() {}

bool gardenZone::contains(coord a) {
  return geometry_->contains(a);
}

bool gardenZone::onMoveWithin(monster &mon, const coord &dest) {
  switch (lvl_.terrainAt(dest).type()) {
  case terrainType::GROUND:
    if (!hostile_ && mon.align().element() == Element::plant)
      lvl_.holder(dest).addItem(createItem(itemTypeKey::sunflower));
    if (hostile_ && dPc() < 50)
      lvl_.changeTerrain(dest, terrainType::KNOTWEED); 
    return false;
  case terrainType::WATER:
    if (mon.align().element() == Element::plant)
      lvl_.holder(dest).
	addItem(createItem(dPc() < 30 ? itemTypeKey::lily : itemTypeKey::lotus));
    return false;
  default:
    return false;
  }
}

bool gardenZone::onExit(monster &mon, itemHolder &next) {
  return false;
}

 bool gardenZone::onMoveWithin(item &it, const coord &dest) {
  if (it.render() == L'{') {
    auto pFlower = dynamic_cast<basicItem *>(&it);
    if (!pFlower) return false;
    auto &type = pFlower->getType();
    terrainType tType;

    itemTypeKey typeKey;
    if (itemTypeRepo::instance()[itemTypeKey::bouquet_mixed] == type) {
      typeKey = itemTypeKey::bouquet_mixed;
      tType = terrainType::GROUND;
    } else if (itemTypeRepo::instance()[itemTypeKey::sunflower] == type) {
      typeKey = itemTypeKey::sunflower;
      tType = terrainType::GROUND;
    } else if (itemTypeRepo::instance()[itemTypeKey::lily] == type) {
      typeKey = itemTypeKey::lily;
      tType = terrainType::WATER;
    } else if (itemTypeRepo::instance()[itemTypeKey::lotus] == type) {
      typeKey = itemTypeKey::lotus;
      tType = terrainType::WATER;
    } else {
      return false;
    }

    if (lvl_.terrainAt(dest).type() == tType)
      callbacks_.emplace_back(new gardenCallback(callbacks_, dest, lvl_, tType, typeKey));
  }
  return false;
}

bool gardenZone::onExit(item &ite, itemHolder &next) {
  return false;
}


gardenCallback::gardenCallback(std::vector<std::shared_ptr<gardenCallback>> &callbacks,
			       const coord &dest, level &lvl,
			       const terrainType & tType,
			       const itemTypeKey &typeKey) :
  temporal::callback(true, std::ref(*this)),
  callbacks_(callbacks),
  dest_(dest),
  lvl_(lvl),
  tType_(tType),
  typeKey_(typeKey) {}

gardenCallback::~gardenCallback() {}

void gardenCallback::operator()() {
    if (dPc() < 20) {
      auto minX = dest_.first - 1; if (minX < 0) minX+=1;
      auto minY = dest_.second - 1; if (minY < 0) minY+=1;
      auto maxX = dest_.first + 1; if (maxX >= level::MAX_WIDTH) maxX-=1;
      auto maxY = dest_.second + 1; if (maxY >= level::MAX_HEIGHT) maxY-=1;
      for (auto x = minX; x <= maxX; ++x)
	for (auto y = minY; y <= maxY; ++y)
	  if (lvl_.terrainAt(coord(x,y)).type() == tType_)
	    lvl_.holder(coord(x,y)).
	      addItem(createItem(typeKey_));
      callbacks_.erase(std::remove_if(callbacks_.begin(), callbacks_.end(),
				      [this](const std::shared_ptr<gardenCallback> &i) { return i.get() == this;}
				      ), callbacks_.end());
    }
  }
