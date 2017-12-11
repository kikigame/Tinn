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
  tools,
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
  case tools: return L"Toolshop"; // L'('
  default: throw type;
  }
};


enum class serviceType {
  enchantment,
    proofing,
    fixing,
};

// declared in items.cpp
double forIou(const item &i, double d, std::wstring &buf);

class shopImpl {
private:
  monster &inventory_;
  const io & io_;
  shopType shopType_;
  std::wstring keeperName_;
  std::wstring name_;
  std::vector<std::shared_ptr<item>> forSale_;
  std::vector<serviceType> services_;
  std::vector<std::shared_ptr<item>> basket_;
  const damage & damage_; // for proofing primarily
  deity & align_;
  bool isFriendly_;
  std::map<std::wstring, double> servicesBought_;
public:
  shopImpl(monster &inventory,
	   const io & ios, shopType type,
	   const std::wstring & keeperName, 
	   const std::wstring & adjective) : 
    inventory_(inventory),
    io_(ios),
    shopType_(type),
    keeperName_(keeperName),
    name_(keeperName + L"'s " + adjective + L" " + shopName(type)),
    damage_(rndPick(damageRepo::instance().begin(), damageRepo::instance().end())->second),
    align_(*rndPick(deityRepo::instance().begin(), deityRepo::instance().end())),
    isFriendly_(adjective == L"Friendly") {
    // What does the shop have to sell?
    restock();
    // TODO: When does the shop get restocked? Or do we get new shops each time?
    // "enchanting" shops sell enchantments:
    if (adjective == L"Enchanting")
      services_.push_back(serviceType::enchantment);
    // "protective" shops sell a proofing service:
    if (adjective == L"Protective")
      services_.push_back(serviceType::proofing);
  }

  const wchar_t * const name() const {
    return name_.c_str();
  }

