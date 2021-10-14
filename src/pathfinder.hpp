/* License and copyright go here*/

// path finding algorithm of some sort

#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

//#define DEBUG_PATHS 1

#include <climits>
#include <set>
#ifdef DEBUG_PATHS
#include <iostream>
#include <iomanip>
#endif //def DEBUG_PATHS
#include <cstdlib> // abs
#include "coord.hpp" // coordinates

/*
 * Brute force pathfinder alrogithm.
 * 
 * maxDistance - how far before we give up and just move in vaguely
 * the right direction?
 */
template<int maxDistance>
class pathfinder {
 private:
  static constexpr size_t gridsize = 2*maxDistance+1;
  const coord origin_;
  std::function<bool(const coord&)> pass_;
  /*
   * start position is [maxDistance][maxDistance]
   */
  std::array<std::array<unsigned char, gridsize>, gridsize> costs_;
public:
  pathfinder(std::function<bool(const coord&)> pass) :
    origin_(0,0), pass_(pass), costs_() {}

  // access the array using coordinate translation:
  // 0,0 is the start postion:
  unsigned char &cost(const int &x, const int &y) {
    return costs_[x + maxDistance][y +maxDistance];
  }
  unsigned char &cost(const coord &c) {
    return costs_[c.first + maxDistance][c.second + maxDistance];
  }
  unsigned char icost(const int &x, const int &y) const {
    return costs_.at(x + maxDistance).at(y + maxDistance);
  }

  constexpr unsigned char min(unsigned char a, unsigned char b) const {
    return a < b ? a : b;
  }

  constexpr int min(int a, int b) const {
    return a < b ? a : b;
  }

  constexpr unsigned char max(unsigned char a, unsigned char b) const {
    return a > b ? a : b;
  }

  constexpr int max(int a, int b) const {
    return a > b ? a : b;
  }

  constexpr unsigned char min(unsigned char a, unsigned char b, unsigned char c) const {
    return a < b ? a : b < c ? b : c;
  }

  /*
   * start - where are we?
   * end - where do we want to go?
   * returns direction to head in
   */
  dir find(const coord & start, const coord & end) {
    using std::abs;

    if (start == end) return dir(0,0);

    // target is where we're aiming for; if "end" is too far, go as far as we can (actual coords)
    coord target;
    auto adx = abs(end.first - start.first);
    auto ady = abs(end.second - start.second);
    if (adx > maxDistance || ady > maxDistance) {
      int div = adx > ady ? adx : ady;
      target = coord(maxDistance * (end.first - start.first) / div + start.first,
		     maxDistance * (end.second - start.second) / div + start.second);
    } else {
      target = end;
    }

#ifdef DEBUG_PATHS
    std::cerr << "find path from " << start << "-> " << end << ":" << target
	      << "\t" << maxDistance
	      << std::endl;
#endif //def DEBUG_PATHS
    
    cost(0,0) = 0;
    coord cursor(0,0); // relative coordinates!
    for (unsigned char d = 1; d <= maxDistance; ++d)
      if (fillCosts(start, end, d, cursor)) break;
    
#ifdef DEBUG_PATHS
    std::cerr << " - costs are: " << std::endl;
    for (int y = 0; y < maxDistance*2+1; ++y) {
      for (int x = 0; x < maxDistance*2+1; ++x)
	std::cerr << std::setw(5) << static_cast<int>(costs_[x][y]);
      std::cerr << std::endl;
    }
    std::cerr << " - bigest pass value set at " << cursor << std::endl;
#endif //def DEBUG_PATHS
    
    // find end (or the nearest square to it) and the cost of that square,
    // at this point, "start" contains the coord of the largest filled cost on the shortest path, in actual coords
    unsigned char c;
    while (true) {
      c = cost(cursor);
      if (c > 127) { // can't move to target, so try each square closer until we can
	coord cc = cursor.towards(origin_);
	if (cc == origin_)
	  return dir(0,0); // can't move any closer
	cursor = cc;
      } else break;
    }
    // then work backwards by the next-lowest cost to find the route.
    coord prevCursor(0,0);
    do {
      prevCursor = cursor;
      moveCursor(cursor, --c);
    } while (c > 0);
    
    // for now, we can just return the dir based on the final reverse-step.
    dir rtn(prevCursor.first - cursor.first, prevCursor.second - cursor.second);

#ifdef DEBUG_PATHS
    std::cerr << "cursor="
              << prevCursor << " to "
              << cursor
	      << std::endl
	      << "dir=" << static_cast<int>(rtn.first)
	      <<    "," << static_cast<int>(rtn.second)
	      << std::endl;
#endif //def DEBUG_PATHS
    
    return rtn;
    // there's a possiblily to cache some things provided we invalidate some or all of the cache when the board changes.
  }

  // find a square adjacent to (x,y) with cost c.
  // returned by modifying cursor param (which is relative coords).
  // NB: This is used to work out the shortest route FROM the target position TOWARDS the start.
  void moveCursor(coord &cursor, unsigned char c) {
    const int maxX = min(cursor.first+1, maxDistance);
    const int maxY = min(cursor.second+1, maxDistance);
#ifdef DEBUG_PATHS
    std::cerr << "\nCosts:";
#endif //def DEBUG_PATHS
    for (int x=max(cursor.first-1,-maxDistance); x <= maxX; ++x)
      for (int y=max(cursor.second-1,-maxDistance); y <= maxY; ++y) {
#ifdef DEBUG_PATHS
	std::cerr << "\tcost (" << x << "," << y << ") == " << cost(x,y) << "<=>" << c;
#endif //def DEBUG_PATHS
	if (cost(x, y) == c) {
	  cursor = coord(x,y);
	  return;
	}
      }
#ifdef DEBUG_PATHS
    std::cerr << "no " << static_cast<int>(c) << " found next to " << cursor << std::endl;
#endif //def DEBUG_PATHS
    // no route; fall back to moving in a straight line so we can at least get closer
    cursor = cursor.towards(origin_);
  }
  
  bool fillCosts(const coord & start, const coord & end, const unsigned char &d, coord &cursor) {
    coord c;

    // corners
    for (int x=-d; x <= d; x += d*2)
      for (int y=-d; y <= d; y += d*2) {
	coord rel(x,y);
	if (pass_(rel + start)) {
	  coord earlier = rel.towards(origin_);
	  cost(rel) = cost(earlier)+1;
	  if (cost(rel) < 128) cursor=rel;
	} else cost(rel) = 128;
	if (c == end) return true;
      }
    
    for (int x = 1- d; x < +d; ++x)
      // top & bottom rows
      for (int y=-d; y <= d; y += d*2) {
	coord rel(x,y);
	if (pass_(rel + start)) {
	  coord earlier = rel.towards(origin_);
	  cost(rel) = min(icost(earlier.first-1, earlier.second), cost(earlier), icost(earlier.first+1, earlier.second))+1;
	  if (cost(rel) < 128) cursor=rel;
	} else cost(rel) = 128;
	if (c == end) return true;
      }
    
    for (int y = - d+1; y < +d; ++y)
      for (int x=-d; x <= d; x += d*2) {

	// left & right cols
	coord rel(x,y);
	if (pass_(rel + start)) {
	  coord earlier = rel.towards(origin_);
	  cost(rel) = min(icost(earlier.first, earlier.second-1), cost(earlier), icost(earlier.first, earlier.second+1))+1;
	  if (cost(rel) < 128) cursor=rel;
	} else cost(rel) = 128;
	if (c == end) return true;
      }
    return false;
  }
};


#endif //ndef PATHFINDER_HPP
