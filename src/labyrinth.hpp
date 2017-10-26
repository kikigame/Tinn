/* License and copyright go here*/

// Labyrinthine algorithm

#ifndef LABYRINTH_HPP__
#define LABYRINTH_HPP__

#include <vector>
#include <utility> // for std::swap. Was in Algorithm prior to C++-11
#include <memory> // std::shared_ptr
#include <cmath> // std::abs, math::min etc
#include <sstream> // stringstream, used for error messages
#include <algorithm> // reverse, used by basicpath

struct rectangle {
  int xOff;
  int yOff;
  int width;
  int height;

  rectangle() : xOff(0), yOff(0), width(0), height(0) {}
  rectangle(const int x, const int y, const int w, const int h) : xOff(x), yOff(y), width(w), height(h) {}

  bool contains(const std::pair<int,int> & coords) const {
    if (xOff > coords.first) return false;
    if (yOff > coords.second) return false;
    if (xOff + width < coords.first) return false;
    if (yOff + height < coords.second) return false;
    return true;
  }
};

// utility:
// <T> is an iterator of iterables.
// reverses each iterator between begin and end.
// when given column iterators, reverses each row.
template <typename T>
void reverseRows(T begin, T end) {
  for (T i = begin; i != end; ++i)
    std::reverse(i->begin(), i->end());
}

// utility:
// <T> is a vector (or similar)
// takes a matrix and returns its transposition.
template <typename T>
std::vector<T> transpose(const std::vector<T> &in) {
  using namespace std;
  auto rtn = vector<T> (in[0].size(), T(in.size()));
  for (int x=0; x < in.size(); ++x)
    for (int y=0; y < in[x].size(); ++y)
      rtn[y][x] = in[x][y];
  return rtn;
}

template <typename T>
class emptyfiller;
template <typename T, typename F, bool goal = true>
class spiralfiller;
template <typename T>
class snakefiller;
template <typename T>
class castellationFillerW;
template <typename T>
class castellationFillerH;

// used by snakefiller and imperfect labyrinths when doSnake is set.
template <typename T, class L>
class snaker;

// interface:
// T is a datatype
// allow a data type of T to be recorded at a given pair of coordinates (first=x, second=y)
template <typename T>
class placable {
public:
  virtual void place(const std::pair<int,int> coords, const T &type) =0;
};

// this helper is used in building imperfect labyrinths.
template <typename T = char,
	  typename F = spiralfiller<T,snakefiller<T> > ,
	  bool doStripDoors = true,
	  bool doSnake = true
	  >
class imperfectRecursor;

// this helper will always build a perfect labyrinth, guaranteed
// - provided that the filler it uses will also build a perfect labyrinth.
template <typename T = char,
	  typename F = spiralfiller<T,snakefiller<T> > ,
	  bool doStripDoors = true,
	  bool doSnake = false
	  >
class perfectRecursor;

/*
 * This is the main entrypoint.
 * It draws unicorser labyrinths.
 * "unicursor" means that there is only one path, with no branching.
 * "labyrinth" refers to a maze, particularly if unicursor (though there are some exceptions).
 * By default, it draws perfect rectangular labyrinths.
 * "perfect" is a technical term meaning that every available space is used by the path.
 *  - i.e. there will be no 2x2 squares on the board of either black or white.
 * Other effects are possible by changing the template parameters from the defaults.
 * Call build() to generate, then begin() and end() to access the columns.
 *
 * <T> is the type of cell value.
 */
template <typename T = char,
	  typename F = spiralfiller<T,snakefiller<T> > ,
	  bool doStripDoors = true,
	  bool doSnake = false, // not needed with a perfect recorsor
	  typename R = perfectRecursor<T, F, doStripDoors, doSnake>
	  >
class labyrinth : public placable<T> {
  const rectangle bounds_;
  friend snaker<T, labyrinth>; // needs access to "place" function with force
  friend R;
  F filler_;
  std::shared_ptr<std::vector<std::vector<T> > > ts_;
  const T& in_;
  const T& out_;
  const T& unassigned_;
  const T& impass_;
  const T& pass_;
  const T& join_;
  R recursor_;
public:
    // Construct a labyrinth builder. Call build() before using.
  // NB: "entrance" and "exit" must be distinct symbols, but may otherwise be an arbitrary distinction.
  labyrinth(const T &in, // symbol to use to mark the entrance
	    const T& out, // symbol to use to mark the exit
	    const T& unassigned, // symbol used internally; should not appear on output but must be distinct.
	    const T& impass, // symbol for an impassible wall
	    const T& pass, // symbol for a passable space
	    const T& join, // used to join 
	    const int width, // maximum X-coordinate (width + 1) of cells
	    const int height) :// maximum Y-coordinate (width + 1) of cells
    bounds_(0, 0, width, height),
    filler_(pass, impass, unassigned, *this),
    ts_(new std::vector<std::vector<T> >(width+1, std::vector<T> (height+1, unassigned) )),
    in_(in),
    out_(out),
    unassigned_(unassigned),
    impass_(impass),
    pass_(pass),
    join_(join),
    recursor_(R(*this)) {}

  // begin() points to the first column in the labyrinth. Only valid after build() is called.
  typename std::vector<std::vector<T> >::const_iterator
  begin() const { return ts_->begin(); };
  // end() points to the one-past the end column in the labyrinth. Only valid after build() is called.
  typename std::vector<std::vector<T> >::const_iterator
  end() const { return ts_->end(); };

  // place a specific value into a specific cell.
  // Warning: some generators ignore this, so you may end up breaking the unicursor feature or introducing a dead end.
  // Not recommended unless you've checked the template parameters carefully.
  // See also in() and out().
  // This has no effect when called after build().
  virtual void place(const std::pair<int,int> coords, const T &type) {
    place(coords, type, false);
  }

  /*
   * Call before build() to override the location of the entrance.
   * Only valid if "in" and "out" characters are different; otherwise, you'll get 
   * a circular path ignoring one of in() and out().
   */
  void in(const int x, const int y) {
    std::pair<int,int> coord(x,y);
    place(coord, in_);
  }
  /*
   * Call before build() to override the location of the exit
   * Only valid if "in" and "out" characters are different; otherwise, you'll get 
   * a circular path ignoring one of in() and out().
   */
  void out(const int x, const int y) {
    std::pair<int,int> coord(x,y);
    place(coord, out_);
  }

