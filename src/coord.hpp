/* License and copyright go here*/

// utility class to hord a coordinate pair

#ifndef COORD_HPP__
#define COORD_HPP__

#include <utility> // for pair
#include <ostream> // for ostream

/*
 * first  = x coordinate (right from left on screen)
 * secord = y coordinate (down from top of screen)
 * (assuming default coordinate space with no transmutations)
 */
class coord : public std::pair<int,int> {
public:
  coord() {
    first = 0, second = 0;
  }
  virtual ~coord() {}
  coord(int xx, int yy) {
    first = xx, second=yy;
  }
  coord(const coord & rhs) {
    first = rhs.first, second = rhs.second;
  }
  //  coord & operator=(const coord & rhs) = default;
  /*
   * If the movement from (this) to (then) is a chess-piece move,
   * Apply the same move to (then) and return the
   * next in sequence. eg if this = (2,3)
   * and then = (5,7)
   * would return (8,11) (each move being +3,+4)
   */
  coord next(const coord &then) {
    coord rtn;
    rtn.first = then.first + then.first - first;
    rtn.second = then.second + then.second - second;
    return rtn;
  }
};

std::ostream & operator << (std::ostream & out, const coord & c);
std::wostream & operator << (std::wostream & out, const coord & c);

#endif // ndef COORD
