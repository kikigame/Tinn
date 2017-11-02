/* License and copyright go here*/

// ncurses output routine implementation

#include "output.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "player.hpp"

#include <cwchar>
#include <iostream>
#include <sstream> // wstringstream
#include <limits> // numeric_limits for ignore
#include <algorithm> // find_if_not for trim
extern "C" {
#include <ncursesw/curses.h> // lookit! A dependency!
}

// RAII to enable the ncurses interface
struct ncursIface {
  ncursIface() {
    setlocale(LC_ALL, ""); // enable unicode magic
    initscr();
    //keypad();
    start_color();
    init_color(COLOR_BLACK, 0,0,0); // we want BLACK black!
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    echo(); // yes we want to echo user keystrokes most of the time. We're weird like that.
  }
  virtual ~ncursIface() {
    /*
    std::ofstream debug;
    debug.open("/tmp/debug.log");
    debug << "Shutting down 1/3" << std::endl;*/
    attroff(COLOR_PAIR(1));
    //debug << "Shutting down 2/3" << std::endl;
    endwin();
    //debug << "Shutting down 3/3" << std::endl;
    //debug.close();
  }
};

// RAII class to set the terminal to raw mode temporarily.
// NB: when using get_ch and friends, we mustn't use nocbreak and echo at the same time.
struct rawTerm {
  rawTerm() {
    cbreak(); // turn off cooked mode
    noecho(); // do not echo user keypresses
  }
  ~rawTerm() {
    echo(); // reenable regular echo
    nocbreak(); // turn on cooked mode again
  }
};



// ncurses output class
class ncurse : public io {
private:
  ncursIface iface_;
  mutable std::wstring lastMsg;// mutable as only an internal buffer; we guarantee it's shown to the user before next update, so state is effectively unaffected. It's just here for formatting.
public:
  ncurse() {
    attron(WA_DIM | WA_UNDERLINE);    
    mvaddwstr(24,0, L"             font test            ¬¬¬ ¶¶¶ ¬¬¬           ready to amble         ");
    attroff(WA_DIM | WA_UNDERLINE);    
  }
  virtual ~ncurse() {
    ::clear();
    longMsg(std::wstring(L"See you next time.")); // sort-of pun on "Be seeing you" Nethack end; itself a quote from the Prisoner TV series
  }

  // called when moving from one screen to another.
  virtual void clear() const {
    flushLastMsg(L"");
    ::clear();
    refresh();
  }

  // print a 1-line message to the user:
  virtual void message(const std::wstring &msg) const {
    if (lastMsg.empty()) lastMsg = msg;
    else {
      // we rely on the fact that the dungeon is redrawn each time, and that calls flushLashMsg.
      auto numLines = std::count(lastMsg.begin(), lastMsg.end(), L'\n');
      if (numLines > 22) // assuming an 80x24 terminal
	flushLastMsg(msg);
      else lastMsg += L"\n" + msg;
    }
  }

  // print a longer message to the user, eg for an OK dialog
  virtual void longMsg(const std::wstring &msg) const {
#if MSG_ON_SCROLL_TERM
    if (lastMsg.empty()) lastMsg = msg;
    else lastMsg = lastMsg + L"\n" + msg;

    def_prog_mode(); // save program mode
    endwin(); // leave ncurses temporarily
    std::wcout << lastMsg << std::endl
	      << L" -- MORE --" << std::flush;
    // read a line and discard:
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), L'\n');
    lastMsg = L"";

    reset_prog_mode(); // restore program mode
    refresh(); // redraw the dungeon
#else
    
    /*
    savetty();
    ::clear();
    */
    if (lastMsg.empty()) lastMsg = msg;
    else lastMsg = lastMsg + L"\n" + msg;
    mvaddwstr(0,0, lastMsg.c_str());
    printw(" -- MORE --"); // ascii...
    refresh();
    getch();
    lastMsg = L"";
/*
    scr_dump()
    resetty(); // restore to last save
    refresh();
    */
