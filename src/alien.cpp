/* License and copyright go here*/

// Where aliens come from. Ref: Elite (BBC Micro)
#include <iostream>
#include "alien.hpp"
#include "terrain.hpp"
#include "random.hpp"
#include "optionalRef.hpp"

extern const wchar_t * const shopAdjectives[]; // adjectives.cpp

#include <cstdint>
#include <cstring>
#include <array>
#include <algorithm>


std::wstring wordWrap(const std::wstring &str) {
  if (str.length() < 80) return str;
  // write out partial lines, else we could break a truncated line
  auto lastBreak = str.begin() + 80;
  auto p = std::find(str.begin(), lastBreak, L'\n');
  if (p != lastBreak)
    return std::wstring(str.begin(), p+1) + wordWrap(std::wstring(p+1, str.end()));
  for (int i=80; i > 0; --i)
    if (str[i] == L' ' || str[i] == L'\n') return std::wstring(str.begin(), str.begin() + i) + L"\n" + wordWrap(std::wstring(str.begin() + (i+1), str.end()));
  return std::wstring(str.begin(), str.begin() + 79) + L"-\n" + wordWrap(std::wstring(str.begin() + 79, str.end()));
}
    


namespace alien {
  static std::uniform_int_distribution<int> dCharDist(0,0xff);
  static auto dChar = std::bind ( dCharDist, generator );
  
  class worldFactory {
  private:
    uint8_t w;
    uint8_t x;
    uint8_t y;
    uint8_t z;
    std::wstring systemName_;
    bool isShip_;
    mutable optionalRef<deity> align_;

    std::wstring systemNameGen() {
      // Elite uses seeding to ensure that generated names are okay.
      // We don't, so let's take out a few options:
    
      // disallow blank:
      if (((w | x | y | z) & 0x1f) == 0) return L"Zog"; // Ref: Hyperspace, Edward Packard 1984 (Choose Your Own Adventure book) (Popularised by the Big Breakfast, 1991)
    
      // they allowed a slightly-too-interesting word:
      if ((w == 0x1b || x == 0x1b || y == 0x1b) && (x == 0x8 || y == 0x8 || z == 0x8))
	return L"Uranus"; // Ref: sensible name for a real planet.
      
      std::wstring rtn(pair(w) + pair(x) + pair(y) + pair(z));
      rtn[0] = std::toupper(rtn[0]);
      return rtn;
    };
    

    std::wstring pair(uint8_t c) {
      static const wchar_t* pairs[] = {
	L"",   L"le", L"xe", L"ge", L"za", L"ce", L"bi", L"so",
	L"us", L"es", L"ar", L"ma", L"in", L"di", L"re", L"a",
	L"er", L"at", L"en", L"be", L"ra", L"la", L"ve", L"ti",
	L"ed", L"or", L"qu", L"an", L"te", L"is", L"ri", L"on"};
      return pairs[c & 0x1f];
    }

    template<int numEntries>
    const wchar_t *const expand(const uint8_t &x, const std::array<const wchar_t *const, numEntries> &table) {
      return table[x % numEntries];
    }

