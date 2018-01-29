/* License and copyright go here*/

// define the player in the dungeon

#include "player.hpp"
#include "level.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "items.hpp"
#include "religion.hpp"
#include "role.hpp"

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
    intrinsics().speedy(intrinsics().speedy() + bonus(false));
    {auto dam = damageRepo::instance()[damageType::hot];
    intrinsics().resist(&dam, -1);
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
    intrinsics().eatVeggie(true);
    {auto dam = damageRepo::instance()[damageType::edged];
      intrinsics().resist(&dam, -2);
    }
    break;
  case Element::air:
    intrinsics().speedy(true);
    intrinsics().hear(true);
    {auto dam = damageRepo::instance()[damageType::sonic];
    intrinsics().extraDamage(&dam, +4);
    }
    nullResist--;
    break;
  case Element::fire:
    intrinsics().dblAttack(true);
    nullResist-=2;
    break;
  case Element::water:
    intrinsics().swim(true);
    intrinsics().speedy(false);
    break;
  case Element::plant:
    intrinsics().climb(true);
    intrinsics().eatVeggie(true);
    {auto dam = damageRepo::instance()[damageType::bashing];
    intrinsics().resist(&dam, -2);
    }
    break;
  case Element::time:
    intrinsics().speedy(true);
    intrinsics().dblAttack(true);    
    {auto dam = damageRepo::instance()[damageType::disintegration];
    intrinsics().resist(&dam, -4);
    }
    break;
  default:
    break; // nothing to do
  }
  intrinsics().resist(nullptr, nullResist); // can't add directly

  // starting inventory:
  // I feel a player should start with a deterministic inventory, perhaps based
  // on their class and race.
  auto begin = job().questsBegin();
  auto end = job().questsEnd();
  for (auto pQ = begin; pQ != end; ++pQ)
    pQ->setupPlayer(*this);
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
    int wt = weight / 3000;
    switch (wt) {
    case 0: break;
    case 1: inv << L"\n\tThis stuff is heavy!"; break;
    default: inv << L"\n\tThis stuff is ";
      for (int i=2; i < wt; ++i) inv << L"really ";
      inv << "slowing you down!";
    }
    ioFactory::instance().longMsg(inv.str()); // TODO: description hints
  }
}

void player::equip() {
  auto &ios = ioFactory::instance();
  auto result = firstItem([this, &ios](item &i) {
      if (slotsOf(i)[0] != nullptr) return false; // already equipped
      auto type = i.equippable();
      if (type == item::equipType::none) return false; // unequippable
      std::wstring msg = L"Do you ";
      bool weap = type == item::equipType::wielded;
      if (weap) msg += L"wield ";
      else msg += L"wear ";
      msg += i.name();
      msg += L"?";
      if (ios.ynPrompt(msg)) {
	auto rtn = i.equip(*this);
	if (rtn) return true;
	else if (weap)
	  ios.message(L"This " + i.name() + L" won't be your weapon");
	else ios.message(L"This " + i.name() + L" doesn't fit anywhere");
      }
      return false;
    });
  if (!result)
    ios.message(L"Nothing to equip");
}

void player::drop(level &lvl) {
  auto c = lvl.pcPos();
  std::function<void(item&, std::wstring)> f = 
    [this, &lvl, &c](item& i, std::wstring name){
    auto &ios = ioFactory::instance();
    if (ios.ynPrompt(L"Drop " + name + L"?"))
      if (!monster::drop(i, c))
	ios.message(L"You cannot drop the " + i.name());
  };
  forEachItem(f);
}

void player::use() {
  std::function<void(item&, std::wstring)> f = [this](item &i, std::wstring name){
    if (&(i.holder()) != this) return; // item has moved out of this container by a previously used item
    auto &ios = ioFactory::instance();
    if (ios.ynPrompt(L"Use " + name + L"?"))
      if (!i.use())
	ios.message(L"That doesn't seem to work.");
  };
  forEachItem(f);
}

void player::death() {
  curLevel().dung().playerDeath();
  monster::death();
}

playerBuilder::playerBuilder()
  : monsterBuilder(false),
    role_(NULL) {}

playerBuilder::~playerBuilder() {}

void playerBuilder::name(const std::wstring &c) { name_ = c; }

void playerBuilder::job(role &r) { role_  = &r; }
const role & playerBuilder::job() const { return *role_; }
role & playerBuilder::job() { return *role_; }

