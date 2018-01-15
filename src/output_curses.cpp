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
#include <functional>
extern "C" {
#ifdef __MINGW64__
#define _XOPEN_SOURCE_EXTENDED // not set implicitly on mingw. Previous README said:
#endif
  /*
If you have an older Linux compiler/libraries, some of the console functions won't link. You may need to modify output_curses.hpp to start with the following line:

#define _XOPEN_SOURCE_EXTENDED
   */

#include <ncursesw/curses.h> // lookit! A dependency!
}

// portable way to mark a variable as deliberately unused.
template<typename T>
void unused(T const &) {}


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
    longMsg(L"See you next time."); // sort-of pun on "Be seeing you" Nethack end; itself a quote from the Prisoner TV series
  }

  // called when moving from one screen to another.
  virtual void clear() const {
    flushLastMsg(L"");
    ::clear();
  }

  // print a 1-line message to the user:
  virtual void message(const std::wstring &msg) const {
    if (lastMsg.empty()) lastMsg = msg;
    else {
      // we rely on the fact that the dungeon is redrawn each time, and that calls flushLashMsg.
      auto numLines = std::count(lastMsg.begin(), lastMsg.end(), L'\n');
      int xMax,yMax;
      getmaxyx(stdscr, yMax, xMax); // macro; updates by reference
      if (numLines > yMax - 2) 
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
    getwch();
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
    wchar_t i[2] { key(), L'\0' }; // actually grab one key and shove it in a string
    return std::wstring(i);
  }

  virtual std::wstring keyPrompt(const std::wstring &msg, const std::wstring &help) const {
    helpPrompt(msg, help);
    rawTerm noCook; // this function will not block input.
    wchar_t i[2] { key(), L'\0' }; // actually grab one key and shove it in a string
    return std::wstring(i);
  }


  virtual bool ynPrompt(std::wstring msg) const {
    message(msg + L" (Y/N) >");
    flushLastMsg(L"");
    rawTerm noCook; // this function will not block input.
    while(true) {
      wchar_t rtn = getwch();
      switch (rtn) {
      case L'Y': case L'y': return true;
      case L'N': case L'n': return false;
      }
    }
  }

  virtual wchar_t dirPrompt() const {
    message(L"(wsad<>.) >");
    flushLastMsg(L"");
    const auto mask = std::wstring(L"WASD<>.wasd");
    wchar_t rtn;
    do {
      rtn = key();
    } while(!mask.find(rtn));
    return rtn;
  }

  virtual std::wstring linePrompt() const {
    return getstr_safe();
  }


  virtual std::wstring linePrompt(const std::wstring &msg, const std::wstring &help) const {
    helpPrompt(msg, help);
    return getstr_safe();
  }


  // draw the dungeon:
  virtual void draw(const renderByCoord & l) const {
    draw(l, [] (const renderable &renderable, int, int) {
      wchar_t r = renderable.render();
      if (renderable.highlight()) attron(A_STANDOUT);
      addnwstr(&r, 1);
      if (renderable.highlight()) attroff(A_STANDOUT);
      });
  }
public:

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
  }
  virtual void draw(const dungeon & d) const {
    ::clear();
    draw(d.cur_level());
    draw(*(d.pc()));
    // don't flushLastMsg here; that'll be taken care of by the key prompt.
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
      blankToEol();
      if (!msg.empty())
	addwstr(L" -- MORE --"); // ascii...
      if (!msg.empty())
	getwch();
    }
    lastMsg = msg;
  }
  void blankToEol() const {
    int xMax,yMax;
    getmaxyx(stdscr, yMax, xMax); // macro; updates by reference
    unused(yMax); // can't get xMax without yMax
    for (int i=0; i < xMax; ++i)
      delch();
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
  void helpPrompt(const std::wstring &msg, const std::wstring &help) const {
    flushLastMsg(L""); // output any buffered message before showing string
    ::clear();
    mvaddwstr(3,0,help.c_str());
    mvaddwstr(0,0,msg.c_str());
  }

  /*
   * Interrogate things on the screen
   * l = level to draw/interrogate
   * c = coordinate to stat cursor at
   */
  void interrogate(const renderByCoord &l, const coord &start) const {
    coord c = start;
    do {
      ::clear();
      const renderable* toShow = 0;
      draw(l, [&c, &toShow] (const renderable &renderable, int x, int y) {
	wchar_t r = renderable.render();
	bool cursor = (x == c.first && y == c.second);
	if (cursor) {
	  attron(A_STANDOUT);
	  addnwstr(&r, 1);
	  attroff(A_STANDOUT);
	  toShow = &renderable;
	} else {
	  addnwstr(&r, 1);
	}});
      if (toShow != 0) {
	mvaddwstr(0,0,L"[wasdq] ");
	if (toShow->highlight()) attron(A_STANDOUT);
	addwstr(toShow->name().c_str());
	if (toShow->highlight()) attroff(A_STANDOUT);
	//	flushLastMsg(toShow->name(), toShow->highlight());
	//message(toShow->description());
	mvaddwstr(1,0, std::wstring(L" ", 80).c_str()); // 80 bytes of ascii...
	mvaddwstr(1,0, toShow->description().c_str());
      } else {
	mvaddwstr(0,0, L"Nothing here...");
      }
      switch (key()) {
      case 'A': case 'a':
	if (c.first > 0) c.first--;
	break;
      case 'W': case 'w':
	if (c.second > 0) c.second--;
	break;
      case 'D': case 'd':
	if (c.first < level::MAX_WIDTH - 1) c.first++;
	break;
      case 'S': case 's':
	if (c.second < level::MAX_HEIGHT - 1) c.second++;
	break;
      case 'Q': case 'q':
	// exit the "look" function
	return;
      }
    } while (true);
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
   * reimplemtation of getch(); adds 256 to chars to indicate Alt/Meta key
   */
  wchar_t getwch() const {
    ::refresh();
    auto key = getch();

    if (key == 27) // Alt modifier
      key = 256 + getch();

    return key;
  }

  // read and return a wide key, non-blocking
  wchar_t key() const {
    rawTerm noCook; // this function will not block input.
    return getwch();
  }

  /*
   * Implementation of draw; moves cursor to the start of each cell, then call renderer.
   * Renderer: must advance the cursor by 1 cell.
   */
  void draw(const renderByCoord & l, std::function<void (const renderable&, int x, int y)> renderer) const {
    int y=0,x=0;
    move(y+2,0);
    auto i = l.drawBegin(), e = l.drawEnd();
    while (true) {
      auto &renderable = (*i).second;
      renderer(renderable, x,y);
      ++i, ++x;
      if (i == e) break;
      if ((*i).first.first <= 0) {move(++y,0); x=0;}
    }
    move(++y,0);
  }
};

// logic to create the user's preferred I/O goes here:
std::shared_ptr<io> ioFactory::create() {
  if (impl_.lock()) throw "Already implemented";
  // we only have one implementation thus far:
  // Warning: putting the parens on "new ncurse()" causes the object
  // to be sliced and the destructor not called. I am unsure why exactly.
  auto rtn = std::shared_ptr<io>(new ncurse());
  impl_ = rtn;
  return rtn;
}

const io &ioFactory::instance() {
  return *(impl_.lock());
}

std::weak_ptr<io> ioFactory::impl_;
