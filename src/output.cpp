/* License and copyright go here*/

// general output routine contract

#include <cwctype> // locale-specific functions

#include "output.hpp"

#include "dungeon.hpp"
#include "level.hpp"
#include "player.hpp"

io::~io(){}

template <typename T>
bool fillExtraHelp(std::vector<std::wstring> &extraHelps,
		   const std::vector<std::pair<T, std::wstring> > &choices,
		   const std::function<std::wstring(const T &)> &extraHelp,
		   size_t offset, size_t numChoices /*>=1*/) {
  std::set<std::wstring> extraHelpIdx;
  for (size_t i=0; i < numChoices; ++i) {
    std::wstring help = extraHelp(choices[offset + i].first);
    extraHelps.emplace_back(help);
    extraHelpIdx.emplace(help);
  }
  return extraHelpIdx.size() > 1 || *extraHelpIdx.begin() != std::wstring(L"");
}

template <typename T>
size_t choicePrompt(const io &ios, const std::wstring &prompt, const std::wstring &help, 
		    const std::vector<std::pair<T, std::wstring> > &choices,
		    const std::function<std::wstring(const T &)> &extraHelp,
		    size_t offset) {
  unsigned int res, i;
  size_t numChoices = choices.size() - offset;
  bool hasMoreChoices = numChoices > 8;
  size_t maxInput = (hasMoreChoices) ? 9 : numChoices;
  size_t highlight = 0;
  std::vector<std::wstring> extraHelps;
  bool hasExtraHelp = fillExtraHelp(extraHelps, choices, extraHelp, offset, numChoices);
  if (hasMoreChoices) numChoices = 8;
  do {
  LOOP:
    std::wstring msg = prompt + L"; please press [1] to [" + std::to_wstring(numChoices) +  L"]:";
    std::wstring list;
    for (i=0; i < numChoices; ++i) {
      list += L"  " + std::to_wstring(i+1) + (i == highlight ? L" > " : L" - ") + choices[offset + i].second + L"\n";
      auto str = extraHelps.at(i);
    }
    std::wstring extraHelpMsg = extraHelp(choices[highlight + offset].first);
    if (hasExtraHelp)
      extraHelpMsg = L"\t([W] & [S] navigate help)\n\n" + extraHelpMsg;
    if (hasMoreChoices)
      list += L"  9 - --MORE--\n";
    wchar_t ch = ios.keyPrompt(msg, help + L"\n" + list + L"\n" + extraHelpMsg)[0];
    if ((ch == L'W' || ch == L'w') && highlight > 0) highlight--;
    if ((ch == L'S' || ch == L's') && highlight < maxInput-1) highlight++;
    if (ch == L'\n') return offset + highlight + 1; // 1-based index in return
    try {
      res = std::stoi(std::wstring({ch, L'\0'}));
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
	     const std::function<std::wstring(const T &)> &extraHelp) const {
  size_t offset=0, choice, res;
  do {
    choice = choicePrompt(*this, prompt,help,choices,extraHelp,offset);
    res = choice + offset;
    if (choice == 9 && res < choices.size()) offset += 8;
  } while (choice == 9 && res < choices.size());
  auto rtn = choices.begin();
  for (unsigned int i=1; i < res; ++i) ++rtn;
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
    if (keys.count(i)) {
      std::wstring k(L"!\"£$%^&*(){}[]?+/=_~-#|:\\;@'");
      for (auto pi = k.begin(); pi != k.end() && keys.count(i); ++pi) i=*pi;
    }
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

template <typename T>
T* io::choice(const std::wstring &prompt, const std::wstring &help,
	      const std::vector<T*> &opts,
	      const bool allowNone,
	      const std::function<std::wstring(const T&)> namer,
	      const std::function<bool(const T&)> f,
	      const std::wstring &extraHelp) const {
  if (opts.empty()) return nullptr;
  std::vector<std::pair<int, std::wstring>> choices;
  std::vector<T *> res;
  int i=0;
  for (auto &t : opts)
    if (f(*t)) {
      choices.emplace_back(i++, namer(*t));
      res.emplace_back(t);
    }
  if (allowNone) {
    choices.emplace_back(i++, L"Nothing");
    res.emplace_back(nullptr);
  }
  // if we must choose something, but have nothing to choose, we arbitrarily don't.
  // NB: this can happen when shopping for a repair service without anything to repair.
  if (choices.size() == 0) return nullptr;
  int it = choice(prompt, help, choices, extraHelp);
  return *(res.begin() + it);
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
  std::function<std::wstring (wchar_t const&)> fn = [](wchar_t){return L"";};
  std::vector<std::pair<wchar_t, std::wstring>> v;
  ios.choice<wchar_t>(L"", L"", v, fn);
  std::vector<item*> v2;
  std::function<std::wstring(const item &i)> namer;
  ios.choice<item>(L"", L"", v2, false, namer);
}
