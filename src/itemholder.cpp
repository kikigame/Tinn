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
  bool contains(item &i, const itemHolder &h) const {
    auto pi = from(i);
    auto find = map_.find(pi);
    return find != map_.end() && &(*(find->second)) == &h;
  }
  bool beforeFirstAdd(const item &i) const {
    auto pi = from(const_cast<item&>(i)); // pinky-swear not to change it
    return map_.at(pi) == nullptr;
  }
private:
  std::shared_ptr<const item> from(const item &i) const {
    // all items implement enable_shared_from_this:
    return static_cast<const shared_item&>(i).shared_from_this();
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
bool itemHolderMap::contains(item & i, const itemHolder &h) const { return pImpl_->contains(i,h); }
void itemHolderMap::enroll(item & i) { pImpl_->enroll(i); }
bool itemHolderMap::beforeFirstAdd(const item & i) const { return pImpl_->beforeFirstAdd(i); }

bool itemHolder::addItem(item &item) {
  auto &map = itemHolderMap::instance();
  auto pi = item.shared_from_this(); // keep alive
  if (!map.beforeFirstAdd(item)) {
    auto &h = map.forItem(item);
    if (!h.removeItemForMove(item)) return false;
  }
  map.move(item, *this);
  contents_.push_back(pi);
  return true;
}

bool itemHolder::removeItemForMove(item &it) {
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

bool itemHolder::contains(item &item) const {
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
bool itemHolder::replaceItem(item &from, item &to) {
  for (auto i = contents_.begin(); i != contents_.end(); ++i)
    if (i->lock().get() == &from) {
      i = contents_.erase(i);
      contents_.insert(i, to.shared_from_this());
      return true;
    }
  return false;
}
