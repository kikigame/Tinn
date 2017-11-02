/* License and copyright go here*/

// Things which can be drawn

#ifndef RENDERABLE_HPP__
#define RENDERABLE_HPP__


// interface
class renderable {
public:
  // virtual constructor always a good idea in a base class
  virtual ~renderable() = default;
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
