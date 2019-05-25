/* License and copyright go here*/

#include "level.hpp"
#include "levelGen.hpp"
#include "levelFactory.hpp"
#include "itemTypes.hpp"
#include "monsterType.hpp"
#include "monster.hpp"
#include "items.hpp" // createItem
#include "random.hpp"
#include "religion.hpp"
#include "spaceZone.hpp"

#include <functional>

class waterLevelGen : public levelGen {
private:
  const bool addDownRamp_; // do we need a downwards exit?
  coord downRampPos_; // if set
public:
  waterLevelGen(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),
    addDownRamp_(addDownRamp),
    downRampPos_(-1,-1) {}
  virtual ~waterLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {
    /*
    if (addDownRamp_ && next) {
      downRampPos_ = next.value().upRampPos();
      if (downRampPos_.first >= 0)
	place(downRampPos_, terrainType::DOWN);
	}*/
  }
  virtual void build() {
    setName(L"The perilous seaside");

    for (coord c : coordRectIterator(0,1,level::MAX_WIDTH-1,level::MAX_HEIGHT-1))
      if (at(c) == terrainType::ROCK)
	place(c, terrainType::WATER);
    place(coordRectIterator(1,1,3,3), terrainType::GROUND);
    place(coord(0,2), terrainType::UP);
    place(coordRectIterator(3,level::MAX_HEIGHT-4,6,level::MAX_HEIGHT-1), terrainType::GROUND);
    place(coord(2,level::MAX_HEIGHT-2), terrainType::DOWN);
    for (int d=0; d < 4; ++d)
      place(coordRectIterator(3+10*d,3+d,3+10*(d+1),4+d), terrainType::GROUND);
    place(coordRectIterator(40,8,50,10), terrainType::GROUND);
    for (auto c : coordRectIterator(45,11,45,18))
      pub_.holder(c).addItem(createItem(itemTypeKey::bridge));
    place(coordRectIterator(35,18,55,19), terrainType::GROUND);
    place(coordRectIterator(4,19,60,19), terrainType::GROUND);
    place(coordRectIterator(15,18,22,18), terrainType::GROUND);
    place(coord(17,17), terrainType::GROUND);

    if (dPc() < 50) { // add a watery shrine:
      addShrine();
    }

    pub_.holder(coord(1,4)).addItem(createItem(itemTypeKey::ship));

    // monsters. Let's start with 5 kelpie and 2 sirens, then half a dozen merfolk:
    for (int i=0; i < 5; ++i)
      addMonster(monsterTypeKey::kelpie);
    for (int i=0; i < 2; ++i)
      addMonster(monsterTypeKey::siren);
    for (int i=0; i < 6; ++i)
      addMonster(monsterTypeKey::merfolk);
    addEnchantedItem(itemTypeKey::conch);
    for (int i=0; i < 3; ++i) {
      addWetItem(itemTypeKey::lily);
      addWetItem(itemTypeKey::lotus);
    }
  }

private:
  void addMonster(monsterTypeKey mtk) {
    coord c;
    do {
      c.first = rndPickI(1, level::MAX_WIDTH-1);
      c.second = rndPickI(1, level::MAX_HEIGHT-1);
    } while (at(c) != terrainType::WATER);
    levelGen::addMonster(mtk, c);
  }
  void addWetItem(itemTypeKey itk) {
    coord c = findRndTerrain(terrainType::WATER);
    auto &it = createItem(itk);
    pub_.holder(c).addItem(it);
  }
  void addEnchantedItem(itemTypeKey itk) {
    coord c = findRndTerrain(terrainType::GROUND);
    auto &it = createItem(itk);
    it.bless(true);
    it.enchant(1 + dPc() / 25);
    pub_.holder(c).addItem(it);
  }
  void addShrine() {
    optionalRef<deity> d;
    auto &dr = deityRepo::instance();
    do {
      d = optionalRef<deity>(*rndPick(dr.begin(), dr.end()));
    } while (d.value().element() != Element::water);
    
    levelGen::addShrine(coord(51,7),coord(57,12), d);
    place(coord(56,9), terrainType::ALTAR);
    place(coord(56,7), terrainType::WATER); // round the corners
    place(coord(56,11), terrainType::WATER); // round the corners
  }
};

