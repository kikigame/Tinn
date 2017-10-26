/* License and copyright go here*/

// define a shop in the dungeon

#include "shop.hpp"
#include "random.hpp"
#include "output.hpp"
#include "damage.hpp"
#include "religion.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "appraise.hpp"

// ${keeper}'s ${adjective} ${shopType}
extern const wchar_t * const keeperNames[]; // defined in shopKeepers.cpp
extern const int numKeeperNames;

extern const wchar_t * const shopAdjectives[]; // defined in adjectives.cpp
extern const int numShopAdjectives;

// NB: most common are in the middle due to bell curve:
// NB: Last shop is very rare (role of 99 or 100 given 11 shops)
enum shopType {
  stylii,
  groceries,
  weapons,
  thrown,
  clothes,
  readable,
  jewellery,
  gambling,
  luggage,
  bottles,
  // TODO: tools
  END
};

const wchar_t * shopName(const shopType & type) {
  switch (type) {
  case stylii: return  L"Stylii Shop"; // L'/'
  case groceries: return  L"shop of Groceries"; // L'%'
  case weapons: return  L"Weaponsmiths' Forge"; // L'!'
  case thrown: return  L"shop of Flight"; // L'¬'
  case clothes: return  L"Costumier and Armourer"; // L'['
  case readable: return  L"shop of Writing"; // L'¶'
  case jewellery: return  L"jeweller's"; // L'*'
  case gambling: return  L"house of Gambling"; // L'$' -- exchanging valuables for other valuables by bartering = casino
  case luggage: return  L"Luggage Emporium"; // L'='
  case bottles: return  L"shop of Potables and Curios"; // L'8'
  // TODO: tools => Equipment
  default: throw type;
  }
};


enum class serviceType {
  enchantment,
    proofing,
    fixing,
};

class shopImpl {
private:
  itemHolder &inventory_;
  const io & io_;
  shopType shopType_;
  std::wstring name_;
  std::vector<std::shared_ptr<item>> forSale_;
  std::vector<serviceType> services_;
  std::vector<std::shared_ptr<item>> basket_;
  const damage & damage_; // for proofing primarily
  deity & align_;
  bool isFriendly_;
public:
  shopImpl(itemHolder &inventory,
	   const io & ios, shopType type,
	   const std::wstring & keeperName, 
	   const std::wstring & adjective) : 
    inventory_(inventory),
    io_(ios),
    shopType_(type),
    name_(keeperName + L"'s " + adjective + L" " + shopName(type)),
    damage_(rndPick(damageRepo::instance().begin(), damageRepo::instance().end())->second),
    align_(*rndPick(deityRepo::instance().begin(), deityRepo::instance().end())),
    isFriendly_(adjective == L"Friendly") {
    // TODO: What does the shop have to sell?
    // TODO: When does the shop get restocked? Or do we get new shops each time?
    // "enchanting" shops sell enchantments:
    if (adjective == L"Enchanting")
      services_.push_back(serviceType::enchantment);
    if (adjective == L"Protective")
      services_.push_back(serviceType::proofing);
  }

  const wchar_t * const name() const {
    return name_.c_str();
  }

  const wchar_t * const description() const {
    return
      L"Shopping has been known to relax the mind and enhance wellbeing,\n"
      "although you must beware of the gruen transfer. The modern shop -\n"
      "meaning a barn or shed for work or trade - uses a process called\n"
      "scripted disorientation to psychologically encourage the shopper to \n"
      "pick up more bargains and spend more money than originally planned.\n\n"
      "You can turn the tables by attempting to haggle or barter, or even by\n"
      "offering a different currentcy for your trade.";
  }

  shop::itemIter begin() const {
    return forSale_.begin();
  }

  shop::itemIter end() const {
    return forSale_.end();
  }

  void enter() {
    if (isFriendly_ && io_.ynPrompt(L"\"Welcome! May I interest you in a complimentary tea?\""))
      io_.message(L"This takes almost but not quite entirely unlike tea"); // ref:h2g2, of course.
    // TODO: Would be nice to heal a touch of damage for that. But then shops would need a way to become undamaged.
    handleSale();
  }
private:
  void handleSale() {
    if (forSale_.empty() && services_.empty()) {
      // TODO: restock timer; or new shops each time?
      io_.message(L"This shop is empty. Please try again later.");
      return;
    }
    std::vector<std::pair<int, const wchar_t*>> choices;
    // position 0 is usually the enchantment:
    int i=0;
    auto proofName = std::wstring(L"Resistance against ") + damage_.name();
    for (auto s : services_)
      switch (s) {
      case serviceType::enchantment:
	choices.emplace_back(i++, L"Enchantment");
	break;
      case serviceType::proofing:
	choices.emplace_back(i++, proofName.c_str());
	break;
      case serviceType::fixing:
	choices.emplace_back(i++, damage_.mendName());
	break;
      default:
	throw s;
      };
    if (i == 0) ++i; // start at 1
    for (auto it : forSale_)
      choices.emplace_back(i++, it->name());
    int idx = io_.choice((const wchar_t*)L"What would you like to buy?",
			 (const wchar_t*)L"Choose the item you want to add to your shopping basket",
			 choices,
			 (const wchar_t*)L"per-item help is TODO.");
    i=0;
    //TODO:payment
    for (auto s : services_) {
      if (i++ == idx) switch (s) {
	case serviceType::enchantment:
	  enchant();
	  return;
	case serviceType::proofing:
	  proof();
	  return;
	case serviceType::fixing:
	  mend();
	  return;
	default:
	  throw s;
	}
    }
    if (i == 0) ++i;
    basket_.push_back(forSale_[idx-i]);
    forSale_.erase(forSale_.begin() + (idx-i));
    if (!io_.ynPrompt(std::wstring(L"You have ") + std::to_wstring(basket_.size()) + 
		      std::wstring(L" items in your basket. Check out?")))
      enter();
    io_.message(L"TODO: Payment");
  }

