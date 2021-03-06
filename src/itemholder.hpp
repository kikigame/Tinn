/* License and copyright go here*/

// Places where items are

#ifndef ITEM_HOLDER_HPP__
#define ITEM_HOLDER_HPP__

#include <memory>
#include <vector>
#include "optionalRef.hpp"

class item;

#include "iterable.hpp"

class itemHolder;
class itemHolderMapImpl;

class itemHolderMap {
  friend class itemHolder;
  friend class item;
private:
  std::unique_ptr<itemHolderMapImpl> pImpl_;
public:
  static itemHolderMap& instance();
  itemHolderMap();
  ~itemHolderMap();
  itemHolder &forItem(item &);
  itemHolder &forItem(const item &); // does linear search to avoid const issues
  void enroll(item &); // called from item constructor
  bool beforeFirstAdd(const item &) const; // has this enrolled item been added via itemHolder.addItem() yet?
  optionalRef<item> rndIf(std::function<bool(item &)>); // pick a random item matching functor from the entire game.
protected:
  void destroy(item &);
  void move(item &, itemHolder &);
  bool contains(const item &, const itemHolder &) const; // NB item is only non-const so we can take a temporary shared_ptr to it. This would destroy the item if it doesn't have another shared_ptr reference elsewhere.
};




/*
 * Class representing somewhere an item can be.
 * All items have a holder, which keeps the reference alive.
 */
class itemHolder {
private:
  std::vector<std::weak_ptr<item> > contents_;
public:
  virtual ~itemHolder() = default;
  // add an item to this container (removing it from any previous container).
  // add may fail if previous container's removeItemForMove() returns false.
  virtual bool addItem(item &item);
  // remove an item from this container with the intention of putting it somewhere else
  // If overridden to return false, the move is aborted.
  // If returning true (default), the item is removed, but may be lost if the other container's addItem() returns false.
  virtual bool removeItemForMove(item &item, itemHolder &next);
  // destroy an item from the game. To move an item, call addItem() on its new location
  virtual bool destroyItem(item &item);
  // for each item in inventory; param takes the item and its rendered name
  virtual void forEachItem(std::function<void(item&, std::wstring name)>);
  virtual void forEachItem(std::function<void(const item&, std::wstring name)>) const;
  // iterate over all items, stopping at first match:
  virtual optionalRef<item> firstItem(std::function<bool(item&)> fn = [](item &){ return true; }) const;
  // test if the item is in this container
  bool contains(const item &item) const;
  // test for an empty container
  bool empty() const;
  // get number of items; returns 0 if empty() == true
  unsigned int size() const;
  // count the number of items matching functor
  unsigned int countIf(std::function<bool(item&)>);
  // take "from" out of this container and put "to" in its place.
  // Used for transmutation, so position shouldn't change and cursed status should be ignored.
  // Return of true should mean than "from" remains in the container and "to" has not been added.
  // if adding "to" to the container fails, "from" must be replaced.
  bool replaceItem(item &from, item &to);
  // convenience method to get the total weight in container.
  double totalWeight() const;
  // prompt the player for an item via io::instance():
  item* pickItem(const std::wstring & prompt,
		 const std::wstring & help,
		 const std::wstring & extraHelp,
		 const std::function<bool(const item &)> f=[](const item &){return true;},
		 // NB: If allowNone = false, caller must ensure that at least one item in container matches f().
		 const bool allowNone = true) const;
};



#endif // ndef ITEM_HOLDER_HPP__
