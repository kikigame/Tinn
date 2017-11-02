#include "xo.hpp"

#include <iostream>

void print(const xo &xo);

int main() {
  xo xo;

  print(xo);

  while (xo) {

  int x,y;
  std::string c;
  while (c.empty()) {
  while (c.empty())
    std::cin >> c;
  switch (c.c_str()[0]) {
  case 'a': case 'A': x=0; break;
  case 'b': case 'B': x=1; break;
  case 'c': case 'C': x=2; break;
  default: c = "";
  }}
  
  std::cin >> y;

  std::wcout << xo.go(y,x); // that'll confuse 'em!

  print(xo);
  }
}


void print(const xo &xo) {
  std::cout << std::endl
	    << "  |A|B|C|" << std::endl
	    << "--+-+-+-|" << std::endl;

  int y=0;
  for (auto r : xo) {
    std::cout << ' ' << y++ << '|';
    for (auto c : r)
      std::cout << (c == tile::empty ? '.' : c == tile::x ? 'X' : 'O')
		<< '|';
    std::cout << std::endl;
  }
  std::cout << std::endl
	    << "Your move ?";
}