  // pick an item to be used below.
  std::shared_ptr<item> pickItem(const std::wstring & prompt,
				 const std::wstring & help,
				 const std::wstring & extraHelp) const {
    // TODO: filter out foo-proof and/or foo-undamaged items as appropriate
    std::vector<std::pair<int, const wchar_t*>> choices;
    auto con = inventory_.contents();
    int i=0;
    for (auto it : con)
      choices.emplace_back(i++, it->name());
    int it = io_.choice(prompt, help, choices, extraHelp);
    return *(con.begin() + it);
  }

  void enchant() {
    auto item = pickItem(L"What would you like to enchant?",
			 L"Choose the item whose enchantment you want to enhance",
			 L"Enchantment adds charges to wands, resistance to armour and\n"
			"damage to weapons. You may choose one item.");
    bool blessed = item->isBlessed();
    bool cursed = item->isCursed();
    
    int de = 0;
    if (blessed && cursed) de = 2; // cursed items get half enchantment (ie 50:50 for nbc)
    else if (blessed) de = 4; // blessed items get *4 enchantment
    else if (!cursed || dPc() < 50) de = 1;
    item->enchant(de);

    if (de > 0)
      io_.message(std::wstring(L"Magical enenergy flows into your ") + item->name());
    else // let's see if the user's paying attention...
      io_.message(std::wstring(L"Magical enenergy flows through your ") + item->name());      
  }

  void proof() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto item = pickItem(prompt, std::wstring(L"Choose the item on which to bestow resistance"),
			 L"This will completely protect your item against " + 
			 std::wstring(damage_.name()) +
			 L"attacks, traps and effects. When you wear an item with this protection,"
			 "it will also protect any items worn under it.");
    
    if (item->proof(damage_.type()))
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item->name()) +
		 L" seems just as complete as it can be.");
    else
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item->name()) +
		 L" could use a little more work");
  }

  void mend() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto item = pickItem(prompt, L"Choose the item on which to repair",
			 L"This will repair your item against " + std::wstring(damage_.name()) +
			 L"damage already taken.");
    
    if (item->repair(damage_.type()))
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item->name()) +
		 L" improves.");
    else
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item->name()) +
		 L" seems unchanged");
  }

};

const wchar_t shop::render() const {
  return L'£';
};
const wchar_t * const shop::name() const {
  return pImpl_->name();
};
const wchar_t * const shop::description() const {
  return pImpl_->description();
};

shop::shop(const io & ios, itemHolder & inventory) {
  const auto type = static_cast<shopType>(dPc() / shopType::END);
  const std::wstring keeperName = *rndPick(keeperNames, keeperNames + numKeeperNames);
  const std::wstring adjective = *rndPick(shopAdjectives, shopAdjectives + numShopAdjectives);
  //  const std::wstring adjective = *rndPick(shopAdjectives.begin(), shopAdjectives.end());
  pImpl_.reset(new shopImpl(inventory, ios, type, keeperName, adjective));
}

shop::~shop() {}

shop::itemIter shop::begin() const {
  return pImpl_->begin();
}

shop::itemIter shop::end() const {
  return pImpl_->end();
}

void shop::enter() {
  pImpl_->enter();
}


class shoppingCentre {
private:
  shop a;
  shop b;
  shop c;
  shop d;
  shop e;
  shop f;
  shop g;
  shop h;
public:
  shoppingCentre(const io & ios, itemHolder &inventory) :
    a(ios,inventory),
    b(ios,inventory),
    c(ios,inventory),
    d(ios,inventory),
    e(ios,inventory),
    f(ios,inventory),
    g(ios,inventory),
    h(ios,inventory) {}
  shoppingCentre(const shoppingCentre &rhs) = delete;
  ~shoppingCentre() {}
  shop& operator[](const int idx) {
    switch (idx) {
    case 0: return a;
    case 1: return b;
    case 2: return c;
    case 3: return d;
    case 4: return e;
    case 5: return f;
    case 6: return g;
    case 7: return h;
    default: throw std::out_of_range(std::to_string(idx));
    }
  }
};

void goShopping(const io & ios, itemHolder &inventory) {
  // how to choose the number of shops?
  // When you need a magic number, and don't have any other criteria, turn to ref:Pratchett's Discworld.
  shoppingCentre shops(ios, inventory);
  std::vector<std::pair<int,const wchar_t*> > choices;
  for (int c=0; c < 8; ++c)
    choices.emplace_back(c, shops[c].name());
  do {
    auto shop = 
      ios.choice(L"There are 8 shops open presently. Would you like to visit:",
		 L"Welcome to the Area of Shopping Adventures.", // ref: the start screen
		 choices,
		 L"The choice of shop determines the goods you can buy and\n"
		 "sell. Some shops also provide other services. Shops are a\n"
		 "great way to convert useless items into useful ones."
		 );
    shops[shop].enter();
  } while (ios.ynPrompt(L"Do you want to continue shopping?"));
}



void shop::buy( itemIter & basket_begin, itemIter & basket_end,
		itemIter & barter_begin, itemIter & barter_end,
		monster & buyer) {
  // how much stuff do you want to buy?
  double value = 0;
  for (auto i = basket_begin; i != basket_end; ++i) {
    value += appraise(buyer, **i);
  }

  double payment = 0;
  for (auto i = barter_begin; i != barter_end; ++i) {
    payment += appraise(buyer, **i);
  }

  // TODO: should there be a random amount?
  if (payment >= value) {
    // TODO: We can buy the things
  } else {
    // TODO: We can't buy the things
  }
}
