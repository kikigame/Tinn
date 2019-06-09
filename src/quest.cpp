/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in future.

#include "quest.hpp"
#include "player.hpp"
#include "levelGen.hpp"
#include "output.hpp"
#include "action.hpp"
#include "items.hpp"
#include "random.hpp"

std::uniform_int_distribution<int> xPosD(0,level::MAX_WIDTH - 1);
std::uniform_int_distribution<int> yPosD(0,level::MAX_HEIGHT - 1);


class questImpl {
private:
  const wchar_t * const name_;
  const wchar_t * const questData_;
  const wchar_t * const incompletePrompt_;
  const wchar_t * const completeMsg_;
  const int questLevel_;
  bool isComplete_;
  const std::function<levelGen*(questImpl &, levelImpl &, level &)> lg_;
  const std::function<void(questImpl &, levelGen &, level &, int)> ls_;
  const std::function<void(player &)> ps_;
public:
  questImpl(const wchar_t* const name,
	    const wchar_t* const questData,
	    const wchar_t* const incompletePrompt,
	    const wchar_t* const completeMsg,
	    int questLevel,
	    // quest level generator:
	    const std::function<levelGen*(questImpl &, levelImpl &, level &)> lg,
	    // level setup filter
	    const std::function<void(questImpl &, levelGen &, level &, int)> ls,
	    // player equipment generator:
	    const std::function<void(player &)> ps)
    : name_(name), questData_(questData), incompletePrompt_(incompletePrompt),
      completeMsg_(completeMsg),
      questLevel_(questLevel), isComplete_(false), lg_(lg), ls_(ls), ps_(ps) {}
  questImpl(const questImpl &) = delete;
  questImpl(questImpl &&) = delete;
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const questData() const {
    return questData_;
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
  void setupPlayer(player &p) { ps_(p); }
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
void quest::setupPlayer(player &p) { return pImpl_->setupPlayer(p); }
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


std::vector<quest> questsForRole(roleType t) {
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
       [](questImpl &qI, levelGen &lg, level &l, int depth) {},
       [](player &p) {
	 for (auto k : std::array<itemTypeKey, 5>{{
	       itemTypeKey::mace, itemTypeKey::jerkin, itemTypeKey::underpants,
		 itemTypeKey::boots, itemTypeKey::shirt}}) {
	   auto &it = createItem(k);
	   p.addItem(it);
	   it.equip(p);
	 }
       }
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
	 try {
	   while (true) {
	     coord c = l.findTerrain(terrainType::PIT_HIDDEN);
	     l.changeTerrain(c, terrainType::PIT);
	   }
	 } catch (std::wstring) {} // no more; just break loop	 
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
       },
       [](player &p) {
	 // thief should be less tough. but faster
	 p.intrinsics()->speedy(true);
	 p.dodge().bonus(5);
	 p.strength().cripple(10);
	 p.fighting().cripple(10);
	 // equip the player
	 for (auto k : std::array<itemTypeKey, 8>{{
	       itemTypeKey::sling, itemTypeKey::gloves, itemTypeKey::underpants,
		 itemTypeKey::boots, itemTypeKey::shirt, itemTypeKey::shorts,
		 itemTypeKey::socks, itemTypeKey::wooden_ring}}) {
	   auto &it = createItem(k);
	   p.addItem(it);
	   it.equip(p);
	 }
	 p.addItem(createItem(itemTypeKey::pie));
	 p.addItem(createItem(itemTypeKey::fizzy_pop));
	 for (int i=0; i < 10; ++i)
	   p.addItem(createItem(itemTypeKey::rock));
	 p.addItem(createItem(itemTypeKey::bottling_kit));
	 p.addItem(createItem(itemTypeKey::lyre));
	 auto &wand = createItem(itemTypeKey::stick);
	 p.addItem(wand);
	 wand.enchant(5);
       }
       ));       
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
       },
       [](player &p) {
	 p.addItem(createItem(itemTypeKey::apple));
	 p.addItem(createItem(itemTypeKey::shop_card));
	 p.addItem(createItem(itemTypeKey::hitch_guide));
	 auto &bow = createItem(itemTypeKey::bow);
	 bow.sexUp(true);
	 p.addItem(bow);
	 bow.equip(p);
       }
       ));
    break;
  default: throw t;
  };
  return rtn;
}