  /*
   * Call before begin() and end() to actually build the labyrinth.
   */
  void build() {
    using namespace std;

    recursor_.build();

    if (doStripDoors)
      stripDoors();

    if (doSnake)
      snake();
  }

private:
  /*
   * Place type at coords. If force is true, it will overwrite any pass_, impass_, unassigned_
   * that was there before. Otherwise, only overwrites unassigned_.
   */
  void place(std::pair<int,int> coords, const T &type, bool force) {
    if (!bounds_.contains(coords))
      throw coords;
    auto cp = (*ts_)[coords.first][coords.second];
    if ((force && (pass_ == cp || impass_ == cp)) || unassigned_ == cp) 
      (*ts_)[coords.first][coords.second] = type;
  }

  /*
   * Find the first occurance of what and return its coordinates.
   * x=first, y=second.
   * Returns -1,-1 if not found, as this is always outside the coordinate system.
   */
  std::pair<int,int> find(const T& what) const {
    int x=0, y=0;
    for (auto ix : *ts_) {
      for (auto iy : ix) {
	if (iy == what)
	  return std::pair<int,int>(x,y);
	y++;
      }
      x++, y=0;
    }
    return std::pair<int,int>(-1,-1); 
  }

  // removes all join_ symbols and replaces with pass_.
  void stripDoors() {
    while (true) {
      const auto loc = find(join_);
      if (loc.first < 0) return;
      (*ts_)[loc.first][loc.second] = pass_;
    }
  } 

  // apply snakefiller to the whole map to obtain nearer perfection
  void snake() {
    snaker<T, labyrinth> f(pass_, impass_, unassigned_, *this);
    f.fill(bounds_);
  }

};



/*
 * Snake is a utility class that uses pattern matching to make straight paths snake more.
 */
template <typename T>
class snake {
protected:
  const T pass_;
  const T impass_;
  const T unassigned_;
  std::vector<std::vector<T> > cp_;
  rectangle bd_;
public:
  // used internally. You probably want snaker() or snakefill() instead.
  snake(const T & pass, const T &impass, const T &unassigned) :
    pass_(pass), impass_(impass), unassigned_(unassigned) {}
  virtual ~snake() {}
protected:
  // set the boundry and working buffer
  void setup(const rectangle &bd) {
    cp_ = std::vector<std::vector<T> >(bd.width+1, std::vector<T>(bd.height+1, unassigned_));
    bd_ = bd;
  }
  // improve the pathway within the given bounds, treating the area outside the bounds as impass_ (walls).
  // returns true if an improvement has been made, or false if further improvements are impossible.
  bool improve() {
    using namespace std;

    // NB: outside the bounds is treted as impass_
    // Translate
    // this:  to:   or this: to:
    // '?#?'  '?#?'   '##?' '##?'
    // ' ##'  '  #'   '## ' '#  '
    // ' ##'  '# #'   '## ' '# #'
    // ' ##'  '  #'   '## ' '#  '
    // '?#?'  '?#?'   '##?' '##?'

    // if we're not blocking diagonal movement, we don't care about diagonal cells.
    bool blockDiagonals = true;
    const T &  diag = blockDiagonals ? impass_ : unassigned_;
    {
      vector<vector<T> > pattern = { 
	{ diag       , impass_, impass_, impass_, diag },
	{ impass_    , impass_, impass_, impass_, impass_ },
	{ unassigned_, pass_, pass_, pass_, unassigned_ }
      };
      vector<vector<T> > replace = {
	{ unassigned_, impass_, impass_, impass_, unassigned_ },
	{ unassigned_, pass_, pass_, pass_, unassigned_ },
	{ unassigned_, pass_, impass_, pass_, unassigned_ }
      };
      if (findAndReplaceRR(pattern, replace))
	return true;
    }

    {
      vector<vector<T> > pattern = { 
	{ unassigned_, impass_, diag }, 
	{ pass_,       impass_, impass_ },
	{ pass_,       impass_, impass_ },
	{ pass_,       impass_, impass_ },
	{ unassigned_, impass_, diag },
      };
      vector<vector<T> > replace = {
	{ unassigned_, impass_, unassigned_ },
	{ pass_,       pass_,   impass_ },
	{ impass_,     pass_,   impass_ },
	{ pass_,       pass_,   impass_ },
	{ unassigned_, impass_, unassigned_ },
      };
      if (findAndReplaceRR(pattern, replace))
      	return true;
    }

    /*
    // Here's another pattern I've seen; happens when 2 rooms snake sub-optimally
    // '?? #??'    '?  #??'
    // '##   ?'    '# ## ?'
    // '######'    '#    #'
    // '   ##D'    '#####D'
    // ' # #??'    '   #??'
    */
    {
      vector<vector<T> > pattern = { 
	{ unassigned_, unassigned_, pass_,   impass_, unassigned_, unassigned_ },
	{ impass_,     impass_,     pass_,   pass_,   pass_,       unassigned_ },
	{ impass_,     impass_,     impass_, impass_, impass_,     impass_ },
	{ pass_,       pass_,       pass_,   impass_, impass_,     diag },
	{ pass_,       impass_,     pass_,   impass_, unassigned_, unassigned_ }
      };
      vector<vector<T> > replace = {
	{ unassigned_, pass_,       pass_,   impass_, unassigned_, unassigned_ },
	{ impass_,     pass_,       impass_, impass_, pass_,       unassigned_ },
	{ impass_,     pass_,       pass_,   pass_,   pass_,       impass_ },
	{ impass_,     impass_,     impass_, impass_, impass_,     unassigned_ },
	{ pass_,       pass_,       pass_,   impass_, unassigned_, unassigned_ }
      };
      if (findAndReplaceRR(pattern, replace))
      	return true;
    }

    {
      vector<vector<T> > pattern = { 
	{ unassigned_, impass_,     impass_, pass_,   pass_      },
	{ unassigned_, impass_,     impass_, pass_,   impass_    },
	{ pass_,       pass_,       impass_, pass_,   pass_      },
	{ impass_,     pass_,       impass_, impass_, impass_    },
	{ unassigned_, pass_,       impass_, impass_, unassigned_},
	{ unassigned_, unassigned_, impass_, diag,    unassigned_}
      };
      vector<vector<T> > replace = {
	{ unassigned_, impass_,     impass_, impass_, pass_},
	{ pass_,       pass_,       pass_,   impass_, pass_},
	{ pass_,       impass_,     pass_,   impass_, pass_},
	{ impass_,     impass_,     pass_,   impass_, impass_},
	{ unassigned_, pass_,       pass_,   impass_, unassigned_},
	{ unassigned_, unassigned_, impass_, unassigned_, unassigned_ }
      };
      if (findAndReplaceRR(pattern, replace))
      	return true;
    }

    // many other patterns are possible, but many are quite large to accommodate
    // moving of multiple paths to free up enough space to elongate one path and remove 2 extra impass_ cells.
    // Each pattern here needs to actually remove at least one impass_ cell, or you can get infinite loops.

    return false;
  }
private:
  /*
   * I tried to do each of these, but transposing proved unreliable, so it's now called twice with
   * transposed maps.
   *    A       B       C       D       E       F       G       H
   *  1 2 3   7 8 9   7 4 1   9 6 3   3 6 9   1 4 7   9 8 7   3 2 1
   *  4 5 6   4 5 6   8 5 2   8 5 2   2 5 8   2 5 8   6 5 4   6 5 4
   *  7 8 9   1 2 3   9 6 3   7 4 1   1 4 7   3 6 9   3 2 1   9 8 7
   */
  bool findAndReplaceRR(std::vector<std::vector<T> > find,
			std::vector<std::vector<T> > replace) {
    // each pattern should work reversed and rotated too:
    bool modified = false;

    // A
    modified |= findAndReplace(find, replace);

    // H
    reverseRows(find.begin(), find.end()); reverseRows(replace.begin(), replace.end());
    modified |= findAndReplace(find, replace);

    // G
    reverse(find.begin(), find.end());    reverse(replace.begin(), replace.end());
    modified |= findAndReplace(find, replace);

    // B
    reverseRows(find.begin(), find.end());    reverse(replace.begin(), replace.end());
    modified |= findAndReplace(find, replace);

    return modified;
  }

