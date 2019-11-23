/* License and copyright go here*/

// define the player in the dungeon

#include "player.hpp"
#include "level.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "items.hpp"
#include "religion.hpp"
#include "role.hpp"
#include "terrain.hpp"

#include <sstream>
#include <set>

player::player(playerBuilder &b) :
  monster(b), // we start at level 1, but not yet...
  name_(b.name_),
  role_(b.job()) {
  // attributes are handled by the monster builder itself,
  // but intrinsics are done here, as monsters don't get them
  // from alignment:
  char nullResist = 0;
  switch (align().domination()) {
  case Domination::concentration:
    intrinsics()->speedy(intrinsics()->speedy() + bonus(false));
    {auto dam = damageRepo::instance()[damageType::hot];
    intrinsics()->resist(&dam, -1);
    }
    break;
  case Domination::aggression:
    nullResist = -1;
    break;
  default:
    break; // nothing to do
  }
  switch (align().element()) {
  case Element::earth:
    intrinsics()->eatVeggie(true);
    {auto dam = damageRepo::instance()[damageType::edged];
      intrinsics()->resist(&dam, -2);
    }
    break;
  case Element::air:
    intrinsics()->speedy(true);
    intrinsics()->hear(true);
    {auto dam = damageRepo::instance()[damageType::sonic];
    intrinsics()->extraDamage(&dam, +4);
    }
    nullResist--;
    break;
  case Element::fire:
    intrinsics()->dblAttack(true);
    nullResist-=2;
    break;
  case Element::water:
    intrinsics()->move(tFactory.get(terrainType::WATER), true);
    intrinsics()->speedy(false);
    break;
  case Element::plant:
    intrinsics()->climb(true);
    intrinsics()->eatVeggie(true);
    {auto dam = damageRepo::instance()[damageType::bashing];
    intrinsics()->resist(&dam, -2);
    }
    break;
  case Element::time:
    intrinsics()->speedy(true);
    intrinsics()->dblAttack(true);    
    {auto dam = damageRepo::instance()[damageType::disintegration];
    intrinsics()->resist(&dam, -4);
    }
    break;
  default:
    break; // nothing to do
  }
  intrinsics()->resist(nullptr, nullResist); // can't add directly

  // starting inventory:
  // I feel a player should start with a deterministic inventory, perhaps based
  // on their class and race.
  job().setupPlayer(*this);

  // proper cheat modes: if you have the player name of "Taqris" (deity of trees), you get one of each stick:
  if (name() == L"Taqris")
    for (sharedAction<monster,monster>::key action = static_cast<sharedAction<monster,monster>::key>(0);
	 action != sharedAction<monster,monster>::key::END;
	 action = static_cast<sharedAction<monster,monster>::key>(static_cast<int>(action)+1)) {
      auto &wand = createWand(action);
      wand.enchant(10);
      addItem(wand);
    }
  // and if your name is Kithus, you do need help:
  if (name() == L"Kithus")
    for (sharedAction<item,monster>::key action = static_cast<sharedAction<item,monster>::key>(0);
	 action != sharedAction<item,monster>::key::END;
	 action = static_cast<sharedAction<item,monster>::key>(static_cast<int>(action)+1)) {
      auto &ring = createEquippable(itemTypeKey::wooden_ring, action);
      addItem(ring);
    }


}

player::~player() {}

const wchar_t player::render() const {
  return L'@';
}

std::wstring player::name() const {
  return name_.c_str();
}

std::wstring player::description() const {
  return L"A bold and dangerous hero, quosting for adventure";
}

// inventory an item, with recursive inventory of containers.
// this will break if we have a loop, but we don't currently allow loops.
// we may need to rethink this with some bags of holding :)
void player::containerInventory(std::wstringstream &inv,
				const std::wstring indent,
				const item &i) const {
  inv << indent << i.render() << L" " << i.name();
  auto sl = monster::slotsOf(i);
  // "mace: wielded (left hand)" or "wooden ring: right ring finger"
  if (sl[0] != nullptr)
    inv << L": " << sl[0]->name(monster::type().category());
  if (sl[1] != sl[0])
    inv << L": " << sl[1]->name(monster::type().category());
  inv << L"\n";
  auto container = dynamic_cast<const itemHolder*>(&i);
  if (container) {
    if (container->empty()) {
      inv << indent << L" ∙ your " << i.name() << " is unoccupied" << L"\n";
    } else {
      inv << indent << L" ∙ containing:" << L"\n";
      container->forEachItem([this, &inv, indent](const item &c, std::wstring) {
	  containerInventory(inv, indent + L"  ", c);
	});
    }
  }
}

void player::takeInventory() {
  if (empty()) {
    ioFactory::instance().message(L"You have no weapons.\n"
		 "You are naked.\n"
		 "You have no possessions on your person.\n\n"
		 "Hint: You may want to get some things");
  } else {
    std::wstringstream inv;
    forEachItem([this, &inv](const item &i, std::wstring) {
	containerInventory(inv, L"", i);
      });
    double weight = totalWeight();
    inv.precision(2);
    inv << std::scientific
	<< L"Total weight: " << weight << L" Newtons. ";
    //    int wt = weight / 3000;
    int wt = weight / abilities()->carryWeightN();
    switch (wt) {
    case 0: break;
    case 1: inv << L"\n\tThis stuff is heavy!"; break;
    default: inv << L"\n\tThis stuff is ";
      for (int i=2; i < wt; ++i) inv << L"really ";
      inv << "slowing you down!";
    }
    ioFactory::instance().longMsg(inv.str());
  }
}

