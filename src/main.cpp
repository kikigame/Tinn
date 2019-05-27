/* License and copyright go here*/

// TINN

#include "dungeon.hpp"
#include "output.hpp"
#include "xo.hpp"
#include "religion.hpp"
#include "time.hpp"
#include "role.hpp"
#include "damage.hpp"
#include "itemTypes.hpp"
#include "args.hpp"

#include <iostream>
#include <sstream>

unsigned char dPc(); // avoid including all of random.hpp

speed playerSpeed(dungeon &d) {
  return d.pc()->abilities()->adjust(d.pc()->movement().speed_);
}

bool validateSpeed(dungeon &d) {
  auto rtn = playerSpeed(d) != speed::stop;
  if (!rtn) ioFactory::instance().message(L"You can't move.");
  return rtn;
}

// handle a user keystroke.
void processInput(dungeon & d, const std::wstring &c, const std::shared_ptr<io> ios);
std::wstring print(const xo &xo);

int play(const args &opt) {

  ioFactory iof;
  auto io = iof.create(opt);

  std::wstring endDetails;
  //  io->message(;
  bool repeat = false;
  do {
    auto ch = io->keyPrompt
      (
       L"TINN: Ambling Around the Areas of Adventure\n" // Reference to Nethack's Exploring the Mazes of Menace, of course.
       "\n"
       " - a game by Robert Lee,\n"
       "        who had better things to do, \n"
       "                but was procrastinating.\n"
       "\n"
       "Shall we play a game? (Y/N) >" // wargames film, obvs.
       );
  if (ch.length() == 0 || ch[0] == 'N' || ch[0] == 'n') {
    io->longMsg(L"They say the only winning move is not to play."); // Wargames again, but this time in Nethack fortune-style.
    repeat = false;
  } else if (ch.length() == 1 && ch[0] == 'H') { // let's not make it too obvious; you must type the egg in the right case...
    io->clear();
    io->message(L"How about global thermonuclear war?"); // Wargames *again*...
    io->longMsg(L"Sometimes people do make mistak"); // Wargames *again*... [sic] the typo on "mistake" (as on the screen on the film)
    xo xo;
    std::wstring msg = L"Let's play Naughts and Crosses!";
    while (xo) {
	int x=0,y=0;
	std::wstring c,d;
	while (c.empty()) {
	  while (c.empty())
	    c = io->keyPrompt(msg + L"\nYour move [A][B][C] ?", print(xo));
	  switch (c.c_str()[0]) {
	  case L'a': case L'A': x=0; break;
	  case L'b': case L'B': x=1; break;
	  case L'c': case L'C': x=2; break;
	  default: c = L"";
	  }}
	do { y=-1; d = io->keyPrompt(msg + L"\nYour move [1][2][3] ? " + c, print(xo));
	} while (d.empty() || (y = (static_cast<int>(d.c_str()[0]-'0')-1)) < 0 || y >= 3 );
	msg = xo.go(y,x); // that'll confuse 'em!
    }
    io->longMsg(msg + L"\n\n" + print(xo));
    repeat = false;
  } else if (ch.length() == 1 && (ch[0] == L'Y' || ch[0] == L'y')) {
  
    dungeon d;
    while (d.alive()) {
      // always draw as the last thing before input; this guarantees that it
      // happens after any other timers.
      d.draw(); 
      auto ch = io->keyPrompt(L"Your move... (? for help; q to quit) ");
      processInput(d, ch, io);
    }
    endDetails = d.score();
  } else {
    repeat=true;
  }
} while (repeat);

  io->longMsg(L"You do not get your possessions identified.\n\n" + endDetails); // pun on Nethack end quote.

  return 0;
}

const wchar_t * help() {
  return L"Help on game keys:\n"
    "q) Quit the game\n"
    "wasd) Cardinally move around the map, or attack a monster in that direction\n"
    "       (use Alt/meta to deliberately move into traps)\n"
    "/) Interrogate the map\n"
    "<>) Climb up or down a ramp, pit, stair or ladder\n"
    ",p) Collect (pick up) items\n"
    "i) Take inventory of your items\n"
    "e) Equip (wield/wear) weapons, clothing, jewellery or armour\n"
    "l) Leave (drop) items\n"
    "u) Use an individual item\n"
    "c) Consume something\n"
    "\n"
    "Have fun!";
}

void doTick(dungeon &d){
  static bool lastTick;
  auto s = playerSpeed(d);

  switch (s) {
  case speed::stop:
    time::tick(true);
    time::tick(true);
    time::tick(true);
    break;
  case speed::slow3: 
    time::tick(true);
    time::tick(true);
    break;
  case speed::slow2: 
    time::tick(true);
    if ((lastTick = !lastTick) && dPc() < 75)
      time::tick(true);
    break;
  case speed::perturn:
    time::tick(true);
    break;
  case speed::turn2:
    if ((lastTick = !lastTick))
      time::tick(true);
    break;
  case speed::turn3:
    if ((lastTick = !lastTick) && dPc() < 75)
      time::tick(true);
    break;
  default:
    throw s;
  }
}

