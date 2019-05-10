/* License and copyright go here*/

#include "action.hpp"
#include "monster.hpp"
#include "items.hpp"
#include "output.hpp"
#include "random.hpp"
#include "terrain.hpp"
#include <map>

// source steals an item from target
class steal : public renderedAction<monster, monster> {
private:
  std::function<bool(item &)> f_;
public:
  steal(const wchar_t * const name, const wchar_t * const description, std::function<bool(item &)> f) : 
    renderedAction(name, description), f_(f) {};
  virtual ~steal() {};
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &io = ioFactory::instance();
    auto pitem = target.firstItem(f_);
    if (pitem) {
      auto &item = pitem.value();
      if (source.addItem(item)) {
	if (source.isPlayer())
	  io.message(L"You take the " + item.name() + L" from " + target.name());
	else if (target.isPlayer())
	  io.message(source.name() + L" now has your " + item.name());
	else
	  io.message(L"Something passes from " + target.name() + L" to " + source.name());
	return true;
      } else {
	if (source.isPlayer())
	  io.message(L"You can't take the " + item.name() + L" from " + target.name());
	else if (target.isPlayer())
	  io.message(source.name() + L" can't get your " + item.name());
	else
	  io.message(L"Something doesn't pass from " + target.name() + L" to " + source.name());
      }
    } else {
      if (source.isPlayer())
	io.message(L"You can't take anything from " + target.name());
      else if (target.isPlayer())
	io.message(source.name() + L" can't get anything of yours!");
      else
	io.message(L"Nothing passes from " + target.name() + L" to " + source.name());
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &d = damageRepo::instance()[damageType_];
    auto name = target.name();
    int damage = target.wound(source, damagePc, d);
    auto &io = ioFactory::instance();
    bool rtn = damage > 0;
    if (!rtn)
      io.message(L"The " + std::wstring(d.name()) + L" ray fires harmlessly at " + name);
    else 
      io.message(name + L" takes " + std::wstring(d.name()) + L" damage.");
    return rtn;
  }
};

// swap places with the given monster's location. All monsters and items on squares are moved.
class exchangeAction : public renderedAction<monster, monster> {
public:
  exchangeAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~exchangeAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
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
    auto sMonsters = sLevel.monstersAt(sPos);
    auto tMonsters = sLevel.monstersAt(sLevel.posOf(source));
    for (auto m : tMonsters)
      sLevel.moveTo(m.value(), sPos);
    for (auto m : sMonsters)
      tLevel.moveTo(m.value(), tPos);

    if (source.isPlayer() || target.isPlayer()) // commutable action
      ioFactory::instance().message(L"Things seem to switch around.");

    return true;
  }
};

// move the target towards the main
class attractAction : public renderedAction<monster, monster> {
public:
  attractAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~attractAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    char distance = 5;
    if (blessed) distance *= 2;
    if (cursed) distance /= 3;
    auto &sLevel = source.curLevel();
    auto &tLevel = target.curLevel();

    if (&sLevel != &tLevel) return false;
    auto sPos = tLevel.posOf(source);
    auto tPos = tLevel.posOf(target);
    bool rtn = false;
    for (; distance >= 0; --distance) {
      if (target.abilities().entrapped()) return false;
      coord cc = tPos;
      tPos = tPos.towards(sPos);
      if (tPos == sPos) return true;
      if (tLevel.movable(cc,tPos, target, false, false)) {
	tLevel.moveTo(target, tPos);
	rtn = true;
      }
      else break;
    }
    return rtn;
  }
};