class spaceLevelGen : public levelGen {
private:
  const bool addDownRamp_; 
  coord downRampPos_;
public:
  spaceLevelGen(levelImpl* const level, ::level& pub, bool addDownRamp) :
    levelGen(level, pub),
    addDownRamp_(addDownRamp),
    downRampPos_(-1,-1) {}
  virtual ~spaceLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {
    /*
    if (addDownRamp_ && next) {
      downRampPos_ = next.value().upRampPos();
      if (downRampPos_.first >= 0)
	place(downRampPos_, terrainType::DOWN);
	}*/
  }
  virtual void build() {
    setName(L"Space Station Alpha Beta"); // ref: Airplane II

    const int alpha=10;
    const int beta =level::MAX_WIDTH-12;

    for (coord c : coordRectIterator(0,0,level::MAX_WIDTH-1,level::MAX_HEIGHT-1))
      if (at(c) == terrainType::ROCK)
	place(c, terrainType::SPACE);
    // place alpha airlock
    placeAirlock(alpha,10, L"Alpha");
    // place beta airlock
    placeAirlock(beta, 10, L"Beta");

    auto overall = rectarea::create(0,0,level::MAX_WIDTH, level::MAX_HEIGHT);
    auto airlock1 = airlockArea(alpha,10);
    auto airlock2 = airlockArea(beta,10);
    //    std::unique_ptr<geometry> spaceArea = shape::subtract(overall, airlock1, airlock2);
    auto spaceArea = shape::subtract(std::move(overall), std::move(airlock1), std::move(airlock2));

    auto zone = std::make_shared<spaceZone>(std::move(spaceArea), pub_);
    monsterZone(zone);
    itemZone(zone);

    place(coord(alpha,10), terrainType::UP);
    place(coord(beta,10), terrainType::DOWN);

    auto &ft = monsterTypeRepo::instance()[monsterTypeKey::ferret];
    levelGen::addMonster(ft.spawnSpace(pub_, *ft.builder()), findRndTerrain(terrainType::SPACE));

    // hide an item in the starfield:
    pub_.holder(findRndTerrain(terrainType::SPACE))
      .addItem(createRndItem(pub_.depth(), L'*'));
  }
  static std::unique_ptr<geometry> airlockArea(unsigned char x, unsigned char y) {
    using shape::add;
    using std::move;
    auto a = rectarea::create(x-2,y-2,x+2,y+2);
    auto b = rectarea::create(x-4,y-1,x+4,y+1);
    auto c = rectarea::create(x-6,y-1,x+6,y+1); // include 2 spaces outside airlock; handled by airlock zone *not* space zone
    return add(move(a), move(b), move(c));
  }
  /*
   * Airlocks work on both sides; only the middle row is active:
   *    #####      #####      #####      #####      #####      #####      #####      #####    
   *  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  
   *  #.#.<.#.#  #.#.<@..#  #.#.<.@.#  #.#.<..@#  #.#.<.#@.  #.#.<.#.@  #.#.<.#..@ #.#.<.#.# @
   *  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  ###...###  
   *    #####      #####      #####      #####      #####      #####      #####      #####    
   * ABCDEFGHIJK
   * Monsters nearest the outside get priority.
   * when another monster stands on an airlock square that would be closed, it gets pushed *outwards*.
   */
  void placeAirlock(unsigned char posX, unsigned char posY, const wchar_t * const name) {
    for (coord c : coordRectIterator(posX-2,posY-2,posX+2,posY+2))
      place(c, terrainType::BULKHEAD);
    for (coord c : coordRectIterator(posX-4,posY-1,posX+4,posY+1))
      place(c, terrainType::BULKHEAD);
    for (coord c : coordRectIterator(posX-1,posY-1,posX+1,posY+1))
      place(c, terrainType::DECK);
    place(coord(posX-3, posY), terrainType::DECK);
    place(coord(posX+3, posY), terrainType::DECK);
    auto &p(pub_);
    monsterZone(std::make_shared<airlock>(posX, posY, pub_,
					  [&p, posY](int x) { 
      // open door
      p.changeTerrain(coord(x,posY), terrainType::DECK); // TODO: bulkhead/deck
    },
					  [&p, posY](int x) {
      // shut door
      p.changeTerrain(coord(x,posY), terrainType::BULKHEAD); // TODO: bulkhead/deck
    }));
  }


