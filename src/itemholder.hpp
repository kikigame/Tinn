/* License and copyright go here*/

// Places where items are

#ifndef ITEM_HOLDER_HPP__
#define ITEM_HOLDER_HPP__

#include <memory>
#include <vector>

class item;

#include "iterable.hpp"

/*
 * Mostly-virtual class representing somewhere an item can be
 * (forEachItem() isn't pure, but useful).
 */
class itemHolder {
public:
  virtual ~itemHolder() = default;
  virtual bool addItem(std::shared_ptr<item> item) = 0;
  virtual iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > contents() = 0;
  // for each item in inventory; param takes the item and its rendered name
  virtual void forEachItem(std::function<void(std::shared_ptr<item>, std::wstring name)>);
};

/*
 * implementation of itemHolder that owns (for C++ lifecycle purposes) a list of items
 */
class itemOwner : public itemHolder {
private:
  std::vector<std::shared_ptr<item> > items_;
public:
  virtual ~itemOwner();
  virtual bool addItem(std::shared_ptr<item> item);
  virtual iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > contents();
  virtual iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> >, true > ccontents() const;
};


#endif // ndef ITEM_HOLDER_HPP__
