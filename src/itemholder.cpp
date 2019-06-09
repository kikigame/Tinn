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


itemHolderMap& itemHolderMap::instance() {
  static itemHolderMap map;
  return map;
}

class itemHolderMapImpl {
private:
  std::map<std::shared_ptr<item>, itemHolder*> map_;
public:
  itemHolderMapImpl() : map_() {}
  void enroll(item &i) {
    // we can't call shared_from_this on an item until it has at least one
    // shared_ptr; as shared_ptr simply stores a weak_ptr on the item if it
    // extends std::enable_shared_from_this.
    // so we have to create a shared_ptr. This acts as a consumer that is
    // called as soon as the object becomes "live".
    // this breaks RAII, but is a design issue with shared_ptr.
    auto pi = std::shared_ptr<item>(&i);
    map_[pi] = nullptr;
  }
  itemHolder &forItem(item &i) {
    auto pi = from(i);
    return *map_[pi];
  }
  itemHolder &forItem(const item &i) {
    for (auto p : map_) {
      if (p.first.get() == &i) return *(p.second);
    }
    throw L"Not found";
  }
  void move(item &i, itemHolder &h) {
    auto pi = from(i);
    map_[pi] = &h;
  }
  void destroy(item &i) {
    auto pi = from(i);
    map_.erase(pi);
  }
  bool contains(const item &i, const itemHolder &h) const {
    auto pi = from(i);
    auto find = map_.find(pi);
    return find != map_.end() && &(*(find->second)) == &h;
  }
  bool beforeFirstAdd(const item &i) const {
    auto pos =
    std::find_if(map_.begin(), map_.end(), [&i](std::pair<std::shared_ptr<item>, itemHolder*> entry){
	return entry.first.get() == &i;
      });
    return pos == map_.end() // maybe still in constructor; no shared_ptr
      || pos->second == nullptr; // not registered yet
  }
  optionalRef<item> rndIf(std::function<bool(item &)> pred) {
    for (auto entry : map_)
      if (pred(*(entry.first)))
	return optionalRef<item>(*(entry.first));
    return optionalRef<item>();
  }
private:
  std::shared_ptr<item> from(const item &i) const {
    // all items implement enable_shared_from_this:
    return static_cast<shared_item&>(const_cast<item &>(i)).shared_from_this();
  }
  std::shared_ptr<item> from(item &i) const {
    // all items implement enable_shared_from_this:
    return static_cast<shared_item&>(i).shared_from_this();
  }
};

itemHolderMap::itemHolderMap() : pImpl_(new itemHolderMapImpl()) {}
itemHolderMap::~itemHolderMap() {}
itemHolder &itemHolderMap::forItem(item & i) { return pImpl_->forItem(i); }
itemHolder &itemHolderMap::forItem(const item & i) { return pImpl_->forItem(i); }
void itemHolderMap::move(item & i, itemHolder &h) { pImpl_->move(i,h); }
void itemHolderMap::destroy(item & i) { pImpl_->destroy(i); }
bool itemHolderMap::contains(const item & i, const itemHolder &h) const { return pImpl_->contains(i,h); }
void itemHolderMap::enroll(item & i) { pImpl_->enroll(i); }
bool itemHolderMap::beforeFirstAdd(const item & i) const { return pImpl_->beforeFirstAdd(i); }

optionalRef<item> itemHolderMap::rndIf(std::function<bool(item &)> pred) {
  return pImpl_->rndIf(pred);
}

bool itemHolder::addItem(item &item) {
  auto &map = itemHolderMap::instance();
  auto pi = item.shared_from_this(); // keep alive
  if (!map.beforeFirstAdd(item)) {
    auto &h = map.forItem(item);
    if (!h.removeItemForMove(item, *this)) return false;
  }
  map.move(item, *this);
  contents_.push_back(pi);
  pi->onAdd(*this);
  return true;
}

bool itemHolder::removeItemForMove(item &it, itemHolder &holder) {
  const std::weak_ptr<item> i = it.shared_from_this();
  for (auto iter = contents_.begin(); iter != contents_.end(); ++iter)
    if (iter->lock().get() == &it) {
      contents_.erase(iter);
      break;
    }
  return true;
}

