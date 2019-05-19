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



const wchar_t * shopName(const shopType & type) {
  switch (type) {
  case stylii: return  L"Stylii Shop"; // L'❘'
  case groceries: return  L"shop of Groceries"; // L'%'
  case weapons: return  L"Weaponsmiths' Forge"; // L'!'
  case thrown: return  L"shop of Flight"; // L'¬'
  case clothes: return  L"Costumier and Armourer"; // L'['
  case readable: return  L"shop of Writing"; // L'¶'
  case jewellery: return  L"Jeweller's"; // L'*'
  case gambling: return  L"House of Gambling"; // L'$' -- exchanging valuables for other valuables by bartering = casino
  case luggage: return  L"Luggage Emporium"; // L'='
  case bottles: return  L"shop of Potables and Curios"; // L'8'
  case tools: return L"Toolshop"; // L'('
  case music: return L"Lutherie and Music Shop"; // L'♪'
  case END: return L"Shop"; // used for pop-up shops
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


class itemDestroyer : public itemHolder {
public:
  virtual bool addItem(item &it) {
    return it.holder().destroyItem(it);
  }
  static itemDestroyer instance;
};
itemDestroyer itemDestroyer::instance;

class shopImpl {
private:
  monster &inventory_;
  shopType shopType_;
  std::wstring keeperName_;
  std::wstring name_;
  std::vector<std::shared_ptr<item>> forSale_;
  std::vector<serviceType> services_;
  std::vector<std::shared_ptr<item>> basket_;
  const damage & damage_; // for proofing primarily
  const deity & align_;
  const bool isFriendly_;
  const bool isGenerous_;
  itemHolder &disposer_;
  std::map<std::wstring, double> servicesBought_;
public:
  // shopping-center shop
  shopImpl(monster &inventory,
	   const shopType type,
	   const std::wstring & keeperName, 
	   const std::wstring & adjective) : 
    inventory_(inventory),
    shopType_(type),
    keeperName_(keeperName),
    name_(keeperName + L"'s " + adjective + L" " + shopName(type)),
    forSale_(),
    services_(),
    basket_(),
    damage_(rndPick(damageRepo::instance().begin(), damageRepo::instance().end())->second),
    align_(*rndPick(deityRepo::instance().begin(), deityRepo::instance().end())),
    isFriendly_(adjective == L"Friendly"),
    isGenerous_(adjective == L"Generous"),
    disposer_(itemDestroyer::instance), // shop gets traded items to sell on via the trading network, but we won't keep the shop.
    servicesBought_() {
    // What does the shop have to sell?
    restock();
    // "enchanting" shops sell enchantments:
    if (adjective == L"Enchanting")
      services_.push_back(serviceType::enchantment);
    // "protective" shops sell a proofing service:
    if (adjective == L"Protective")
      services_.push_back(serviceType::proofing);
  }
  // pop-up shop
  shopImpl(monster &inventory,
	   monster &stock) :
    inventory_(inventory),
    shopType_(shopType::END),
    keeperName_(stock.name()),
    name_(std::wstring(stock.name()) + L"'s Pop-Up Shop"),
    forSale_(),
    services_(),
    basket_(),
    damage_(rndPick(damageRepo::instance().begin(), damageRepo::instance().end())->second),
    align_(stock.align()),
    isFriendly_(false),
    isGenerous_(false),
    disposer_(stock),
    servicesBought_() {
    stock.forEachItem([this](item &it, const std::wstring) {
	forSale_.push_back(it.shared_from_this());
      });
    // TODO: some monsters should provide services?
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
    auto &ios = ioFactory::instance();
    if (isFriendly_ && ios.ynPrompt(L"\"Welcome! May I interest you in a complimentary tea?\""))
      ios.message(L"This takes almost but not quite entirely unlike tea"); // ref:h2g2, of course.
    if (isGenerous_)
      ios.longMsg(L"Special offer! Free delivery on everything in store!"); // ref:every supermarket deal ever. Looks good, but vacuous.
    // TODO: Would be nice to heal a touch of damage for that. But then shops would need a way to become undamaged.
    if (handleDebts())
      handleSale();
  }
private:
  wchar_t itemCat() {
      switch (shopType_) {
      case stylii: return L'❘';
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
      case music: return L'♪';
      default: throw shopType_;
      }
  }
  void restock() {
    unsigned int numItems = 4 + (dPc() / 20); // 4 to 9
    auto cat = itemCat();
    itemHolder h;
    while (forSale_.size() < numItems) {
      auto &i = createRndItem(100);
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
      ioFactory::instance().longMsg(L"You must first handle your existing debt for:\n\n" + services);
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
      ioFactory::instance().message(L"This shop is empty. Please try again later.");
      return;
    }
    std::vector<std::pair<int, std::wstring>> choices;
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
    const std::function<std::wstring(const int &)> extraHelp =
      [this](const int &idx){ // 0-based
      int numServices = services_.size();
      if (idx < numServices)
	switch (services_[idx]) {
	case serviceType::enchantment:
	  return std::wstring(L"Magical enchantments restore charges and improve effectiveness.");
	case serviceType::proofing:
	  return std::wstring(L"Preventing further damage of a given type.");
	case serviceType::fixing:
	  return std::wstring(L"Fixing damage of a given type.");
	default:
	  return std::wstring(L"");
	}
      else
	return forSale_[idx-numServices]->description();
	
    };
    int idx = ioFactory::instance().choice(L"What would you like to buy?",
			 L"Choose the item you want to add to your shopping basket",
			 choices, extraHelp);
    i=0;
    auto &ios = ioFactory::instance();
    for (auto s : services_) {
      if (i++ == idx) switch (s) {
	case serviceType::enchantment:
	  if (!inventory_.empty()) {
	    enchant();
	    handlePayment();
	  } else ios.message(L"You have nothing to enchant!");
	  return;
	case serviceType::proofing:
	  if (!inventory_.empty()) {
	    proof();
	    handlePayment();
	  } else ios.message(L"You have nothing to protect!");
	  return;
	case serviceType::fixing:
	  if (!inventory_.empty()) {
	    mend();
	    handlePayment();
	  } else ios.message(L"You have nothing to repair!");
	  return;
	default:
	  throw s;
	}
    }
    if (i == 0) ++i;
    basket_.push_back(forSale_[idx-i]);
    forSale_.erase(forSale_.begin() + (idx-i));
    if (forSale_.empty()) {
      ios.message(L"You have " + std::to_wstring(basket_.size()) + 
		  std::wstring(L" items in your basket."));
      handlePayment();
    } else if (!ios.ynPrompt(L"You have " + std::to_wstring(basket_.size()) + 
		      std::wstring(L" items in your basket. Check out?")))
      handleSale();
    else
      handlePayment();
  }

  void handlePayment() {
    double p=0;
    for (auto &s : servicesBought_)
      p += s.second;
    for (auto &i : basket_)
      p += appraise(inventory_, *i, transaction::buy);
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
      ioFactory::instance().message(L"You don't have enough valuables to interest " + keeperName_);
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
      double b=0;
      for (auto &i : barter)
	b += appraise(inventory_, *i, transaction::sell);
      if (b > p) {
	completeSale(barter);
	return true;
      } else {
	ioFactory::instance().longMsg(L"\"Thank ye for your kind offer, but I think you've undervaluing my wares.\"");
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
	if (inventory_.slotsOf(i)[0] != nullptr) return; // skip clothing & wields
	if (i.isCursed()) return; // skip cursed items, including ious!
	auto adj = i.adjectives();
	if (std::find(adj.begin(), adj.end(), L"dead") != adj.end()) return; // skip dead items (unless bagged); corpses are too valuable because of their weight
	auto pi = i.shared_from_this();
	auto end = barter.end();
	if (std::find(barter.begin(), end, pi) == end) { // not already for barter
	  double price = appraise(inventory_,i, transaction::sell);
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
	t += appraise(inventory_, *i, transaction::sell);
    } while (t < q);
    return barter;
  }

  std::vector<std::shared_ptr<item> > doBarter(std::vector<std::shared_ptr<item> > barter) {
    int choice;
    do {
      auto offer = L"Your offer:\n" + toList(barter);
      std::vector<std::pair<int, std::wstring> > choices = {
	{1, L"Clear list and start again"},
	{2, L"Remove an item from your offer"},
	{3, L"Offer more things"},
	{4, L"Attempt the barter"}
      };
      choice = ioFactory::instance().choice(L"Please select: ", offer, choices);
      switch (choice) {
      case 1:
	barter.clear();
	break;
      case 2:  // remove item
	if (!barter.empty()) {
	  std::vector<std::pair<std::shared_ptr<item>, std::wstring> > c;
	for (auto i : barter)
	  c.emplace_back(i, i->name());
	auto &ios = ioFactory::instance();
	auto toRemove = ios.choice(L"Shelve offer for:", L"", c);
	barter.erase(std::find(barter.begin(), barter.end(), toRemove));
	break;
      }
      case 3: // add item
	auto numItemsAvailable =  // count the number of non-cursed items in inventory
	  inventory_.countIf([](item &i) { return !i.isCursed(); });
	if (numItemsAvailable > barter.size()) { // still some items to add
	  auto toAdd = pickItem(L"What do you want to offer:", L"Choose the item to add to your offer",
/*Njorthrbiartr is longest keeper name */
keeperName_ + L" will appraise the value of the items you offer, and decide if\n"
"it is a fair trade. If it isn't, you can modify your offer or decline.",
				 [&barter](const item & i) {
				   auto adj = i.adjectives();
				   return 
				   (!i.isCursed()) && // can't sell cursed items, such as IOUs
				   (std::find(adj.begin(), adj.end(), L"dead") == adj.end()) && // skip dead items (unless bagged); corpses are too valuable because of their weight
				   std::find(barter.begin(), barter.end(), i.shared_from_this()) == barter.end();
				 }
			   );
	  if (toAdd)
	    barter.push_back(toAdd->shared_from_this());
	} else {
	  auto &ios = ioFactory::instance();
	  ios.message(L"Thou hast nothing to offer!");
	  return barter;
	}
	break;
      }
    } while (choice != 4);
    return barter;
  }

  void completeSale(std::vector<std::shared_ptr<item> > &barter) {
    ioFactory::instance().message(L"Your offer is accetable.\nThank you; come again."); // ref:Simpsons, Apu
    for (auto i : barter) disposer_.addItem(*i); // popup shop, so switch items
    for (auto i : basket_) inventory_.addItem(*i);
  }

  void paymentFailed() {
    // add any IOUs:
    for (auto s : servicesBought_)
      inventory_.addItem(createIou(s.second, keeperName_, s.first));
    servicesBought_.clear();
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
    return ioFactory::instance().ynPrompt(prompt);
  }

  // pick an item to be used below.
  // TODO: filter out foo-proof and/or foo-undamaged items as appropriate
  item *pickItem(const std::wstring & prompt,
		 const std::wstring & help,
		 const std::wstring & extraHelp,
		 const std::function<bool(const item &)> f = [](const item &){return true;}) const {
    return inventory_.pickItem(prompt, help, extraHelp, f, false);
  }

  void enchant() {
    auto pItem = pickItem(L"What would you like to enchant?",
			  L"Choose the item whose enchantment you want to enhance",
			  L"Enchantment adds charges to wands, resistance to armour and\n"
			  "damage to weapons. You may choose one item.");
    if (!pItem) return;
    auto &item = *pItem;
    bool blessed = item.isBlessed();
    bool cursed = item.isCursed();
    
    /*
     *    dPc: 0     50     100
     * normal: 1     3      5
     * blessed:2     6      10
     * cursed: 0:1   0:3    0:5
     * bc:     0     4      8
     */

    int de;
    if (blessed && cursed) de = 2 * (dPc() / 25);
    else if (blessed) de = 2 * (1+ (dPc() / 25)); // blessed items get *4 enchantment
    else if (!cursed || dPc() < 50) de = 1+ (dPc() / 25);
    else de = 0;
    item.enchant(de);

    auto &ios = ioFactory::instance();
    if (de > 0)
      ios.message(L"Magical enenergy flows into your " + item.name());
    else // let's see if the user's paying attention...
      ios.message(L"Magical enenergy flows through your " + item.name());

    servicesBought_.emplace(L"Enchantment to " + item.name(),
			    appraise(inventory_, item, transaction::buy));
  }

  void proof() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto pItem = pickItem(prompt, std::wstring(L"Choose the item on which to bestow resistance"),
			  L"This will completely protect your item against " + 
			  std::wstring(damage_.name()) +
			  L" attacks, traps and effects. When you wear an item with this protection,"
			  "it will also protect any items worn under it.");
    if (!pItem) return;
    auto &item = *pItem;
    auto &ios = ioFactory::instance();
    if (item.proof(damage_.type()))
      ios.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + item.name() +
		 L" seems just as complete as it can be.");
    else
      ios.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + item.name() +
		 L" could use a little more work");

    servicesBought_.emplace(std::wstring(damage_.mendName()) + L" protection unto " + item.name(),
			    appraise(inventory_, item, transaction::buy));
  }

  void mend() {
    auto prompt = std::wstring(L"What would you like to protect against ") + damage_.name();
    prompt += L"?";
    auto *pItem = pickItem(prompt, L"Choose the item on which to repair",
			   L"This will repair your item against " + std::wstring(damage_.name()) +
			   L"damage already taken.");
    if (!pItem) return;
    auto &item = *pItem;
    auto &ios = ioFactory::instance();
    if (item.repair(damage_.type()))
      ios.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" improves.");
    else
      ios.message((std::wstring(L"The ") + damage_.mendName()) + 
		 L" of your " + std::wstring(item.name()) +
		 L" seems unchanged");

    servicesBought_.emplace(std::wstring(damage_.mendName()) + L" reparation unto " + item.name(),
			    appraise(inventory_, item, transaction::buy));
  }

};