// forcably cause the target monster to consume some sprouts
class forceHealAction : public renderedAction<monster, monster> {
public:
  forceHealAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~forceHealAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &io = ioFactory::instance();
    const bool onSelf = &source == &target;
    // perform the first applicable effect:
    // - trousers fall down (deequip & become dropped)
    optionalRef<item> trousers = target.inSlot(slotType::shorts);
    if (trousers) {
      target.unequip(trousers.value());
      std::wstring msg = L"Falling " + (trousers.value().name() + L"! Hahaha!");
      if (cursed && onSelf)
	trousers.value().sexUp(false);
      else
	target.drop(trousers.value());
      if (blessed && !onSelf) {
	trousers.value().strike(damageType::edged);
	msg += L" Rrrrippp!";
      }
      io.longMsg(msg);
      return true;
    }
    // - primary weapon turns to flowers (50% chance sexy, not normally equippable)
    auto weapon = target.inSlot(slotType::primary_weapon);
    if (weapon) {
      // TODO: flowers & polymorphing objects
      // no polymorphing objects yet, so let's just destroy it with a silly message:
      std::wstring name = weapon.value().name();
      if (weapon.value().destroy()) {
	auto pronoun = (&source == &target) ? L"Your " : L"Their ";
	io.longMsg(pronoun + name + L"\n\tbecomes a beautiful bouquet! Hahaha!");
	return true;
      }
    }
    // - fall over (1% damage, stun for 1 turn)
    auto speedy = target.abilities().speedy();
    if (speedy != bonus(false)) {
      target.intrinsics().speedy(false);
      if (target.abilities().speedy() != speedy) {
	target.fall(1);
	// TODO: stunning for limited time?
	std::wstring whowhat = (&source == &target) ? L"Your fall" : (target.name() + L" falls");
	io.longMsg(L"Whoops! " + whowhat + L" over! Hahaha!");
	return true;
      }
    }
    // - cream pie in face to blind them (ref:Nethack, keystone kops)
    if (target.abilities().see()) {
      target.intrinsics().see(false);
      if (!target.intrinsics().see()) {
	// TODO: equipped creampie of some sort? Too nethack to wipe off?
	std::wstring msg = (&source == &target) ? L"cream pie to the eyes! Hahaha!" :
	  (target.name() + L": cream pie to the eyes! Hahaha!");
	io.longMsg(msg);
	return true;
      }
    }
    // - levitation (ref: Mary Poppins)
    if (!target.abilities().fly()) {
      target.intrinsics().fly(true);
      if (target.abilities().fly()) {
	std::wstring msg = (&source == &target) ? (target.name() + L": come down at once! Hahaha!") :
	  L"Come down from there at once! Hahaha!";
	io.longMsg(msg);
	return true;
      }
    }
    // - "there's glitter everywhere". Add glittery adjective to monster/corpse.
    auto adj = target.adjectives();
    const auto gl = L"glittery";
    if (std::find(adj.begin(), adj.end(), gl) == adj.end()) {
      io.longMsg(L"there's glitter everywhere!");
      source.addDescriptor(gl);
      target.addDescriptor(gl);
      return true;
    }
    // - summon another monster to chase them
    level &l = target.curLevel();
    coord pos = l.posOf(target);
    monsterBuilder b(true);
    b.startOn(l);
    b.male(100 - target.male().cur());
    b.female(100 - target.male().cur());
    b.alignAgainst(target.align());
    auto &zombie = monsterTypeRepo::instance()[monsterTypeKey::zombie];
    b.type(zombie);
    auto mon = zombie.spawn(l, b);
    l.moveTo(*mon, pos); // TODO: Chase somehow?
    io.longMsg(L"Yakety Sax! Hahaha!"); // Ref: Benny Hill (in turn referencing Keystone Cops)
    return true;
  }
};

// make the target monster sad
class tragedyAction : public renderedAction<monster, monster> {
public:
  tragedyAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~tragedyAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto sPos = source.curLevel().posOf(source);
    auto &level = target.curLevel();
    auto tPos = level.posOf(target);
    int maxDist=0;
    coord dPos;
    for (int x=0; x < level::MAX_WIDTH; ++x)
      for (int y=0; y < level::MAX_HEIGHT; ++y) {
	coord pos(x,y);
	if (!level.movable(pos, pos, target, true, true)) continue;
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    // Look for a 3x3 solid ground
    try {
      auto &l = target.curLevel();
      l.moveTo(target, l.createPrison());
      return true;
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &tLevel = target.curLevel();
    auto tSpeed = target.movement().speed_;
    const int maxDist = (cursed ? 0 : 1) + static_cast<int>(tSpeed);
    coord sPos = source.curLevel().posOf(source);
    coord tPos = tLevel.posOf(target);
    coord pos = tPos;
    if (maxDist == 0) return false;
    for (int i=0; i < maxDist; ++i) {
      coord cc = tPos;
      pos = tPos.away(sPos);
      if (!tLevel.movable(cc,pos,target,blessed,blessed))
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target);
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
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
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    if (target.isPlayer()) {
      // Reverse shop; Monsters demand to shop with the player!
      auto &io = ioFactory::instance();
      if (source.size() == 0 && target.size() == 0) {
	io.longMsg(L"You trade insults with " + source.name());
      } else if (source.size() == 0) {
	io.longMsg(source.name() + L" can't barter with nothing to spend.");
      } else if (target.size() == 0) {
	io.longMsg(source.name() + L" tries to trade, but you can't pay!");
      } else {
	auto &sItem = source.firstItem([](item &i) { return true; }).value();
	auto &tItem = source.firstItem([](item &i) { return true; }).value();
	const auto tItemName = tItem.name();
	const auto sItemName = sItem.name();
	auto tOk = target.addItem(sItem);
	auto sOk = target.addItem(tItem);
	if (tOk && sOk) {
	  io.longMsg(L"You trade your " + tItemName + L" for the " + sItemName + L" of the " + source.name());
	} else if (sOk) {
	  io.longMsg(source.name() + L" seems disappointed to present: " + tItemName);
	} else if (tOk) {
	  io.longMsg(L"You seem disappointed to present to " + source.name() + L" your " + sItemName);
	} else {
	  io.longMsg(L"You are both disappointed not to trade.");
	}
      }
    }
    //      popUpShop(target, source);
    else if (source.isPlayer())
      popUpShop(source,target);
    else
      return false; // monsters don't bother shopping with each other. This would be a faff to write and not do anything for gameplay.
    return true;
  }  
};

class petrifyAction :  public renderedAction<monster, monster> {
public:
  petrifyAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~petrifyAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    auto &a = target.abilities();
    if (a.fearless() == bonus(true))
      return false; // monster is not susceptible
    int turns = 2;
    if (target.abilities().fearless() == bonus(false)) turns *= 2;
    if (blessed) turns += (turns / 2);
    if (cursed) turns /=2;
    ioFactory::instance().message(target.name() + L" is scared stiff!");
    a.entrap(turns);
    return true; // even cursed is always 1 turn
  }
};

