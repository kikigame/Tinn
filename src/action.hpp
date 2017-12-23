/* License and copyright go here*/

// action functors library
// an action can be used intrisicly by a moster, triggered by an item, cast as a spell, and so on.

#ifndef ACTION_HPP__
#define ACTION_HPP__

#include "renderable.hpp"
#include "bonus.hpp"

class item;
class monster;

// actions
template <class ...T>
class sharedAction {
public:
  virtual ~sharedAction() {};
  // takes targets; returns true if action was successful, false if blocked or failed.
  virtual bool operator ()(bool blessed, bool cursed, T & ... ) = 0;
};


// actions for a source and target monster:
template<>
class sharedAction<monster, monster>{
 public:  
  virtual bool operator ()(bool blessed, bool cursed, monster &source, monster &target) = 0;
  enum class key {
    // steal small items (1N or less) from target
    steal_small,
    // steal treasure item (category '*')
    steal_shiny,
    // attack rays @ 5%
    attack_ray_small_edged,
    attack_ray_small_bashing,
    attack_ray_small_hot,
    attack_ray_small_cold,
    attack_ray_small_water,
    attack_ray_small_sonic,
    attack_ray_small_disintegration,
    attack_ray_small_starvation,
    attack_ray_small_electric,
    // attack rays @ 20%
    attack_ray_med_edged,
    attack_ray_med_bashing,
    attack_ray_med_hot,
    attack_ray_med_cold,
    attack_ray_med_water,
    attack_ray_med_sonic,
    attack_ray_med_disintegration,
    attack_ray_med_starvation,
    attack_ray_med_electric,
    // attack rays @ 100%
    death_ray_edged,
    death_ray_bashing,
    death_ray_hot,
    death_ray_cold,
    death_ray_water,
    death_ray_sonic,
    death_ray_disintegration,
    death_ray_starvation,
    death_ray_electric,
    // swap places
    exchange_ray,
    // heal by veggie food consumption
    heal_ray_veggie,
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
  virtual bool operator()(bool, bool, T &...) = 0;
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
