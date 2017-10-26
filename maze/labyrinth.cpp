/*icense and copyright go here*/

// labyrinth test class

#include "labyrinth.hpp"

#include <iostream>
#include <random>

struct myPlacable : public placable<char> {
  std::vector<std::vector<char> > b;
  myPlacable(int w, int h) :
    b(h, std::vector<char>(w, '.')) {}
  virtual ~myPlacable() {}
  void place(std::pair<int,int> c, const char & t) {
    b[c.first][c.second] = t;
  }
};

int main() {
  using namespace std;

  for (int w = 30; w > 18; w -=4)
    for (int h = 20; h > 11; h--)
      for (int v = 4; v < h-5; ++v) {
      //  int w = 30, h = 40;
	std::cout << "[0,0]--[" << w << "," << h << "]  from (0,0) to (" << (4*(w/8)+2) << ',' << v << ')' << std::endl;
  
  //  labyrinth<char, spiralfiller<char, snakefiller<char> >, true, true, imperfectRecursor<char, spiralfiller<char, snakefiller<char> >, true, true> > 
  labyrinth<>
    l('<','>','.','#',' ','+',w,h);
  //labyrinth<char, snakefiller<char> > l('<','>','.','#',' ','+',w,h);
  //l.in(0,0);
  //l.in(1,1);
  //  l.in(6,33);
  //l.in(2,3);
  //  l.out(28,20);
  l.out(4*(w/8)+2, v);

  try {
    l.build();
  } catch (char const * err) {
    ::std::cerr << err << std::endl;
    throw;
  } catch (std::string & err) {
    ::std::cerr << err << std::endl;
    throw;
    /*
  } catch (std::pair<int,int> c) {
    ::std::cerr<< "Out of bounds: " << c.first << ',' << c.second << std::endl;
    throw;
*/
    }

  //  for (auto i : x) {
  //    cout << '(' << i.xOff << ',' << i.yOff << ") by (" << i.width << ',' << i.height << ')' << endl;
  //  }

  cout << "    ";
  for (int x=0; x <= w; ++x) {
    if (x < 10) cout << ' ';
    else cout << (x/10);
  }
  cout << endl << "    ";
  for (int x=0; x <= w; ++x)
    cout << (x%10);
  cout << endl << "    ";;

  for (int x=0; x <= w; ++x)
    cout << '-';
  cout << endl;
  for (int y=0; y <= h; ++y) {
    cout << y << ' ';
    if (y < 10) cout << ' ';
    cout << '|';
    for (int x=0; x <= w; ++x) {
      cout << l.begin()[x][y];
    }
    cout << '|' << endl;
  }
  cout << "    ";
  for (int x=0; x <= w; ++x)
    cout << '-';
  cout << endl;
    }


  /*
  myPlacable t(20,20);

  snakefiller<char> sf(' ','#','.', t);

  for (auto r = t.b.begin(); r != t.b.end(); ++r) {
    for (auto c : *r)
      cout << c;
    cout << endl;
  }

  sf.fill({5,5,8,4}, {5,7}, {9,7});

  cout << endl << endl;
  for (auto r = t.b.begin(); r != t.b.end(); ++r) {
    for (auto c : *r)
      cout << c;
    cout << endl;
  }

  cout << endl;
  cout << endl;
  cout << "Trassposed:";
  cout << endl;
  t.b = transposeCopy<char, typename vector<vector<char> >::iterator>(t.b.begin(), t.b.end());

  cout << endl << endl;
  for (auto r = t.b.begin(); r != t.b.end(); ++r) {
    for (auto c : *r)
      cout << c;
    cout << endl;
  }

  
  const char unassigned_ = ',', pass_ = ' ', impass_ = '#';

  vector<vector<char> > find = { 
    vector<char>(5, impass_),
    vector<char>(5, impass_),
    { unassigned_, pass_, pass_, pass_, unassigned_ }
  };
  vector<vector<char> > replace = {
    vector<char>(5, impass_),
    { unassigned_, pass_, pass_, pass_, unassigned_ },
    { unassigned_, pass_, impass_, pass_, unassigned_ }
  };
  
  for (auto r = find.begin(); r != find.end(); ++r) {
    for (auto c : *r)
      cout << c;
    cout << endl;
  }
  cout << endl;
  cout << endl;

  find = transposeCopy<char, typename std::vector<std::vector<char> >::iterator>(find.begin(), find.end());

  for (auto r = find.begin(); r != find.end(); ++r) {
    for (auto c : *r)
      cout << c;
    cout << endl;
  }
  cout << endl;
*/
}
