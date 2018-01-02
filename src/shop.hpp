/* License and copyright go here*/

// define a monster in the dungeon

#ifndef SHOP_HPP
#define SHOP_HPP

#include "renderable.hpp"
#include "itemholder.hpp"
#include <memory>
#include <vector>
class item;
class shopImpl;
class monster;

// NB: most common are in the middle due to bell curve:
// NB: Last shop is very rare (role of 99 or 100 given 11 shops)
enum shopType {// rough chance per shop (NB: shopping centre is 8 *distinct* types)
  stylii,     // 2%
  weapons,    // 5%
  groceries,  // 8%
  thrown,     // 11.5%
  readable,   // 14.5%
  jewellery,  // 17%
  gambling,   // 14.5%
  luggage,    // 11.5%
  bottles,    // 8%
  tools,      // 5%
  clothes,    // 2%
  music,      // 0.04%
  END
};

class shop : public renderable {
private:
  std::unique_ptr<shopImpl> pImpl_;
public:
  typedef std::vector<std::shared_ptr<item>>::const_iterator itemIter;
  // create a new shop
  // ios -> used for user interaction
  // inventory -> itemholder for customer's inventory to buy and sell
  shop(monster & inventory, std::wstring adjective, shopType type);
  // create a pop-up shop
  // inventory -> itemholder for customer's inventory to buy and sell
  // shopInventory -> monster whose items can be bought and sold
  shop(monster & inventory, monster &shopInventory);
  shop(const shop &) = delete;
  shop(shop &&) = default;
  ~shop();
  shop & operator = (const shop &) = delete;

  virtual const wchar_t render() const;
  virtual const wchar_t * const name() const;
  virtual const wchar_t * const description() const;

  itemIter begin() const;
  itemIter end() const;

  void enter();
};

// run the shop routine using the given monster's inventory
// (probably the PC for now):
void goShopping(monster & inventory);

// create a transient shop using the inventory of "to".
void popUpShop(monster & from, monster &to);

#endif // ndef SHOP_HPP