  // look for an occurrance of find" within the bounds and, if found, replae it with "replace".
  // find may contain _pass to match exactly, impass_ to match any impassible square or square outside the bounds,
  // or unasigned_ to match any square.
  // replace may contain _pass or impass_ to replace with that (but only if within the bounds), or unassigned_
  // to leave a square alone.
  bool findAndReplace(const std::vector<std::vector<T> > & find,
		      const std::vector<std::vector<T> > & replace) {
    bool modified = false;
    if (bd_.width < find[0].size() - 2) return modified; // too small (we can only overlap by 2)
    if (bd_.height < find.size() - 2) return modified; // too small (we can only overlap by 2)
    int maxXX = bd_.width + 2 - find[0].size();
    for (int xx=-1; xx <= maxXX; ++xx) {
      int maxYY = bd_.height + 2 - find.size();
      for (int yy=-1; yy <= maxYY; ++yy) {

	bool found = true;

	// go through the map at this offset, and compare each cell with the pattern.
	int x,y;
	auto ry = find.begin(), ryend = find.end();
	for (y=yy; ry != ryend && found; ++y,++ry) {
	  auto col = *ry;
	  auto rx = col.begin(), rxend = col.end();
	  for (x=xx; rx != rxend && found; ++x,++rx) {
	    const T & cellToMatch = *rx;
	    if (cellToMatch != unassigned_) {// unassigned (don't care) cells never clear found

	      T cp;
	      if (x < 0 || y < 0 || x > bd_.width || y > bd_.height)
		cp = impass_;
	      else cp = cp_[x][y];
	      
	      if (cellToMatch != cp)
		found = false;
	    }
	  }
	}
	// if the pattern matched, perform the replace.
	if (found) {
	  modified = true;
	  int x,y;
	  auto ry = replace.begin();
	  for (y=yy; ry != replace.end() && found; ++y,++ry) {
	    auto rx = ry->begin();
	    for (x=xx; rx != ry->end() && found; ++x,++rx) {
	      if (!(x < 0 || y < 0 || x > bd_.width || y > bd_.height)) {
		const T &cellToReplace = *rx;
		if (cellToReplace != unassigned_)
		  cp_[x][y] = cellToReplace;
	      }
	    }
	  }
	}
      }
    }
    return modified;
  }

};

/*
 * emptyfiller: pass this as the second parameter to labyrinth to fill each "room" with passable space.
 * This creates a "dungeon" consisting of holes in walls.
 */
