/* License and copyright go here*/

// Things which can be drawn

#ifndef RENDERABLE_HPP__
#define RENDERABLE_HPP__

#include <set>

// interface
class renderable {
private:
  // https://isocpp.org/wiki/faq/ctors#static-init-order
  static std::set<const renderable*> &all_(); // using raw ptr because we don't know what owns renderable
  static long count_;
  const long id_;
public:
  // constructor and destructor maintain a reference in the static map
  // virtual constructor always a good idea in a base class
  renderable() : id_(count_++) {
    all_().insert(this);
  }
  virtual ~renderable() {
    all_().erase(this);
  }
  static const std::set<const renderable*>& all() {return all_();}
  // return the unicode character to render on screen:
  virtual const wchar_t render() const = 0; // This is handy, but we are (not) based off it: http://nethack.wikia.com/wiki/Category:ASCII
  // name of the thingy:
  virtual const wchar_t * const name() const = 0;
  // encyclopedium of the thingy:
  virtual const wchar_t * const description() const = 0;
  // true to highlight thingies. Only the player, pets and very special items should be highlighted:
  virtual bool highlight() const { return false; }
};

#endif // ndef RENDERABLE_HPP__~
