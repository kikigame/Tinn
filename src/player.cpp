/* License and copyright go here*/

// define the player in the dungeon

#include "player.hpp"
#include "level.hpp"
#include "dungeon.hpp"
#include "output.hpp"
#include "items.hpp"
#include "religion.hpp"

#include <sstream>
#include <set>

player::player(playerBuilder &b) :
  monster(b), // we start at level 1, but not yet...
  name_(b.name_),
  foodLevel_(characteristic::MAX_MAX / 2),
  io_(b.io_) {
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
  // But for now, we'll just let them go shopping:
  addItem(createItem(itemTypeKey::shop_card, *io_, *this));
}

player::~player() {}

const wchar_t player::render() const {
  return L'@';
}

const wchar_t* const player::name() const {
  return name_.c_str();
}

const wchar_t* const player::description() const {
  return L"A bold and dangerous hero, quosting for adventure";
}

void player::takeInventory() {
  auto it = contents();
  if (it.begin() == it.end()) {
    io_->message(L"You have no weapons.\n"
		 "You are naked.\n"
		 "You have no possessions on your person.\n\n"
		 "Hint: You may want to get some things");
  } else {
    std::wstringstream inv;
    for (auto i : it) {
      inv << i->render() << L" " << i->name();
      auto sl = monster::slotOf(i);
      // "mace: wielded (left hand)" or "wooden ring: right ring finger"
      if (sl != nullptr)
	inv << L": " << sl->name(monster::type().category());
      inv << L"\n";
    }
    io_->longMsg(inv.str()); // TODO: description hints; what is equipped
  }
}

void player::equip() {
  auto ws = weaponSlots();
  auto it = contents();
  for (auto i : it) {
    if (slotOf(i) != nullptr) continue; // don't multi-wield/equip things yet
    std::set<slotType> iSlots = i->slots();
    for (auto s : iSlots) {
      auto sl = slotBy(s);
      if (slotAvail(sl)) {
	std::wstring msg = L"Do you ";
	bool weap = ws.count(sl) != 0;
	if (weap) msg += L"wield ";
	else msg += L"wear ";
	msg += i->name();
	msg += L"?";
	if (io_->ynPrompt(msg)) {
	  if (monster::equip(i, s))
	    return;
	  else if (weap)
	    io_->message(std::wstring(L"This ") + i->name() + L" won't be your " + sl->name(monster::type().category()) + L" weapon");
	  else io_->message(std::wstring(L"This ") + i->name() + L" won't fit your " + sl->name(monster::type().category()));
	}
      }
    }
  }
  io_->message(L"Nothing to equip");
}

void player::drop(level &lvl) {
  auto c = lvl.pcPos();
  std::function<void(std::shared_ptr<item>, std::wstring)> f = 
    [this, &lvl, &c](std::shared_ptr<item> i, std::wstring name){
    if (io_->ynPrompt(std::wstring(L"Drop ") + name + L"?"))
      if (!monster::drop(i, c))
	io_->message(std::wstring(L"You cannot drop the ") + i->name());
  };
  forEachItem(f);
}

void player::use() {
  std::function<void(std::shared_ptr<item>, std::wstring)> f = 
    [this](std::shared_ptr<item> i, std::wstring name){
    if (io_->ynPrompt(std::wstring(L"Use ") + name + L"?"))
      if (!i->use())
	io_->message(L"That doesn't seem to work.");
  };
  forEachItem(f);
}

const wchar_t* const player::fall(unsigned char reductionPc) {
  auto rtn = monster::fall(reductionPc);
  io_->message(rtn);
  return rtn;
};

void player::forEachItem(const std::function<void(std::shared_ptr<item>, std::wstring)> f) {
  iterable<std::shared_ptr<item>, std::vector<std::shared_ptr<item>>, 
	   true> it(contents()); // true -> take a copy (in case items are removed)
  for (auto i : it) {
    std::wstring msg = i->name();
    auto slot = slotOf(i);
    if (slotOf(i) != nullptr) {
      msg += L": ";
      msg += slot->name(monster::type().category());
      msg += L" ";
    }
    f(i, msg);
  }
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

void playerBuilder::job(const role &r) { role_  = &r; }
const role & playerBuilder::job() { return *role_; }

void playerBuilder::ios(const io * ios) { io_ = ios; }