template <typename T>
class emptyfiller {
private:
  const T & pass_;
  const T & impass_;
  placable<T> & l_;
public:
  emptyfiller(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    pass_(pass), impass_(impass), l_(l) {}
  void fill(const rectangle & bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    for (int x = bd.xOff, xc=0; xc <= bd.width; ++x, ++xc)
      for (int y = bd.yOff, yc=0; yc <= bd.height; ++y, ++yc)
	l_.place(std::pair<int, int>(x,y), pass_); // skips cells that already have stuff in them
  }
};

/*
#include <iostream>
std::ostream & operator << (std::ostream & out, const rectangle & bd) {
  out << "(" << bd.xOff << ',' << bd.yOff << ") + ("
      << bd.width << ',' << bd.height << ") = ("
      << (bd.xOff + bd.width) << ',' << (bd.yOff + bd.height) << ")";
  return out;
}
*/


/*
 * spralfiller: attempts to fill rooms with spirals, if the exit is not against the wall.
 *
 * T - type in cell
 * D - delegate filler of <T> in case we're too off-centre to spiral
 * goal - true to place a 3x3 room "target" around the exit, falso not to. The goal may break up walls.
 */
template <typename T, typename D, bool goal>
class spiralfiller {
private:
  D delegate;
  const T & pass_;
  const T & impass_;
  placable<T> & l_;
public:
  spiralfiller(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    delegate(pass, impass, unassigned, l),
    pass_(pass), impass_(impass), l_(l) {}
  void fill(const rectangle &bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    // are we near the middle?
    const long l2 = 2;
    const long midX2 = bd.xOff * l2 + bd.width;
    const long midY2 = bd.yOff * l2 + bd.height;

    const bool aAtEdge = (a.first == bd.xOff ||
			  a.first == bd.xOff + bd.width ||
			  a.second == bd.yOff ||
			  a.second == bd.yOff + bd.height);
    const bool bAtEdge = (b.first == bd.xOff ||
			  b.first == bd.xOff + bd.width ||
			  b.second == bd.yOff ||
			  b.second == bd.yOff + bd.height);
    if (bd.width < 6 || bd.height < 6) {
      delegate.fill(bd, a, b); // not enough space to spiral
      return;
    } 

    if (!aAtEdge && !bAtEdge) {
      std::stringstream errMsg;
      errMsg << "spiralfiller requires at least one exit on the edge. Bounds are ("
	     << bd.xOff << ',' << bd.yOff << ") size ("
	     << bd.width << ',' << bd.height << "); exits at "
	     << a.first << ',' << a.second << ';'
	     << b.first << ',' << b.second;
      throw errMsg.str();
    }

    if (aAtEdge && bAtEdge) {
      delegate.fill(bd, a,b);
      return;
    } 

    /*    
    std::cout << "spiralfiller ("
	     << bd.xOff << ',' << bd.yOff << ") size ("
	     << bd.width << ',' << bd.height << "); exits at "
	     << a.first << ',' << a.second << ';'
	     << b.first << ',' << b.second 
	      << (aAtEdge ? " A at edge " : " ")
	      << (bAtEdge ? " B at edge " : " ")
	      << std::endl;
    */
    if (aAtEdge) {
      edge_ = a;
      if (std::abs(b.first * l2 - midX2) >= 4 || 
	  std::abs(b.second * l2 - midY2) >= 4)
	delegate.fill(bd, a, b); // too off-center to spiral
      else
	spiral(bd, b, a, pass_, impass_);

      return;
    } 
    if (bAtEdge) {
      edge_ = b;
      if (std::abs(a.first * l2 - midX2) >= 4 || 
	  std::abs(a.second * l2 - midY2) >= 4)
	delegate.fill(bd, a, b); // too off-center to spiral
      else
	spiral(bd, a, b, pass_, impass_);
      return;
    }

    // !aAtEdge && !bAtEdge - could happen with a tiny map
    delegate.fill(bd, a,b);

  }
private:

  // as the 3-arg version but draws a 3x3 square of passable space around the exit first
  // This is a maze convention for points of interest/exits, and should help avoid unexpected branches if we're not
  // entirely on center.
  // It also fills in the exit as a passable square, in case we have the colours backwards on the last pass.
  void spiral(const rectangle &bd, const std::pair<int,int> exit, const std::pair<int,int> entrance,
	      const T black, const T red) {
    using namespace std;
    pair<int,int> e = exit;
    if (goal)
      for (e.second = max(bd.yOff, exit.second - 1); e.second <= min(bd.yOff + bd.height, exit.second + 1); e.second++)
	for (e.first = max(bd.xOff, exit.first - 1); e.first <= min(bd.xOff + bd.width, exit.first + 1); e.first++)
	  l_.place(e,pass_);
    l_.place(entrance, pass_);
    spiral(bd, black, red);
  }

  // recursive function. Each iteracion places a black square around the edge of the bd rectangle, with a gap of red.
  // by swapping the definition of red and black each turn, a spiral is achieved.
  void spiral(const rectangle &bd, const T black, const T red) {
    // we cheat. Just fill the space, and if don't worry if we overwrite the mid point, as place() will
    // not clobber the exit.

    // We know that the entrance is a square along the outside edge. We want to fill the entire outside edge,
    // aside from one square adjacent to the entrance.
    squaresPainted = 0;
    visit(bd.xOff, bd.yOff, black, red, 1, 1, true);
    for (int x=bd.xOff+1, xc=0; xc < bd.width - 1; x++, xc++)
      visit(x, bd.yOff, black, red, 0, 1);
    visit(bd.xOff + bd.width, bd.yOff, black, red, -1, 1, true);
    for (int y=bd.yOff+1, yc=0; yc < bd.height - 1; y++, yc++)
      visit(bd.xOff + bd.width, y, black, red, -1, 0);
    visit(bd.xOff + bd.width, bd.yOff + bd.height, black, red, -1, -1, true);
    for (int x=bd.xOff+1, xc=0; xc < bd.width - 1; x++, xc++)
      visit(x, bd.yOff+bd.height, black, red, 0, -1);
    visit(bd.xOff, bd.yOff + bd.height, black, red, 1, -1, true);
    for (int y=bd.yOff+1, yc=0; yc < bd.height; y++, yc++)
      visit(bd.xOff, y, black, red, +1, 0);

    // Recurse
    auto r = bd;
    r.xOff++;
    r.yOff++;
    r.width -=2;
    r.height -=2;
    if (r.width > 0 && r.height > 0)
      spiral(r, red, black);

  }

  int squaresPainted;
  bool lastPaintedCorner;
  std::pair<int,int> edge_;

  // called repeatedly to visit each point in the outer square.
  // paints the square red, apart from ones nearest the active point, which are black.
  // except in corners where the first 2 squares next to the active point are painted black instead.
  void visit(const int x, const int y, const T& red, const T& black, const int dx, const int dy, const bool iscorner = false) {
    const std::pair<int, int> coord(x,y);
    bool isRed = true;
    if (squaresPainted == 0 || (squaresPainted == 1 && lastPaintedCorner)) {
      const int dex = std::abs(edge_.first - x);
      const int dey = std::abs(edge_.second - y);
      const int de = dex + dey; // distance to edge
      if ((squaresPainted == 0 && de == 0) || (squaresPainted == 1 && de == 1)) {
	++squaresPainted;
	isRed = false;
	lastPaintedCorner = iscorner; // flag to paint a second square when starting at a corner
	//	if (!iscorner || de == 1) {
	  edge_ = coord;
	  edge_.first += dx;
	  edge_.second += dy;
	  l_.place(edge_, black);
	  //}
      }
    }
    l_.place(coord, isRed ? red : black);
  }

};


/*
 * pathfiller: place a minimal path between the two points.
 * This produces very imperfect labrynths, with short paths carved through thick walls.
 */
template <typename T>
class pathfiller {
  const T & pass_;
  const T & impass_;
  placable<T> & l_;
public:
  pathfiller(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    pass_(pass), impass_(impass), l_(l) {}
  void fill(const rectangle &bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    // start at A, go towards B
    std::pair<int,int> cur = a;
    while (cur != b) {
      l_.place(cur, pass_);
      if (cur.first == b.first && cur.second < b.second)
	cur.second++;
      else if (cur.first == b.first && cur.second > b.second)
	cur.second--;
      else if (cur.first < b.first)
	cur.first++;
      else
	cur.first--;
    }
    // fill in anything else with impassibility:
    for (int x = bd.xOff, xc=0; xc <= bd.width; x++,xc++)
      for (int y = bd.yOff, yc=0; yc <= bd.height; y++,yc++)
	l_.place(std::pair<int,int>(x,y), impass_);
  }
};

/*
 * snakefiller: force-fill the room by placing a path and deforming it, like a snake uncurling
 * as best it can.
 *
 * This is relatively slow.
 * Some things I expect to be matched still aren't.
 *
 */
template <typename T>
class snakefiller : private snake<T> {
private:
  placable<T> & l_;
  std::pair<int,int> ra_, rb_; // A and B in relative coords
public:
  snakefiller(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    snake<T>(pass, impass, unassigned), l_(l) {}
  virtual ~snakefiller(){}
  void fill(const rectangle &bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    snake<T>::setup(bd);
    // translate coords:
    ra_ = a, rb_ = b;
    ra_.first -= bd.xOff, ra_.second -= bd.yOff;
    rb_.first -= bd.xOff, rb_.second -= bd.yOff;

    // step 1: place a basic path
    basicPath();

    // step 2: look to see if we can improve it
    //for (int i=0; i < 100 && improve(); ++i);
    while(snake<T>::improve());

    // step 3: copy it back
    for (int x = bd.xOff, xc=0; xc <= bd.width; x++,xc++)
      for (int y = bd.yOff, yc=0; yc <= bd.height; y++,yc++)
	l_.place(std::pair<int,int>(x,y), snake<T>::cp_[xc][yc]);
  }

  private:
  void basicPath() {
    // start at ra_, go towards rb_
    std::pair<int,int> cur = ra_;
    while (cur != rb_) {
      snake<T>::cp_[cur.first][cur.second] = snake<T>::pass_;
      if (cur.first == rb_.first && cur.second < rb_.second)
	cur.second++;
      else if (cur.first == rb_.first && cur.second > rb_.second)
	cur.second--;
      else if (cur.first < rb_.first)
	cur.first++;
      else if (cur.first > rb_.first)
	cur.first--;
    };
    snake<T>::cp_[cur.first][cur.second] = snake<T>::pass_;
    // fill in anything else with impassibility:
    for (int x = 0; x <= snake<T>::bd_.width; x++)
      for (int y = 0; y <= snake<T>::bd_.height; y++)
	if (snake<T>::cp_[x][y] != snake<T>::pass_) snake<T>::cp_[x][y] = snake<T>::impass_;
  }
};



// applies a snake to the whole map:
template <typename T, class L>
class snaker : private snake<T> {
private:
  L & labyrinth_;
public:
  snaker(const T & pass, const T &impass, const T &unassigned, L & labyrinth) :
    snake<T>(pass, impass, unassigned), labyrinth_(labyrinth) {}
  virtual ~snaker() {};
  void fill(const rectangle &bd) {
    snake<T>::setup(bd);

    int x=0,y=0;
    for (auto i : labyrinth_) {
      for (auto j : i) {
	snake<T>::cp_[x][y] = j;
	++y;
      }
      ++x; y=0;
    }

    while (snake<T>::improve());

    for (int x = bd.xOff, xc=0; xc <= bd.width; x++,xc++)
      for (int y = bd.yOff, yc=0; yc <= bd.height; y++,yc++) {
	const T cp = snake<T>::cp_[xc][yc];
	bool out = cp == snake<T>::impass_ || cp == snake<T>::pass_;
	if (out)
	  labyrinth_.place(std::pair<int,int>(x,y), cp, true);
      }
  }
};


/*
 * castellationFillerW will fill the space with vertical lines, looking vaguely like the top of a castle 
 * (sort-of), but interlocking.
 * This only works properly if the exits are in opposite corners of the space, and that the number of 
 * horizontal cells are odd, which is only guaranteed by the imperfect filler. May result in an insolvable
 * maze if used as a general filler at present.
 */
template <typename T>
class castellationFillerW {
private:
  const T & pass_;
  const T & impass_;
  const T & unassigned_;
  placable<T> & l_;
public:
  castellationFillerW(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    pass_(pass), impass_(impass), unassigned_(unassigned), l_(l) {}
  void fill(const rectangle & bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    if (bd.width == 0 || bd.height == 0) {emptyfiller<T>(pass_, impass_, unassigned_, l_).fill(bd, a, b); return;}
    //    std::cout << "Castellation filler W with " << bd << " from " << a.first << ',' << a.second
    //	      << " to " << b.first << ',' << b.second << std::endl;
    // NB: We assume that a and b are in opposite corners; required for a perfect map.
    // lowerSide is true if the exit is on the side with the lowest numeric coord value
    bool lowerSide = (a.second < b.second) != (a.first < b.first);
    std::pair<int,int> pos(bd.xOff,bd.yOff);
    for (int countH = 0; countH <= bd.width; ++countH,++pos.first) {
      pos.second = bd.yOff;
      if ((countH % 2) == 0) {
	for (; pos.second <= (bd.height + bd.yOff); ++pos.second)
	  l_.place(pos, pass_);
      } else {
	l_.place(pos, lowerSide ? pass_ : impass_);
	for (++pos.second; pos.second < (bd.height + bd.yOff); ++pos.second)
	  l_.place(pos, impass_);
	l_.place(pos, lowerSide ? impass_ : pass_);
	lowerSide = !lowerSide;
      }
    }
  }
};


/*
 * castellationFillerH will fill the space with horizonal lines, looking vaguely like the top of a castle 
 * (sort-of), but interlocking.
 * This only works properly if the exits are in opposite corners of the space, and that the number of 
 * vertical cells are odd, which is only guaranteed by the imperfect filler. May result in an insolvable
 * maze if used as a general filler at present.
 */
template <typename T>
class castellationFillerH {
private:
  const T & pass_;
  const T & impass_;
  const T & unassigned_;
  placable<T> & l_;
public:
  castellationFillerH(const T & pass, const T &impass, const T &unassigned, placable<T> & l) :
    pass_(pass), impass_(impass), unassigned_(unassigned), l_(l) {}
  void fill(const rectangle & bd,
	    const std::pair<int,int> & a, 
	    const std::pair<int,int> & b) {
    if (bd.width == 0 || bd.height == 0) {emptyfiller<T>(pass_, impass_, unassigned_, l_).fill(bd, a, b); return;}
    //    std::cout << "Castellation filler H with " << bd << " from " << a.first << ',' << a.second
    //	      << " to " << b.first << ',' << b.second << std::endl;
    // NB: We assume that a and b are in opposite corners; required for a perfect map.
    bool lowerSide = (a.second < b.second) != (a.first < b.first);
    std::pair<int,int> pos(bd.xOff,bd.yOff);
    for (int countW = 0; countW <= bd.height; ++countW,++pos.second) {
      pos.first = bd.xOff;
      if ((countW % 2) == 0) {
	for (; pos.first <= (bd.width + bd.xOff); ++pos.first)
	  l_.place(pos, pass_);
      } else {
	l_.place(pos, lowerSide ? pass_ : impass_);
	for (++pos.first; pos.first < (bd.width + bd.xOff); ++pos.first)
	  l_.place(pos, impass_);
	l_.place(pos, lowerSide ? impass_ : pass_);
	lowerSide = !lowerSide;
      }
    }
  }
};



/*
 * Draw a (probably imperfect) rectangular labyrinth by successive recursion.
 *
 * The map will be divided into areas called "rooms" (not necessarily equivalent to map rooms), divided by walls,
 * then paths filled in around the walls. See the constructor for constants used.
 *
 * T - type to put in labyrinth cells
 * F - filter to use when rooms become too small to recurse into
 * doSnake - true to lengthen paths over the whole map where possible.
 * doStripDoors - true to remove join_ characters from the map before snaking. This may improve perfection of the maze, at the cost of losing your doors.
 */
template <typename T,
	  typename F,
	  bool doStripDoors,
	  bool doSnake
	  >
class imperfectRecursor {
  typedef labyrinth<T, F, doStripDoors, doSnake, imperfectRecursor<T,F,doStripDoors, doSnake> > lab;
  lab &l_;
  const int rd_; // recursion depth
  const rectangle bounds_;
public:
  imperfectRecursor(lab &l) :
    l_(l),
    rd_(0),
    bounds_(l.bounds_) {}

private:
  // for a sub-map
  imperfectRecursor(const int rd,
		    lab &l,
		    const rectangle bounds) :
    l_(l),
    rd_(rd+1),
    bounds_(bounds) {
  }

public:
  void build() {
    using namespace std;

    const T &in = l_.in_;
    const T &out = l_.out_;

    // has the entrance been set?
    pair<int,int> entrance = l_.find(in);
    if (entrance.first < bounds_.xOff) l_.place(entrance = pair<int,int>(0,0),in);

    pair<int,int> exit = l_.find(out);
    // build the exit room:
    if (exit.first < bounds_.xOff) {
      // place the exit in the middle
      exit.first = bounds_.xOff + bounds_.width / 2;
      exit.second = bounds_.yOff + bounds_.height / 2;
      l_.place(exit, out, true);
    }

    pair<int,int> & exitin = exit;

    rDivide(entrance, exitin);
  }

private:
  // divide recursively
  void rDivide(std::pair<int,int> entrance, std::pair<int,int> exit) {
    if (!bounds_.contains(entrance))
      throw entrance;
    if (!bounds_.contains(entrance))
      throw exit;

    //    if (rd_ == 6) return; // stop early

    // now splint into two submaps, ensuring that entrance and exitin don't fall on a boundry.
    // NB: the subdomains do not allow space for the boundry cells.
    const auto subdomains = divide(entrance, exit);

    if (subdomains.empty()) {
      infill(entrance,exit);
      return;
    }

    if (subdomains[0].first.contains(entrance)) {
      imperfectRecursor<T,F,doStripDoors,doSnake>(rd_, l_,
	       subdomains[0].first).rDivide(entrance, subdomains[0].second);
      imperfectRecursor<T,F,doStripDoors,doSnake>(rd_, l_,
	       subdomains[1].first).rDivide(subdomains[1].second, exit);
    } else {
      imperfectRecursor<T,F,doStripDoors,doSnake>(rd_, l_,
	       subdomains[0].first).rDivide(subdomains[0].second, exit);
      imperfectRecursor<T,F,doStripDoors,doSnake>(rd_, l_,
	       subdomains[1].first).rDivide(entrance, subdomains[1].second);
    }

  }

