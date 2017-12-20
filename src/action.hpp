/* License and copyright go here*/

// action functors library
// an action can be used intrisicly by a moster, triggered by an item, cast as a spell, and so on.

#ifndef ACTION_HPP__
#define ACTION_HPP__

#include "renderable.hpp"

class item;
class monster;

// actions
template <class ...T>
class sharedAction {
public:
  virtual ~sharedAction() {};
  // takes targets; returns true if action was successful, false if blocked or failed.
  virtual bool operator ()(T & ... ) = 0;
};


// actions for a source and target monster:
template<>
class sharedAction<monster, monster>{
 public:  
  virtual bool operator ()(monster &source, monster &target) = 0;
  enum class key {
    // steal small items (1N or less) from target
    steal_small,
    // steal treasure item (category '*')
    steal_shiny,
    // water ray 5%
      attack_ray_small_water,
    END
  };
};

template<class ...T>
class renderedAction : public sharedAction<T...>, public renderable {
private:
  const wchar_t * const name_;
  const wchar_t * const description_;
public:
renderedAction(const wchar_t * const name, const wchar_t * const description) :
  sharedAction<T...>(), name_(name), description_(description) {}
  virtual bool operator()(T &...) = 0;
  virtual const wchar_t render() const { return L'☇'; } // lightning bolt for magic
  virtual const wchar_t * const name() const { return name_; };
  const wchar_t * const description() const { return description_; };
};

template<class ...T>
class actionFactory {
public:
  static renderedAction<T...> & get(const typename sharedAction<T...>::key k);
};

#endif //ndef ACTION_HPP__
