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
	std::make_pair(action::key::attack_ray_small_water, std::shared_ptr<action>(new attackRay<5>(L"water jet",
L"A water jet will squirt a target with a small amount of water.\n",
damageType::wet)))
	};
    return *(m[k]);
  };
};

void ignored() {
  actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::END);
}