class charmAction : public renderedAction<monster, monster> {
public:
  charmAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~charmAction() {}
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) {
    if (source.isPlayer() && target.isPlayer()) {
      ioFactory::instance().message(L"Charming!");
      return true;
    }
    if (!target.setCharmedBy(source)) return false;
    else if (source.isPlayer())
      ioFactory::instance().message(L"You charm the " + target.name());
    else if (target.isPlayer())
      ioFactory::instance().message(L"You find the " + source.name() + L" quite charming.");
    return true;
  }
};

template<>
class actionFactory<monster, monster> {
public:
  typedef sharedAction<monster, monster> action;
  static std::map<action::key, std::unique_ptr<action> > makeMap() {
    std::map<action::key, std::unique_ptr<action> > rtn;
    rtn[action::key::steal_small] = std::unique_ptr<action>(new steal(L"pickpocketing",
L"Pickpocketing is an act of petty larceny.\n"
"It does not require actual pockets; indeed, objects placed in containers may\n"
"evade the scope of this activity, as are those which are too large - say,\n"
"bigger than an apple.",
[](item &i) { return i.weight() <= 1; }));
      rtn[action::key::steal_shiny] = std::unique_ptr<action>(new steal(L"petty larcony",
L"Larcony - or theft - is the act of taking items from another for personal\n"
"gain. This applies to jewellery and other valuables.",
[](item &i) { return i.render() == L'*' || i.render() == L'$'; }));
	rtn[action::key::attack_ray_small_edged] = std::unique_ptr<action>(new attackRay<5>(L"cutting jet",
L"A cutting jet will cause many small lacerations on its target.\n",
damageType::edged));
	rtn[action::key::attack_ray_small_bashing] = std::unique_ptr<action>(new attackRay<5>(L"clobber jet",
L"A clobber jet will pummel a target with small stones.\n",
damageType::bashing));
	rtn[action::key::attack_ray_small_hot] = std::unique_ptr<action>(new attackRay<5>(L"heat ray",
L"A heat ray will burn a target with a directed plasma burn.\n",
damageType::hot));
	rtn[action::key::attack_ray_small_cold] = std::unique_ptr<action>(new attackRay<5>(L"freeze ray", //ref: Despicable Me (film)
L"A freeze ray will remove some of the heat enercy from the target.\n",
damageType::cold));
	rtn[action::key::attack_ray_small_water] = std::unique_ptr<action>(new attackRay<5>(L"water jet",
L"A water jet will squirt a target with a small amount of water.\n",
damageType::wet));
	rtn[action::key::attack_ray_small_sonic] = std::unique_ptr<action>(new attackRay<5>(L"sonic ray",
L"A sound ray can vibrate the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic));
	rtn[action::key::attack_ray_small_disintegration] = std::unique_ptr<action>(new attackRay<5>(L"disintegration ray",
L"A disintegration ray will age a small part of its target, often with\n"
"damaging effect.\n",
damageType::disintegration));
	rtn[action::key::attack_ray_small_starvation] = std::unique_ptr<action>(new attackRay<5>(L"starvation ray",
L"A starvation ray saps the goodness out of the food powering your target.\n",
damageType::starvation));
	rtn[action::key::attack_ray_small_electric] = std::unique_ptr<action>(new attackRay<5>(L"zapper",
L"A zapper fires small lightning bolts at your victim.\n",
damageType::electric));

	rtn[action::key::attack_ray_med_edged] = std::unique_ptr<action>(new attackRay<20>(L"cutting beam",
L"A cutting beam will cause many lacerations on its target.\n",
damageType::edged));
	rtn[action::key::attack_ray_med_bashing] = std::unique_ptr<action>(new attackRay<20>(L"clobber beam",
L"A clobber beam will pummel a target with stones.\n",
damageType::bashing));
	rtn[action::key::attack_ray_med_hot] = std::unique_ptr<action>(new attackRay<20>(L"heat beam",
L"A heat beam will burn a target with a directed plasma blast.\n",
damageType::hot));
	rtn[action::key::attack_ray_med_cold] = std::unique_ptr<action>(new attackRay<20>(L"ice beam", 
L"An ice beam will remove much of the heat enercy from the target.\n",
damageType::cold));
	rtn[action::key::attack_ray_med_water] = std::unique_ptr<action>(new attackRay<20>(L"water power-jet",
L"A water jet will squirt a target with water.\n",
damageType::wet));
	rtn[action::key::attack_ray_med_sonic] = std::unique_ptr<action>(new attackRay<20>(L"sonic beam",
L"A sound ray can rattle the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic));
	rtn[action::key::attack_ray_med_disintegration] = std::unique_ptr<action>(new attackRay<20>(L"disintegration beam",
L"A disintegration beam will age a part of its target, often with very\n"
"damaging effect.\n",
damageType::disintegration));
	rtn[action::key::attack_ray_med_starvation] = std::unique_ptr<action>(new attackRay<20>(L"starvation beam",
L"A starvation beam saps food powering your target.\n",
damageType::starvation));
	rtn[action::key::attack_ray_med_electric] = std::unique_ptr<action>(new attackRay<20>(L"zappier",
L"A zappier fires lightning bolts at your victim.\n",
damageType::electric));

	rtn[action::key::death_ray_edged] = std::unique_ptr<action>(new attackRay<100>(L"cutting death ray",
L"A cutting death ray achieves mortal damage by lacerations.\n",
damageType::edged));
	rtn[action::key::death_ray_bashing] = std::unique_ptr<action>(new attackRay<100>(L"clobbering death ray",
L"A form of death ray involving mortally beating one's target.\n",
damageType::bashing));
	rtn[action::key::death_ray_hot] = std::unique_ptr<action>(new attackRay<100>(L"ray of consuming fire",
L"A mortal directed plasma blast.\n",
damageType::hot));
	rtn[action::key::death_ray_cold] = std::unique_ptr<action>(new attackRay<100>(L"death by absolution of void", 
L"Removes all heat enercy from the target, vital for most forms of life.\n",
damageType::cold));
	rtn[action::key::death_ray_water] = std::unique_ptr<action>(new attackRay<100>(L"drowning jet",
L"A drowning jet will wholly envelop a target with water.\n",
damageType::wet));
	rtn[action::key::death_ray_sonic] = std::unique_ptr<action>(new attackRay<100>(L"sonic death ray",
L"A sound ray to rattle apart the bodily structure of your opponent.\n"
"This is not a screwdriver", // ref: Doctor Who (Sonic Screwdriver)
damageType::sonic));
	rtn[action::key::death_ray_disintegration] = std::unique_ptr<action>(new attackRay<100>(L"time-eater ray",
L"A time-eater ray will age its target beyond its mortal life-span.\n",
damageType::disintegration));
	rtn[action::key::death_ray_starvation] = std::unique_ptr<action>(new attackRay<100>(L"starvation death ray",
L"A starvation beam saps all food powering your target.\n",
damageType::starvation));
	rtn[action::key::death_ray_electric] = std::unique_ptr<action>(new attackRay<100>(L"lightning of the mortal storm",
L"Few, if any, can survive the lightning of a storm.\n",
damageType::electric));

	rtn[action::key::exchange_ray] = std::unique_ptr<action>(new exchangeAction(L"exchange ray",
L"Useful for getting out of a scrape, especially when trapped.\n"
"The effect of an exchange ray is to swap places with everyone and everything\n"
"at the target's location."));
	rtn[action::key::attract] = std::unique_ptr<action>(new attractAction(L"monster attraction",
L"To somman another creature towards you along a line of movement."));
	rtn[action::key::heal_ray_veggie] = std::unique_ptr<action>(new forceHealAction(L"Vegan food ray",
L"Force-feeds a few sprouts. This can be a life-saver, but is also unpleasant."));
	/*	rtn[action::key::comedy] = std::unique_ptr<action>(new comedyAction(L"comedy",
L"Comedy and Tragedy are two sides of the same coin. Neither is predictable,\n"
"but like all social commentary, both can be effective when used carefully."));
	rtn[action::key::comedy] = std::unique_ptr<action>(new tragedyAction(L"tragedy",
L"Comedy and Tragedy are two sides of the same coin. Neither is predictable,\n"
"but like all social commentary, both can be effective when used carefully."));
	*/
	rtn[action::key::teleport_away] = std::unique_ptr<action>(new teleportAwayAction(L"teleport away",
L"Sometimes you just need to put some distance between yourself and an\n"
"adversary. Teleport Away causes the target to move as far away from you as\n"
"possible (but in a line)."));
	rtn[action::key::lock_away] = std::unique_ptr<action>(new teleportAwayAction(L"lock away",
L"Designed for use against criminals, locking a monster away is a very safe\n"
"escape plan. It works less well on those creatures that are at home within\n"
"the earth"));
	rtn[action::key::repulsion] = std::unique_ptr<action>(new teleportAwayAction(L"repulsion",
L"Some magical effects move the enemy away from you. Repulsion usually just\n"
"buys you a little time, but it can be used to good effect if you time it\n"
"right and get your enemy where they don't want to be."));

	rtn[action::key::curse_item] = std::unique_ptr<action>(new monsterItemAction(L"curse",
L"A curse can prevent items from being dropped as well as reducing their effectiveness.",
[](const item &i) {return !i.isCursed();}, [](item &i) {i.curse(true);}));
	rtn[action::key::uncurse_item] = std::unique_ptr<action>(new monsterItemAction(L"remove curse",
L"Curses on items can be difficult to remove, but a specialist tool can sometimes be effective.",
[](const item &i) {return i.isCursed();}, [](item &i) {i.curse(false);}));
	rtn[action::key::bless_item] = std::unique_ptr<action>(new monsterItemAction(L"bless",
L"A blessing can increase the effectiveness or utility of an item.",
[](const item &i) {return !i.isBlessed();}, [](item &i) {i.bless(true);}));
	rtn[action::key::unbless_item] = std::unique_ptr<action>(new monsterItemAction(L"remove blessing",
L"Blessings on items can be difficult to remove, but a specialist tool can sometimes be effective.",
[](const item &i) {return i.isBlessed();}, [](item &i) {i.bless(false);}));
	rtn[action::key::enchant_item] = std::unique_ptr<action>(new monsterItemAction(L"enchantment",
L"An active item can be enchanted to reveal or recover its effect.\n" 
"A weapon or armour can be enchanted to increase its effectiveness.",
[](const item &i) {return true;}, [](item &i) {i.enchant(dPc() / 20);})); //TODO: should this match shop's blessed/cursed rules?
	rtn[action::key::disenchant_item] = std::unique_ptr<action>(new monsterItemAction(L"disenchantment",
L"An active item can be disenchanted to hide or limit its effect.\n"
"A weapon or armour can be disenchanted to reduce its effectiveness.",
[](const item &i) {return i.enchantment() > 0;}, [](item &i) {i.enchant(-dPc() / 20);}));
	rtn[action::key::sex_up_item] = std::unique_ptr<action>(new monsterItemAction(L"sexiness",
L"Sexy items will increase your physical appearance when visibly worn.\n",
[](const item &i) {return !i.isSexy() && i.equippable() == item::equipType::worn;}, [](item &i) {i.sexUp(true);}));
	rtn[action::key::sex_down_item] = std::unique_ptr<action>(new monsterItemAction(L"chastity",
L"Unsexy items do nothing for your physical appearance, meaning you can wear\n"
"them without affecting your appearance percentage.",
[](const item &i) {return i.isSexy();}, [](item &i) {i.sexUp(false);}));
	rtn[action::key::nudity] = std::unique_ptr<action>(new monsterItemAction(L"nudity",
L"Clothing and armour can provide a number of benifits, to defensibility and\n"
"also to outward appearance. Some can also provide less helpful effects.\n"
"The main tenet of nudism; however, suggests that nothing can compare to the\n"
"simple effect of feeling the forces of the world directly on your bare skin.",
[](const item &i) {return i.equippable() == item::equipType::worn && 
                   dynamic_cast<monster&>(i.holder()).slotsOf(i)[0] != nullptr;}, 
[](item &i) {return dynamic_cast<monster&>(i.holder()).unequip(i);}));
	rtn[action::key::disarm] = std::unique_ptr<action>(new monsterItemAction(L"disarm",
L"Weapons can be used to intimidate, attack, harm or destroy an opponent.\n"
"Sometimes; however, a more inclusive, open-handed (open-clawed,\n"
"open-beaked) approach can be effective.\n"
"This power will not assist against an opponent who uses its own body as a\n"
"weapon.",
[](const item &i) {return i.equippable() == item::equipType::wielded && 
                   dynamic_cast<monster&>(i.holder()).slotsOf(i)[0] != nullptr;}, 
[](item &i) {return dynamic_cast<monster&>(i.holder()).unequip(i);}));
	rtn[action::key::popup_shop] = std::unique_ptr<action>(new popupShopAction(L"shopping",
L"A pop-up shop is a small retail event lasting for a short length of time."));
	rtn[action::key::petrify] = std::unique_ptr<action>(new petrifyAction(L"petrify",
L"The action of petrification is to cause someone to become stiff or like\n"
"stone, to deaden in fear. Some monsters are unaffected by fear, and some\n"
"will be affected for longer than others."));
	rtn[action::key::charm] = std::unique_ptr<action>(new charmAction(L"charm",
L"Charming a monster will make it less inclined to attack you, and more\n"
"inclined to approach. The effectiveness of the charm depends on the charmer's\n"
"physical beauty."));
	rtn[action::key::comedy] = std::unique_ptr<action>(new comedyAction(L"comedy",
L"Always fun for a floor show. Use with caution"));
	return rtn;
  }
  static action &get(const typename action::key k) {
    static std::map<action::key, std::unique_ptr<action> > m = makeMap(); 
    auto &rtn = m[k];
    return *rtn;
  };
};


class proofAction : public renderedAction<item, monster> {
private:
  const damageType dt_;
public:
  proofAction(damageType dt, const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description), dt_(dt) {}
  virtual ~proofAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().proof(damageRepo::instance()[dt_], true);
    return true;
  }
  virtual bool undo(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().proof(damageRepo::instance()[dt_], false);
    return true;
  }
};

class resistAction : public renderedAction<item, monster> {
private:
  const int amount_;
  const damageType dt_;
public:
  resistAction(int amount, damageType dt, const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description), amount_(amount), dt_(dt) {}
  virtual ~resistAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().resist(&damageRepo::instance()[dt_], amount_);
    return true;
  }
  virtual bool undo(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().resist(&damageRepo::instance()[dt_], amount_);
    return true;
  }
};


class extraDamageAction : public renderedAction<item, monster> {
private:
  const int amount_;
  const damageType dt_;
public:
  extraDamageAction(int amount, damageType dt, const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description), amount_(amount), dt_(dt) {}
  virtual ~extraDamageAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().extraDamage(&damageRepo::instance()[dt_], amount_);
    return true;
  }
  virtual bool undo(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().extraDamage(&damageRepo::instance()[dt_], amount_);
    return true;
  }
};

class intrinsicsBonusAction : public renderedAction<item, monster> {
  typedef void (monsterAbilities::*methodType)(const bonus &);
private:
  const methodType method_;
  const bonus set_;
  const bonus clear_;
public:
  intrinsicsBonusAction(methodType method,
			const bonus &set, const bonus &clear,
			const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description),
    method_(method), set_(set), clear_(clear) {}
  virtual ~intrinsicsBonusAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    auto &able = target.abilities();
    (able.*method_)(set_);
    return true;
  }
  bool undo(bool blessed, bool cursed, item &source, monster &target) {
    auto &able = target.abilities();
    (able.*method_)(clear_);
    return true;
  }
};

