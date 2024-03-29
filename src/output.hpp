/* License and copyright go here*/

// general output routine contract

#ifndef OUTPUT_HPP__
#define OUTPUT_HPP__

#include "coord.hpp"
#include "sense.hpp"

#include <functional>
#include <string>
#include <memory>
#include <vector>

class args;
class dungeon;
class player;
class renderable;
class renderByCoord;

// abstract input/output class
class io {
public:
  virtual ~io();

  // called when moving from one screen to another.
  virtual void clear() const = 0;

  // print a 1-line message to the user:
  virtual void message(const std::wstring &msg) const = 0;
  virtual void message(const wchar_t *msg) const {if (msg != NULL) message(std::wstring(msg));}

  // print a longer message to the user, eg for an OK dialog
  // NB: You should re-call draw(dungion) when ready after this.
  virtual void longMsg(const std::wstring &msg) const = 0;
  virtual void longMsg(const wchar_t *msg) const {if (msg != NULL) longMsg(std::wstring(msg));}

  // print a directional message to the user.
  // This implementation delegates to message(), prefixing with the coordinates, but other outputs may prefer to highlight the message on screen somehow.
  virtual void message(const coord c, const std::wstring &msg) const;

  // print a directional message to the user.
  // This implementation delegates to message(), prefixing with the coordinates, but other outputs may prefer to highlight the message on screen somehow.
  virtual void longMsg(const coord c, const std::wstring &msg) const;

  // request an input key from the user:
  virtual std::wstring keyPrompt() const = 0;
  virtual std::wstring keyPrompt(const std::wstring & msg) const { message(msg); return keyPrompt(); }
  virtual std::wstring keyPrompt(const wchar_t *msg) const { message(msg); return keyPrompt(); }
  virtual std::wstring keyPrompt(const std::wstring & msg, const std::wstring &help) const = 0;
  virtual std::wstring keyPrompt(const wchar_t *msg, const wchar_t *help) const { return keyPrompt(std::wstring(msg), std::wstring(help)); }

  // request a directior from the user (WASD<>.)
  virtual wchar_t dirPrompt() const = 0;

  // as keyPrompt() but will only accept Y or N (or aliases y/n)
  virtual bool ynPrompt(std::wstring msg) const = 0;
  virtual bool ynPrompt(const wchar_t *msg) const { return ynPrompt(std::wstring(msg)); }
  

  virtual std::wstring linePrompt() const = 0;
  virtual std::wstring linePrompt(const std::wstring & msg) const { message(msg); return linePrompt(); }
  virtual std::wstring linePrompt(const wchar_t *msg) const { message(msg); return linePrompt(); }
  virtual std::wstring linePrompt(const std::wstring &msg, const std::wstring &help) const = 0;
  virtual std::wstring linePrompt(const wchar_t *msg, const wchar_t *help) const { return linePrompt(std::wstring(msg), std::wstring(help)); }
  
  // present a menu of options to the player. See shop.cpp
  // T is the type
  template <typename T>
  T choice(const std::wstring &prompt, const std::wstring &help, 
	   const std::vector<std::pair<T, std::wstring>> &choices,
	   const std::function<std::wstring(const T &)> &extraHelp) const;

  // present a menu of options to the player. See chargen.cpp
  // T is the type
  template <typename T>
  T choice(const std::wstring &prompt, const std::wstring &help, 
	    const std::vector<std::pair<T, std::wstring>> &choices,
	    const std::wstring &extraHelp = L"") const {
    return choice<T>(prompt, help, choices, [&extraHelp](const T&) { return extraHelp;});
  }

  // pick one from a list, by index
  template <typename T>
  T* choice(const std::wstring &prompt, const std::wstring &help,
	    const std::vector<T*> &choices,
	    const bool allowNone,
	    const std::function<std::wstring(const T&)> namer,
	    const std::function<bool(const T&)> f = [](const T&){return true;},
	    const std::wstring &extraHelp = L"") const;
  
  // prompt for gender, returning male % and female %.
  // this could be generalised in valious ways, but let's not do that until we need to prompt for
  // multiple values at once again
  virtual std::pair<unsigned char, unsigned char> 
  genderPrompt(const wchar_t * msg, const wchar_t * help,
	       const wchar_t * female0help, const wchar_t * female100help,
	       const wchar_t * male0help, const wchar_t * male100help) const = 0;

  // draw the dungeon:
  virtual void draw(const dungeon & d) const = 0;
  virtual void draw(const renderByCoord & d) const = 0;
  virtual void draw(const player & d) const = 0;

  // interrogate the dungeon (or whatever is showing):
  virtual void interrogate(const renderByCoord &, const coord &c) const = 0;
};

// we're going to specialize the template, so declare in in the header.
template <>
wchar_t io::choice(const std::wstring &prompt, const std::wstring &help, 
		   const std::vector<std::pair<wchar_t, std::wstring>> &choices,
		   const std::wstring &extraHelp) const;



class ioFactory {
private:
  static std::weak_ptr<io> impl_;
public:
  // logic to create the user's preferred I/O goes here:
  std::shared_ptr<io> create(const args &opts);
  std::shared_ptr<io> connectFifos(const args &opts);
  static const io &instance();
};

class drawIter;

// interface for things which supply a renderable object by coordinates
class renderByCoord {
public:
  virtual const renderable & renderableAt(const coord &) const = 0;
  virtual drawIter drawBegin() const = 0;
  virtual drawIter drawEnd() const = 0;
};

// input iterator for supplying renderables
struct drawIter : public std::iterator<std::input_iterator_tag, std::pair<coord, const renderable&>> {
  private:
    const renderByCoord & l_;
    coord cur_;
    const int maxWidth, maxHeight;
  public:
    drawIter(const renderByCoord & l, const coord & cur, int w, int h) : 
      l_(l), cur_(cur), maxWidth(w), maxHeight(h) {}
    drawIter(const drawIter & rhs) : l_(rhs.l_), cur_(rhs.cur_), maxWidth(rhs.maxWidth), maxHeight(rhs.maxHeight) {}
    drawIter& operator++() { advance(); return *this; }
    drawIter operator++(int) { drawIter tmp(*this); advance(); return tmp; }
    bool operator==(const drawIter& rhs) {return cur_==rhs.cur_;}
    bool operator!=(const drawIter& rhs) {return cur_!=rhs.cur_;}
    const std::pair<coord, const renderable&> operator*() const {
      return std::pair<coord, const renderable& >(cur_, l_.renderableAt(cur_));
    }

  private:
    void advance() {
      cur_.first++;
      if (cur_.first == maxWidth) {
	cur_.first = 0;
	cur_.second++;
      }
      if (cur_.second == maxHeight) {
	cur_.first = cur_.second = -1;
	//::std::cout << " cur_ is now " << cur_ << std::endl;
      }
    }
  };



#endif // ndef OUTPUT_HPP__
