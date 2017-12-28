/* License and copyright go here*/

#include "action.hpp"
#include "monster.hpp"
#include "items.hpp"
#include "output.hpp"
#include "random.hpp"
#include <map>

// source steals an item from target
class steal : public renderedAction<monster, monster> {
private:
  std::function<bool(item &)> f_;
public:
  steal(const wchar_t * const name, const wchar_t * const description, std::function<bool(item &)> f) : 
    renderedAction(name, description), f_(f) {};
  virtual ~steal() {};
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &io = ioFactory::instance();
    auto pitem = target.firstItem(f_);
    if (pitem) {
      auto &item = pitem.value();
      if (source.addItem(item)) {
	if (source.isPlayer())
	  io.message(std::wstring(L"You take the ") + item.name() + L" from " + target.name());
	else if (target.isPlayer())
	  io.message(source.name() + std::wstring(L" now has your ") + item.name());
	else
	  io.message(std::wstring(L"Something passes from ") + target.name() + L" to " + source.name());
	return true;
      } else {
	if (source.isPlayer())
	  io.message(std::wstring(L"You can't take the ") + item.name() + L" from " + target.name());
	else if (target.isPlayer())
	  io.message(source.name() + std::wstring(L" can't get your ") + item.name());
	else
	  io.message(std::wstring(L"Something doesn't pass from ") + target.name() + L" to " + source.name());
      }
    } else {
      if (source.isPlayer())
	io.message(std::wstring(L"You can't take anything from ") + target.name());
      else if (target.isPlayer())
	io.message(source.name() + std::wstring(L" can't get anything of yours!"));
      else
	io.message(std::wstring(L"Nothing passes from ") + target.name() + L" to " + source.name());
    }
    return false;
  }
};

// basic weapon damage action
template<unsigned char damagePc>
class attackRay : public renderedAction<monster, monster> {
private:
  damageType damageType_;
public:
  attackRay(const wchar_t * const name, const wchar_t * const description, damageType type) :
    renderedAction(name, description), damageType_(type) {};
  virtual ~attackRay() {};
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &d = damageRepo::instance()[damageType_];
    auto name = std::wstring(target.name());
    int damage = target.wound(damagePc, d);
    auto &io = ioFactory::instance();
    bool rtn = damage > 0;
    if (!rtn)
      io.message(std::wstring(L"The " + std::wstring(d.name()) + L" ray fires harmlessly as " + name));
    else 
      io.message(name + L" takes " + d.name() + L" damage.");
    return rtn;
  }
};

// swap places with the given monster's location. All monsters and items on squares are moved.
class exchangeAction : public renderedAction<monster, monster> {
public:
  exchangeAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~exchangeAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &sLevel = source.curLevel();
    auto sPos = sLevel.posOf(source);

    auto &tLevel = target.curLevel();
    auto tPos = tLevel.posOf(target);

    if (&sLevel == &tLevel && sPos == tPos)
      return false;

    auto &sHolder = sLevel.holder(sPos);
    auto &tHolder = tLevel.holder(tPos);

    // swap items:
    std::vector<item *> sItems;
    sHolder.forEachItem([&sItems](item &it, std::wstring) { sItems.push_back(&it); });
    tHolder.forEachItem([&sHolder](item &it, std::wstring) { sHolder.addItem(it); });
    for (auto pIt : sItems) tLevel.holder(tPos).addItem(*pIt);

    // swap monsters:
    auto iSMonsters = sLevel.monstersAt(sPos);
    std::vector<std::pair<coord, std::shared_ptr<monster> > > sMonsters(iSMonsters.first, iSMonsters.second);
    auto iTMonsters = sLevel.monstersAt(sLevel.posOf(source));
    for (auto pM = iTMonsters.first; pM != iTMonsters.second; ++pM)
      sLevel.moveTo(*(pM->second), sPos);
    for (auto pM : sMonsters)
      tLevel.moveTo(*(pM.second), tPos);

    if (source.isPlayer() || target.isPlayer()) // commutable action
      ioFactory::instance().message(L"Things seem to switch around.");

    return true;
  }
};

// forcably cause the target monster to consume some sprouts
class forceHealAction : public renderedAction<monster, monster> {
public:
  forceHealAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~forceHealAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &sprouts = createItem(itemTypeKey::apple);
    source.addItem(sprouts);
    return target.eat(sprouts, true);
  }
};