class intrinsicsBoolAction : public renderedAction<item, monster> {
  typedef void (monsterAbilities::*methodType)(const bool);
private:
  const methodType method_;
  const bool set_;
public:
  intrinsicsBoolAction(methodType method,
			const bool &set,
			const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description),
    method_(method), set_(set) {}
  virtual ~intrinsicsBoolAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    auto &able = target.abilities();
    (able.*method_)(set_);
    return true;
  }
  bool undo(bool blessed, bool cursed, item &source, monster &target) {
    auto &able = target.abilities();
    (able.*method_)(!set_);
    return true;
  }
};

class tesseractAction : public renderedAction<item, monster> {
public:
  tesseractAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~tesseractAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().move(tFactory.get(terrainType::ALTAR), true);
    target.abilities().move(tFactory.get(terrainType::GROUND), true);
    target.abilities().move(tFactory.get(terrainType::BULKHEAD), true);
    return true;
  }
  bool undo(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().move(tFactory.get(terrainType::ALTAR), false);
    target.abilities().move(tFactory.get(terrainType::GROUND), false);
    target.abilities().move(tFactory.get(terrainType::BULKHEAD), false);
    return true;
  }
};


class fireWalkerAction : public renderedAction<item, monster> {
public:
  fireWalkerAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {}
  virtual ~fireWalkerAction() {}
  virtual bool operator ()(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().move(tFactory.get(terrainType::FIRE), true);
    return true;
  }
  bool undo(bool blessed, bool cursed, item &source, monster &target) {
    target.abilities().move(tFactory.get(terrainType::FIRE), false);
    return true;
  }
};

