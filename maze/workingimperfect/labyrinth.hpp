/* License and copyright go here*/

// Labyrinthine algorithm

#ifndef LABYRINTH_HPP__
#define LABYRINTH_HPP__

#include <vector>
#include <utility> // for std::swap. Was in Algorithm prior to C++-11
#include <memory> // std::shared_ptr
#include <cmath> // std::abs, math::min etc
#include <sstream> // strstream, used for error messages
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

template <typename T>
class emptyfiller;
template <typename T, typename F>
class spiralfiller;
template <typename T>
class snakefiller;

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
template <typename T = char,
	  typename F = spiralfiller<T,emptyfiller<T> > ,
	  bool doSnake = true,
	  bool doStripDoors = true
	  >
class labyrinth : public placable<T> {
  friend snaker<T, labyrinth>; // needs access to "place" function with force
  const int rd_; // recursion depth
  std::shared_ptr<std::vector<std::vector<T> > > ts_;
  const T& in_;
  const T& out_;
  const T& unassigned_;
  const T& impass_;
  const T& pass_;
  const T& join_;
  const rectangle bounds_;
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
    rd_(0),
    ts_(new std::vector<std::vector<T> > ),
    in_(in),
    out_(out),
    unassigned_(unassigned),
    impass_(impass),
    pass_(pass),
    join_(join),
    bounds_(0, 0, width, height) {
    for (int x=0; x <= width; ++x) {
      ts_->push_back(std::vector<T>());
      for (int y=0; y <= height; ++y)
	(*ts_)[x].push_back(unassigned_);
    }
  }
  virtual ~labyrinth() {}
  typename std::vector<std::vector<T> >::const_iterator
  begin() const { return ts_->begin(); };
  typename std::vector<std::vector<T> >::const_iterator
  end() const { return ts_->end(); };

  virtual void place(const std::pair<int,int> coords, const T &type) {
    place(coords, type, false);
  }
private:
  // for a sub-map
  labyrinth(const int rd,
	   std::shared_ptr<std::vector<std::vector<T> > > ts,
	   const T&in,
	   const T&out,
	   const T& unassigned,
	   const T& impass,
	   const T& pass,
	   const T& join,
	   const rectangle bounds) :
    rd_(rd+1),
    ts_(ts),
    in_(in),
    out_(out),
    unassigned_(unassigned),
    impass_(impass),
    pass_(pass),
    join_(join),
    bounds_(bounds) {
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
  std::pair<int,int> find(const T& what) {
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

public:
  /*
   * Call before build() to override the location of the entrance
   */
  void in(const int x, const int y) {
    std::pair<int,int> coord(x,y);
    place(coord, in_);
  }
  /*
   * Call before build() to override the location of the exit
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

    // has the entrance been set?
    pair<int,int> entrance = find(in_);
    if (entrance.first < bounds_.xOff) place(entrance = pair<int,int>(0,0),in_);

    pair<int,int> exit = find(out_);
    // build the exit room:
    if (exit.first < bounds_.xOff) {
      // place the exit in the middle
      exit.first = bounds_.xOff + bounds_.width / 2;
      exit.second = bounds_.yOff + bounds_.height / 2;
      place(exit, out_, true);
    }

    pair<int,int> & exitin = exit;

    rDivide(entrance, exitin);

    if (doStripDoors)
      stripDoors();

    if (doSnake)
      snake();
  }

private:
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
      labyrinth(rd_, ts_, in_, out_, unassigned_, impass_, pass_, join_, 
	       subdomains[0].first).rDivide(entrance, subdomains[0].second);
      labyrinth(rd_, ts_, in_, out_, unassigned_, impass_, pass_, join_, 
	       subdomains[1].first).rDivide(subdomains[1].second, exit);
    } else {
      labyrinth(rd_, ts_, in_, out_, unassigned_, impass_, pass_, join_, 
	       subdomains[0].first).rDivide(subdomains[0].second, exit);
      labyrinth(rd_, ts_, in_, out_, unassigned_, impass_, pass_, join_, 
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
	  place (pair<int,int>(inExitX - 1, y), pass_);
	  place (pair<int,int>(inExitX, y), join_);
	  place (pair<int,int>(inExitX + 1, y), pass_);
	} else {
	  place (pair<int,int>(inExitX, y), impass_);
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
	  place (pair<int,int>(x, inExitY - 1), pass_);
	  place (pair<int,int>(x, inExitY), join_);
	  place (pair<int,int>(x, inExitY + 1), pass_);
	} else {
	  place (pair<int,int>(x, inExitY), impass_);
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
    F f(pass_, impass_, unassigned_, *this);
    f.fill(bounds_, entrance, exit);
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
	vector<T>(5, impass_),
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
      //      if (findAndReplaceRR(pattern, replace))
      //	return true;
    }

    // There may be other patterns we can try here, but somehow I doubt it.

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
  bool findAndReplaceRR(std::vector<std::vector<T> > & find,
			std::vector<std::vector<T> > & replace) {
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
    int maxXX = bd_.width + 1 - find[0].size();
    for (int xx=-1; xx <= maxXX; ++xx) {
      int maxYY = bd_.height + 1 - find.size();
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
	      if (x < 0 || y < 0 || x > bd_.width+1 || y > bd_.height+1)
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
 * spralfiller: attempts to fill rooms with spirals, if the exit is not against the wall.
 */
template <typename T, typename D>
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

    if (aAtEdge && bAtEdge) {
      delegate.fill(bd, a,b);
      return;
    } 

    if (aAtEdge) {
      edge_ = a;
      if (std::abs(b.first * l2 - midX2) >= 4 || 
	  std::abs(b.second * l2 - midY2) >= 4)
	delegate.fill(bd, a, b); // too off-center to spiral
      else
	spiral(bd, b, pass_, impass_);

      return;
    } 
    if (bAtEdge) {
      edge_ = b;
      if (std::abs(a.first * l2 - midX2) >= 4 || 
	  std::abs(a.second * l2 - midY2) >= 4)
	delegate.fill(bd, a, b); // too off-center to spiral
      else
	spiral(bd, a, pass_, impass_);
      return;
    }

    // !aAtEdge && !bAtEdge - could happen with a tiny map
    delegate.fill(bd, a,b);

  }
private:

  // as the 3-arg version but draws a 3x3 square of passable space around the exit first
  // This is a maze convention for points of interest/exits, and should help avoid unexpected branches if we're not
  // entirely on center.
  void spiral(const rectangle &bd, const std::pair<int,int> exit, const T black, const T red) {
    using namespace std;
    pair<int,int> e = exit;
    for (e.second = max(bd.yOff, exit.second - 1); e.second <= min(bd.yOff + bd.height, exit.second + 1); e.second++)
      for (e.first = max(bd.xOff, exit.first - 1); e.first <= min(bd.xOff + bd.width, exit.first + 1); e.first++)
	l_.place(e,pass_);
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

#endif //ndef LABYRINTH_HPP__
