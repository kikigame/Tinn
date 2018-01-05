/* License and copyright go here*/

// general output routine contract

#include <cwctype> // locale-specific functions

#include "output.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "player.hpp"

io::~io(){}

template <typename T>
size_t choicePrompt(const io &ios, const std::wstring &prompt, const std::wstring &help, 
		    const std::vector<std::pair<T, std::wstring> > &choices,
		    const std::wstring &extraHelp, size_t offset) {
  unsigned int res, i;
  size_t numChoices = choices.size() - offset;
  bool hasMoreChoices = numChoices > 9;
  size_t maxInput = (hasMoreChoices) ? 9 : numChoices;
  if (hasMoreChoices) numChoices = 8;
  do {
  LOOP:
    std::wstring msg = prompt + L"; please press [1] to [" + std::to_wstring(numChoices) +  L"]:";
    std::wstring list;  
    for (i=0; i < numChoices; ++i)
      list += L"  " + std::to_wstring(i+1) + L" - " + choices[offset + i].second + L"\n";
    if (hasMoreChoices)
      list += L"  9 - --MORE--\n";
    auto ch = ios.keyPrompt(msg, help + L"\n" + list + L"\n" + extraHelp);
    try {
      res = std::stoi(ch.c_str());
    } catch (std::invalid_argument) {
      goto LOOP; // retry after invalid key
    } catch (std::out_of_range) {
      goto LOOP; // retry after invalid string typed
    }
  } while (res < 1 || res > maxInput);
  return res;
}

// template methods can't be virtual; score 1 for Java generics.
// may have to rethink this, but currently only support terminal-type I/O, so this can go here safely.
template <typename T>
T io::choice(const std::wstring &prompt, const std::wstring &help, 
	     const std::vector<std::pair<T, std::wstring> > &choices,
	     const std::wstring &extraHelp) const {
  size_t offset=0, choice, res;
  do {
    choice = choicePrompt(*this, prompt,help,choices,extraHelp,offset);
    res = choice + offset;
    if (choice == 9 && res < choices.size()) offset += 9;
  } while (choice == 9 && res < choices.size());
  auto rtn = choices.begin();
  for (int i=1; i < res; ++i) ++rtn;
  return rtn->first;
}

// this is a full specialisation of io::choice() which allows selecting based on keystrokes
template<>
wchar_t io::choice<wchar_t>(const std::wstring &prompt, const std::wstring &help, 
			    const std::vector<std::pair<wchar_t, std::wstring>> &choices,
	     const std::wstring &extraHelp) const {
  // we want to use the values in the choices.*.first, provided they are keys on the keyboard.
  // we want to be case-insensitive.
  std::map<wchar_t, wchar_t> keys; // map of typable keystroke to return value
  std::map<wchar_t, wchar_t> keystroke; // map of return value to displayed keystroke
  std::vector<wchar_t> replacementKeys; // buffer for non-typable chars
  // 1) build up keys and keystroke for all keys on the keyboard:
  for (auto &p: choices) {
    auto key=p.first;
    if (key >= L'a' && key <= L'z') { // allow as-is and upcased on keyboard
      wchar_t uc = std::towupper(key);
      keys[uc] = key;
      keys[key] = key;
      keystroke[key]=uc;
    } else if (key >= L'A' && key <= L'Z') { // allow as-is and downcased on keyboard
      wchar_t lc = std::towlower(key);
      keys[lc]=key;
      keys[key]= key;
      keystroke[key]=key;
    } else if (key >= L'1' && key <= L'9') { // numbers: allow as-is
      keys[key]= key;
      keystroke[key]= key;
    } else if (key == L'0' || key == L'!' || key == L'\"' || key == L'$' || 
	       key == L'%' || key == L'^' || key == L'&' || key == L'*' || 
	       key == L'(' || key == L')' || key == L'[' || key == L']' || 
	       key == L'{' || key == L'}' || key == L'/' || key == L'\\' || 
	       key == L'?' || key == L'=' || key == L'-' || key == L'+' || 
	       key == L'_' || key == L'#' || key == L'~' || key == L';' || 
	       key == L':' || key == L'.' || key == L'<' || key == L'>' || 
	       key == L',' || key == L' ' || key == L'\'' ) { // other as-is keys
      keys[key]= key;
      keystroke[key]= key;
    } else {
      replacementKeys.push_back(key); // we don't know this yet.
    }
  }
  // now we know every key we do have, we can fill in the gaps:
  wchar_t i=L'1';
  for (auto k: replacementKeys) {
    while (i >= L'1' && i <= L'8' && keys.count(i)) i++;
    if (keys.count(i)) i = L'A';
    while (i >= L'A' && i <= L'Y' && keys.count(i)) i++;
    // shouldn't happen as I write this because there's always a compile-time limit on the selection.
    // but going forward I may need to have a bigger string to iterate through:
    if (keys.count(i)) throw L"Not enough known keys for this menu!";
    keys[i]=k;
    keystroke[k]=i;
    wchar_t lc = std::towlower(i); // returns lc if not a letter
    if (!keys.count(lc)) keys[lc]=k; // also allow letters in lowercase (non-letters will already be in the map)
  }

  std::wstring ch;
  while(true) {
    std::wstring msg = prompt + L"; please select:" + ch;
    std::wstring list;
    for (auto &p : choices)
      list += L"  [" + std::wstring(1,keystroke[p.first]) + L"] - " + p.second + L"\n";
    ch = keyPrompt(msg, help + L"\n" + list + L"\n" + extraHelp);
    for (auto &c : keys)
      if (ch.length() > 0 && c.first == ch[0]) return c.second;
  }
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
  ios.choice<std::shared_ptr<item> >(L"", L"", {}); // used in shop
  ios.choice<wchar_t>(L"", L"", {}); // used in encyclopedia
}
