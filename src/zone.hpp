/* License and copyright go here*/

// areas of a level with special behaviour

#ifndef ZONE_HPP__
#define ZONE_HPP__

#include "coord.hpp"
#include "itemholder.hpp"
#include <memory> // shared_ptr
#include <set>
#include <algorithm>

class monster;

// abstract class for a zone providing specific behaviour
// T may be "monster", "player" or "item"
template<class T>
class zoneArea {
public:
  // determine if a given square contains this zone
  virtual bool contains(coord area) = 0;

  // NB "moving" means moving for a monster/player, or
  // being fired, thrown, kicked, etc for an item

  // called whenever entering the zone
  // NB: For an item, this can also be by being dropped
  virtual bool onEnter(T &, itemHolder &prev); // impremented for monsters & items
  // called whenever leaving the zone
  // NB: For an item, this can also be by being taken
  virtual bool onExit(T &, itemHolder &next); // implemented for monsters
  // Called whenever a monster in the zone attacks another monster or item
  // (currently, zones affect whether you can attack, not whether you can be attacked)
  virtual bool onAttack(monster&, T&); // implemented for monsters
  // called before moving between two squares in the zone; dest is the destination square
  virtual bool onMoveWithin(T &, const coord &dest); // implemented for monsters
  // called whenever the location is a coord within the zone and a tick passes

};

template <typename T>
class zoneActions {
private:
  typedef std::set<std::shared_ptr<zoneArea<T> > > sorted_type;
  typedef std::vector<std::shared_ptr<zoneArea<T> > > unsorted_type;
  unsorted_type intersection_, entering_, leaving_;
public:
  zoneActions() :
    intersection_(), entering_(), leaving_() {}
  zoneActions(filteredIterable<std::shared_ptr<zoneArea<T> >,std::vector<std::shared_ptr<zoneArea<T> > > > before,
	      filteredIterable<std::shared_ptr<zoneArea<T> >,std::vector<std::shared_ptr<zoneArea<T> > > > after) {
    sorted_type pre, post;
    pre.insert(before.begin(), before.end());
    post.insert(after.begin(), after.end());
    std::set_intersection(pre.begin(), pre.end(), post.begin(), post.end(), 
			  std::back_inserter(intersection_));
    std::set_difference(post.begin(), post.end(), intersection_.begin(), intersection_.end(), 
			std::back_inserter(entering_));
    std::set_difference(pre.begin(), pre.end(), intersection_.begin(), intersection_.end(), 
			std::back_inserter(leaving_));
  }
  // return set of zones in both locations
  unsorted_type& same() { return intersection_; }
  // set of zones we are entering
  unsorted_type& entering() { return entering_; }
  // set of zones we are leaving
  unsorted_type& leaving() { return leaving_; }
};


#endif //ndef ZONE_HPP__