const wchar_t shop::render() const {
  return L'£';
};
std::wstring shop::name() const {
  return pImpl_->name();
};
std::wstring shop::description() const {
  return pImpl_->description();
};

shop::shop(monster & inventory, std::wstring adjective, shopType type) {
  const std::wstring keeperName = *rndPick(keeperNames, keeperNames + numKeeperNames);
  pImpl_.reset(new shopImpl(inventory, type, keeperName, adjective));
}

shop::shop(monster & inventory, monster &shopInventory) :
  pImpl_(new shopImpl(inventory, shopInventory)) {}

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

/*
#include <iostream>
int main() {
  int count[shopType::END+1];
  for (int i=0; i <= shopType::END; ++i) count[i] = 0;
  std::wcout << static_cast<int>(shopType::END) << std::endl;

  for (int i=0; i < 100000; ++i)
    count[(shopType::END-1) * dPc() / 100]++;
  
  for (int i=0; i <= shopType::END; ++i) {
    auto type = static_cast<shopType>(i);
    auto n = shopName(type);
    std::wcout << n << '\t' << count[i] << std::endl;
  }
  }*/

class shoppingCentre {
private:
  std::vector<shopType> types_;
  std::vector<std::wstring> adjectives_;
  shop a;
  shop b;
  shop c;
  shop d;
  shop e;
  shop f;
  shop g;
  shop h;
private:
  shopType sType() {
    shopType type;
    do {
      type = static_cast<shopType>((shopType::END - 1) * dPc() / 100);
    } while (std::find(types_.begin(), types_.end(), type) != types_.end());
    types_.push_back(type);
    return type;
  }
  std::wstring adjective() {
    std::wstring adjective;
    do {
      adjective = *rndPick(shopAdjectives, shopAdjectives + numShopAdjectives);
    } while (std::find(adjectives_.begin(), adjectives_.end(), adjective) != adjectives_.end());
    adjectives_.push_back(adjective);
    return adjective;
  }
public:
  shoppingCentre(monster &inventory) :
    types_(), adjectives_(),
    a(shop(inventory, adjective(), sType())),
    b(shop(inventory, adjective(), sType())),
    c(shop(inventory, adjective(), sType())),
    d(shop(inventory, adjective(), sType())),
    e(shop(inventory, adjective(), sType())),
    f(shop(inventory, adjective(), sType())),
    g(shop(inventory, adjective(), sType())),
    h(shop(inventory, adjective(), sType())) {
	types_.clear();
	adjectives_.clear();
}  
public:
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

void goShopping(monster &inventory) {
  // how to choose the number of shops?
  // When you need a magic number, and don't have any other criteria, turn to ref:Pratchett's Discworld.
  shoppingCentre shops(inventory);
  std::vector<std::pair<int,std::wstring> > choices;
  for (int c=0; c < 8; ++c) {
    choices.emplace_back(c, shops[c].name());
  }
  auto &ios = ioFactory::instance();
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

// ref: Started with Vienna December market of 1928, according to Wikipedia article on Pop-Up Retail.
void popUpShop(monster &from, monster &to) {
  shop s(from, to);
  s.enter();
}
