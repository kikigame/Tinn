/* License and copyright go here*/

// Things which can be drawn

#ifndef RENDERABLE_HPP__
#define RENDERABLE_HPP__


// interface
class renderable {
public:
  virtual ~renderable() = default;
  virtual const wchar_t render() const = 0; // This is handy, but we are (not) based off it: http://nethack.wikia.com/wiki/Category:ASCII
  virtual const wchar_t * const name() const = 0;
  virtual const wchar_t * const description() const = 0;
};

#endif // ndef RENDERABLE_HPP__~
