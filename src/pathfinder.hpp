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
  std::function<bool(const coord&)> pass_;
  std::array<std::array<unsigned char, gridsize>, gridsize> costs_;
public:
  pathfinder(std::function<bool(const coord&)> pass) :
    pass_(pass), costs_() {}

  // 0,0 is the start postion:
  unsigned char &cost(const int &x, const int &y) {
    return costs_[x + maxDistance][y +maxDistance];
  }
  unsigned char icost(const int &x, const int &y) const {
    return costs_.at(x + maxDistance).at(y + maxDistance);
  }

  unsigned char min(unsigned char a, unsigned char b) const {
    return a < b ? a : b;
  }

  unsigned char min(unsigned char a, unsigned char b, unsigned char c) const {
    return a < b ? a : b < c ? b : c;
  }

  /*
   * start - where are we?
   * end - where do we want to go?
   * nextMoves - functor taking a coord and returning a
   * set<pathfinder::dir> for the legal moves from that point
   */
  dir find(const coord & start, const coord & end) {
    using std::abs;

    if (start == end) return dir(0,0);
    
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
    for (unsigned char d = 1; d <= maxDistance; ++d)
      if (fillCosts(start, end, d)) break;

#ifdef DEBUG_PATHS
    std::cerr << " - costs are: " << std::endl;
    for (int x = 0; x < maxDistance*2+1; ++x) {
      for (int y = 0; y < maxDistance*2+1; ++y)
	std::cerr << std::setw(5) << static_cast<int>(costs_[x][y]);
      std::cerr << std::endl;
    }
#endif //def DEBUG_PATHS
    
    // find end (or the nearest square to it) and the cost of that square,
    /*   coord target = end;
    if (!(abs(start.first - end.first) < maxDistance) ||
	!(abs(start.second - end.second) < maxDistance)) {
      do {
	target = target.towards(start);;
      } while (!(abs(start.first - end.first) < maxDistance) ||
	       !(abs(start.second - end.second) < maxDistance));
	       }*/
    int cursorX = target.first - start.first + maxDistance;
    int cursorY = target.second - start.second + maxDistance;
    unsigned char c = cost(cursorX, cursorY);
    // then work backwards by the next-lowest cost to find the route.
    int prevCursorX, prevCursorY;
    while (c > 0) {
      prevCursorX = cursorX, prevCursorY = cursorY;
      moveCursor(cursorX, cursorY, c--);
    }
    
    // for now, we can just return the dir based on the final reverse-step.
    dir rtn(cursorX - prevCursorX, cursorY - prevCursorY);

#ifdef DEBUG_PATHS
    std::cerr << "cursor="
              << coord(prevCursorX, prevCursorY) << " to "
              << coord(cursorX, cursorY)
	      << std::endl
	      << "dir=" << static_cast<int>(rtn.first)
	      <<    "," << static_cast<int>(rtn.second)
	      << std::endl;
#endif //def DEBUG_PATHS
    
    return rtn;
    // there's a possiblily to cache some things provided we invalidate some or all of the cache when the board changes.
  }

  // find a square adjacent to (x,y) with cost c.
  // returned by modifying cursor params.
  void moveCursor(int &cursorX, int &cursorY, unsigned char c) {
    bool xgt = cursorX > -maxDistance;
    bool xlt = cursorX < +maxDistance;
    bool ygt = cursorY > -maxDistance;
    bool ylt = cursorY < +maxDistance;
    int x = cursorX, y = cursorY;

    if (xgt && ygt)
      if (cost(cursorX = x-1, cursorY = y-1) == c) return;
    if (ygt)
      if (cost(cursorX = x  , cursorY = y-1) == c) return;
    if (xlt && ygt)
      if (cost(cursorX = x+1, cursorY = y-1) == c) return;

    if (xgt)
      if (cost(cursorX = x-1, cursorY = y  ) == c) return;
    if (xlt)
      if (cost(cursorX = x+1, cursorY = y  ) == c) return;
  
    if (xgt && ylt)
      if (cost(cursorX = x-1, cursorY = y+1) == c) return;
    if (ylt)
      if (cost(cursorX = x  , cursorY = y+1) == c) return;
    /* guaranteed
    if (xlt && ylt)
    if (cost(cursorX = x+1, cursorY = y+1) == c) return;*/
    cursorX = x+1, cursorY = y+1;
  }
  
  bool fillCosts(const coord & start, const coord & end, const unsigned char &d) {
    coord c;
    // top-left corner
    cost(-d,-d) = pass_(c = coord(start.first-d, start.second-d)) ?
      cost(-d+1, -d+1)+1 : LONG_MAX/2;
    if (c == end) return true;
    // top-right corner
    cost(+d,-d) = pass_(c = coord(start.first+d, start.second-d)) ?
      cost(+d-1, -d+1)+1 : LONG_MAX/2;
    if (c == end) return true;
    // btm-left corner
    cost(-d,+d) = pass_(c = coord(start.first-d, start.second+d)) ?
      cost(-d+1, +d-1)+1 : LONG_MAX/2;
    if (c == end) return true;
    // btm-right corner
    cost(+d,+d) = pass_(c = coord(start.first+d, start.second+d)) ?
      cost(+d-1, +d-1)+1 : LONG_MAX/2;
    if (c == end) return true;
    
    for (int x = 1- d; x < +d; ++x) {
      // top row
      cost(x,-d) = pass_(c = coord(start.first+x, start.second-d)) ?
	min(icost(x-1, 1-d), icost(x, 1-d), icost(x+1, 1-d))+1
	: LONG_MAX/2;
      if (c == end) return true;
      // bottom row
      cost(x,+d) = pass_(c = coord(start.first+x, start.second+d)) ?
	min(icost(x-1, d-1), icost(x, d-1), icost(x+1, d-1))+1
	: LONG_MAX/2;
      if (c == end) return true;
    }
    for (int y = - d+1; y < +d; ++y) {
      // left row
      cost(-d,y) = pass_(c = coord(start.first-d, start.second+y)) ?
	min(icost(1-d, y-1), icost(1-d, y), icost(1-d, y+1))+1
	: LONG_MAX/2;
      if (c == end) return true;
      // right row
      cost(+d,y) = pass_(c = coord(start.first+d, start.second+y)) ?
	min(icost(d-1, y-1), icost(d-1, y), icost(d-1, y+1))+1
	: LONG_MAX/2;
      if (c == end) return true;
    }
    return false;
  }
};


#endif //ndef PATHFINDER_HPP