void player::equip() {
  auto &ios = ioFactory::instance();

  std::vector<std::pair<const item *, std::wstring>> choices;

  item *sel =
    pickItem(L"What would you like to equip?", // prompt
	     L"Pick an item to wield or wear.", // help
	     L"",
	     [this](const item &i) {
	       if (slotsOf(i)[0] != nullptr) return false; // already equipped
	       auto type = i.equippable();
	       if (type == item::equipType::none) return false; // unequippable
	       return true;
	     }, true);

  
  if (sel) {
    auto rtn = sel->equip(*this);
    if (rtn) return;
    else if (sel->equippable() == item::equipType::wielded)
      ios.message(L"This " + sel->name() + L" won't be your weapon");
    else ios.message(L"This " + sel->name() + L" doesn't fit anywhere");
  } else
    ios.message(L"Nothing to equip");

}

/*
 * Drop an item.
 ***
 * Originally, this prompted the player to drop each item in turn,
 * giving one chance to drop each item.
 * This meant hitting "N" a lot, and missing the item you wanted.
 * So now we do a custom choice prompt.
 */
void player::drop(level &lvl) {
  auto pos = lvl.pcPos();
  std::vector<item *> worn;
  std::vector<item *> wielded;
  std::vector<item *> other;
  std::map<wchar_t, std::vector<item *>> byChar;
  forEachItem([this, &worn, &wielded, &other, &byChar](item &i,
						       std::wstring name) {
    auto s = slotsOf(i);
    if (s[0] == nullptr && s[1] == nullptr) other.emplace_back(&i);
    else if (i.equippable() == item::equipType::wielded)
	wielded.emplace_back(&i);
    else worn.emplace_back(&i);
    byChar[i.render()].emplace_back(&i);
  });

  auto &ios = ioFactory::instance();
  while (byChar.size() > 0) {

    std::vector<std::pair<wchar_t, std::wstring>> choices;
    
    if (!wielded.empty())
      choices.emplace_back(L'W', L"Weapon");
    if (!worn.empty())
      choices.emplace_back(L'A', L"Apparel/Armour");
    if (!other.empty())
      choices.emplace_back(L'U', L"Unequipped");
    
    for (auto c : byChar)
      if (!c.second.empty())
	choices.emplace_back(c.first, std::wstring({c.first}));

    if (choices.empty()) return; // nothing left to drop
    
    choices.emplace_back(L'D', L"Done");

    wchar_t c = ios.choice<wchar_t>(L"Leave",L"Select what you want to drop", choices);
    if (c == L'd' || c == L'D') return;

    std::vector<item*> itemSet;
    
    switch (c) {
    case L'W':
      std::copy(wielded.begin(), wielded.end(), back_inserter(itemSet));
      break;
    case L'A':
      std::copy(worn.begin(), worn.end(), back_inserter(itemSet));
      break;
    case L'U':
      std::copy(other.begin(), other.end(), back_inserter(itemSet));
      break;
    default:
      std::copy(byChar[c].begin(), byChar[c].end(), back_inserter(itemSet));
    }

    std::function<std::wstring(const item &i)> namer = [](const item &i) {
      return i.name();
    };

    item *sel = ios.choice(L"Leave", L"Emplace in this location:",
			   itemSet, true, namer);
    if (sel != nullptr) {

      worn.erase(std::remove(worn.begin(), worn.end(), sel), worn.end());
      wielded.erase(std::remove(wielded.begin(), wielded.end(), sel), wielded.end());
      other.erase(std::remove(other.begin(), other.end(), sel), other.end());
      for (auto &i : byChar)
	i.second.erase(std::remove(i.second.begin(), i.second.end(), sel), i.second.end());

      if (!monster::drop(*sel, pos))
	ios.message(L"You cannot drop the " + sel->name());
    }
  }
}

void player::use() {
  std::set<const item *> prepicked;
  std::function<bool(const item&)> f = [this, &prepicked](const item &i){
    return 
    (&(i.holder()) == this) // in case item has moved out of this container by a previously used item
    && prepicked.find(&i) == prepicked.end(); // can't use an item twice in one move
  };
  auto &ios = ioFactory::instance();
  while (true) {
    item *i = pickItem(L"What to use?", L"", L"", f);
    if (!i) break;
    prepicked.insert(i);
    auto result = i->use();
    if (result == item::useResult::FAIL) {
      ios.message(L"That doesn't seem to work.");
      break;
    }
    if (result == item::useResult::DONE) break;
  }
}

void player::death(bool allowCorpse) {
  curLevel().dung().playerDeath();
  monster::death(allowCorpse);
}

playerBuilder::playerBuilder()
  : monsterBuilder(false),
    role_(NULL) {}

playerBuilder::~playerBuilder() {}

void playerBuilder::name(const std::wstring &c) { name_ = c; }

void playerBuilder::job(role &r) { role_  = &r; }
const role & playerBuilder::job() const { return *role_; }
role & playerBuilder::job() { return *role_; }

