/* License and copyright go here*/

// action functors library
// an action can be used intrisicly by a moster, triggered by an item, cast as a spell, and so on.

#ifndef ACTION_HPP__
#define ACTION_HPP__

class item;
class monster;

// actions
template <class ...T>
class sharedAction {
public:
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
    END
  };
};

template<class ...T>
class actionFactory {
public:
  static sharedAction<T...> &get(const typename sharedAction<T...>::key k);
};

#endif //ndef ACTION_HPP__
