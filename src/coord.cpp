/* License and copyright go here*/

// utility class to hold a coordinate pair

#include "coord.hpp"

#include <random>

std::ostream & operator << (std::ostream & out, const coord & c) {
  out << '(' << c.first << ',' << c.second << ')';
  return out;
}

std::wostream & operator << (std::wostream & out, const coord & c) {
  out << L'(' << c.first << L',' << c.second << L')';
  return out;
}

static std::default_random_engine generator; // random.cpp

/*
 * Pick a random value matching f.
 * Algorithm is to iterate over it, setting rtn to the Nth value with a probability of 1:N.
 * This gives a uniform distribution to all values.
 */
coord rnd(coordRectIterator &it, std::function<bool(const coord&)> f) {
  coord rtn(-1,-1);
  int counter=0;
  for (coord c : it) {
    if (f(c)) {
      std::uniform_int_distribution<int> dist(1,++counter);
      int ran = dist(generator);
      if (ran == 1) rtn = c;
    }
  }
  return rtn;
}