  // return a whole number between lowest & highest (inclusive) that is furthest from both a and b.
  // that means any value which has the greatest distance from the nearest number.
  // this implementation will always return the highest-value candidate number.
  int exitPoint(const int & lowest, const int & highest, const int & a, const int &b) const {
    // brute force for now:
    long maxDist = 0;
    int rtn = lowest;
    for (int i = lowest; i <= highest; ++i) {
      const long aDist = std::abs(i - a);
      const long bDist = std::abs(i - b);
      const long dist = std::min(aDist, bDist);
      if (dist > maxDist) {
	maxDist = dist; 
	rtn = i;
      }
    }
    return rtn;
  }

  // do the division of a single block
  std::vector<std::pair<rectangle,std::pair<int,int> > > divide(std::pair<int,int> in, std::pair<int,int> out) {
    using namespace std;
    if (in == out) {
      stringstream err;
      err << "Input and output art too similar ("
	  << in.first << ',' << in.second << "); ("
	  << out.first << ',' << out.second << ')';
      throw err.str();
    }
    long xDiff = (in.first - out.first);
    xDiff *= xDiff;
    long yDiff = (in.second - out.second);
    yDiff *= yDiff;

    vector<pair<rectangle, pair<int,int> > > rtn;

    const bool vertical = (xDiff > yDiff);
    int inExitX;
    int inExitY;
    if (vertical) {
      inExitX = (in.first + out.first) / 2;
      if (inExitX - bounds_.xOff < 3) return rtn; // need at least room to snake
      if (bounds_.width - (inExitX - bounds_.xOff) < 3) return rtn; // need at least room to snake
      if (in.first == inExitX || out.first == inExitX) throw "breakPos would intersect target";
      inExitY = exitPoint(bounds_.yOff, bounds_.yOff + bounds_.height, in.second, out.second);
      // fill boundry with exit
      for (int y = bounds_.yOff, c=0; c <= bounds_.height; ++y, ++c)
	if (y == inExitY) {
	  l_.place (pair<int,int>(inExitX - 1, y), l_.pass_);
	  l_.place (pair<int,int>(inExitX, y), l_.join_);
	  l_.place (pair<int,int>(inExitX + 1, y), l_.pass_);
	} else {
	  l_.place (pair<int,int>(inExitX, y), l_.impass_);
	}
      // inExitY is on the boundry, but the entrance and exit need to be the adjacent square in the subdomain
      rtn.push_back(pair<rectangle, pair<int,int> >
		    (rectangle(bounds_.xOff, bounds_.yOff, inExitX - bounds_.xOff - 1,                 bounds_.height),
		     pair<int,int>(inExitX-1, inExitY)));
      rtn.push_back(pair<rectangle, pair<int,int> >
		    (rectangle(inExitX+1,    bounds_.yOff, bounds_.width - inExitX + bounds_.xOff -1, bounds_.height),
		     pair<int,int>(inExitX+1, inExitY)));
    } else {
      inExitY = (in.second + out.second) / 2;
      if ((inExitY - bounds_.yOff) < 3) return rtn; // need at least room to snake
      if (bounds_.height - (inExitY - bounds_.yOff) < 3) return rtn; // need at least room to snake
      if (in.second == inExitY || out.second == inExitY) throw "breakPos would intersect target";
      // determine exit point
      inExitX = exitPoint(bounds_.xOff, bounds_.xOff + bounds_.width, in.first, out.first);
      // fill boundry with exit
      for (int x = bounds_.xOff, c=0; c <= bounds_.width; ++x, ++c)
	if (x == inExitX) {
	  l_.place (pair<int,int>(x, inExitY - 1), l_.pass_);
	  l_.place (pair<int,int>(x, inExitY), l_.join_);
	  l_.place (pair<int,int>(x, inExitY + 1), l_.pass_);
	} else {
	  l_.place (pair<int,int>(x, inExitY), l_.impass_);
	}
      // inExitX is on the boundry, but the entrance and exit need to be the adjacent square in the subdomain
      rtn.push_back(pair<rectangle, pair<int,int> >
		    (rectangle(bounds_.xOff, bounds_.yOff, bounds_.width, inExitY - bounds_.yOff -1),
		     pair<int,int>(inExitX, inExitY-1)));
      rtn.push_back(pair<rectangle, pair<int,int> >
		    (rectangle(bounds_.xOff, inExitY+1,    bounds_.width, bounds_.height - inExitY + bounds_.yOff -1),
		     pair<int,int>(inExitX, inExitY+1)));
    }
    return rtn;
  }