    std::wstring expandTag(const wchar_t key, std::wstring &rtn) {
      	switch (key) {
	case L'(': {
	  auto next = expand<8>(x, {
	      L"The %aplanet %N %{",
		L"The %aplanet %N %{",
		L"%N %{",
		L"This planet %N %{",
		L"The %aworld %N %{",
		L"The land of %N %{",
		L"The inhabited moon of %N %{",
		L"The %aspace ship %N %{"
		});
	  isShip_ = wcscmp(next, L"The %aspace ship %N %{") == 0;
	  return next;
	}
	case L'N': return systemName_;
	case L'R': return systemNameGen();
	case L'{':
	  return expand<8>(x, {
	      L"is famous for its %d",
		L"is sometimes considered for its %d",
		L"is unfortunately known for its %d",
		L"is in the record because of its %d",
		L"is fabled for its %d",
		L"has rumours about its %d",
		L"is most noted for its %d",
		L"gives visitors 3 guesses about its %d"
		});
	case L'd': // description
	  return expand<3>(x, {
	      L"%p, and its %p",
		L"%p",
		L"%p%s also has %n",
		});
	case L'a': // adjective, then space
	  if ((x % 27) < 26) {
	    auto adj = shopAdjectives[x % 27];
	    std::wstring rtn;
	    rtn += std::tolower(adj[0]);
	    rtn += adj+1;
	    rtn += L" ";
	    return rtn;
	  }
	  return L"";;
	case L'A': // "[a|an] %a"
	  if ((x % 27) < 26) {
	    auto adj = shopAdjectives[x % 27];
	    std::wstring rtn = (adj[0] == L'A' || adj[0] == L'E' || adj[0] == L'I' || adj[0] == L'O' || adj[0] == L'U') ? L"an " : L"a ";
	    rtn += std::tolower(adj[0]);
	    rtn += adj+1;
	    rtn += L" ";
	    return rtn;
	  }	  
	  return L"";
	case L'p': // positive trait (its)
	  return expand<22>(x, {
	      L"%around things" /*ref:TARDIS, Dr Who*/,
		L"%atulips", // Ref: from Elite
		L"%aburgers", // Ref: from Elite
		L"%asit-coms", // Ref: from Elite
		L"%afood blenders", // Ref: from Elite
		L"%a%<%g", // Ref: some possible in Elite (I believe)
		L"%agames of cricket", // Ref: from Elite
		L"%agargle blasters", // Ref: from Elite, referencing Hitch-hikers' Guide to the Galaxy
		L"%aeducation system",
		L"%asunrises",
		L"%asunsets",
		L"%abinary stars",
		L"great %aoceans",
		L"bartering-based economy",
		L"%amead",
		L"%adragon infestations",
		L"%amountain ranges",
		L"%atourism",
		L"%asecrets",
		L"bouquet garni",
		L"%a%w",
		L"%w"
		});
	case L'n': // negative trait
	  return expand<12>(x, {
	      L"%acivil wars", // Ref: from Elite
		L"%adiseases", // Ref: from Elite (disease)
		L"%aearthquakes", // Ref: from Elite
		L"%j fish", // Ref: deadly fish possible in Elite (I believe)
		L"%j %g contests",
		L"%j beauty contests",
		L"%j solar activity", // Ref: "occasional solar activity" is possible in Elite (I believe)
		L"%j %abackground humming",
		L"%j rules under a system of martial law",
		L"extreme wealth inequality",
		L"%j weather",
		L"%j piracy"
		});
	case L'r': // religious path
	  return align().name();
	case L'g': // game (competitive activity)
	  { return expand<21>(x, {
		L"karate", // Ref: from Elite
		  L"polo", // Ref: from Elite
		  L"hockey", // Ref: from Elite
		  L"cricket", // Ref: from Elite
		  L"tennis", // Ref: from Elite
		  L"karaoke",
		  L"competitive waving",
		  L"%mpainting",
		  L"%mwrestling",
		  L"tiddlywinks",
		  L"quidditch", // ref: Potter. We had to have on somewhere
		  L"foot-the-ball", // ref: Unseen Academicals (Pratchett, book)
		  L"push-penny",
		  L"cabinet assembly", // ref: Dr Who (sonic screwdriver jokes)
		  L"dom-jot", // ref: Star Trek
		  L"podracing", // ref: Star Wars
		  L"poker",
		  L"Thud", // ref: Pratchtt [Thud]
		  L"Ski-surfing", // ref: Judge Dredd
		  L"Poohsticks", // ref: Winnie the Pooh
		  L"Mornington Crescent" // ref: I'm Sorry, I haven't A Clue (BBC Radio 4 series)
		  });
	  }
	case L'j': // negative adjectives (eg solar activity)
	  return expand<12>(x, {
	      L"draconian",
		L"bad",
		L"unpredictable", // Ref: from Elite
		L"dreadful", // Ref: from Elite
		L"deadly", // Ref: from Elite
		L"dangerous",
		L"hazardous",
		L"unfortunate",
		L"boring", // Ref: from Elite (but not used here)
		L"unexplained",
		L"nausea-inducing",
		L"smelly"
		});
	case L'm': // media (prefixes for painting)
	  return expand<9>(x, {
	      L"egg-",
		L"body-",
		L"un",
		L"tin-",
		L"oil ",
		L"digital ",
		L"artistic ",
		L"pseudorandom ",
		L"abstract "
		});
	case L'<': // prefix
	  return expand<12>(x, {
	      L"",
		L"mud-", // Ref: from Elite
		L"Zero-G ", // Ref: from Elite
		L"speed-",
		L"tabletop ",
		L"lazy ",
		L"naked ",
		L"extreme ",
		L"ritual ",
		L"spontaneous ",
		L"sub-aquatic ",
		L"bungee "
		});
	case L'>': // codas: split by type or else we get duplications
	  return expand<13>(x, {
	      L"", L"%!", L"%)", L"%!%)", L"%!%}%)", L"%}%!%)",
	      L"%l", L"%!%l", L"%l%)", L"%l%!%)", L"%!%}%l%)", L"%}%l%!%)", L"%!%}%l%)"
		});
	case L'b': // aboard (yes, I know the difference between on board and aboard, but no-one else seems to)
	  return expand<5>(x, {
	      L"aboard",
		L"on board",
		L"on",
		L"travelling on",
		L"during the flight of"
	    });
	case L'w': // whereabouts (place, singular)
	  return isShip_ ?
	    expand<10>(x, {
		L"third deck",
		  L"lido deck",
		  L"bridge",
		  L"canteen",
		  L"poop deck",
		  L"star deck", // like a sun deck, but on a spaceship
		  L"observation lounge", // ref: Star Trek briefing room
		  L"on-board cinema",
		  L"gift shop",
		  L"brig"
	      }) :
	    expand<8>(x, { 
		L"northern hemisphere",
		  L"southern hemisphere",
		  L"eastern hemisphere",
		  L"western hemisphere",
		  L"largest continent",
		  L"second largest continent",
		  L"capital city",
		  L"single ocean"
	      });
	case L'W': // whereabouts (places, plural)
	  return isShip_ ?
	    expand<2>(x, {
		  L"aft quarters",
		  L"heads" // nautical for the loo
	      }) :
	    expand<9>(x, {
		  L"equatorial regions",
		  L"polar regions",
		  L"oceans",
		  L"red-light districts",
		  L"slums",
		  L"%t fields",
		  L"%<%g fields",
		  L"%t plantations",
		  L"%<%t farms"
	      });
	case L'l':
	  { std::wstring next = 
	  isShip_ ?
	    expand<14>(x, {
		L".\n%b %N, rum rations are issued %c times per day",
		  L".\n%b %N, the lighting is changed on %Aschedule",
		  L".\nPassenger services %b %N are available during office hours",
		  L".\nIn the canteen %b %N, ice-cream is served every sundae", // spelling joke.
		  L".\nThe engines of %N could power %c entire cities, if they weren't on a ship in space",
		  L".\nThe lido deck of %N contains %c whole lidos", // lido = swimming pool area
		  L".\nThe bridge of %N has no river",
		  L".\nPassenger access to the recreational facilities %b %N are %flimited",
		  L".\nThe crew of %N is made up entirely of aliens from the planet %R",
		  L".\nThe quartermaster of %N keeps ordering too much %t pie",
		  L".\nPassengers %b %N report strange dreams of %<%g",
		  L".\nThe %w of %N %{",
		  L".\nMuch of %w of %N is dedicated just to %<%g",
		  L".\nThe %W of %N are dedicated just to %<%g",
	      }) :
	    expand<8>(x, {
		L".\nA year on %N lasts up to %c years, %c weeks and an hour on %R",
		  L".\n%N spins up to %c years in %C week",
		  L".\n%N experiences up to %c years in %C week",
		  L".\n%N spins up to %c years in %C week. May cause dizziness",
		  L".\nOn %N, %C year incurs annual taxes",
		  L".\nThe %w of %N %{",
		  L".\nThe entirety of the %w of %N is dedicated just to %<%g",
		  L".\n%w of %N are dedicated just to %<%g",
		  });
	    // fix the up-case of the letter. Expand first to remove the possible %b.
	    std::wstring enext = expand(next);
	    enext[2] = std::toupper(enext[2]);
	    return enext;
	  }
	case L'!': // religious coda
	  return expand<10>(x, {
	      L"",
		L".\nThe inhabitants believe in %r",
		L".\nThe people believe in %r",
		L".\nThe people worship %r",
		L".\nThe locals worship %r",
		L".\nThere are many temples to %r",
		L".\n%r's spiritual teachings are revered",
		L".\nNatives pay homage to %r",
		L".\nIts %h is known as the High Priest of %r",
		L".\nCollections to repair the roof of the church of %r broke all records this year"
		});
	case L')': // end coda
	  return expand<9>(x, {
	      L"",
		L".\nThere is a large %y %j %<%t industry",
		L".\nThe locals are very %j - even on a Monday",
		L".\nYou'd better enjoy their %p",
		L".\nDid you know about their %p? You do now",
		L".\nSo, overall:\n\tMostly Harmless", // ref: Hitch-hiker's guide to the galaxy again (entry on Earth)
		L".\nAliens from %N have been seen on %R, %R, and the Areas of Adventure",
		L".\nAliens from %N have been seen on %R, the Areas of Adventure, and %R",
		L".\nAliens from %N like %<%g, %t farming, and serial commas"
		});
	case L'}': // holiday coda
	  return expand<6>(x, {
	      L"",
		L".\nIt has to be seen to be believed",
		L".\nVisiting is not recommended",
		L".\nPlease consult the bureau of tourism for more details",
		L".\nIt is a very family-friendly holiday destination",
		L".\nChildren must be accompanied",
		});
	case L'f': // frequency, then space
	  { auto next = expand<13>(x, {
		L"",
		  L"occasionally ",
		  L"oft ",
		  L"often ",
		  L"too often ",
		  L"usually ",
		  L"sometimes ",
		  L"rarely ",
		  L"conceivably ",
		  L", by some, ",
		  L", by many, ",
		  L", by most, ",
		  L"always "
		  });
	    if (next[0] == L',' && rtn.back() == L' ') rtn.pop_back();
	    return next;
	  }
	case L'y': // yet
	  return expand<2>(x, {
	      L"yet", L"but"
		});
	case L's': // <space> yet sadly (also has)
	  return expand<8>(x, {
	      L" %y sadly it",
		L" %y unfortunately it",
		L" %y disconcertingly it",
		L" %y, although it tries to hide it, it",
		L" %y the latest intelligence highlights that it",
		L" %y is %favoided because it",
		L", %y it",
		L" and yet it",
		});
	case L't': // things planted or farmed
	  return expand<8>(x, {
	      L"apple",
		L"fruit",
		L"gravy",
		L"tree",
		L"dirt",
		L"seed",
		L"strawberry",
		L"agronomics" 
		});
	case 'h': // head honcho
	  if (isShip_) return expand<6>(x, {
	      L"captain", L"commander", L"commodore", L"rear-admiral", L"admiral", L"boss"});
	  else return expand<7>(x, {
	      L"leader", L"president", L"prime minister", L"king", L"queen", L"great and glorious leader", L"senior cultural curator"});
	case 'C': // counting (singular)
	  return expand<6>(x, {
	      L"a", L"1", L"one", L"a single", L"only one", L"just one"
	    });
	case 'c': // counting (plural)
	  return expand<12>(x, {
	      L"2", L"3", L"5", L"7", L"9",
		L"two", L"four", L"five", L"eight",
		L"several", L"many", L"a few"
		});
	default:
	  throw std::wstring({L'%', key, L'\0'});
	}
    }
    