bool itemHolder::destroyItem(item &item) {
  auto i = item.shared_from_this(); // keep alive
  bool rtn = contains(item);
  itemHolderMap::instance().destroy(item);
  for (auto iter = contents_.begin(); iter != contents_.end(); ++iter)
    if (iter->lock().get() == &item) {
      contents_.erase(iter);
      break;
    }
  return rtn;
}

bool itemHolder::contains(const item &item) const {
  return itemHolderMap::instance().contains(item, *this);
}

void itemHolder::forEachItem(const std::function<void(item &, std::wstring name)> f) {
  // take copy of pointers in case list is changed during iteration
  auto copy = contents_; // copy of pointers in case list is changed
  for (auto i : copy) {
    std::shared_ptr<item> ii = i.lock();
    if (ii) // may be cleared, eg by use of previous item
      f(*ii, ii->name());
  }
}
void itemHolder::forEachItem(const std::function<void(const item &, std::wstring name)> f) const {
  auto copy = contents_; // copy of pointers in case list is changed
  for (auto i : copy) {
    std::shared_ptr<item> ii = i.lock();
    if (ii) // may be cleared, eg by use of previous item
      f(*ii, ii->name());
  }
}

optionalRef<item> itemHolder::firstItem(std::function<bool(item&)> f) {
  auto copy = contents_;
  for (auto i : copy) {
    std::shared_ptr<item> ii = i.lock();
    if (f(*ii)) { return *ii; }
  }
  return optionalRef<item>();
}
bool itemHolder::empty() const {
  return contents_.empty();
}
unsigned int itemHolder::size() const {
  return contents_.size();
}
unsigned int itemHolder::countIf(std::function<bool(item&)> f) {
  return std::count_if(contents_.begin(), contents_.end(), [f](std::weak_ptr<item> i){ return f(*(i.lock())); });
}
bool itemHolder::replaceItem(item &from, item &to) {
  for (auto i = contents_.begin(); i != contents_.end(); ++i)
    if (i->lock().get() == &from) {
      i = contents_.erase(i);
      contents_.insert(i, to.shared_from_this());
      to.onAdd(*this);
      return true;
    }
  return false;
}

double itemHolder::totalWeight() const  {
  double totalWeight=0;
  forEachItem([&totalWeight](const item &i, std::wstring) {
      totalWeight += i.weight();
    });
  return totalWeight;
}

item *itemHolder::pickItem(const std::wstring & prompt,
			   const std::wstring & help,
			   const std::wstring & extraHelp,
			   const std::function<bool(const item &)> f,
			   const bool allowNone) const {
  if (empty()) return nullptr;
  std::vector<std::pair<int, std::wstring>> choices;
  std::vector<item *> res;
  int i=0;
  // pinkie-swear not to change anything:
  const_cast<itemHolder *>(this)->
  forEachItem([&choices, &i, &res, f](item &it, std::wstring name) {
      if (f(it)) {
	choices.emplace_back(i++, it.name());
	res.emplace_back(&it);
      }
    });
  if (allowNone) {
    choices.emplace_back(i++, L"Nothing");
    res.emplace_back(nullptr);
  }
  // if we must choose something, but have nothing to choose, we arbitrarily don't.
  // NB: this can happen when shopping for a repair service without anything to repair.
  if (choices.size() == 0) return nullptr;
  int it = ioFactory::instance().choice(prompt, help, choices, extraHelp);
  return *(res.begin() + it);
}

optionalRef<monster> whoHolds(const item &i) {
  auto holder = &(i.holder());
  monster *m;
  do {
    m = dynamic_cast<monster *>(holder);
    item * it = dynamic_cast<item *>(holder);
    if (it) holder = &(it->holder());
    if (!it && !m) return optionalRef<monster>();
  } while (m == nullptr);
  return m ? optionalRef<monster>(*m) : optionalRef<monster>();
}
