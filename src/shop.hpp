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

class shop : public renderable {
private:
  std::unique_ptr<shopImpl> pImpl_;
public:
  typedef std::vector<std::shared_ptr<item>>::const_iterator itemIter;
  // create a new shop
  // ios -> used for user interaction
  // inventory -> itemholder for customer's inventory to buy and sell
  shop(monster & inventory, std::wstring adjective, shopType type);
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


#endif // ndef SHOP_HPP
