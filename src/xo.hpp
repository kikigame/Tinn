/* License and copyright go here*/

// A naughts and crosses game

#include <array>
#include <string>

enum class tile { empty, o, x /* goes 1st */ };

class xo{
private:
  typedef std::array<std::array<tile, 3>, 3 > ar;
  ar b;
  std::array<tile, 2> x_o;
  int xcount=0;
public:
  xo() : b({{
	    {tile::empty, tile::empty, tile::empty},
	    {tile::empty, tile::empty, tile::empty},
	    {tile::empty, tile::empty, tile::empty}
      }}), x_o( {tile::o, tile::x} ) {}
  std::wstring go(const int x, const int y) {
    if (b[x][y] != tile::empty) return L"Invalid move";
    b[x][y] = tile::x;
    if (++xcount == 5) return L"Draw!";
    return ogo();
  }
  ar::const_iterator begin() const { return b.begin(); }
  ar::const_iterator end() const { return b.end(); }
  operator bool() const { return xcount < 5; }
private:
  std::wstring ogo() {
    std::wstring rtn;
    int x=0,y=0;
    for (auto t : x_o) {
    if (b[0][0] == tile::empty && b[1][1] == t && b[2][2] == t) return ogo(0,0);
    if (b[1][1] == tile::empty && b[0][0] == t && b[2][2] == t) return ogo(1,1);
    if (b[2][2] == tile::empty && b[0][0] == t && b[1][1] == t) return ogo(2,2);
    if (b[2][0] == tile::empty && b[1][1] == t && b[0][2] == t) return ogo(2,0);
    if (b[1][1] == tile::empty && b[0][0] == t && b[0][2] == t) return ogo(1,1);
    if (b[0][2] == tile::empty && b[2][0] == t && b[1][1] == t) return ogo(0,2);
    for (int i=0; i < 3; ++i) {
      if (b[i][0] == tile::empty && b[i][1] == t && b[i][2] == t) return ogo(i,0);
      if (b[i][1] == tile::empty && b[i][0] == t && b[i][2] == t) return ogo(i,1);
      if (b[i][2] == tile::empty && b[i][0] == t && b[i][1] == t) return ogo(i,2);
      if (b[0][i] == tile::empty && b[1][i] == t && b[2][i] == t) return ogo(0,i);
      if (b[1][i] == tile::empty && b[0][i] == t && b[2][i] == t) return ogo(1,i);
      if (b[2][i] == tile::empty && b[0][i] == t && b[1][i] == t) return ogo(2,i);
      if (b[i][0] == tile::empty) x=i,y=0;
      if (b[i][1] == tile::empty) x=i,y=1;
      if (b[i][2] == tile::empty) x=i,y=2;
    }}
    if (b[1][1] == tile::empty) return ogo(1,1);
    for (int xx=0; xx < 3; ++xx) for (int yy=0; yy < 3; ++yy)
	if (b[xx][yy] == tile::empty) {
	  int m=0,n=0;
	  for (int i=0; i <3; ++i) {
	    if (b[xx][i] == tile::x) m++;
	    else if (b[xx][i] == tile::o) m+=2;
	    if (b[i][yy] == tile::x) n++;
	    else if (b[i][yy] == tile::x) n+=2;
	  }
	  if (m == 1 && n == 1) return ogo(xx,yy);
	}
    return ogo(x,y);
  }
  std::wstring ogo(const int x, const int y) {
    b[x][y] = tile::o;
    auto rtn = std::wstring(L"O goes ") + (y == 0 ? L"A" : y == 1 ? L"B" : L"C") + std::to_wstring(x+1);
    if (b[0][0] == tile::o && b[1][1] == tile::o && b[2][2] == tile::o) return rtn + win();
    if (b[2][0] == tile::o && b[1][1] == tile::o && b[0][2] == tile::o) return rtn + win();
    for (int i=0; i < 3; ++i) {
      if (b[i][0] == tile::o && b[i][1] == tile::o && b[i][2] == tile::o) return rtn + win();
      if (b[0][i] == tile::o && b[1][i] == tile::o && b[2][i] == tile::o) return rtn + win();
    }
    return rtn;
  }
  std::wstring win() {
    xcount += 10;
    return L"\nI win";
  }

};
