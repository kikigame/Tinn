/* License and copyright go here*/

// path finding algorithm of some sort

#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <climits>
#include <set>
#include "coord.hpp" // coordinates

/*
 * Simple path finder tha returns the best direction. Performs a recursive
 * search to determine which direction a) can get there and b) has the
 * lowest number of steps to reach the destination.
 * Returns only the direction.
 * 
 * maxDistance - how far before we give up and just move in vaguely
 * the right direction?
 */
template<int maxDistance>
class pathfinder {
 private:
  std::function<std::set<coord>(const coord&)> nextMoves_;
  std::set<coord> visited_;
public:
  pathfinder(std::function<std::set<coord>(const coord&)> nextMoves) :
    nextMoves_(nextMoves), visited_() {}

  /*
   * start - where are we?
   * end - where do we want to go?
   * nextMoves - functor taking a coord and returning a
   * set<pathfinder::dir> for the legal moves from that point
   */
  dir find(const coord & start, coord end) {
    visited_.emplace(start);

    std::set<coord> next = nextMoves_(start);
    unsigned long lowestCost = ULONG_MAX;
    coord best;
    for (coord nextC : next) {
      unsigned long cst = cost(nextC, end, 0);
      if (cst < lowestCost) {
	lowestCost = cst;
	best = nextC;
      }
    }
    return dir(static_cast<char>(best.first - start.first), 
	       static_cast<char>(best.second - start.second));
  }

  // returns the Manhatten distance
  static int absdistance(const coord &start, const coord &end) {
    return
      std::abs(start.first - end.first) +
      std::abs(start.second - end.second);      
  }

private:
  /*
   * c - start square for the next hop
   * end - target square
   * depth - number of hops so far on this route
   */
  unsigned long cost(const coord &c, const coord &end, int depth) {
    if (c == end) return depth; // found a solution!
    // if we are beyond the algorithm depth, return a big number, but add the distance
    // - that way, we will move and go as close as we can in the area we can "see"
    if (depth > maxDistance) return ULONG_MAX/2 + absdistance(c, end); // too far!
    visited_.emplace(c);

    // get all possible next steps
    std::set<coord> moves = nextMoves_(c);
    std::set<coord> availableMoves; // moves not in visited_
    std::set_difference(moves.begin(), moves.end(), visited_.begin(), visited_.end(), 
			std::inserter(availableMoves, availableMoves.end()));
    
    // lowest cost we've seen so far (look invariant) = max cost before the loop!
    unsigned long lowestCost = ULONG_MAX/2 + absdistance(c, end);
    if (availableMoves.empty()) return lowestCost; // don't go this way

    coord bestNext;
    for (coord cc : availableMoves) {
      unsigned long cst = cost(cc, end, depth+1);
      if (cst < lowestCost) {
	cst = lowestCost;
	bestNext = cc;
      }
    }
    visited_.erase(c); // remove this coord
    return lowestCost; // depth = number of moves
  }
};

#endif //ndef PATHFINDER_HPP