// do something funny, and possibly slightly inconvenient, to the target
class comedyAction : public renderedAction<monster, monster> {
public:
  comedyAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~comedyAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    // perform the first applicable effect:
    // - trousers fall down (deequip & become dropped)
    // - primary weapon turns to flowers (50% chance sexy, not normally equippable)
    // - fall over (1% damage, stun for 1 turn)
    // - summon another monster to chase them
    // - cream pie in face to blind them (ref:Nethack, keystone kops)
    // - levitation (ref: Mary Poppins)
    // - "there's glitter everywhere". Add glittery adjective to corpse.
    throw "Not implemented TODO!";
  }
};

// make the target monster sad
class tragedyAction : public renderedAction<monster, monster> {
public:
  tragedyAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~tragedyAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    // monster becomes sad.
    // - reduce attack by 20% of current value (rounded down)
    // + reduce movement rate by 1 slot
    throw "Not implemented TODO!";
  }
};

// move the target as far as possible (manhatten distance) from the source, 
// leaving it on a movable, untrapped square
class teleportAwayAction : public renderedAction<monster, monster> {
public:
  teleportAwayAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~teleportAwayAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto sPos = source.curLevel().posOf(source);
    auto &level = target.curLevel();
    auto tPos = level.posOf(target);
    int maxDist=0;
    coord dPos;
    for (int x=0; x < level::MAX_WIDTH; ++x)
      for (int y=0; y < level::MAX_HEIGHT; ++y) {
	coord pos(x,y);
	if (!level.movable(pos, target, true, true)) continue;
	int dist = std::abs(tPos.first-pos.first)
	         + std::abs(tPos.second-pos.second);
	if (dist > maxDist) {
	  maxDist = dist; dPos = pos;
	}
      }
    if (maxDist > 0)
      level.moveTo(target, dPos);
    // no message
    return level.posOf(target) == dPos;
  }
};

// create a prison on the level, putting the target monster in it
// may fail if insufficient space to create a prison
class lockAwayAction : public renderedAction<monster, monster> {
public:
  lockAwayAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~lockAwayAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    // Look for a 3x3 solid ground
    try {
      auto &l = target.curLevel();
      l.moveTo(target, l.createPrison());
    } catch (std::wstring x) {
      // no pos; give up
      return false;
    }
  }
};

// repulse target by (movement speed + 1 square away from player, stopping at impassible)
// if blessed, will move into traps; if cursed, will move 1 fewer squares
class repulsionAction : public renderedAction<monster, monster> {
public:
  repulsionAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~repulsionAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &tLevel = target.curLevel();
    auto tSpeed = target.movement().speed_;
    const int maxDist = (cursed ? 0 : 1) + static_cast<int>(tSpeed);
    coord sPos = source.curLevel().posOf(source);
    coord tPos = tLevel.posOf(target);
    coord pos = tPos;
    if (maxDist == 0) return false;
    for (int i=0; i < maxDist; ++i) {
      pos = tPos.away(sPos);
      if (!tLevel.movable(pos,target,blessed,blessed))
	break;
      else tPos = pos;
    }
    tLevel.moveTo(target, tPos);
    return true;
  }
};

// foocubi attacks
template <bool incubus>
class foocubusAction : public sharedAction<monster, monster> {
public:
  virtual ~foocubusAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target);
};


// actions to promote/demote an item attribute based on a monster
class monsterItemAction : public renderedAction<monster, monster> {
private:
  std::function<bool(item &)> filter_;
  std::function<void(item &)> apply_;
public:
  monsterItemAction(const wchar_t * const name, const wchar_t * const description,
		  std::function<bool(item &)> filter, std::function<void(item &)> apply) :
    renderedAction(name, description),
    filter_(filter), apply_(apply) {}
  virtual ~monsterItemAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto t = target.firstItem(filter_);
    if (!t) return false;
    apply_(t.value());
    return true;
  }
};

// defined in shop.cpp:
void popUpShop(monster & from, monster &to);

class popupShopAction : public renderedAction<monster, monster> {
public:
  popupShopAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~popupShopAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    if (!source.isPlayer()) return false; // TODO: Monsters demand to shop with the player!
    popUpShop(source,target);
    return true;
  }  
};

