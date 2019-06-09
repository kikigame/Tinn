/* License and copyright go here*/

// utility class to hold a coordinate pair

#include "coord.hpp"

std::ostream & operator << (std::ostream & out, const coord & c) {
  out << '(' << c.first << ',' << c.second << ')';
  return out;
}

std::wostream & operator << (std::wostream & out, const coord & c) {
  out << L'(' << c.first << L',' << c.second << L')';
  return out;
}
