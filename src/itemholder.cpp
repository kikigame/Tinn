/* License and copyright go here*/

// Things and stuff

#include "items.hpp"
#include "damage.hpp"
#include "random.hpp"
#include "monster.hpp"
#include "shop.hpp"
#include "encyclopedia.hpp"
#include "output.hpp"
#include <set>
#include <bitset>
#include <algorithm>

itemOwner::~itemOwner() {}

bool itemOwner::addItem(std::shared_ptr<item> item) {
  for (auto i : items_)
    if (i == item) return false;
  items_.push_back(item);
  item->move(*this);
  return true;
}
iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > itemOwner::contents() {
  return iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > >(items_);
}
iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> >, true > itemOwner::ccontents() const {
  return iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> >, true >(items_);
}


void itemHolder::forEachItem(const std::function<void(std::shared_ptr<item>, std::wstring name)> f) {
  iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item>>, 
	   true> it(contents()); // true -> take a copy (in case items are removed)
  for (auto i : it) f(i, i->name());
}