class petrifyAction :  public renderedAction<monster, monster> {
public:
  petrifyAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~petrifyAction() {}
  bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &a = target.abilities();
    if (a.fearless() == bonus(true))
      return false; // monster is not susceptible
    int turns = 2;
    if (target.abilities().fearless() == bonus(false)) turns *= 2;
    if (blessed) turns += (turns / 2);
    if (cursed) turns /=2;
    ioFactory::instance().message(target.name() + std::wstring(L" is scared stiff!"));
    a.entrap(turns);
    return true; // even cursed is always 1 turn
  }
};

template<>
class actionFactory<monster, monster> {
public:
  typedef sharedAction<monster, monster> action;
  static action &get(const typename action::key k) {
    static std::map<action::key, std::shared_ptr<action> > m{
      std::make_pair(action::key::steal_small, std::shared_ptr<action>(new steal(L"pickpocketing",
L"Pickpocketing is an act of petty larceny.\n"
"It does not require actual pockets; indeed, objects placed in containers may\n"
"evade the scope of this activity, as are those which are too large - say,\n"
"bigger than an apple.",
[](item &i) { return i.weight() <= 1; }))),
      std::make_pair(action::key::steal_shiny, std::shared_ptr<action>(new steal(L"petty larcony",
L"Larcony - or theft - is the act of taking items from another for personal\n"
"gain. This applies to jewellery and other valuables.",
[](item &i) { return i.render() == L'*' || i.render() == L'$'; }))),
	std::make_pair(action::key::attack_ray_small_edged, std::shared_ptr<action>(new attackRay<5>(L"cutting jet",
L"A cutting jet will cause many small lacerations on its target.\n",
damageType::edged))),
	std::make_pair(action::key::attack_ray_small_bashing, std::shared_ptr<action>(new attackRay<5>(L"clobber jet",
L"A clobber jet will pummel a target with small stones.\n",
damageType::bashing))),
	std::make_pair(action::key::attack_ray_small_hot, std::shared_ptr<action>(new attackRay<5>(L"heat ray",
L"A heat ray will burn a target with a directed plasma burn.\n",
damageType::hot))),
	std::make_pair(action::key::attack_ray_small_cold, std::shared_ptr<action>(new attackRay<5>(L"freeze ray", //ref: Despicable Me (film)
L"A freeze ray will remove some of the heat enercy from the target.\n",
damageType::cold))),
	std::make_pair(action::key::attack_ray_small_water, std::shared_ptr<action>(new attackRay<5>(L"water jet",
L"A water jet will squirt a target with a small amount of water.\n",
damageType::wet))),
	std::make_pair(action::key::attack_ray_small_sonic, std::shared_ptr<action>(new attackRay<5>(L"sonic ray",
L"A sound ray can vibrate the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic))),
	std::make_pair(action::key::attack_ray_small_disintegration, std::shared_ptr<action>(new attackRay<5>(L"disintegration ray",
L"A disintegration ray will age a small part of its target, often with\n"
"damaging effect.\n",
damageType::disintegration))),
	std::make_pair(action::key::attack_ray_small_starvation, std::shared_ptr<action>(new attackRay<5>(L"starvation ray",
L"A starvation ray saps the goodness out of the food powering your target.\n",
damageType::starvation))),
	std::make_pair(action::key::attack_ray_small_electric, std::shared_ptr<action>(new attackRay<5>(L"zapper",
L"A zapper fires small lightning bolts at your victim.\n",
damageType::electric))),

	std::make_pair(action::key::attack_ray_med_edged, std::shared_ptr<action>(new attackRay<20>(L"cutting beam",
L"A cutting beam will cause many lacerations on its target.\n",
damageType::edged))),
	std::make_pair(action::key::attack_ray_med_bashing, std::shared_ptr<action>(new attackRay<20>(L"clobber beam",
L"A clobber beam will pummel a target with stones.\n",
damageType::bashing))),
	std::make_pair(action::key::attack_ray_med_hot, std::shared_ptr<action>(new attackRay<20>(L"heat beam",
L"A heat beam will burn a target with a directed plasma blast.\n",
damageType::hot))),
	std::make_pair(action::key::attack_ray_med_cold, std::shared_ptr<action>(new attackRay<20>(L"ice beam", 
L"An ice beam will remove much of the heat enercy from the target.\n",
damageType::cold))),
	std::make_pair(action::key::attack_ray_med_water, std::shared_ptr<action>(new attackRay<20>(L"water power-jet",
L"A water jet will squirt a target with water.\n",
damageType::wet))),
	std::make_pair(action::key::attack_ray_med_sonic, std::shared_ptr<action>(new attackRay<20>(L"sonic beam",
L"A sound ray can rattle the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic))),
	std::make_pair(action::key::attack_ray_med_disintegration, std::shared_ptr<action>(new attackRay<20>(L"disintegration beam",
L"A disintegration beam will age a part of its target, often with very\n"
"damaging effect.\n",
damageType::disintegration))),
	std::make_pair(action::key::attack_ray_med_starvation, std::shared_ptr<action>(new attackRay<20>(L"starvation beam",
L"A starvation beam saps food powering your target.\n",
damageType::starvation))),
	std::make_pair(action::key::attack_ray_med_electric, std::shared_ptr<action>(new attackRay<20>(L"zappier",
L"A zappier fires lightning bolts at your victim.\n",
damageType::electric))),

	std::make_pair(action::key::death_ray_edged, std::shared_ptr<action>(new attackRay<100>(L"cutting death ray",
L"A cutting death ray achieves mortal damage by lacerations.\n",
damageType::edged))),
	std::make_pair(action::key::death_ray_bashing, std::shared_ptr<action>(new attackRay<100>(L"clobbering death ray",
L"A form of death ray involving mortally beating one's target.\n",
damageType::bashing))),
	std::make_pair(action::key::death_ray_hot, std::shared_ptr<action>(new attackRay<100>(L"ray of consuming fire",
L"A mortal directed plasma blast.\n",
damageType::hot))),
	std::make_pair(action::key::death_ray_cold, std::shared_ptr<action>(new attackRay<100>(L"death by absolution of void", 
L"Removes all heat enercy from the target, vital for most forms of life.\n",
damageType::cold))),
	std::make_pair(action::key::death_ray_water, std::shared_ptr<action>(new attackRay<100>(L"drowning jet",
L"A drowning jet will wholly envelop a target with water.\n",
damageType::wet))),
	std::make_pair(action::key::death_ray_sonic, std::shared_ptr<action>(new attackRay<100>(L"sonic death ray",
L"A sound ray to rattle apart the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic))),
	std::make_pair(action::key::death_ray_disintegration, std::shared_ptr<action>(new attackRay<100>(L"time-eater ray",
L"A time-eater ray will age its target beyond its mortal life-span.\n",
damageType::disintegration))),
	std::make_pair(action::key::death_ray_starvation, std::shared_ptr<action>(new attackRay<100>(L"starvation death ray",
L"A starvation beam saps all food powering your target.\n",
damageType::starvation))),
	std::make_pair(action::key::death_ray_electric, std::shared_ptr<action>(new attackRay<100>(L"lightning of the mortal storm",
L"Few, if any, can survive the lightning of a storm.\n",
damageType::electric))),

	std::make_pair(action::key::exchange_ray, std::shared_ptr<action>(new exchangeAction(L"exchange ray",
L"Useful for getting out of a scrape, especially when trapped.\n"
"The effect of an exchange ray is to swap places with everyone and everything\n"
"at the target's location."))),
	std::make_pair(action::key::heal_ray_veggie, std::shared_ptr<action>(new forceHealAction(L"Vegan food ray",
L"Force-feeds a few sprouts. This can be a life-saver, but is also unpleasant."))),
	/*	std::make_pair(action::key::comedy, std::shared_ptr<action>(new comedyAction(L"comedy",
L"Comedy and Tragedy are two sides of the same coin. Neither is predictable,\n"
"but like all social commentary, both can be effective when used carefully."))),
	std::make_pair(action::key::comedy, std::shared_ptr<action>(new tragedyAction(L"tragedy",
L"Comedy and Tragedy are two sides of the same coin. Neither is predictable,\n"
"but like all social commentary, both can be effective when used carefully."))),
	*/
	std::make_pair(action::key::teleport_away, std::shared_ptr<action>(new teleportAwayAction(L"curse",
L"Sometimes you just need to put some distance between yourself and an\n"
"adversary. Teleport Away causes the target to move as far away from you as\n"
"possible (but in a line)."))),
	std::make_pair(action::key::lock_away, std::shared_ptr<action>(new teleportAwayAction(L"lock away",
L"Designed for use against criminals, locking a monster away is a very safe\n"
"escape plan. It works less well on those creatures that are at home within\n"
"the earth"))),
	std::make_pair(action::key::repulsion, std::shared_ptr<action>(new teleportAwayAction(L"repulsion",
L"Some magical effects move the enemy away from you. Repulsion usually just\n"
"buys you a little time, but it can be used to good effect if you time it\n"
"right and get your enemy where they don't want to be."))),

	std::make_pair(action::key::curse_item, std::shared_ptr<action>(new monsterItemAction(L"curse",
L"A curse can prevent items from being dropped as well as reducing their effectiveness.",
[](const item &i) {return !i.isCursed();}, [](item &i) {i.curse(true);}))),
	std::make_pair(action::key::uncurse_item, std::shared_ptr<action>(new monsterItemAction(L"remove curse",
L"Curses on items can be difficult to remove, but a specialist tool can sometimes be effective.",
[](const item &i) {return i.isCursed();}, [](item &i) {i.curse(false);}))),
	std::make_pair(action::key::bless_item, std::shared_ptr<action>(new monsterItemAction(L"bless",
L"A blessing can increase the effectiveness or utility of an item.",
[](const item &i) {return !i.isBlessed();}, [](item &i) {i.bless(true);}))),
	std::make_pair(action::key::unbless_item, std::shared_ptr<action>(new monsterItemAction(L"remove blessing",
L"Blessings on items can be difficult to remove, but a specialist tool can sometimes be effective.",
[](const item &i) {return i.isBlessed();}, [](item &i) {i.bless(false);}))),
	std::make_pair(action::key::enchant_item, std::shared_ptr<action>(new monsterItemAction(L"enchantment",
L"An active item can be enchanted to reveal or recover its effect.\n" 
"A weapon or armour can be enchanted to increase its effectiveness.",
[](const item &i) {return true;}, [](item &i) {i.enchant(dPc() / 20);}))), //TODO: should this match shop's blessed/cursed rules?
	std::make_pair(action::key::disenchant_item, std::shared_ptr<action>(new monsterItemAction(L"disenchantment",
L"An active item can be disenchanted to hide or limit its effect.\n"
"A weapon or armour can be disenchanted to reduce its effectiveness.",
[](const item &i) {return i.enchantment() > 0;}, [](item &i) {i.enchant(-dPc() / 20);}))),
	std::make_pair(action::key::sex_up_item, std::shared_ptr<action>(new monsterItemAction(L"sexiness",
L"Sexy items will increase your physical appearance when visibly worn.\n",
[](const item &i) {return !i.isSexy() && i.equippable() == item::equipType::worn;}, [](item &i) {i.sexUp(true);}))),
	std::make_pair(action::key::sex_down_item, std::shared_ptr<action>(new monsterItemAction(L"chastity",
L"Unsexy items do nothing for your physical appearance, meaning you can wear\n"
"them without affecting your appearance percentage.",
[](const item &i) {return i.isSexy();}, [](item &i) {i.sexUp(false);}))),
	std::make_pair(action::key::nudity, std::shared_ptr<action>(new monsterItemAction(L"nudity",
L"Clothing and armour can provide a number of benifits, to defensibility and\n"
"also to outward appearance. Some can also provide less helpful effects.\n"
"The main tenet of nudism; however, suggests that nothing can compare to the\n"
"simple effect of feeling the forces of the world directly on your bare skin.",
[](const item &i) {return i.equippable() == item::equipType::worn && 
                   dynamic_cast<monster&>(i.holder()).slotsOf(i)[0] != nullptr;}, 
[](item &i) {return dynamic_cast<monster&>(i.holder()).unequip(i);}))),
	std::make_pair(action::key::disarm, std::shared_ptr<action>(new monsterItemAction(L"disarm",
L"Weapons can be used to intimidate, attack, harm or destroy an opponent.\n"
"Sometimes; however, a more inclusive, open-handed (open-clawed,\n"
"open-beaked) approach can be effective.\n"
"This power will not assist against an opponent who uses its own body as a\n"
"weapon.",
[](const item &i) {return i.equippable() == item::equipType::wielded && 
                   dynamic_cast<monster&>(i.holder()).slotsOf(i)[0] != nullptr;}, 
[](item &i) {return dynamic_cast<monster&>(i.holder()).unequip(i);}))),
	std::make_pair(action::key::popup_shop, std::shared_ptr<action>(new popupShopAction(L"shopping",
L"A pop-up shop is a small retail event lasting for a short length of time."))),
	std::make_pair(action::key::petrify, std::shared_ptr<action>(new petrifyAction(L"petrify",
L"The action of petrification is to cause someone to become stiff or like\n"
"stone, to deaden in fear. Some monsters are unaffected by fear, and some\n"
"will be affected for longer than others."))),
	};
    auto &rtn = m[k];
    return *rtn;
  };
};

void ignored() {
  actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::END);
}


// uses template, so define acter specialisation:
template<bool incubus>
bool foocubusAction<incubus>::operator () (bool blessed, bool cursed, monster &source, monster &target) {
    /*
     * Let's think about this. *cubi are very bad demons who basically rape people in their dreams.
     * BUT, we don't want to say that sex is bad. So, in homage to Nethack, and to make gender have
     * an in-game effect, we will say that there are 3 possible outcomes:
     * 1 target resists the *cubus's dreamy advances
     * 2 dream-sex happens and the *cubus wins, feeding off the life-force of its victim
     * 3 dream-sex happens and the target wins, causing positive benifits.
     *
     * Source & target have strength, appearance, fighting, dodge,
     * damage and 2xgender attributes.  Of these, appearance and
     * gender are the most applicable.  Since *cubi are heterosexual
     * in mythology, we'll use gender to determine 1 (if the *fucubus
     * in interested).
     * We differentiate 2-3 on appearance.
     */
    bool isProtected = incubus ? !target.isFemale() : !target.isMale();
    bool isCompatible = incubus ? target.isFemale() : target.isMale();
    auto &ios = ioFactory::instance();
      return false;
    if (!isProtected) {
      if (source.isPlayer()) ios.message(L"You aren't in the mood.");
      else ios.message(source.name() + std::wstring(L" doesn't seem to be in the mood."));
      return false;
    }
    if (isCompatible && dPc() < target.appearance().cur()) {
      // good outcome; consent happened.
      ios.message(target.isPlayer() ? L"You consent to the advances of the " + std::wstring(source.name())
		  : source.isPlayer() ? L"The " + std::wstring(target.name()) + L" consents to your advances..."
		  : L"The " + std::wstring(target.name()) + L" consents to the advances of the " + source.name() + L"..."
		  );
      // target receives a good outcome; source receives a moderatly good outcome
      target.strength() += 5; target.dodge() += 5;
      source.dodge() += 1;
      sharedAction<monster,monster> &act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::sex_up_item);
      act(false, false, source, target);
      (act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::uncurse_item))(false, false, source, target) ||
	(act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::bless_item))(false, false, source, target) ||
	(act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::enchant_item))(false, false, source, target);
    } else {
      // bad outcome
      ios.message(target.isPlayer() ? L"You are violated by the "  + std::wstring(source.name() + L'!')
		  : source.isPlayer() ? L"You feel bad about violating the " + std::wstring(target.name()) + L"..."
		  : L"The " + std::wstring(target.name()) + L" does not consents to the advances of the " + source.name() + L"..."
		  );
      // target receives a bad outcome; source receives a good outcome
      source.strength() += 5;
      target.dodge() -= 10; target.strength() -= 10;
      sharedAction<monster,monster> &act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::sex_up_item);
      act(false, false, target, source);
      (act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::uncurse_item))(false, false, target, source) ||
	(act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::bless_item))(false, false, target, source) ||
	(act = actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::enchant_item))(false, false, target, source);
    }
    return true;
}


sharedAction<monster, monster> &incubusAction() {
  static foocubusAction<true> incubusAction;
  return incubusAction;
}

sharedAction<monster, monster> &sucubusAction() {
  static foocubusAction<false> sucubusAction;
  return sucubusAction;
}


// TODO: area-effect actions
// wand of health & safety: reveals all traps on level, clears any damage, removes any cloak, dons safety glasses & doublet becomes a fluorescent jacket.
