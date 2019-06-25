/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in future.

#include "quest.hpp"
#include "levelGen.hpp"
#include "output.hpp"
#include "action.hpp"
#include "items.hpp"
#include "random.hpp"
#include "dungeon.hpp"
#include "religion.hpp"
#include "geometry.hpp"

std::uniform_int_distribution<int> xPosD(0,level::MAX_WIDTH - 1);
std::uniform_int_distribution<int> yPosD(0,level::MAX_HEIGHT - 1);


class questImpl {
private:
  const wchar_t * const name_;
  const std::wstring questData_;
  const wchar_t * const incompletePrompt_;
  const wchar_t * const completeMsg_;
  const int questLevel_;
  bool isComplete_;
  const std::function<levelGen*(questImpl &, levelImpl &, level &)> lg_;
  const std::function<void(questImpl &, levelGen &, level &, int)> ls_;
public:
  questImpl(const wchar_t* const name,
	    const std::wstring questData,
	    const wchar_t* const incompletePrompt,
	    const wchar_t* const completeMsg,
	    int questLevel,
	    // quest level generator:
	    const std::function<levelGen*(questImpl &, levelImpl &, level &)> lg,
	    // level setup filter
	    const std::function<void(questImpl &, levelGen &, level &, int)> ls)
    : name_(name), questData_(questData), incompletePrompt_(incompletePrompt),
      completeMsg_(completeMsg),
      questLevel_(questLevel), isComplete_(false), lg_(lg), ls_(ls) {}
  questImpl(const questImpl &) = delete;
  questImpl(questImpl &&) = delete;
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const questData() const {
    return questData_.c_str();
  }
  const wchar_t * const incompletePrompt() const { 
    return incompletePrompt_;
  }
  const wchar_t * const completeMsg() const { 
    return completeMsg_;
  }
  bool isQuestLevel(int depth) const {
    return depth == questLevel_;
  }
  levelGen * newQuestLevelGen(levelImpl & li, level &l) {
    return lg_(*this, li, l);
  }
  void setupLevel(levelGen &lg, level &l, int depth) { ls_(*this,lg,l,depth); }
  bool isSuccessful() const {
    return isComplete_;
  }
  void complete() {
    isComplete_ = true;
    ioFactory::instance().longMsg(completeMsg());
  }
};


quest::quest(questImpl *pImpl)
  : pImpl_(pImpl) {};

const wchar_t * const quest::name() const { return pImpl_->name(); }
const wchar_t * const quest::questData() const { return pImpl_->questData(); }
const wchar_t * const quest::incompletePrompt() const { return pImpl_->incompletePrompt(); }
const wchar_t * const quest::completeMsg() const { return pImpl_->completeMsg(); }
bool quest::isQuestLevel(int depth) const { return pImpl_->isQuestLevel(depth); }
levelGen *quest::newQuestLevelGen(levelImpl &li, level &l) const {
  return pImpl_->newQuestLevelGen(li,l);
}
void quest::setupLevel(levelGen &lg, level &l, int depth) { return pImpl_->setupLevel(lg,l,depth); }
bool quest::isSuccessful() const { return pImpl_->isSuccessful(); }


// ref: Many, many slay-the-dragon myths. St George arguably, although
// he was a real Turkish Christian who never slew a dragon (martyred
// in 303AD for refusing to convert to the Roman religion).
class warriorQuestLevelGen : public levelGen {
private:
  questImpl &q_;
public:
  warriorQuestLevelGen(questImpl &q, levelImpl &li, level &l) :
    levelGen(&li,l), q_(q) {}
  virtual ~warriorQuestLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual coord upRampPos() { return coord(2,10); }
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::GROUND);
    place(upRampPos(), terrainType::UP);
    //    place(coord(level::MAX_WIDTH-2,10), terrainType::DOWN);
    auto db = monsterTypeRepo::instance()[monsterTypeKey::dragon].builder();
    db->highlight();
    auto &q = q_; // can't capture "this" usefully...
    db->onDeath([&q]{q.complete();});
    addMonster(*db, coord(level::MAX_WIDTH-2, 10));
    setName(L"The Arena");
  }
};

class shopQuestLevelGen : public levelGen {
private:
  questImpl &q_;
public:
  shopQuestLevelGen(questImpl &q, levelImpl &li, level &l) :
    levelGen(&li,l), q_(q) {}
  virtual ~shopQuestLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual coord upRampPos() { return coord(1,10); }
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::GROUND);
    place(upRampPos(), terrainType::UP);
    //    place(coord(30,10), terrainType::DOWN);
    auto tb = monsterTypeRepo::instance()[monsterTypeKey::troll].builder();
    tb->highlight();
    tb->onDeath([]{
	ioFactory::instance().message(L"This looks like useful loot...\nI wonder to whom it might be sold?");
      });
    // Trophy to sell
    auto &wand = createWand(sharedAction<monster,monster>::key::popup_shop);
    wand.bless(true);
    auto &grail=createQuestItem<questItemType::grail>();
    auto &mon = addMonster(*tb, coord(level::MAX_WIDTH-2, 10));
    mon.addItem(wand);
    mon.addItem(grail);
    setName(L"The Shopping Arena");
  }
};