  class airlock : public zoneArea<monster> {
  private:
    enum class state {CLOSED, INNER, OUTER};
    const unsigned char x_, y_;
    const level &pub_;
    std::unique_ptr<geometry> g_;
    // functions take exact X coord and open/close door in that space:
    std::function<void(int)> openDoor_, closeDoor_;
    state state_;
  public:
    airlock(unsigned char x, unsigned char y, const level &pub,
	    std::function<void(int)> openDoor, std::function<void(int)> closeDoor) :
      x_(x), y_(y), pub_(pub), g_(airlockArea(x,y)),
      openDoor_(openDoor), closeDoor_(closeDoor),
      state_(state::CLOSED) {}
    virtual ~airlock() {}
    virtual bool contains(coord area) {
      return g_->contains(area);
    }
    virtual bool onMoveWithin(monster &m, const coord &dest) {
      // monster in A or K: open outer airlock
      if (dest == coord(x_-5, y_) || dest == coord(x_+5, y_) ||
      // monster in B or J: open outer airlock
	  dest == coord(x_-4,y_) || dest == coord(x_+4,y_))
	openOuter();
      // monster in C or I: cycle airlock
      else if (dest == coord(x_-3,y_) || dest == coord(x_+3,y_))
	cycle();
      // monster in D or H: open inner airlock
      else if (dest == coord(x_-2,y_) || dest == coord(x_+2,y_) ||
      // monster in E or G: open inner airlock
	       dest == coord(x_-1,y_) || dest == coord(x_+1,y_))
	openInner();
      // monster outside (leaving?): lock down airlock
      else if (dest == coord(x_-5, y_) || dest == coord(x_+5, y_))
	close();
      // otherwise close airlock.
      else close();
      return true;
    }
    void openOuter() {
      state_ = state::OUTER;
      openDoor_(x_-4); openDoor_(x_+4); 
      closeDoor_(x_-2); closeDoor_(x_+2);
    }
    void openInner() {
      state_ = state::INNER;
      openDoor_(x_-2); openDoor_(x_+2); 
      closeDoor_(x_-4); closeDoor_(x_+4);
    }
    void close() {
      closeDoor_(x_-2); closeDoor_(x_+2);
      closeDoor_(x_-4); closeDoor_(x_+4);
    }
    void cycle() {
      switch (state_) {
      case state::CLOSED: openInner(); // we start inside, so give priority to things going out.
	break;
      case state::INNER: openOuter(); // we start inside, so give priority to things going out.
	break;
      case state::OUTER: openInner(); // we start inside, so give priority to things going out.
	break;
      default: close(); // shouldn't happen, but just in case
      }
    }
  };
};

levelGen *newGen(specialLevelKey key, levelImpl *l, level *level, bool addDownRamp) {
  switch (key) {
  case specialLevelKey::WATER:
    return new waterLevelGen(l, *level, addDownRamp);
  case specialLevelKey::SPACE:
    return new spaceLevelGen(l, *level, addDownRamp);
  default:
    throw (std::wstring(L"Unknown special level: ") + std::to_wstring(static_cast<int>(key)));
  }
}
