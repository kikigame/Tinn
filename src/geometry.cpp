/* License and copyright go here*/

// utility class to hord a coordinate pair

#include "geometry.hpp"

geometry::~geometry() {}


rectarea::rectarea(const coord &ul, const coord &lr) :
  ul_(ul), lr_(lr) {}
rectarea::rectarea(int l, int t, int r, int b) :
  ul_(l, t), lr_(r, b) {}

rectarea::~rectarea() {}

bool rectarea::contains(const coord &a) const {
  return a.first >= ul_.first && a.first <= lr_.first &&
    a.second >= ul_.second && a.second <= lr_.second;
}

std::unique_ptr<geometry> rectarea::create(const coord &ul, const coord &lr) {
  return std::unique_ptr<geometry>(new rectarea(ul, lr));
}
std::unique_ptr<geometry> rectarea::create(int l, int t, int r, int b) {
  return std::unique_ptr<geometry>(new rectarea(l, t, r, b));
}


class areaWithHole : public geometry {
private:
  std::unique_ptr<geometry> plus_;
  std::unique_ptr<geometry> hole_;
public:
  areaWithHole(std::unique_ptr<geometry> &&plus, std::unique_ptr<geometry> &&hole) :
    plus_(), hole_() {
    plus_.swap(plus); hole_.swap(hole);
  }
  ~areaWithHole() {}
  virtual bool contains(const coord &pos) const {
    return plus_->contains(pos) && !hole_->contains(pos);
  }
};

class combinedArea : public geometry {
private:
  std::unique_ptr<geometry> a_;
  std::unique_ptr<geometry> b_;
public:
  combinedArea(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b) :
    a_(), b_() {
    a_.swap(a); b_.swap(b);
  }
  ~combinedArea() {}
  virtual bool contains(const coord &pos) const {
    return a_->contains(pos) || b_->contains(pos);
  }
};

namespace shape {
  // consumes arguments:
  std::unique_ptr<geometry> add(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b) {
    return std::unique_ptr<geometry>(new combinedArea(std::move(a),std::move(b)));
  }
  std::unique_ptr<geometry> add(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b, std::unique_ptr<geometry> &&c) {
    auto d = add(std::move(a),std::move(b));
    return add(std::move(d),std::move(c));
  }
  // consumes arguments:
  std::unique_ptr<geometry> subtract(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b) {
    return std::unique_ptr<geometry>(new areaWithHole(std::move(a),std::move(b)));
  }
  std::unique_ptr<geometry> subtract(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b, std::unique_ptr<geometry> &&c) {
    auto d = subtract(std::move(a),std::move(b));
    return subtract(std::move(d),std::move(c));
  }
}