class thiefQuestLevelGen : public levelGen {
private:
  questImpl &q_;
public:
  thiefQuestLevelGen(questImpl &q, levelImpl &li, level &l) :
    levelGen(&li,l), q_(q) {}
  virtual ~thiefQuestLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual coord upRampPos() { return coord(0,0); }
  virtual void build() {
    for (int x=0; x<level::MAX_WIDTH; ++x)
      for (int y=0; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::GROUND);
    place(upRampPos(), terrainType::UP);
    place(coord(level::MAX_WIDTH-2, level::MAX_HEIGHT-2), terrainType::DOWN);
    std::vector<coord> coords;
    for (int i=0; i < 50; ++i) {
      coord c(xPosD(generator), yPosD(generator));
      if (at(c) == terrainType::GROUND) {
	coords.push_back(c);
	place(c, terrainType::PIT);
      }
    }
    constexpr auto nil = static_cast<std::vector<coord>::size_type>(0);
    for (auto c : coords)
      pub_.holder(coords[rndPickI(nil, coords.size())])
	.addItem(createItem(static_cast<itemTypeKey>(rndPickI(static_cast<int>(itemTypeKey::kalganid), static_cast<int>(itemTypeKey::gpl_brick)))));
    for (int i=0; i < 50; ++i) {
      coord c(xPosD(generator), yPosD(generator));
      if (at(c) == terrainType::GROUND)
	place(c, terrainType::PIANO_HIDDEN);
    }
    // collecting qI completes quest
    std::function<void(const itemHolder &)> f = [this](const itemHolder &holder) {
      auto m = dynamic_cast<const player*>(&holder);
      if (m) q_.complete();
    };
    auto &qI = createQuestItem<questItemType::diamond>(f);    
    pub_.holder(coords[rndPickI(nil, coords.size())])
      .addItem(qI);
  }
};

template <bool lowerLevel>
class crusadeQuestLevelGen {};

template <>
class crusadeQuestLevelGen<false> : public levelGen {
private:
  questImpl &q_;
  const deity &pcAlign_;
public:
  crusadeQuestLevelGen(questImpl &q, levelImpl &li, level &l, const deity & pcAlign) :
    levelGen(&li,l), q_(q), pcAlign_(pcAlign) {}
  virtual ~crusadeQuestLevelGen() {}
  virtual coord upRampPos() { return coord(60,10); } // TODO: ramp negotiation
  virtual coord downRampPos() { return coord(35,10); }
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::ROCK);

    auto &dr = deityRepo::instance();
    auto &opposed = dr.getOpposed(pcAlign_);

    // place a non-aligned, non-opposed shrine, to ensure the game is winnable.
    dir downRight(1,1);
    coord shrineTopLeft(dPc() / 10, dPc() / 10);
    coord shrineCenter = shrineTopLeft.inDir(downRight);
    coord shrineBotRight = shrineCenter.inDir(downRight);
    for (coord c : coordRectIterator(shrineTopLeft, shrineBotRight))
      place(c, terrainType::GROUND);
    deity *d = &(dr.nonaligned());
    do {
      d = &*rndPick(dr.begin(), dr.end());
    } while (d == &pcAlign_ || d == &opposed || d->nonaligned());
    addShrine(shrineTopLeft,shrineBotRight, optionalRef<deity>(*d));
    
    // create a big temple; opposite alignment to player
    coord templeTopLeft(20,5);
    coord templeBtmRight(50,15);
    for (coord c : coordRectIterator(templeTopLeft, templeBtmRight))
      place(c, terrainType::GROUND);
    addShrine(templeTopLeft, templeBtmRight, optionalRef<deity>(opposed));

    // collecting icon completes quest
    std::function<void(const itemHolder &)> f = [this](const itemHolder &holder) {
      auto m = dynamic_cast<const player*>(&holder);
      if (m) q_.complete();
    };
    // NB: Icon is aligned to player, as it's been stolen.
    item & ic=createQuestItem<questItemType::icon>(f, pcAlign_);
    ic.highlight();
    ic.bless(true);
    pub_.holder(coord(rndPickI(21, 49), rndPickI(6,14)))
      .addItem(ic);
    
    coord c = addCorridor(shrineCenter, upRampPos());
    std::vector<coord> nodes;
    nodes.emplace_back(upRampPos());
    for (int i=0; i< 5; ++i) nodes.emplace_back(mid(addRoom()));
    for (auto i = nodes.begin(), j =i+1; j < nodes.end(); ++i, ++j)
      addCorridor(*i,*j);

    place(shrineCenter.away(c), terrainType::ALTAR);
    place(coord(29,20), terrainType::ALTAR);

    place(upRampPos(), terrainType::UP);
    place(downRampPos(), terrainType::DOWN);

    // Some monsters (no demons in the temple please).
    std::vector<std::pair<coord, coord>> vec({{templeTopLeft, templeBtmRight},
	  {shrineTopLeft, shrineBotRight}});
    auto ms = addMonsters(vec, [](const monsterType *t) {return t->renderChar() != L'&';});
  }
};

