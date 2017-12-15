/* License and copyright go here*/

// define an encyclopedia of the dungeon

#include "encyclopedia.hpp"
#include "items.hpp"
#include "output.hpp"
#include "damage.hpp"
#include "religion.hpp"
#include "items.hpp"
#include "monster.hpp"

// we don't bother with a renderable.cpp file, so define the static fields here:
// NB: This is the construct-on-first-use idiom, to work around C++'s "interesting"
// initialsation order fiasco:
// https://isocpp.org/wiki/faq/ctors#static-init-order
std::set<const renderable*> & renderable::all_() {
  static auto all = new std::set<const renderable*>();
  return *all;
}
long renderable::count_ = 0;


class encyclopedia {
private:
  const std::set<const renderable*> &renderables_;
  const bool allowFreeSearch_;
  wchar_t category_;
  mutable std::vector<std::wstring> opts_; // buffer used by choiceKeys() to keep returned pointers alive (used internally)
  
public:
  /*
   * Constructor. Note we construct these as needed rather than attaching them to the item, as they can
   * do quite a bit of work and hold transient data.
   */
  encyclopedia(const bool allowFreeSearch) :
    renderables_(renderable::all()), allowFreeSearch_(allowFreeSearch),
    category_(L'\0') {}

  /*
   * Use the electronic encyclopedia of this game
   * item is an (optionally null/empty) shared_ptr of the item to look up.
   */
  void invoke(optionalRef<item> item) { 
    if (item) category_ = item.value().render();
    while (true)
      switch (category_) {
      case L'\0':
	category_ = pickCategory();
	break;
      case L'q':
      case L'Q':
	return;
      default:
	if (item) { // if called with an item, just show it.
	  show(item.value());
	  item.reset(); // don't loop over showing it!
	} else {
	  search();
	}
	if (!ioFactory::instance().ynPrompt(L"Try again?"))
	  category_ = L'\0';
      }
  }

private:
  /*
   * Does "haystack" start with "needle", case-insensitive?
   * needleLc is the downcased needle.
   */
  bool match(const wchar_t *haystack, std::basic_string<wint_t> needleLc) const {
    int needleLength = needleLc.length();
    for (int i=0; i < needleLength; ++i)
      if (std::towlower(haystack[i]) != needleLc[i]) return false;
    return true;
  }

  // look for user-entered string
  void search() const {
    std::wstring prompt;
    bool freeSearch = allowFreeSearch_ && (category_ == L'/' || category_ == '\0');
    if (!freeSearch) prompt = std::wstring(L"You have selected \"") + category_ + std::wstring(L"\". ");
    prompt += std::wstring(L"Search for ? ");
    std::wstring needle;
    auto &ios = ioFactory::instance();
    while (needle.length() == 0)
      needle =
	ios.linePrompt(prompt, std::wstring(L"Powered by Grail")); // Ref:The quest for the grail; hard to find.
    std::basic_string<wint_t> needleLc;
    for (unsigned int i=0; i < needle.length(); ++i)
      needleLc += std::towlower(needle[i]);
    for (auto e : renderables_) {
      if ((freeSearch || category_ == e->render()) && match(e->name(), needleLc)) {
	show(*e);
	return;
      }
    }
    ios.longMsg(L"Have you seen my towel?"); // ref:Hitchhiker's Guide to the Galaxy, in which it is explained that a hitchhiker can get away with anything provided he has a towel with him.
  }

  // show an encyclopedium
  void show(const renderable &e) const {
    auto &ios = ioFactory::instance();
    std::wstring msg;
    if (e.highlight()) msg += L"★"; // ref:Netiquette tradition of using asterisks for bold; well established by the mid '80s. Probably a 60's/70's BBS tradition?
    msg += e.name();
    if (e.highlight()) msg += L"★";
    ios.longMsg(msg + L"\n" + e.description());
  }
  
  // return the available keys for pickCategory()
  const std::vector<std::pair<wchar_t, const wchar_t*>> choiceKeys() const {
    std::set<wchar_t> choiceChars;
    for (auto e : renderables_)
      choiceChars.insert(e->render());

    std::vector<std::pair<wchar_t, const wchar_t*>> choiceKeys;
    opts_.clear();
    int i=0;
    for (auto choice : choiceChars) {
      wchar_t str[] = {choice, '\0'};
      opts_.emplace_back(str);
      choiceKeys.emplace_back(choice, opts_[i++].c_str());
    }

    if (allowFreeSearch_)
      choiceKeys.emplace_back(L'/', L"Search");
    choiceKeys.emplace_back(L'Q', L"Quit (Back to Game)");
    return choiceKeys;
  }

  // prompt the user for a category and return the category code
  wchar_t pickCategory() const {
    return 
      ioFactory::instance().choice<wchar_t>
      (std::wstring(L"Welcome to the Guide. Choose your category"),
       L"Choosing a category will assist us in processing your enquiry",//REF:every call centre ever
       choiceKeys());
  }
};



void invokeGuide(const bool allowFreeSearch, const optionalRef<item> other) {
  encyclopedia(allowFreeSearch).invoke(other);
}
