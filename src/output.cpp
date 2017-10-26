/* License and copyright go here*/

// general output routine contract

#include "output.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "player.hpp"

io::~io(){}


// template methods can't be virtual; score 1 for Java generics.
// may have to rethink this, but currently only support terminal-type I/O, so this can go here safely.
template <typename T>
T io::choice(const std::wstring &prompt, const std::wstring &help, 
	     const std::vector<std::pair<T, const wchar_t*>> choices,
	     const std::wstring &extraHelp) const {
  unsigned int res, i;
  const int numChoices = choices.size();
  do {
  LOOP:
    std::wstring msg = prompt + L"; please press [1] to [" + std::to_wstring(numChoices) +  L"]:";
    std::wstring list;
    i=1;
    for (auto p : choices)
      list += L"  " + std::to_wstring(i++) + L" - " + p.second + L"\n";
    auto ch = keyPrompt(msg, help + L"\n" + list + L"\n" + extraHelp);
    try {
      res = std::stoi(ch.c_str());
    } catch (std::invalid_argument) {
      goto LOOP; // retry after invalid key
    } catch (std::out_of_range) {
      goto LOOP; // retry after invalid string typed
    }
  } while (res < 1 || res >= i);
  auto rtn = choices.begin();
  for (i=1; i < res; ++i) ++rtn;
  return rtn->first;
}

// workaround for linker wibbles
#include "religion.hpp"
#include "monsterType.hpp"
#include "role.hpp"
class shop;
void junk(const io &ios) { // this method is not used.
  // define these methods by calling them. That gets them into the object file, so the linker can find them.
  ios.choice<Domination>(L"", L"", {});
  ios.choice<Outlook>(L"", L"", {});
  ios.choice<Element>(L"", L"", {});
  ios.choice<monsterTypeKey>(L"", L"", {});
  ios.choice<roleType>(L"", L"", {});
  ios.choice<int>(L"", L"", {}); // used in shop
}
