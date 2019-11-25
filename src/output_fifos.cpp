/* License and copyright go here*/

// socket output routine implementation

// This will encode all I/O via named fifos (FIFO files), which can be used by an external front-end.

#include "output.hpp"
#include "args.hpp"
#include "dungeon.hpp"

#include <algorithm> // used by trim
#include <functional> // used by trim
#include <cctype> // used by trim
#include <codecvt>
#include <locale>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdio>

extern "C" {
#include <unistd.h> // close and setsid
  //#include <fcntl.h> // unbuffered I/O open
}

// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
				  std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
		       std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}


// http://beej.us/guide/bgnet/html/single/bgnet.html
class fifo_io : public io {
private:
  mutable std::fstream in_; // must be opened I/O otherwise it closes between clientns (https://stackoverflow.com/questions/29355782/why-do-i-get-eof-after-reading-one-line-thourgh-a-named-pipe-using-ifstream)
  //  mutable std::ofstream out_;
  const std::string prefix_;
  mutable std::wstringstream buf_;
public:
  fifo_io(const std::string &prefix) :
    in_(prefix + "Up.fifo"),
    prefix_(prefix) {
    if (!in_) throw "Failed to open " + prefix + "Up.fifo" + std::strerror(errno);
    read(); //CGI always sends data then gets a response, but we need to send first, so discard initial input.
  };
  virtual ~fifo_io() {
    doSend(buf_.str() + L"\nL:That's All Folks!", "\n<!FIN!>\n"); // ref:Looney Tunes
    read(); // get the closing handshake back
  }
  virtual void clear() const {
    send(L"C:");
  }
  virtual void message(const std::wstring &msg) const {
    send(L"M:"+msg);
  }
  virtual void longMsg(const std::wstring &msg) const {
    send(L"L:"+msg);
  }
  virtual std::wstring keyPrompt() const {
    send(L"K:");
    return read();
  }
  virtual std::wstring keyPrompt(const std::wstring & msg) const {
    send(L"K:" + msg);
    return read();
  }
  virtual std::wstring keyPrompt(const wchar_t *msg) const {
    return keyPrompt(std::wstring(msg));
  }
  virtual std::wstring keyPrompt(const std::wstring & msg, const std::wstring &help) const {
    send(L"K:" + help + std::wstring(L"\n") + msg);
    return read();
  }
  virtual wchar_t dirPrompt() const {
    send(L"K:WASD<>.");
    return read()[0];
  }
  virtual bool ynPrompt(std::wstring msg) const {
    send(L"Y:" + msg);
    return read()[0] == L'Y';
  }
  virtual std::wstring linePrompt() const {
    send(L"I:");
    return read();
  }
  virtual std::wstring linePrompt(const std::wstring &msg, const std::wstring &help) const {
    send(L"I:" + help + msg);
    return read();
  }
  virtual std::pair<unsigned char, unsigned char> 
  genderPrompt(const wchar_t * msg, const wchar_t * help,
	       const wchar_t * female0help, const wchar_t * female100help,
	       const wchar_t * male0help, const wchar_t * male100help) const {
    std::wstring nl = L"\n";
    send(L"G:" + std::wstring(msg) + nl + female0help + nl + female100help + nl + male0help + nl + male100help);
    std::wstring fromClient = read();
    int pos = fromClient.find(';');
    return std::pair<unsigned char, unsigned char>(
      std::stoi(fromClient.substr(0,pos)),
      std::stoi(fromClient.substr(pos+1)));
  }
  virtual void draw(const dungeon & d) const {
    clear();
    draw(d.cur_level());
    draw(*(d.pc()));
  }
  virtual void draw(const renderByCoord & d) const {
    std::wstringstream fmt;
    fmt << L"R:";
    auto e = d.drawEnd();
    for (auto r = d.drawBegin(); r != e; ++r) {
      auto pair = *r;
      const renderable &rend = pair.second;
      if (rend.highlight()) fmt << L"*"; else fmt << L" ";
      fmt << std::wstring({rend.render(), L'\0'})
	  << rend.name() << L"\n" << rend.description()
	  << "<!>";
    }
    send(fmt.str());
  }
  virtual void draw(const player & p) const {
    std::wstringstream fmt;
    std::wstring s = p.name(); // sanitise by removing ;
    s.erase(std::remove(s.begin(), s.end(), L';'), s.end());
    fmt
      << L"P:" << s
      << L";" << p.strength()
      << L";" << p.fighting()
      << L";" << p.dodge()
      << L";" << p.injury()
      << L";" << p.appearance()
      ;
    send(fmt.str());
  }

  // interrogate the dungeon (or whatever is showing):
  virtual void interrogate(const renderByCoord &, const coord &c) const {
    // not handled; we preemptively send tooltips to the client
  }
private:
  void send(const std::wstring &str) const {
    buf_ << str << "<!END!>\n";
  }
  // out will block if the other end isn't connected, so we open it on demand
  void doSend(const std::wstring &str,
	      const char *const handshake = "\n<!OVER!>\n") const {
    std::ofstream out(prefix_ + "Down.fifo"); // closed automatically (we lose error data, but we're a game...)
    if (!out) throw "Failed to open " + prefix_ + "Down.fifo"  + std::strerror(errno);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::string bytes = utf8_conv.to_bytes(str);
    out << bytes
	<< handshake << std::flush; // tell CGI we want input
  }
  // in also blocks, but fifo(7) doesn't indicate why:
  std::wstring read() const {
    doSend(buf_.str()); // always send before receiving, even if no data, as the client may be blocking.
    buf_.str(std::wstring()); buf_.clear(); // both needed to reset
    //std::ifstream in(prefix_ + "Up.fifo");
    //if (!in) throw "Failed to open " + prefix_ + "Up.fifo" + std::strerror(errno);
    std::string rtn;
    do {
      do {
	int ch = in_.get();
	if (ch != EOF) rtn += static_cast<char>(ch);
      } while (rtn.back() != '>');
    } while (rtn.size() < 7 ||
	     (rtn.compare(rtn.size() - 7, 7, "<!END!>") != 0 &&
	      rtn.compare(rtn.size() - 7, 7, "<!FIN!>") != 0)
	     );
    rtn.erase(rtn.size() - 7);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.from_bytes(trim(rtn));
  }
};


// create a fifo IO object
std::shared_ptr<io> ioFactory::connectFifos(const args &opts) {
  // daemonize:
  ::setsid();
  ::close(0);
  ::close(1);
  ::close(2);
  
  const char* const prefix = opts.option("fifos");
  return std::make_shared<fifo_io>(prefix);
  
}
