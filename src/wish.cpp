/* License and copyright go here*/

#include "action.hpp"
#include "appraise.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp"
#include "wish.hpp"
#include "itemTypes.hpp"

bool wishFilter(const std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator &i) {
  // wishable classes: '%', '!', '[', '(', '8', '='
  // wish for a '/' or '*' of various types
  // wish for a holy book of given path
  // wish for a bottle of any '~' type
  // wish for a brick of gpl
  switch (i->second->render()) {
  case L'%': case L'!': case L'[': case L']': case L'(': case L'8': case L'=':
    return true;
  default:
    return i->first == itemTypeKey::gpl_brick;
  }
  return true;
}

bool liquidFilter(const std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator &i) {
  return i->second->render() == L'~';
}

void wish(monster &mon, item &it) {

  auto name = it.name();
  auto value = appraise(mon, it, transaction::score);

  // one can wish on coins, stars, or things that are heavier than a coin
  if (value < 0.001 &&
      name.find(L"coin") == std::wstring::npos &&
      name.find(L"star") == std::wstring::npos) return;
  
  auto &io = ioFactory::instance();
  bool doWish = !mon.isPlayer() || io.ynPrompt(L"Make a wish?");
  if (!doWish) {
    mon.curLevel().msg() << sense::SIXTH << L"You felt that wasn't good enough to wish upon" << stop();
    return;
  }

  // offer up a list of items to wish for.
  auto &repo = itemTypeRepo::instance();

  // need to pass things with a unique key to the picker.
  std::vector<std::pair<std::wstring, std::wstring>> choices;
  std::map<std::wstring, itemTypeKey> toOffer;
  std::map<std::wstring, deity*> holyPath;
  std::map<std::wstring, renderedAction<monster,monster>::key> wandType;
  std::map<std::wstring, renderedAction<item,monster>::key> equipType;
  std::map<std::wstring, itemTypeKey> bottledType;

  for (int i=0; i < 16; ++i) { // 2 pages on console output
    // std::map<itemTypeKey, std::unique_ptr<itemType>>::const_iterator
    auto pit = rndPick(repo.begin(), repo.end(), ::wishFilter);
    const itemType &it = *(pit->second);
    std::wstring name = it.name();
    switch (it.render()) {
    case L'/': { // pick random action
      const auto & act = rndPick<sharedAction<monster,monster>::key>();
      const auto &action = actionFactory<monster,monster>::get(act);
      name += L" of " + action.name();
      wandType.emplace(name, act);
      break;
    }
    case L'*': { // pick random action
      const auto & act = rndPick<sharedAction<item,monster>::key>();
      const auto &action = actionFactory<item,monster>::get(act);
      name += L" of " + action.name();
      equipType.emplace(name, act);
      break;
    }
    case L'¶': {// pick random path
      deity &path = rndAlign();
      name += L" of " + path.name();  
      holyPath.emplace(name, &path);
      break;
    }
    case L'8': {// pick random ~
      auto liquid = rndPick(repo.begin(), repo.end(), ::liquidFilter);
      name += L" of " + liquid->second->name();
      itemTypeKey k = liquid->first;
      bottledType.emplace(name, k);
    }
    }// switch

    // NB: it's possible to generate the same name more than once. If so, we just reduce the list size.
    toOffer.emplace(name, pit->first);
    choices.emplace_back(name, name); // don't pass the itemtype as may not be unique
  }

  // now ask the user which item they want.
  std::wstring wish = io.choice(L"What do you wish for?", L"You may wish for an item",
				choices);

  // regardless of what you wish for, wishing on a cursed item gives you a glittery bouquet of flowers:
  auto typeKey = it.isCursed() && !it.isBlessed() ?
    itemTypeKey::bouquet_mixed
    : toOffer[wish];
  auto &type = itemTypeRepo::instance()[typeKey];

  auto typeChar = type.render();
  item & wished =
    typeChar == L'/' ? createWand(wandType[wish]) :
    typeChar == L'*' ? createEquippable(typeKey, equipType[wish]) :
    typeChar == L'¶' ? createHolyBook(*holyPath[wish]) :
    createItem(typeKey);

  if (typeChar == L'8') {
    item &liquid = createItem(bottledType[wish]);
    dynamic_cast<itemHolder&>(wished).addItem(liquid);
  }

  // cursed "it": don't give them the item they wished for, give them something silly instead.
  // also, just for fun, make it glittery.
  if (it.isCursed()) {
    wished.curse(true);
    wished.addAdjective(L"glittery");
  }
  
  // blessed "it": item they wish for will be created in monster's inventory, blessed and with double enchantments. Clothing is sexy.
  if (it.isBlessed()) {
    wished.bless(true);
    wished.enchant(wished.enchantment()); // double: 0-enchanted items at this point shouldn't be enchanted
    mon.addItem(wished);
    wished.addAdjective(L"sexy"); // TODO: only equippable
  } else {
    auto &l = mon.curLevel();
    coord pos = l.posOf(mon);
    l.holder(pos).addItem(wished);
  }


}
