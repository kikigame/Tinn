/* License and copyright go here*/

// define a monster in the dungeon

#ifndef SHOP_HPP
#define SHOP_HPP

#include "renderable.hpp"
#include "itemholder.hpp"
#include <memory>
#include <vector>
class io;
class item;
class shopImpl;
class monster;

class shop : public renderable {
private:
  std::unique_ptr<shopImpl> pImpl_;
public:
  typedef std::vector<std::shared_ptr<item>>::const_iterator itemIter;
  // create a new shop
  // ios -> used for user interaction
  // inventory -> itemholder for customer's inventory to buy and sell
  shop(const io & ios, itemHolder & inventory);
  shop(const shop &) = delete;
  ~shop();
  shop & operator = (const shop &) = delete;

  virtual const wchar_t render() const;
  virtual const wchar_t * const name() const;
  virtual const wchar_t * const description() const;

  itemIter begin() const;
  itemIter end() const;

  void enter();

  // buy things from the shop.
  // Shops work on the barter system, where money is considered a type of object with which
  // to barter.
  // basket: what the player wants to buy (take from the shop)
  // barter: what the player wants to sell to the shop (generally valuables in payment).
  // buyer: who's trading
  void buy(itemIter & basket_begin, itemIter & basket_end,
	   itemIter & barter_begin, itemIter & barter_end,
	   monster &buyer);
};

// run the shop routine using the given monster's inventory
// (probably the PC for now):
void goShopping(const io &ios, itemHolder & inventory);


#endif // ndef SHOP_HPP
