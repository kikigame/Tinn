/* License and copyright go here*/

// Places where items are

#ifndef ITEM_HOLDER_HPP__
#define ITEM_HOLDER_HPP__

#include <memory>
#include <vector>

class item;

/*
 * Useful interface for iterability.
 * Unspecialised form returns by reference to allow modifications.
 */
template <typename T, typename C, bool constant = false>
class iterable {
  friend class iterable<T,C,true>;
  C& container_;
public:
  iterable(C &container) : container_(container) {};
  virtual ~iterable() {};
  virtual typename C::iterator begin() { return container_.begin(); }
  virtual typename C::iterator end()  { return container_.end(); }   
  virtual void erase(typename C::iterator pos) { container_.erase(pos); }
};
/*
 * Specialisation: Stores a copy of temp results so we can use it in return values.
 */
template <typename T, typename C>
class iterable<T,C,true> {
  const C container_;
public:
  iterable(const iterable<T,C,false> &other) : container_(other.container_) {}
  iterable(const C &container) : container_(container) {};
  virtual ~iterable() {}
  typename C::const_iterator begin() { return container_.begin(); }
  typename C::const_iterator end()  { return container_.end(); }   
};

/*
 * Pure virtual class representing somewhere an item can be
 */
class itemHolder {
public:
  virtual ~itemHolder() = default;
  virtual bool addItem(std::shared_ptr<item> item) = 0;
  virtual iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item> > > contents() = 0;
};

#endif // ndef ITEMS_HPP__
