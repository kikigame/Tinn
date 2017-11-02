/* License and copyright go here*/

// terminal I/O routine. This is disabled for now as it doesn't work right in raw mode.

#include "output.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "player.hpp"
#include "time.hpp"

#include <iostream>
#include <sstream> // wstringstream
#include <limits> // numeric_limits for ignore



// Spec: all of these methods return an object that can be appended to an ostream via <<.
class term {
public:
  // clear the screen, excluding the current cursor line:
  static const wchar_t* const clear() { return L"\033[2J"; }
  // move to absolute coordinates:
  static const ::std::wstring move_abs(const int x,const int y) { return
      ::std::wstring(L"\033[") + ::std::to_wstring(x) + L';' + ::std::to_wstring(y) + L'H'; }
  // move to relative coordinates:
  static const ::std::wstring move_rel(const int x,const int y) {
    ::std::wstring rtn;
    if (x < 0) rtn += L"\033[" + ::std::to_wstring(-x) + L'D';
    if (x > 0) rtn += L"\033[" + ::std::to_wstring(x) + L'C';
    if (y < 0) rtn += L"\033[" + ::std::to_wstring(-y) + L'A';
    if (y > 0) rtn += L"\033[" + ::std::to_wstring(y) + L'B';
    return rtn;
  }
  // clear the screen and home the cursor:
  static const wchar_t* const reset() { return L"\033[2J" L"\033[1;1H"; }
};



// terminal output class (raw ANSI; doesn't work too well with getch() prompt)
class ansiterm : public io {
public:
  ansiterm() {
    std::wcout << term::clear() << std::flush;
  }
  virtual ~ansiterm() {
    std::wcout << term::reset();
    longMsg(std::wstring(L"See you next time.")); // pun on "Be seeing you" Nethack end; itself a quote from the Prisoner TV series
    std::wcout << std::endl;
  }

  // called when moving from one screen to another.
  virtual void clear() const {
    std::wcout << term::clear() << std::flush;
  }

  // print a 1-line message to the user:
  virtual void message(const std::wstring &msg) const {
    std::wcout << term::move_abs(0,0)
              << L"                                                                                "
              << term::move_abs(0,0)
	      << msg
	      << std::flush;
  }

  // print a longer message to the user, eg for an OK dialog
  virtual void longMsg(const std::wstring &msg) const {
    std::wcout << term::reset()
	      << msg
	      << L" -- MORE --"
	      << std::flush;
    // read a line and discard:
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
    /*
    do {
      std::cin.ignore(); // wait for a whole line of input, then return the first char (usually the '\n')
    } while (std::cin.available()); // eat anything else the user typed
    */
  }

  virtual std::wstring keyPrompt() const {
    //    rawTerm noCook; // this function will not block input.
    wchar_t i[2] { static_cast<wchar_t>(std::wcin.get()), L'\0' }; // actually grab one key and shove it in a string
// TODO: multibyte chars
    return std::wstring(i);
  }

  virtual std::wstring keyPrompt(const std::wstring & prompt, const std::wstring &help) const {
    helpPrompt(prompt, help);
    return keyPrompt();
  }

  virtual std::wstring linePrompt() const {
    std::wstring rtn;
    std::getline(std::wcin, rtn);
    return rtn;
  }

  virtual std::wstring linePrompt(const std::wstring & prompt, const std::wstring &help) const {
    helpPrompt(prompt, help);
    return linePrompt();
  }

  // draw the dungeon:
  virtual void draw(const level & l) const {
    std::wcout << term::move_abs(0,1);
    auto i = l.drawBegin(), e = l.drawEnd();
    while (true) {
      std::wcout << (*i).second.render();
      ++i;
      if (i == e) break;
      if ((*i).first.first <= 0) std::wcout << std::endl;
    }
    std::wcout << std::endl;
  }
  virtual void draw(const player & p) const {
    std::wcout 
      << p.name()
      << L" Strn: " << p.strength()
      << L" Fght: " << p.fighting()
      << L" Dodg: " << p.dodge()
      << L" Damg: " << p.injury()
      << L" Appr: " << p.appearance()
      << std::flush;
  }
  virtual void draw(const dungeon & d) const {
    draw(d.cur_level());
    draw(*(d.pc()));
  };

private:
  void helpPrompt(const std::wstring msg, const std::wstring help) const {
    std::wcout << term::reset()
	      << term::move_abs(0,3)
	      << help
	      << term::move_abs(0,0)
	      << msg
	      << std::flush;
  }

};

// logic to create the user's preferred I/O goes here:
// namespace ioFactory {
//   std::shared_ptr<io> create() {
//     // we only have one implementation thus far:
//     return std::auto_ptr<io>(new ansiterm);
//   }
// }