template <>
class crusadeQuestLevelGen<true> : public levelGen {
private:
  questImpl &q_;
  const deity &pcAlign_;
public:
  crusadeQuestLevelGen(questImpl &q, levelImpl &li, level &l, const deity & pcAlign) :
    levelGen(&li,l), q_(q), pcAlign_(pcAlign) {}
  virtual ~crusadeQuestLevelGen() {}
  virtual coord upRampPos() { return coord(35,10); }
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::ROCK);

    deity &opp= deityRepo::instance().getOpposed(pcAlign_);
    
    std::vector<std::pair<coord,coord>> rooms({
    // main area
	{{20,5},{49,14}},
    // some rooms
	{{3,1},{9,3}},
	{{11,1},{17,3}},
	{{19,1},{25,3}},
	{{27,1},{33,3}},
	{{35,1},{41,3}},
	{{43,1},{49,3}},
	{{51,1},{68,2}},{{51,3},{60,3}},

        {{1,5},{7,7}},
        {{9,5},{16,7}},
        {{61,4},{69,11}},
        {{61,12},{69,18}},
        {{1,9},{3,16}},
      });

    for (auto &p : rooms)
      for (int x = p.first.first; x <= p.second.first; ++x)
	for (int y = p.first.second; y <= p.second.second; ++y)
	  place(coord(x,y), terrainType::GROUND);
    auto monsters =
      addMonsters(rooms, [](const monsterType *t) {return t->renderChar() != L'&';});
    for (auto pM : monsters)
      pM->align(opp);
    
    // corridors
    rect(17,5,20,5);
    rect(16,6,18,6);
    rect(50,5,53,5);
    rect(53,6,61,6);
    rect(4,12,16,12);
    rect(16,13,18,13);
    rect(18,14,19,14);
    rect(50,14,53,14);
    rect(53,13,61,13);

    // doors
    place(coord(18,2), terrainType::GROUND);
    place(coord(50,2), terrainType::GROUND);
    place(coord(5,4), terrainType::GROUND);
    place(coord(22,4), terrainType::GROUND);
    place(coord(30,4), terrainType::GROUND);
    place(coord(38,4), terrainType::GROUND);
    place(coord(46,4), terrainType::GROUND);
    place(coord(8,6), terrainType::GROUND);
    place(coord(11,68), terrainType::GROUND);

   
    //shrine zone on whole level except up staircase; opposite alignment to player
    addShrine(shape::subtract(rectarea::create(0,0,69,19),
			      rectarea::create(upRampPos(), upRampPos())),
	      opp);
    

    // antipriest
    monsterBuilder mb(false);
    mb.startOn(pub_);
    mb.type(monsterTypeRepo::instance()[monsterTypeKey::human]);
    mb.alignAgainst(pcAlign_);
    mb.highlight();
    mb.progress(100);
    auto &m = addMonster(mb, coord(1,10));
    // quest is completed when the player has their original alignment,
    // and the anti-priest is converted to the same alignment.
    m.eachTick([this, &m]{
	if (m.align() == pcAlign_ &&
	    m.curLevel().dung().pc()->align() == pcAlign_) {
	  q_.complete();
	}
      });
    
    place(upRampPos(), terrainType::UP);
  }
private:
  void rect(int x1, int y1, int x2, int y2, terrainType t = terrainType::GROUND) {
    for (int x = x1; x <= x2; ++x)
      for (int y = y1; y <= y2; ++y)
	place(coord(x,y), t);
  }
};

/*********************************************************************
***       *       *       *       *       *       *                  *1
***       *               *       *       *                          *2
***       *       *       *       *       *       *          *********3
***** **************** ******* ******* ******* ***************       *4
*       *       *                                     ********       *5
*                  *                              ***                *6
*       *       ****                              ************       *7
********************                              ************       *8
*   ****************                              ************       *9
*   ****************               <              ************       *10
*   ****************                              ****************** *11
*   ****************                              ************       *
*                ***                              ************       *
*   ************   *                              ***                *
*   **************                                    ********       *
**************************************************************       *
**************************************************************       *
**************************************************************       *
*********************************************************************/