  /*
   * This is the end-game; we can recurse no smaller. The block is too small to recurse, and at least 3x3 in size.
   * We need to fill in the longest path from start to end, blocking out the rest.
   * Or just leave it open for a more room-like labyrinth.
   */
  void infill(const std::pair<int,int> & entrance, const std::pair<int,int> & exit) {
    l_.filler_.fill(bounds_, entrance, exit);
  }

};



/*
 * Draw a (conditionally perfect) rectangular labyrinth by successive recursion.
 *
 * The map will be divided into areas called "rooms" (not necessarily equivalent to map rooms), divided by walls,
 * then paths filled in around the walls. See the constructor for constants used.
 *
 * T - type to put in labyrinth cells
 * F - filter to use when rooms become too small to recurse into. This filler must produce a perfect solution when called with walls in the corners of an odd-sized room.
 * doSnake - true to lengthen paths over the whole map where possible.
 * doStripDoors - true to remove join_ characters from the map before snaking. This may improve perfection of the maze, at the cost of losing your doors.
 *
 * TODO: is doSnake useful here?
 */
template <typename T,
	  typename F,
	  bool doStripDoors,
	  bool doSnake
	  >
class perfectRecursor {
  typedef labyrinth<T, F, doStripDoors, doSnake, perfectRecursor<T,F,doStripDoors, doSnake> > lab;
  lab &l_;
  rectangle bounds_;
  castellationFillerW<T> delegate_;
public:
  perfectRecursor(lab &l) :
    l_(l),
    bounds_(l.bounds_),
    delegate_(l_.pass_, l_.impass_, l_.unassigned_, l_) {}