  const wchar_t * const description() const {
    return
      L"Shopping has been known to relax the mind and enhance wellbeing,\n"
      "although you must beware of the Gruen Transfer. The modern shop -\n"
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
    if (handleDebts())
      handleSale();
  }
private:
  wchar_t itemCat() {
      switch (shopType_) {
      case stylii: return L'/';
      case groceries: return L'%';
      case weapons: return L'!';
      case thrown: return L'¬';
      case clothes: return L'[';
      case readable: return L'¶';
      case jewellery: return L'*';
      case gambling: return L'$'; // -- exchanging valuables for other valuables by bartering = casino
      case luggage: return L'=';
      case bottles: return L'8';
      case tools: return L'(';
      default: throw shopType_;
      }
  }
  void restock() {
    unsigned int numItems = 4 + (dPc() / 20); // 4 to 9
    auto cat = itemCat();
    itemHolder h;
    while (forSale_.size() < numItems) {
      auto &i = createRndItem(100, io_);
      if (i.render() != cat) {
	h.addItem(i);
	h.destroyItem(i);
      } else {
	forSale_.push_back(i.shared_from_this()); // should shop be an item holder?
      }
    }
  }
  bool handleDebts() {
    double debt = 0;
    std::wstring services;
    inventory_.forEachItem([this, &debt, &services](const item &i, std::wstring){
	debt = forIou(i, debt, services);
      });
    if (debt > 0) {
      io_.longMsg(L"You must first handle your existing debt for:\n\n" + services);
      bool rtn = handlePayment(debt);
      if (rtn)
	inventory_.forEachItem([this, &debt, &services](item &i, std::wstring){
	    if (i.render() == L'✎') // detect IOUs; probably should have exposed the enum key
	      inventory_.destroyItem(i);
	  });
      return rtn;
    }
    else
      return true;
  }
  void handleSale() {
    if (forSale_.empty() && services_.empty()) {
      // we'll eventually get a new shop:
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
    for (auto s : services_) {
      if (i++ == idx) switch (s) {
	case serviceType::enchantment:
	  if (!inventory_.empty()) {
	    enchant();
	    handlePayment();
	  } else io_.message(L"You have nothing to enchant!");
	  return;
	case serviceType::proofing:
	  if (!inventory_.empty()) {
	    proof();
	    handlePayment();
	  } else io_.message(L"You have nothing to protect!");
	  return;
	case serviceType::fixing:
	  if (!inventory_.empty()) {
	    mend();
	    handlePayment();
	  } else io_.message(L"You have nothing to repair!");
	  return;
	default:
	  throw s;
	}
    }
    if (i == 0) ++i;
    basket_.push_back(forSale_[idx-i]);
    forSale_.erase(forSale_.begin() + (idx-i));
    if (forSale_.empty()) {
      io_.message(L"You have " + std::to_wstring(basket_.size()) + 
		  std::wstring(L" items in your basket."));
      handlePayment();
    } else if (!io_.ynPrompt(L"You have " + std::to_wstring(basket_.size()) + 
		      std::wstring(L" items in your basket. Check out?")))
      enter();
    else
      handlePayment();
  }

  void handlePayment() {
    double p;
    for (auto &s : servicesBought_)
      p += s.second;
    for (auto &i : basket_)
      p += appraise(inventory_, *i);
    handlePayment(p);
  }

  /*
   * P) appraised total value of the items for sale
   */
  bool handlePayment(double p) {
    // 2) muliply P by a factor 1-2 based on the player's outward appearance, Q
    double q = p * (200 - inventory_.appearance().pc()) / 100;
    // 3) work out items to ask for from the player, whose price is above Q
    std::vector<std::shared_ptr<item> > barter;
    barter = suggestForSale(q, barter);
    if (barter.empty()) {
      io_.message(L"You don't have enough valuables to interest " + keeperName_);
      paymentFailed();
      return false;
    }
    bool bartering = false;
    while (true) {
      // 4) suggest price to player, or ask to barter
      if (!bartering && suggestForSale(barter)) {
	// 5.a) if player accepts price, adjust inventory and done
	completeSale(barter);
	return true;
      } else {
	// 5.b) if player barters, start an add/remove list
	barter = doBarter(barter);
      }
      // 6) if players items' price is above P, adjust inventory and done
      double b;
      for (auto &i : barter)
	b += appraise(inventory_, *i);
      if (b > p) {
	completeSale(barter);
	return true;
      } else {
	io_.message(L"\"Thank ye for your kind offer, but I think you've undervaluing my wares.\"");
	paymentFailed();
	bartering = true;
      }
    }
  }

  std::vector<std::shared_ptr<item> > suggestForSale(double q, std::vector<std::shared_ptr<item> > barter) {
    double t; // total appraised value
    do {
      // TODO: rework this to try valuables first.
      // find the most valuable item in inventory not already in barter, and add to selection
      std::shared_ptr<item> found; double itemPrice=0;
      inventory_.forEachItem([this, &found, &itemPrice, &barter](item &i, std::wstring) {
	if (inventory_.slotOf(i) != nullptr) return; // skip clothing
	if (i.isCursed()) return; // skip cursed items, including ious!
	auto pi = i.shared_from_this();
	auto end = barter.end();
	if (std::find(barter.begin(), end, pi) == end) { // not already for barter
	  double price = appraise(inventory_,i);
	  if (itemPrice < price) {
	    found = pi;
	    itemPrice = price;
	  }
	}
	});
      if (found == nullptr) { barter.clear(); return barter;} // not enough
      barter.push_back(found);
      t=0;
      for (auto &i : barter)
	t += appraise(inventory_, *i);
    } while (t < q);
    return barter;
  }

  std::vector<std::shared_ptr<item> > doBarter(std::vector<std::shared_ptr<item> > barter) {
    int choice;
    do {
      auto offer = L"Your offer:\n" + toList(barter);
      std::vector<std::pair<int, const wchar_t*> > choices = {
	{1, L"Clear list and start again"},
	{2, L"Remove an item from your offer"},
	{3, L"Offer more things"},
	{4, L"Attempt the barter"}
      };
      choice = io_.choice(L"Please select: ", offer, choices);
      switch (choice) {
      case 1:
	barter.clear();
	break;
      case 2:  // remove item
	if (!barter.empty()) {
	std::vector<std::pair<std::shared_ptr<item>, const wchar_t*> > c;
	for (auto i : barter)
	  c.emplace_back(i, i->name());
	auto toRemove = io_.choice(L"Shelve offer for:", L"", c);
	barter.erase(std::find(barter.begin(), barter.end(), toRemove));
	break;
      }
      case 3: // add item
	if (inventory_.size() > barter.size()) {
	  auto &toAdd = pickItem(L"What do you want to offer:", L"Choose the item to add to your offer",
/*Njorthrbiartr is longest keeper name */
keeperName_ + L" will appraise the value of the items you offer, and decide if\n"
"it is a fair trade. If it isn't, you can modify your offer or decline.",
				 [&barter](const item & i) {
				   return std::find(barter.begin(), barter.end(), i.shared_from_this()) == barter.end();
				 }
			   );
	  barter.push_back(toAdd.shared_from_this());
	} else {
	  io_.message(L"Thou hast nothing to offer!");
	}
	break;
      }
    } while (choice != 4);
    return barter;
  }

  void completeSale(std::vector<std::shared_ptr<item> > &barter) {
    io_.message(L"Your offer is accetable.\nThank you; come again."); // ref:Simpsons, Apu
    for (auto i : barter) inventory_.destroyItem(*i);
    for (auto i : basket_) inventory_.addItem(*i);
  }

  void paymentFailed() {
    // add any IOUs:
    for (auto s : servicesBought_)
      inventory_.addItem(createIou(s.second, keeperName_, s.first, io_));
  }

  std::wstring toList(std::vector<std::shared_ptr<item> > &barter) const {
    if (barter.empty()) return L"\tNothing.\n";
    std::wstring prompt;
    for (auto i = barter.begin(); i != barter.end(); ) {
      prompt += std::wstring(L"\t") + (*i)->name();
      i++;
      if (i == barter.end())
	prompt += L".\n";
      else if ((i + 1) == barter.end())
	prompt += L", and\n";
    }
    return prompt;
  }

  bool suggestForSale(std::vector<std::shared_ptr<item> > &barter) const {
    std::wstring prompt = keeperName_ + L" asks for:\n" + toList(barter)
      + L"Is this an acceptable trade? ";
    return io_.ynPrompt(prompt);
  }

  // pick an item to be used below.
  item& pickItem(const std::wstring & prompt,
		 const std::wstring & help,
		 const std::wstring & extraHelp,
		 const std::function<bool(const item &)> f = [](const item &){return true;}) const {
    // TODO: filter out foo-proof and/or foo-undamaged items as appropriate
    std::vector<std::pair<int, const wchar_t*>> choices;
    std::vector<item *> res;
    int i=0;
    inventory_.forEachItem([&choices, &i, &res, f](item &it, std::wstring name) {
	if (f(it)) {
	  choices.emplace_back(i++, it.name()); // TODO: nice to use formatted name here?
	  res.emplace_back(&it);
	}
      });
    int it = io_.choice(prompt, help, choices, extraHelp);
    return **(res.begin() + it);
  }

  void enchant() {
    auto &item = pickItem(L"What would you like to enchant?",
			 L"Choose the item whose enchantment you want to enhance",
			 L"Enchantment adds charges to wands, resistance to armour and\n"
			"damage to weapons. You may choose one item.");
    bool blessed = item.isBlessed();
    bool cursed = item.isCursed();
    
    int de = 0;
    if (blessed && cursed) de = 2; // cursed items get half enchantment (ie 50:50 for nbc)
    else if (blessed) de = 4; // blessed items get *4 enchantment
    else if (!cursed || dPc() < 50) de = 1;
    item.enchant(de);

    if (de > 0)
      io_.message(std::wstring(L"Magical enenergy flows into your ") + item.name());
    else // let's see if the user's paying attention...
      io_.message(std::wstring(L"Magical enenergy flows through your ") + item.name());

    servicesBought_.emplace(std::wstring(L"Enchantment to ") + item.name(),
			    appraise(inventory_, item));
  }

  void proof() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto &item = pickItem(prompt, std::wstring(L"Choose the item on which to bestow resistance"),
			 L"This will completely protect your item against " + 
			 std::wstring(damage_.name()) +
			 L" attacks, traps and effects. When you wear an item with this protection,"
			 "it will also protect any items worn under it.");
    
    if (item.proof(damage_.type()))
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" seems just as complete as it can be.");
    else
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" could use a little more work");

    servicesBought_.emplace(std::wstring(damage_.mendName()) + L" protection unto " + item.name(),
			    appraise(inventory_, item));
  }

  void mend() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto &item = pickItem(prompt, L"Choose the item on which to repair",
			 L"This will repair your item against " + std::wstring(damage_.name()) +
			 L"damage already taken.");
    
    if (item.repair(damage_.type()))
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" improves.");
    else
      io_.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" seems unchanged");

    servicesBought_.emplace(std::wstring(damage_.mendName()) + L" reparation unto " + item.name(),
			    appraise(inventory_, item));
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

shop::shop(const io & ios, monster & inventory) {
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
  shoppingCentre(const io & ios, monster &inventory) :
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

void goShopping(const io & ios, monster &inventory) {
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
