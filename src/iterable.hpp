/* License and copyright go here*/

// Places where items are

#ifndef ITERABLE_HPP__
#define ITERABLE_HPP__

#include <memory>
#include <vector>
#include <functional>

class item;

template<class C, bool ref=false> 
struct isRef { public: typedef C type; };

template<class C>
struct isRef<C, true> { public: typedef const C &type; };

/*
 * Useful interface for iterability.
 * Can returns by reference to allow modifications, or by value
 */
template <typename T, typename C, bool constant = false, bool copy = false>
class iterable {
  friend class iterable<T,C,true, true>;
  friend class iterable<T,C,false, true>;
  typename isRef<C,constant>::type container_;
public:
  iterable(C &container) : container_(container) {};
  virtual ~iterable() {};
  virtual typename C::iterator begin() { return container_.begin(); }
  virtual typename C::iterator end()  { return container_.end(); }   
  virtual void erase(typename C::iterator pos) { container_.erase(pos); }
};

/*
 * Specialisation: Supply const values
 */
template <typename T, typename C, bool constant>
class iterable<T,C,constant,true> {
  typename isRef<C,constant>::type container_;
public:
  iterable(const iterable<T,C,true, false> &other) : container_(other.container_) {}
  iterable(const iterable<T,C,false, false> &other) : container_(other.container_) {}
  iterable(const C &container) : container_(container) {};
  virtual ~iterable() {}
  typename C::const_iterator begin() { return container_.begin(); }
  typename C::const_iterator end()  { return container_.end(); }
  iterable<T,C,constant,true> & operator=(iterable<T,C,constant,true> &other) {
    container_ = other.container_;
    return *this;
  }
};


/*
 * filtering iterator based off
 * https://stackoverflow.com/questions/44350760/iterators-of-a-vector-filtered
 * (but fixing many typos & bugs!)
 */
template <typename BaseIterator>
class filtered_iterator : public BaseIterator
{
public:
  typedef std::function<bool(const typename std::iterator_traits<BaseIterator>::value_type&)> filter_type;
private:
  BaseIterator end_;
  filter_type filter_;
public:
  filtered_iterator() = default;
  filtered_iterator(filter_type &filter, BaseIterator base, BaseIterator end = {})
    : BaseIterator(base), end_(end), filter_(filter) {
    while (*this != end_ && !filter_(**this)) {
      BaseIterator::operator++();
    }
  }

  filtered_iterator& operator++() {
    do {
      BaseIterator::operator++();
    } while (*this != end_ && !filter_(**this));
    return *this;
  }

  filtered_iterator operator++(int) {
    filtered_iterator copy = *this;
    ++*this;
    return copy;
  }

};
template <typename BaseIterator>
filtered_iterator<BaseIterator>
make_filtered_iterator(typename filtered_iterator<BaseIterator>::filter_type filter,
		       BaseIterator base, BaseIterator end = {}) {
  return {filter, base, end};
}



/*
 * Useful interface for iterability.
 * Unspecialised form returns by reference to allow modifications.
 */
template <typename T, typename C, bool constant = false>
class filteredIterable {
private:
  iterable<T,C,constant> iter_;
  filtered_iterator<typename C::iterator> begin_;
  const filtered_iterator<typename C::iterator> end_;
public:
  typedef std::function<bool(const typename std::iterator_traits<typename C::iterator>::value_type&)> filter_type;
  filteredIterable(iterable<T,C,constant> &iter, filter_type filter) :
    iter_(iter), 
    begin_(make_filtered_iterator(filter, iter.begin(), iter.end())),
    end_(make_filtered_iterator(filter, iter.end(), iter.end())) {}
  filteredIterable(C &container, filter_type filter) :
    iter_(container), 
    begin_(make_filtered_iterator(filter, iter_.begin(), iter_.end())),
    end_(make_filtered_iterator(filter, iter_.end(), iter_.end())) {}
  virtual ~filteredIterable() {};
  virtual typename C::iterator begin() { return begin_; }
  virtual typename C::iterator end() const { return end_; }   
};

/*
 * Specialisation: stores a copy of results for use in return values
 */
template <typename T, typename C>
class filteredIterable<T,C,true> {
private:
  iterable<T,C,true> iter_;
  filtered_iterator<typename C::const_iterator> begin_;
  const filtered_iterator<typename C::const_iterator> end_;
public:
  typedef std::function<bool(const typename std::iterator_traits<typename C::const_iterator>::value_type&)> filter_type;
  filteredIterable(iterable<T,C,true> &iter, filter_type filter) :
    iter_(iter), 
    begin_(make_filtered_iterator(filter, iter.begin(), iter.end())),
    end_(make_filtered_iterator(filter, iter.end(), iter.end())) {}
  filteredIterable(C &container, filter_type filter) :
    iter_(container), 
    begin_(make_filtered_iterator(filter, iter_.begin(), iter_.end())),
    end_(make_filtered_iterator(filter, iter_.end(), iter_.end())) {}
  virtual ~filteredIterable() {};
  virtual typename C::const_iterator begin() { return begin_; }
  virtual typename C::const_iterator end() const { return end_; }   
};


#endif // ndef ITERABLE_HPP__