void processInput(dungeon & d, const std::wstring &c, const std::shared_ptr<io> ios) {
  if (c.length() == 0) return;

  switch (c[0]) {
  case L'?': case 'H': case 'h':
    ios->longMsg(help());
    return; // skip loop; help should take no time
  case L'q':
    d.quit();
    return; // don't count quit as a move
  case L'w': case L'W': // north
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(0,-1), true);
    doTick(d);
    break;
  case L'a': case L'A': // west
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(-1,0), true);
    doTick(d);
    break;
  case L's': case L'S': // south
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(0,+1), true);
    doTick(d);
    break;
  case L'd': case L'D': // east
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(+1,0), true);
    doTick(d);
    break;
  case 256+L'w': case 256+L'W': // north
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(0,-1), false);
    doTick(d);
    break;
  case 256+L'a': case 256+L'A': // west
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(-1,0), false);
    doTick(d);
    break;
  case 256+L's': case 256+L'S': // south
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(0,+1), false);
    doTick(d);
    break;
  case 256+L'd': case 256+L'D': // east
    if (validateSpeed(d))
      d.cur_level().moveOrFight(*(d.pc()), ::dir(+1,0), false);
    doTick(d);
    break;
  case L'/':
    d.interrogate();
    break;
  case L'<':
    if (validateSpeed(d))
      d.cur_level().up(*(d.pc()));
    doTick(d);
    break;
  case L'>':
    if (validateSpeed(d))
      d.cur_level().down(*(d.pc()));
    doTick(d);
    break;
  case L',': case L'p': case L'P':
    d.cur_level().pickUp();
    time::tick(true); // always 1 turn to take an item
    break;
  case L'i': case L'I':
    d.pc()->takeInventory();
    time::tick(false);
    break;
  case L'e': case L'E':
    d.pc()->equip();
    time::tick(false);
    break;
  case L'l': case L'L':
    d.pc()->drop(d.cur_level());
    time::tick(false);
    break;
  case L'u': case L'U':
    d.pc()->use();
    time::tick(true); // always 1 turn to use an item
    break;
  case L'c': case L'C':
    d.pc()->eat();
    time::tick(true); // always 1 extra turn to eat an item
    break;
  }
}

std::wstring print(const xo &xo) {
  std::wstringstream ss;
  ss << L"\n"
     << L"  |A|B|C|" << L"\n"
     << L"--+-+-+-|" << L"\n";

  int y=0;
  for (auto r : xo) {
    ss << L' ' << ++y << L'|';
    for (auto c : r)
      ss << (c == tile::empty ? L'.' : c == tile::x ? L'X' : L'O')
		<< L'|';
    ss << L"\n";
  }
  return ss.str();
}

// shut down singleton repositories to keep valgrind happy
void cleanup() {
  // the repos are used by various builders; don't need them any more
  deityRepo::close();
  monsterTypeRepo::close();
  roleRepo::close();
  damageRepo::close();
  itemTypeRepo::close();
}

int handleActiveError() {
  try {
    throw;
  } catch (std::exception e) {
    std::cerr << e.what() << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;     
  } catch (std::wstring str) {
    std::wcerr << str << std::endl
	       << L"https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (const wchar_t* str) {
    std::wcerr << str << std::endl
	       << L"https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (const char* str) {
    std::cerr << str << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (itemTypeKey key) {
    std::cerr << "Unknown item type key: " << static_cast<int>(key) << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (slotType key) {
    std::cerr << "Unknown item slot type key: " << static_cast<int>(key) << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (terrainType key) {
    std::cerr << "Unknown terrain type key: " << static_cast<int>(key) << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (coord c) {
    std::cerr << "Bad coords:" << c << std::endl
	      << "https://github.com/kikigame/Tinn/issues" << std::endl;
    return -1;
  } catch (...) {
    std::wcerr << "This doth not bode well..." << std::endl
	       << "https://github.com/kikigame/Tinn/issues" << std::endl;
    throw; // encourage a core-dump in this case
  }    
}

int main (int argc, char **argv) {
  renderable::all(); // ensure that the static variables are initialised before use...

  auto opt =
    args(argc, argv)
    .flag('t').optWithArg("transcript")
    .flag('h').flag('?');

  if (opt.isFlag('h') || opt.isFlag('?') || opt.option("help")) {
    std::wcout << L"Welcome to Tinn!" << std::endl
	       << L"Options are:\n"
	       << L"h/?/-help - this help text\n"
	       << L"transcript=<file> - output transcript to file"
	       << std::endl;
    return 0;
  }

  try {
    play(opt);
    cleanup();
  } catch (...) {
    cleanup();
    return handleActiveError();
  }

}
