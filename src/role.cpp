/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#include "role.hpp"
#include "quest.hpp"
#include "player.hpp"
#include "items.hpp"
#include <map>

class roleImpl {
private:
  const roleType type_;
  const wchar_t * const name_;
  const wchar_t * const startGameMessage_;
  std::vector<quest> quests_;
  const std::function<void(player &)> ps_;
public:
  roleImpl(const roleType &type, 
	   const wchar_t* const name,
	   const wchar_t* const startGameMessage,
	   const deity &align,
	   // player equipment generator:
	   const std::function<void(player &)> ps) :
    type_(type), name_(name),
    startGameMessage_(startGameMessage),
    quests_(questsForRole(align, type)),
    ps_(ps) {}
  roleImpl(const roleImpl &) = delete;
  roleImpl(roleImpl &&) = delete;
  const roleType type() const {
    return type_;
  }
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const startGameMessage() const {
    return startGameMessage_;
  }
  std::vector<quest>::iterator questsBegin() { return quests_.begin(); }
  std::vector<quest>::const_iterator questsBegin() const { return quests_.begin();  }
  std::vector<quest>::iterator questsEnd() { return quests_.end();  }
  std::vector<quest>::const_iterator questsEnd() const { return quests_.end();  }
  void setupPlayer(player &p) { ps_(p); }
};


role::role(roleImpl *pImpl)
  : pImpl_(pImpl) {};

const wchar_t * const role::name() const {
  return pImpl_->name();
}
const wchar_t * const role::startGameMessage() const {
  return pImpl_->startGameMessage();
}
roleType role::type() const {
  return pImpl_->type();
}
std::vector<quest>::iterator role::questsBegin() { return pImpl_->questsBegin(); }
std::vector<quest>::const_iterator role::questsBegin() const { return pImpl_->questsBegin();  }
std::vector<quest>::iterator role::questsEnd() { return pImpl_->questsEnd();  }
std::vector<quest>::const_iterator role::questsEnd() const { return pImpl_->questsEnd();  }
void role::setupPlayer(player &p) { return pImpl_->setupPlayer(p); }

class roleRepoImpl {
private:
  std::map<roleType,role> repo_;
public:
  roleRepoImpl(const deity &pcAlign) {
    emplace(new roleImpl(roleType::warrior, L"warrior",
			 L"Contender Ready!", // ref: Gladiators (TV gameshow)
			 pcAlign,
			 [](player &p) {
	 for (auto k : std::array<itemTypeKey, 5>{{
	       itemTypeKey::mace, itemTypeKey::jerkin, itemTypeKey::underpants,
		 itemTypeKey::boots, itemTypeKey::shirt}}) {
	   auto &it = createItem(k);
	   p.addItem(it);
	   it.equip(p);
	 }}
	 ));
    emplace(new roleImpl(roleType::thief, L"thief",
			 L"Is there anybody hiding there in the dark?", // ref: Pink Panther
			 pcAlign,
       [](player &p) {
	 // thief should be less tough. but faster
	 p.intrinsics()->speedy(true);
	 p.dodge().bonus(5);
	 p.strength().cripple(10);
	 p.fighting().cripple(10);
	 // equip the player
	 for (auto k : std::array<itemTypeKey, 8>{{
	       itemTypeKey::sling, itemTypeKey::gloves, itemTypeKey::underpants,
		 itemTypeKey::boots, itemTypeKey::shirt, itemTypeKey::shorts,
		 itemTypeKey::socks, itemTypeKey::wooden_ring}}) {
	   auto &it = createItem(k);
	   p.addItem(it);
	   it.equip(p);
	 }
	 p.addItem(createItem(itemTypeKey::pie));
	 p.addItem(createItem(itemTypeKey::fizzy_pop));
	 for (int i=0; i < 10; ++i)
	   p.addItem(createItem(itemTypeKey::rock));
	 p.addItem(createItem(itemTypeKey::bottling_kit));
	 p.addItem(createItem(itemTypeKey::lyre));
	 auto &wand = createItem(itemTypeKey::stick);
	 p.addItem(wand);
	 wand.enchant(5);
       }
       ));
    emplace(new roleImpl(roleType::crusader, L"crusader",
			 L"Once the mission's over, you could come back here, have yourself a home.", // ref: Crusade (B5 spinoff)
			 pcAlign,
       [&pcAlign](player &p) {
	 for (auto k : std::array<itemTypeKey, 7>{{
	       itemTypeKey::helmet,
	       itemTypeKey::jerkin, itemTypeKey::underpants,
		 itemTypeKey::boots, itemTypeKey::shirt,
		 itemTypeKey::hauberk, itemTypeKey::doublet}}) {
	   auto &it = createItem(k);
	   p.addItem(it);
	   it.equip(p);
	 }
	 auto &it = createItem(itemTypeKey::two_sword);
	 it.bless(true);
	 p.addItem(it);
	 it.equip(p);
	 
	 auto &wand = createWand(sharedAction<monster,monster>::key::conversion);
	 wand.bless(true);
	 wand.enchant(10);
	 p.addItem(wand);

	 auto &book = createHolyBook(pcAlign);
	 book.bless(true);
	 p.addItem(book);
			 }
			 ));
    emplace(new roleImpl(roleType::shopkeeper, L"shopkeeper",
			 L"As if by magic, the shopkeeper appeared.", // Ref: Mr Benn
			 pcAlign,
       [](player &p) {
	 p.addItem(createItem(itemTypeKey::apple));
	 p.addItem(createItem(itemTypeKey::shop_card));
	 p.addItem(createItem(itemTypeKey::hitch_guide));
	 auto &bow = createItem(itemTypeKey::bow);
	 bow.sexUp(true);
	 p.addItem(bow);
	 bow.equip(p);
			 }
			 ));
  }
  void emplace(roleImpl *r) {
    repo_.emplace(r->type(), r);
  }
  role & get(const roleType &r) {
    return repo_.at(r);
  }
};

std::unique_ptr<roleRepo> roleRepo::instance_;


roleRepo::roleRepo(const deity &align)
  : pImpl_(new roleRepoImpl(align)) {};

  
role & roleRepo::operator[](const roleType &r) {
  return pImpl_->get(r);
}

roleRepo& roleRepo::instance(const deity &align) {
  if (!instance_)
    instance_.reset(new roleRepo(align)); // was empty
  return *instance_;
}

void roleRepo::close() {
  instance_.reset(NULL);
}

std::wstring role::questText() const {
  std::wstring rtn(L"Quests:-\n======\n\n");
  auto qe = questsEnd();
  for (auto q = questsBegin(); q != qe; ++q) {
    std::wstring name(q->name());
    rtn += name + L"\n";
    rtn += std::wstring(name.length(), L'-') + L"\n";
    rtn += L"Status: ";
    rtn += (q->isSuccessful() ? L"complete\n\n" : L"active\n\n");
    rtn += q->questData();
    rtn += L"\n\n";
  }
  return rtn;
}