    std::wstring expand(const std::wstring &from) {
      auto posEsc = std::find(from.begin(), from.end(), L'%');
      if (posEsc == from.end()) return from;
      std::wstring rtn(from.begin(), posEsc);
      while (posEsc != from.end()) {
	wchar_t key = *++posEsc;
	rtn += expandTag(key, rtn);
	x = (x>>1 | x <<7) ^ w; // rotate and XOR
	if (x == 0) ++x; // guard against repeated patterns
	auto tail = ++posEsc;
	posEsc = std::find(tail, from.end(), L'%');
	std::copy(tail, posEsc, std::back_inserter(rtn));
      }
      return expand(rtn);
    }

  public:
    worldFactory(uint8_t ww,uint8_t xx,uint8_t yy,uint8_t zz) :
      w(ww), x(xx), y(yy), z(zz),
      systemName_(systemNameGen()), isShip_(false), align_() {};
    worldFactory() :
      w(dChar()), x(dChar()), y(dChar()), z(dChar()),
      systemName_(systemNameGen()), isShip_(false), align_() {};

    // system descriptions in Elite were of the rough form:
    // * "the planet is", "<name> is", "the planet <name> is", "this planet is", "the world <name> is"
    // * followed by up to two randomly picked strings.
    // the strings came from flags, which may also effect gameplay.
    // first flag may be level of fame: "mildly known for", "well known for", "most notable for", "most fabled for", "most famous for" etc.
    // second positive descriptors are introduced with "and", while second negative descriptors are introduced with "but".
    // negative descriptors may then be prefixed with "plagued by", "cursed by" or "ravaged by".
    // if there are no strings, the suffix is "boring planet". Let's not use that; there are no boring planets.
    //
    // I'll not copy the algorithm, but use something in the same style.
    std::wstring systemDesc() {
      return wordWrap(expand(L"%(%>."));
    }
    