template<>
class actionFactory<item,monster> {
public:
  typedef sharedAction<item, monster> action;
  static action & get(const typename sharedAction<item, monster>::key k) {
    static std::map<action::key, std::unique_ptr<action> > m = makeMap();
    auto &rtn = m[k];
    return *rtn;
  };
    
  static std::map<action::key, std::unique_ptr<action> > makeMap() {
    std::map<action::key, std::unique_ptr<action> > rtn;

    rtn[action::key::resist_all_damage_edged] = std::unique_ptr<action>
      (new proofAction(damageType::edged, L"impervious to cuts",
L"When something with this property is equipped, you will never be harmed by an edged\n"
"weapon."));
    rtn[action::key::resist_all_damage_bashing] = std::unique_ptr<action>
      (new proofAction(damageType::bashing, L"impervious to blunt trauma", 
L"When something with this property is equipped, you will never be harmed by\n"
"a blunt force" ));
    rtn[action::key::resist_all_damage_hot] = std::unique_ptr<action>
      (new proofAction(damageType::hot, L"perpetual cool", 
L"When something with this property is equipped, you will never be harmed by excessive\n"
"heat" ));
    rtn[action::key::resist_all_damage_cold] = std::unique_ptr<action>
      (new proofAction(damageType::cold, L"perpetual warmth",
L"When something with this property is equipped, you will never be harmed by excessive\n"
"cold" ));
    rtn[action::key::resist_all_damage_wet] = std::unique_ptr<action>
      (new proofAction(damageType::wet, L"impervious to water damage",
L"When something with this property is equipped, you will never be harmed by water" ));
    rtn[action::key::resist_all_damage_sonic] = std::unique_ptr<action>
      (new proofAction(damageType::sonic, L"impervious to sonic damage",
L"When something with this property is equipped, you will never be harmed by sound" ));
    rtn[action::key::resist_all_damage_disintegration] = std::unique_ptr<action>
      (new proofAction(damageType::disintegration, L"impervious to disintegration",
L"When something with this property is equipped, you will never become incohesive" ));    
    rtn[action::key::resist_all_damage_starvation] = std::unique_ptr<action>
      (new proofAction(damageType::starvation, L"fullness",
L"When something with this property is equipped, you will never want for food.\n"
"This does not prevent other forms of damage that may be healed by eating." ));
    rtn[action::key::resist_all_damage_electric] = std::unique_ptr<action>
      (new proofAction(damageType::electric, L"impervious to electricity",
L"When something with this property is equipped, you will never be harmed by electricity" ));

    rtn[action::key::resist_damage_edged] = std::unique_ptr<action>
      (new resistAction(5, damageType::edged, L"protection from cuts",
L"When something with this property is equipped, you will be less harmed by edged\n"
"weapons."));
    rtn[action::key::resist_damage_bashing] = std::unique_ptr<action>
      (new resistAction(5, damageType::bashing, L"protection from blunt trauma", 
L"When something with this property is equipped, you will be less harmed by\n"
"a blunt force" ));
    rtn[action::key::resist_damage_hot] = std::unique_ptr<action>
      (new resistAction(5, damageType::hot, L"comfortable cool", 
L"When something with this property is equipped, you will be less harmed by excessive\n"
"heat" ));
    rtn[action::key::resist_damage_cold] = std::unique_ptr<action>
      (new resistAction(5, damageType::cold, L"comfortable warmth",
L"When something with this property is equipped, you will be less harmed by excessive\n"
"cold" ));
    rtn[action::key::resist_damage_wet] = std::unique_ptr<action>
      (new resistAction(5, damageType::wet, L"partial drying",
L"When something with this property is equipped, you will be less harmed by water" ));
    rtn[action::key::resist_damage_sonic] = std::unique_ptr<action>
      (new resistAction(5, damageType::sonic, L"sonic mufflers",
L"When something with this property is equipped, you will be less harmed by sound" ));
    rtn[action::key::resist_damage_disintegration] = std::unique_ptr<action>
      (new resistAction(5, damageType::disintegration, L"protection from disintegration",
L"When something with this property is equipped, you will become less incohesive" ));
    rtn[action::key::resist_damage_starvation] = std::unique_ptr<action>
      (new resistAction(5, damageType::starvation, L"improved constitution",
L"When something with this property is equipped, you will want less for food.\n"
"This does not prevent other forms of damage that may be healed by eating." ));
    rtn[action::key::resist_damage_electric] = std::unique_ptr<action>
      (new resistAction(5, damageType::electric, L"protection from electricity",
L"When something with this property is equipped, you will be less harmed by\n"
"electricity" ));

    rtn[action::key::resist_more_damage_edged] = std::unique_ptr<action>
      (new resistAction(20, damageType::edged, L"major protection from cuts",
L"When something with this property is equipped, you will be much less harmed by\n"
"edged weapons."));
    rtn[action::key::resist_more_damage_bashing] = std::unique_ptr<action>
      (new resistAction(20, damageType::bashing, L"major protection from blunt trauma", 
L"When something with this property is equipped, you will be much less harmed by\n"
"a blunt force" ));
    rtn[action::key::resist_more_damage_hot] = std::unique_ptr<action>
      (new resistAction(20, damageType::hot, L"awesome cool", 
L"When something with this property is equipped, you will be much less harmed by\n"
"excessive heat" ));
    rtn[action::key::resist_more_damage_cold] = std::unique_ptr<action>
      (new resistAction(20, damageType::cold, L"awesome warmth",
L"When something with this property is equipped, you will be much less harmed by\n"
"excessive cold" ));
    rtn[action::key::resist_more_damage_wet] = std::unique_ptr<action>
      (new resistAction(20, damageType::wet, L"effective drying",
L"When something with this property is equipped, you will be much less harmed by water" ));
    rtn[action::key::resist_more_damage_sonic] = std::unique_ptr<action>
      (new resistAction(20, damageType::sonic, L"effective sonic dampening",
L"When something with this property is equipped, you will be much less harmed by\n"
"sound" ));
    rtn[action::key::resist_more_damage_disintegration] = std::unique_ptr<action>
      (new resistAction(20, damageType::disintegration, L"active protection from disintegration",
L"When something with this property is equipped, you will become much less incohesive" ));
    rtn[action::key::resist_more_damage_starvation] = std::unique_ptr<action>
      (new resistAction(20, damageType::starvation, L"awesome constitution",
L"When something with this property is equipped, you will rarely want for food.\n"
"This does not prevent other forms of damage that may be healed by eating." ));
    rtn[action::key::resist_more_damage_electric] = std::unique_ptr<action>
      (new resistAction(20, damageType::electric, L"major protection from electricity",
L"When something with this property is equipped, you will be much less harmed by\n"
"electricity" ));

    rtn[action::key::eat_veggie_bonus] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::eatVeggie, bonus(true), bonus(),
				 L"Vegetarianism", 
L"When something with this property is equipped, you will gain a bonus to health\n"
"restored from vegetarian food."));
    rtn[action::key::eat_veggie_penalty] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::eatVeggie, bonus(false), bonus(),
				 L"floraphobia",  //There is no opposite of vegetarianism...
