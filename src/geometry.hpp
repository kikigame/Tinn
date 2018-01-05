/* License and copyright go here*/

// utility class to hord a coordinate pair

#ifndef GEOMETRY_HPP__
#define GEOMETRY_HPP__

#include "coord.hpp"
#include <memory>

class geometry {
public:
  virtual ~geometry();
  virtual bool contains(const coord &pos) const = 0;
};

class rectarea : public geometry {
private:
  const coord ul_;
  const coord lr_;
  rectarea(const coord &ul, const coord &lr);
  rectarea(int l, int t, int r, int b);
public:
  virtual ~rectarea();
  virtual bool contains(const coord &pos) const;
  static std::unique_ptr<geometry> create(const coord &ul, const coord &lr);
  static std::unique_ptr<geometry> create(int l, int t, int r, int b);
};

namespace shape {
  // consumes arguments:
  std::unique_ptr<geometry> add(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b);
  std::unique_ptr<geometry> add(std::unique_ptr<geometry> &&a, std::unique_ptr<geometry> &&b, std::unique_ptr<geometry> &&c);
  // consumes arguments:
  std::unique_ptr<geometry> subtract(std::unique_ptr<geometry> &&plus, std::unique_ptr<geometry> &&hole);
  std::unique_ptr<geometry> subtract(std::unique_ptr<geometry> &&plus, std::unique_ptr<geometry> &&holeA, std::unique_ptr<geometry> &&holeB);
}

#endif //ndef GEOMETRY_HPP__