std::vector<quest> questsForRole(const deity & pcAlign, roleType t) {
  std::vector<quest> rtn;
  switch (t) {
  case roleType::warrior:
    rtn.emplace_back(new questImpl
      (L"Slay the Greatest Dragon",
       L"While there are many dragons in the dungeon, your nemesis lies in the deep.",
       L"The gretest dragon remains unslain. Are you sure?",
       L"You slew the greatest dragon!",
       100,
       [](questImpl &qI, levelImpl &li, level &l) { return new warriorQuestLevelGen(qI, li,l); },
       [](questImpl &qI, levelGen &lg, level &l, int depth) {}
       ));
    break;
  case roleType::thief:
    rtn.emplace_back(new questImpl
      (L"Steal the Oppenheimer Blue",
       L"Amongst the traps in the deepest areas of adventure lies this mysterious\n"
       "emerald-cut diamond. It is well guarded; will it be yours?",
       L"The African treasure lies entombed; really leave it be?",
       L"This is the stone you've been looking for.",
       99,
       [](questImpl &qI, levelImpl &li, level &l) { return new thiefQuestLevelGen(qI, li,l); },
       [](questImpl &qI, levelGen &lg, level &l, int depth) {
	 // reveal all hidden pit traps
	 std::vector<coord> cs = l.findAllTerrain(terrainType::PIT_HIDDEN);
	 for (coord c : cs)
	   l.changeTerrain(c, terrainType::PIT);
	 for (int i = 0 ; i < depth; ++i) {
	   coord c(xPosD(generator), yPosD(generator));
	   if (l.terrainAt(c).type() == terrainType::GROUND)
	     l.changeTerrain(c, terrainType::PIT);
	   else {
	     c = coord(xPosD(generator), yPosD(generator));
	     if (l.terrainAt(c).type() == terrainType::GROUND)
	       l.changeTerrain(c, terrainType::PIANO_HIDDEN);
	   }
	 }
       }));
    break;
  case roleType::crusader:
    rtn.emplace_back(new questImpl
      (L"Convert the anti-priest",
       L"In the deepest area of adventures, protected by a temple you cannot enter,\n"
       "dwells the mortal representative of your deity's nemesis. Slay not this\n"
       "priest, but explain the error of their ways.\n",
       L"Really abandon the unbeliever still walks the wrong path?",
       L"The priest joins you on your path.",
       100,
       [&pcAlign](questImpl &qI, levelImpl &li, level &l) { return new crusadeQuestLevelGen<true>(qI, li, l, pcAlign); },
       [](questImpl &qI, levelGen &lg, level &l, int depth) {
       }));
    rtn.emplace_back(new questImpl
      (L"Retrieve the stolen artifact",
       L"In a temple you cannot enter lies a stolen idol aligned to your the path of " + pcAlign.name() + L"\n"
       "Retrive it so it can be returned to your own temple.",
       L"Really abandon the stolen idol?",
       L"You have retrieved the idol.",
       99,
       [&pcAlign](questImpl &qI, levelImpl &li, level &l) { return new crusadeQuestLevelGen<false>(qI, li, l, pcAlign); },
       [](questImpl &qI, levelGen &lg, level &l, int depth) {
       }));       
  break;
  case roleType::shopkeeper:
    rtn.emplace_back(new questImpl
      (L"Positive Trade",       
       L"Find the greatest treasure in the deep, then sell it to the one who needs it\n"
       "most",
       L"Do you really want to lose the chance of making the greatest sale?",
       L"You sold the grail to the Dungeoneer.",
       100,
       [](questImpl &qI, levelImpl &li, level &l) { return new shopQuestLevelGen(qI, li,l); },
       [](questImpl &qI, levelGen &lg, level &l, int depth) {
	 // make sure there's at least one dungeoneer to get the quest item:
	 if (depth == 1) {lg.addMonster(monsterTypeKey::dungeoneer, l.findTerrain(terrainType::DOWN)); }
	 // any dungeoneer can receive the quest item:
	 l.forEachMonster([&qI](monster &m) {
	     if (m.type().type() == monsterTypeKey::dungeoneer)
	       m.eachTick([&qI,&m]{
		   m.forEachItem([&qI](const item &i, std::wstring) {
		       if (!qI.isSuccessful() && i.highlight() && i.name().find(L"Grail",0) >= 0)
			 qI.complete();
		     });
		 });
	   });
       }));
    break;
  default: throw t;
  };
  return rtn;
}