L"When something with this property is equipped, you will gain less health\n"
"restored from vegetarian food."));
    rtn[action::key::double_attacks] = std::unique_ptr<action>
(new intrinsicsBonusAction(&monsterAbilities::dblAttack, bonus(true), bonus(),
			   L"Double attacks",
L"When something with this property is equipped, you will get twice as many\n"
"attacks against your enemies.\n"
"This property is not cumulative."));
    rtn[action::key::half_attacks] = std::unique_ptr<action>
(new intrinsicsBonusAction(&monsterAbilities::dblAttack, bonus(false), bonus(),
			   L"Half attacks",
L"When something with this property is equipped, you will get half as many\n"
"attacks against your enemies.\n"
"This property is not cumulative."));

    rtn[action::key::extra_damage_edged] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::edged, L"sharpness",
L"When something with this property is equipped, you will deal extra damage\n"
"with edged weapons."));
    rtn[action::key::extra_damage_bashing] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::bashing, L"blunt might", 
L"When something with this property is equipped, you will deal extra damage\n"
"when wielding blunt force" ));
    rtn[action::key::extra_damage_hot] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::hot, L"frostbite", 
L"When something with this property is equipped, you will deal extra damage\n"
"when using cold attacks" ));
    rtn[action::key::extra_damage_cold] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::cold, L"burning",
