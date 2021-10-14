/* License and copyright go here*/

// utility class to hord a coordinate pair

#ifndef COORD_HPP__
#define COORD_HPP__

#include <utility> // for pair
#include <ostream> // for ostream
#include <functional> // for function


class dir : public std::pair<signed char, signed char> {
public:
  dir() : std::pair<signed char, signed char>(0,0) {}
  dir(const dir &other) = default;
  dir(int x, int y) :
    std::pair<signed char, signed char>(x < 0 ? -1 : x > 0 ? +1 : 0,
					y < 0 ? -1 : y > 0 ? +1 : 0) {}
  dir(const wchar_t t) {
    switch (t) {
    case L'W': case L'w': first =  0; second = -1; break;
    case L'A': case L'a': first = -1; second =  0; break;
    case L'S': case L's': first =  0; second = +1; break;
    case L'D': case L'd': first = +1; second =  0; break;
    default: throw t;
    }
  }
  bool operator < (const dir &other) const {
    return first < other.first || (first == other.first && second < other.second);
  }
  bool operator == (const dir &other) const {
    return first == other.first && second == other.second;
  }
};

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
  coord next(const coord &then) const {
    coord rtn;
    rtn.first = then.first + then.first - first;
    rtn.second = then.second + then.second - second;
    return rtn;
  }
  /*
   * Return the coord 1-space closer to "toward"
   * in each dimension. eg if this = (2,3) and toward = (5,7)
   * would return (3,4).
   * any dimension where this and toward are the same are unchanged,
   * eg if this = (2,3) and toward = (2,7)
   * would return (2,4).
   */
  coord towards(const coord &toward) const {
    coord rtn;
    rtn.first = 
      toward.first > first ? first+1 :
      toward.first == first ? first : first - 1;
    rtn.second = 
      toward.second > second ? second+1 :
      toward.second == second ? second : second - 1;
    return rtn;
  }
  /*
   * Return the coord 1-space further from "toward"
   * in each dimension. eg if this = (2,3) and toward = (5,7)
   * would return (1,2).
   * any dimension where this and toward are the same are unchanged,
   * eg if this = (2,3) and toward = (2,7)
   * would return (2,2).
   */
  coord away(const coord &toward) const {
    coord rtn;
    rtn.first = 
      toward.first > first ? first-1 :
      toward.first == first ? first : first + 1;
    rtn.second = 
      toward.second > second ? second-1 :
      toward.second == second ? second : second + 1;
    return rtn;
  }
  dir dirTo(const coord &other) const {
    return dir(other.first - first, other.second - second);
  }
  dir dirFrom(const coord &other) const {
    return dir(first - other.first, second - other.second);
  }
  coord inDir(const dir &d) const {
    coord rtn = *this;
    rtn.first += d.first;
    rtn.second += d.second;
    return rtn;
  }
  // number of squares (cardinals & diagonals) from arg
  unsigned int linearDistance(const coord &other) const {
    if (*this == other) return 0;
    coord c = towards(other);
    return (1+c.linearDistance(other));
  }
  // abitrarily sort on x then y
  bool operator <(const coord &other) const {
    return first < other.first ? true :
      (first == other.first && second < other.second);
  }
  // abitrarily sort on x then y
  bool operator >(const coord &other) const {
    return first > other.first ? true :
      (first == other.first && second > other.second);
  }
  bool operator ==(const coord &other) const {
    return first == other.first && second == other.second;
  }
  coord operator +(const coord &other) const {
    return coord(first + other.first, second + other.second);
  }
  coord operator -(const coord &other) const {
    return coord(first - other.first, second - other.second);
  }
};

// output coordinates
std::ostream & operator << (std::ostream & out, const coord & c);
std::wostream & operator << (std::wostream & out, const coord & c);

// iterate over rectangular coordinates
class coordRectIterator {
private:
  coord tl_, br_, cur_;
public:
  coordRectIterator(int l, int t, 
		    int r, int b) : tl_({l,t}), br_({r,b}), cur_(tl_) {};
  coordRectIterator(coord tl, coord br) : tl_(tl), br_(br), cur_(tl) {};
  coordRectIterator(coord tl, coord br, coord cur) : tl_(tl), br_(br), cur_(cur) {};
  coordRectIterator begin() { return *this; } // copy
  coordRectIterator end() { coord e(tl_.first, br_.second+1); return coordRectIterator(tl_, br_, e); }
  coordRectIterator& operator ++() { cur_.first++; 
    if (cur_.first > br_.first) {cur_.second++; cur_.first = tl_.first; }
    return *this; }
  const coord &operator *() { return cur_; }
  bool operator == (const coordRectIterator &o) { return o.cur_ == cur_ && o.tl_ == tl_ && o.br_ == br_; }
  bool operator != (const coordRectIterator &o) { return o.cur_ != cur_ || o.tl_ != tl_ || o.br_ != br_; }
};

// return a random coord for which the functor returns true
coord rnd(coordRectIterator, std::function<bool(const coord&)>);

#endif // ndef COORD