#endif
  }

  virtual std::wstring keyPrompt() const {
    flushLastMsg(L"");
    rawTerm noCook; // this function will not block input.
    wchar_t i[2] { static_cast<wchar_t>(getwch()), L'\0' }; // actually grab one key and shove it in a string
    return std::wstring(i);
  }

  virtual std::wstring keyPrompt(const std::wstring &msg, const std::wstring &help) const {
    helpPrompt(msg, help);
    rawTerm noCook; // this function will not block input.
    wchar_t i[2] { static_cast<wchar_t>(getwch()), L'\0' }; // actually grab one key and shove it in a string
    return std::wstring(i);
  }

  virtual bool ynPrompt(std::wstring msg) const {
    message(msg + L" (Y/N) >");
    flushLastMsg(L"");
    refresh();
    rawTerm noCook; // this function will not block input.
    while(true) {
      wchar_t rtn = getwch();
      switch (rtn) {
      case L'Y': case L'y': return true;
      case L'N': case L'n': return false;
      }
    }
  }

  virtual std::wstring linePrompt() const {
    return getstr_safe();
  }


  virtual std::wstring linePrompt(const std::wstring &msg, const std::wstring &help) const {
    helpPrompt(msg, help);
    return getstr_safe();
  }


  // draw the dungeon:
  virtual void draw(const level & l) const {
    int y=1;
    move(++y,0);
    auto i = l.drawBegin(), e = l.drawEnd();
    while (true) {
      wchar_t r = (*i).second.render();
      addnwstr(&r, 1);
      ++i;
      if (i == e) break;
      if ((*i).first.first <= 0) move(++y,0);
    }
    move(++y,0);
    //    refresh();
  }
  virtual void draw(const player & p) const {
    std::wstringstream fmt;
    fmt
      << p.name();
    fmt
      << L" Strn: " << p.strength()
      << L" Fght: " << p.fighting()
      << L" Dodg: " << p.dodge()
      << L" Damg: " << p.injury()
      << L" Appr: " << p.appearance()
      ;
    //std::wcerr << fmt.str().c_str() << std::endl;
    addwstr(fmt.str().c_str());
    //    refresh();
  }
  virtual void draw(const dungeon & d) const {
    ::clear();
    draw(d.cur_level());
    draw(*(d.pc()));
    // don't flushLastMsg here; that'll be taken care of by the key prompt.
    refresh();
  }

  virtual std::pair<unsigned char, unsigned char> 
  genderPrompt(const wchar_t * msg, const wchar_t * help,
	       const wchar_t * female0help, const wchar_t * female100help,
	       const wchar_t * male0help, const wchar_t * male100help) const {
    ::clear();
    bool femaleCur = true;
    unsigned char male=0, female=0;
    const std::wstring blank(80, ' ');

    while (true) {
      mvaddwstr(0,0,msg);
      if (femaleCur) attron(WA_BOLD);
      //      wattrset(femaleCur ? WA_BOLD : WA_NORMAL);
      mvaddwstr(3,0, (pcBar(L"Female:", female) + L"  ").c_str());
      if (!femaleCur) attron(WA_BOLD); else attroff(WA_BOLD);
      //      wattrset(!femaleCur ? WA_BOLD : WA_NORMAL);
      mvaddwstr(5,0, (pcBar(L"  Male:", male) + L"  ").c_str());
      attroff(WA_BOLD);
      //      wattrset(WA_NORMAL);
      mvaddwstr(7,3,blank.c_str());
      mvaddwstr(8,3,blank.c_str());
      mvaddwstr(7,3, female < 10 ? female0help : female > 90 ? female100help : L"");
      mvaddwstr(8,3, male < 10 ? male0help : male > 90 ? male100help : L"");
      mvaddwstr(10,0, help);
      switch (keyPrompt().c_str()[0]) {
      case '\n': 
	return std::pair<unsigned char, unsigned char>(male,female);
      case 'A': case 'a':
	if (femaleCur) female=(female == 0 ? 0 : female - 1);
	else male=(male == 0 ? 0 : male - 1);
	break;
      case 'd': case 'D': 
	if (femaleCur) female=(female == 100 ? 100 : female + 1);
	else male=(male == 100 ? 100 : male + 1);
	break;
      case 'w': case 'W': 
      case 'S': case 's':
	femaleCur = !femaleCur;
	break;
      case 'M': case 'm':
	male = (male == 100 ? 0 : 100);
	break;
      case 'F': case 'f':
	female = (female == 100 ? 0 : 100);
	break;
      case 'N': case 'n': // neuter
	male = female = 0;
	break;
      case 'H': case 'h': // hermaphrodite
	if (male == female && male == 100) male = female = 50;
	else male = female = 100;
      }
    }
  }


private:
  // write out any pending short message to the screen and collect a new one (if not empty)
  void flushLastMsg(const std::wstring &msg) const {
    if (!lastMsg.empty()) {
      mvaddwstr(0,0, std::wstring(L" ", 80).c_str()); // 80 bytes of ascii...
      mvaddwstr(0,0, lastMsg.c_str());
      if (!msg.empty())
	addwstr(L" -- MORE --"); // ascii...
      refresh();
      if (!msg.empty())
	getwch();
    }
    lastMsg = msg;
  }
  /*
   * used by genderPrompt - show the current percentage
   */
  std::wstring pcBar(const wchar_t * const prompt, unsigned char value) const {
    std::wstring rtn(prompt);
    rtn += L" ";
    rtn += std::wstring(value/2, '#');
    rtn += std::wstring(50-(value/2), '-');
    rtn += L" " + std::to_wstring(value) + L"%";
    return rtn;
  }

  /*
   * Aim to show help text in a consistent place; leave cursor after msg
   */
  void helpPrompt(const std::wstring msg, const std::wstring help) const {
    ::clear();
    mvaddwstr(3,0,help.c_str());
    mvaddwstr(0,0,msg.c_str());
    refresh();
  }

  /*
   * String trim function from http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring#217605
   * NB: this uses the C locale to identify whitespace; probably okay for input but there's another
   * version of std::isspace() that takes a locale if not.
   */
  inline std::wstring trim(const std::wstring &s) const {
    using namespace std;
    // take an iterator to the furst subchars that are not strings:
    std::wstring::const_iterator wsfront = find_if_not(s.begin(),s.end(),[](int c){return isspace(c);});
    // check for string containing only whitespace (this needs to be trimmed too...)
    if (wsfront == s.end()) return wstring();
    // take a reverse iterator to the first (last) subchars that are not strings
    // and copy everything between the iterators
    return wstring(wsfront,find_if_not(s.rbegin(),
				       wstring::const_reverse_iterator(wsfront),
				       [](int c){return isspace(c);}).base());
  }
  /*
   * reimplementation of curses's getstr() function to avoid a buffer overrun
   * also trims the response
   */
  std::wstring getstr_safe() const {
    std::wstring rtn;
    wchar_t c;
    while (true) {
      c = static_cast<wchar_t>(getwch());
      if (c == '\n') return trim(rtn);
      else rtn += c;
    }
  }
  /*
   * reimplemtation of getch(); plan is for unicode stuff to go here maybe?
   */
  wchar_t getwch() const {
    return getch();
  }
};

// logic to create the user's preferred I/O goes here:
namespace ioFactory {
  std::shared_ptr<io> create() {
    // we only have one implementation thus far:
    // Warning: putting the parens on "new ncurse()" causes the object
    // to be sliced and the destructor not called. I am unsure why exactly.
    return std::shared_ptr<io>(new ncurse());
  }
}