L"When something with this property is equipped, you will deal extra damage\n"
"when using heat attacks" ));
    rtn[action::key::extra_damage_wet] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::wet, L"moisture",
L"When something with this property is equipped, you will deal extra damage\n"
"when using water attacks" ));
    rtn[action::key::extra_damage_sonic] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::sonic, L"acoustic volume",
L"When something with this property is equipped, you will deal extra damage\n"
"when using sonic attacks" ));
    rtn[action::key::extra_damage_disintegration] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::disintegration, L"crumbling",
L"When something with this property is equipped, you will deal extra damage\n"
"when using disintegration attacks" ));
    rtn[action::key::extra_damage_starvation] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::starvation, L"tapeworms",
L"When something with this property is equipped, you will deal extra damage\n"
"when using starvation attacks" ));
    rtn[action::key::extra_damage_electric] = std::unique_ptr<action>
      (new extraDamageAction(5, damageType::electric, L"capacitive charge",
L"When something with this property is equipped, you will deal extra damage\n"
"when using electrical attacks" ));


    rtn[action::key::extra_damage_major_edged] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::edged, L"extreme sharpness",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage with edged weapons."));
    rtn[action::key::extra_damage_major_bashing] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::bashing, L"hefty blunt might", 
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when wielding blunt force" ));
    rtn[action::key::extra_damage_major_hot] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::hot, L"fourth degree frostbite", 
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using cold attacks" ));
    rtn[action::key::extra_damage_major_cold] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::cold, L"consuming inferno",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using heat attacks" ));
    rtn[action::key::extra_damage_major_wet] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::wet, L"extreme wetness",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using water attacks" ));
    rtn[action::key::extra_damage_major_sonic] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::sonic, L"extreme acoustic volume",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using sonic attacks" ));
    rtn[action::key::extra_damage_major_disintegration] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::disintegration, L"extreme disintegration",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using disintegration attacks" ));
    rtn[action::key::extra_damage_major_starvation] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::starvation, L"starvation",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using starvation attacks" ));
    rtn[action::key::extra_damage_major_electric] = std::unique_ptr<action>
      (new extraDamageAction(20, damageType::electric, L"lightning charge",
L"When something with this property is equipped, you will deal very significant\n"
"extra damage when using electrical attacks" ));

    rtn[action::key::move_through_solid] = std::unique_ptr<action>
      (new tesseractAction(L"tesseraction",
L"Bestows the ability to move effortlessly through solid objects.\n"
"Be careful where you lose this ability, or you could become stuck."));
    rtn[action::key::move_through_fire] = std::unique_ptr<action>
      (new fireWalkerAction(L"fire-walking",
L"Bestows the ability to walk through fire.\n"
"Passive fiery terrain will not hurt you."));
    rtn[action::key::hearing] = std::unique_ptr<action>
      (new intrinsicsBoolAction(&monsterAbilities::hear, true,
			   L"Hearing",
L"Bestows the ability to detect airborne vibrations, often conveying\n"
"information"));
    rtn[action::key::sight] = std::unique_ptr<action>
      (new intrinsicsBoolAction(&monsterAbilities::see, true,
			   L"Sight",
L"Bestows the ability to detect waves or particles of electromagnetic\n"
"radiation, often conveying information"));
    rtn[action::key::swim] = std::unique_ptr<action>
      (new intrinsicsBoolAction(&monsterAbilities::swim, true,
			   L"Swimming",
L"Bestows the ability to move through water."));
    rtn[action::key::flight] = std::unique_ptr<action>
      (new intrinsicsBoolAction(&monsterAbilities::fly, true,
			   L"Flying",
L"Bestows the ability to move through the air, ignoring unexpected changes in\n"
"floor level."));
    rtn[action::key::fearless] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::fearless, bonus(true), bonus(),
			   L"fearlessness",
