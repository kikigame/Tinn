/* License and copyright go here*/

#include "action.hpp"
#include "monster.hpp"
#include "items.hpp"
#include "output.hpp"
#include <map>

// source steals an item from target
class steal : public renderedAction<monster, monster> {
private:
  std::function<bool(item &)> f_;
public:
  steal(const wchar_t * const name, const wchar_t * const description, std::function<bool(item &)> f) : 
    renderedAction(name, description), f_(f) {};
  virtual ~steal() {};
  bool operator ()(monster &source, monster &target) {
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
  bool operator ()(monster &source, monster &target) {
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
  bool operator ()(monster &source, monster &target) {
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

class forceHealAction : public renderedAction<monster, monster> {
public:
  forceHealAction(const wchar_t * const name, const wchar_t * const description) :
    renderedAction(name, description) {};
  virtual ~forceHealAction() {}
  bool operator ()(monster &source, monster &target) {
    auto &sprouts = createItem(itemTypeKey::apple);
    source.addItem(sprouts);
    return target.eat(sprouts, true);
  }
};

template<>
class actionFactory<monster, monster> {
public:
  typedef sharedAction<monster, monster> action;
  static const action &get(const typename action::key k) {
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
L"Force-feeds a few sprouts. This can be a life-saver, but is also unpleasant.")))
	};
    auto &rtn = m[k];
    return *rtn;
  };
};

void ignored() {
  actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::END);
}