  void build() {
    using namespace std;

    const T &in = l_.in_;
    const T &out = l_.out_;

    // has the entrance been set?
    pair<int,int> entrance = l_.find(in);
    if (entrance.first < bounds_.xOff) l_.place(entrance = pair<int,int>(0,0),in);

    pair<int,int> exit = l_.find(out);

    // validate preconditions
    if (entrance.first != 0) throw "entrance must presently be at 0,0 for perfect recursor";
    if (entrance.second != 0) throw "entrance must presently be at 0,0 for perfect recursor";

    // build the exit room:
    if (exit.first < bounds_.xOff) {
      // place the exit 2 right of  the middle
      exit.first = 4*((bounds_.xOff + bounds_.width) / 8)+2;
      exit.second = (bounds_.yOff + bounds_.height) / 2;
      l_.place(exit, out, true);
    } else {
      if (exit.first != 4*((bounds_.xOff + bounds_.width) / 8)+2) throw "exit misplaced; must be 2 right of dead centre currently";
      if (exit.second < bounds_.yOff + 4) throw "exit too close to top wall; must be at least 4 away";
      if (exit.second >= bounds_.yOff + bounds_.height - 5) throw "exit too close to bottom wall; must be at least 6 away";
      //      if (exit.second != (bounds_.yOff + bounds_.height) / 2) throw "exit misplaced; must be 2 right of dead centre currently";
    }

    // [(number of cells wide)-1] must be divisible by 4.
    // Note that bounds_.width is 1 more than the width, because it starts counting at zero.
    if ((bounds_.width) % 4 != 2) throw "width must by 4 * integer + 3 in current implementation";
    if (bounds_.width < 18) throw "too narrow";
    if (bounds_.height < 11) throw "too short";

    pair<int,int> & exitin = exit;

    build(entrance, exitin);
  }

private:

  /*
   * Build the room via the unicorsor algorithm I described in Mazezes.txt
   *           X1      X2
   * #############################
   * #<  .   .   .   .   .   .   #
   * #     .   #   .   .   .   . #
   * ########################### # Y1
   * #   .   .         #   .   . #
   * # . . . . #,,,,,. # . . . . #
   * # . . . . #     , # . . . . #
   * # . . . . # , > , # . . . . #
   * # . . . . # ,,,,, # . . . . #
   * # .   .   #       # . . . . #
   * # ################# . . . . # Y2
   * # .   .   .   .   # . . . . #
   * # . . . . . . . . # . . . . #
   * #   .   .   .   .   .   .   #
   * #############################
   */
  void build(const std::pair<int,int> & entrance, const std::pair<int,int> & exit) {
    using std::pair;
    // where does the exit room go?
    int x1, x2, y1, y2;
    // x1 and x2 must be on the 4N+1 grid:
    for (x1=bounds_.xOff + 1; x1 +4 < bounds_.xOff + exit.first; x1+= 4);
    x1 -= 4;
    x2 = x1 + 8;
    y1 = exit.second - 3;
    y2 = y1 + 8;

    // walls:
    placeWallH(y1, bounds_.xOff, bounds_.width - 1);
    placeWallH(y2, bounds_.xOff + 1, x2);
    placeWallV(x1, bounds_.yOff + 1, y1);
    placeWallV(x1, y1 + 2, y2);
    placeWallV(x2, y1, bounds_.height + bounds_.yOff - 1);

    // joiners/doors:
    l_.place(pair<int,int>(x1, bounds_.yOff), l_.join_);
    l_.place(pair<int,int>(bounds_.xOff + bounds_.width, y1), l_.join_);
    l_.place(pair<int,int>(x2, bounds_.yOff + bounds_.height), l_.join_);
    l_.place(pair<int,int>(bounds_.xOff, y2), l_.join_);
    l_.place(pair<int,int>(x1, y1+1), l_.join_);
    // fillers:
    rectangle r1(x1+1, bounds_.yOff,
		 bounds_.width + bounds_.xOff - x1 -1, y1 - bounds_.yOff -1);
    delegate_.fill(r1, std::pair<int,int>(x1 + 1, bounds_.yOff + 1),
		   std::pair<int,int>(bounds_.xOff + bounds_.width - 1, y1-1));
    rectangle r2(x2+1, y1+1,
		 bounds_.width + bounds_.xOff - x2 -1, bounds_.height + bounds_.yOff - y1-1);
    delegate_.fill(r2, pair<int,int>(bounds_.xOff + bounds_.width, y1 + 1), 
		   pair<int,int>(x2 + 1, bounds_.yOff + bounds_.height));
    rectangle r3(bounds_.xOff, y2+1,
		 x2 - bounds_.xOff-1, bounds_.height + bounds_.yOff - y2-1);
    delegate_.fill(r3, pair<int,int>(x2 - 1, bounds_.yOff + bounds_.height - 1),
		   pair<int,int>(bounds_.xOff, y2+1));
    rectangle r4(bounds_.xOff, y1+1,
		 x1 - bounds_.xOff - 1, y2 - y1-2);
    delegate_.fill(r4, pair<int,int>(bounds_.xOff, y2-1), pair<int,int>(x1-1, y1+1));

    // fill the entrance
    fillA(x1, y1, entrance);

    // fill the exit
        spiralfiller<T, emptyfiller<T>, false> fs(l_.pass_, l_.impass_, l_.unassigned_, l_);
    //    emptyfiller<T> fs(l_.pass_, l_.impass_, l_.unassigned_, l_);
    rectangle rExit(x1+1, y1+1, x2 - x1 - 2, y2 - y1 - 2);
        fs.fill(rExit, pair<int,int>(x1+1,y1+1), exit);    
  }

  // fill room A from (0,0) to (x1,0) as best we can.
  // NB: there is no solution here, because the entrance is on the wrong coordinate.
  // NB: But that's okay; we're allowed to start with a goal room.
  void fillA(int x1, int y1, const std::pair<int,int> & entrance) {
    using std::pair;
    rectangle rA(bounds_.xOff + 4, bounds_.yOff, x1-5, y1 - bounds_.yOff - 1);
    delegate_.fill(rA, pair<int,int>(bounds_.xOff+3, bounds_.yOff), pair<int,int>(x1-1, bounds_.yOff));
    if (y1>1+bounds_.yOff) placeWallV(bounds_.xOff + 3, bounds_.yOff, y1-2);

    // castellationFiller doesn't work here, because there isn't usualy a perfect solution
    // (there can be, but it depends on the height, which has already been set to optimise the exit).
    // This is an optimised loop just for the entrance bit.
    int y = y1-1;
    bool exitLeft=false;
    while (y >= bounds_.yOff) {
      l_.place(pair<int,int>(bounds_.xOff,y), l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+1,y), l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+2,y), l_.pass_);
      if (--y < bounds_.yOff) break;
      exitLeft |= y == 3; // set the flag if we are " ##" on y=3.
      l_.place(pair<int,int>(bounds_.xOff,y), l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+1,y), y > 1 ? l_.impass_ : l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+2,y), y > 1 ? l_.impass_ : l_.pass_);
      if (--y < bounds_.yOff) break;
      l_.place(pair<int,int>(bounds_.xOff,y), l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+1,y), l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+2,y), l_.pass_);
      if (--y < bounds_.yOff) break;
      // if we were " ##" on y=3, then we can make this "## " even on row 1.
      // otherwise, keep row 1 blank in case we end up creating a dead end.
      l_.place(pair<int,int>(bounds_.xOff,y), ((y == 1 && exitLeft) || y > 1) ? l_.impass_ : l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+1,y), ((y == 1 && exitLeft) || y > 1) ? l_.impass_ : l_.pass_);
      l_.place(pair<int,int>(bounds_.xOff+2,y), l_.pass_);
      if (--y < bounds_.yOff) break;
    }
    l_.place(pair<int,int>(bounds_.xOff + 3, y1-1), l_.join_);
  }

  // place a vertical wall
  void placeWallV(const int &xPos, const int &yA, const int &yB) {
    const int yMin=std::min(yA,yB), yMax=std::max(yA,yB);
    for (int y=yMin; y<=yMax; ++y) l_.place(std::pair<int,int>(xPos, y), l_.impass_);
  }
  // place a horizontal wall
  void placeWallH(const int &yPos, const int &xA, const int &xB) {
    const int xMin=std::min(xA,xB), xMax=std::max(xA,xB);
    for (int x=xMin; x<=xMax; ++x) l_.place(std::pair<int,int>(x, yPos), l_.impass_);
  }


};



#endif //ndef LABYRINTH_HPP__