    const std::wstring name() const {
      return (isShip_ ? L"the Good Ship " : L"the planet " ) + systemName_;
    }
    
    deity & align() {
      if (!align_)
	align_ = optionalRef<deity>(*rndPick(deityRepo::instance().begin(), deityRepo::instance().end()));
      return align_.value();
    }

    deity & align() const {
      if (!align_)
	return deityRepo::instance().nonaligned();
      return align_.value();
    }
    

  };

  world::world(worldFactory &f) :
    desc_(f.systemDesc()),
    name_(f.name()),
    align_(f.align()) {}

  const wchar_t world::render() const {
    return L'*';
  }
  std::wstring world::name() const {
    return name_;
  }
  std::wstring world::description() const {
    return desc_;
  }
  const deity& world::align() const {
    return align_;
  }

  world world::spawn() {
    worldFactory f;
    world rtn(f);
    return rtn;
  }
  world world::spawn(uint8_t w,uint8_t x,uint8_t y,uint8_t z) {
    worldFactory f(w,x,y,z);
    world rtn(f);
    return rtn;
  }
  
};


// int main() {
//   using namespace alien;
//   for (int count =0; count < 1000; ++count) {
//     world next = world::spawn();
//     //int w=dChar(), x=dChar(), y=dChar(), z=dChar();
//     //std::wcout << alien::systemName(i,i+1,i+2, i+3) << "   \t" << alien::systemDesc(i,i+1,i+2,i+3) << std::endl;
//     std::wcout << next.name() << std::endl << "----" << std::endl
// 	       << next.description() << std::endl << std::endl;
//   }
// }
