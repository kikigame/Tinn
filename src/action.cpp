/* License and copyright go here*/

#include "action.hpp"
#include "monster.hpp"
#include "items.hpp"
#include "output.hpp"
#include <map>

// source steals an item from target
class steal : public sharedAction<monster, monster> {
private:
  std::function<bool(item &)> f_;
public:
  steal(std::function<bool(item &)> f) : f_(f) {};
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

template<>
class actionFactory<monster, monster> {
public:
  typedef sharedAction<monster, monster> action;
  static const action &get(const typename action::key k) {
    static std::map<action::key, std::shared_ptr<action> > m{
      std::make_pair<action::key, std::shared_ptr<action> >
	(action::key::steal_small, std::shared_ptr<action>(new steal([](item &i) { return i.weight() <= 1; }))),
      std::make_pair<action::key, std::shared_ptr<action> >
	(action::key::steal_shiny, std::shared_ptr<action>(new steal([](item &i) { return i.render() == L'*' || i.render() == L'$'; })))
    };
    return *(m[k]);
  };
};

void ignored() {
  actionFactory<monster,monster>::get(sharedAction<monster,monster>::key::END);
}