L"Bestows the confidence to ignore the effects of petrifying terror."));
    rtn[action::key::fearful] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::fearless, bonus(false), bonus(),
			   L"fearfulness",
L"When something with this property is equipped, you will suffer more greatly\n"
"from effects of petrifying terror."));
    rtn[action::key::fast_climb] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::climb, bonus(true), bonus(),
			   L"fast climbing",
L"When something with this property is equipped, you will be able to climb, say\n"
"out of a pit, twice as fast as normal."));
    rtn[action::key::slow_climb] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::climb, bonus(false), bonus(),
			   L"slow climbing",
L"When something with this property is equipped, you will be able to climb, say\n"
"out of a pit, half as fast as normal."));
    rtn[action::key::speed] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::speedy, bonus(true), bonus(),
			   L"speed",
L"When something with this property is equipped, you will be able to move twice\n"
				 "as fast as normal."));
    rtn[action::key::slow] = std::unique_ptr<action>
      (new intrinsicsBonusAction(&monsterAbilities::speedy, bonus(false), bonus(),
			   L"sloth",
L"When something with this property is equipped, you will be able to move half\n"
"as fast as normal."));    
    return rtn;
  }
};

void ignored() {
  actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::END);
  actionFactory<item,monster>::get(sharedAction<item,monster>::key::END);
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
    if (isProtected) {
      if (source.isPlayer()) ios.message(L"You aren't in the mood.");
      else ios.message(source.name() + L" doesn't seem to be in the mood.");
      return false;
    }
    if (isCompatible && dPc() < target.appearance().cur()) {
      // good outcome; consent happened.
      ios.message(target.isPlayer() ? L"You consent to the advances of the " + source.name()
		  : source.isPlayer() ? L"The " + target.name() + L" consents to your advances..."
		  : L"The " + target.name() + L" consents to the advances of the " + source.name() + L"..."
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
      ios.message(target.isPlayer() ? L"You are violated by the "  + source.name() + L'!'
		  : source.isPlayer() ? L"You feel bad about violating the " + target.name() + L"..."
		  : L"The " + target.name() + L" does not consent to the advances of the " + source.name() + L"..."
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

sharedAction<monster, monster> &succubusAction() {
  static foocubusAction<false> succubusAction;
  return succubusAction;
}


// TODO: area-effect actions
// wand of health & safety: reveals all traps on level, clears any damage, removes any cloak, dons safety glasses & doublet becomes a fluorescent jacket.
