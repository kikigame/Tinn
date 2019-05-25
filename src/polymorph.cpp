/* License and copyright go here*/

// Polymorphables

#include "items.hpp"
#include "output.hpp"
#include "damage.hpp"
#include "equippable.hpp"

bool polymorph(item &from, const itemTypeKey &to) {
  if (from.highlight()) return false;
  basicItem* pI = dynamic_cast<basicItem*>(&from);
  if (pI && pI->getType() == itemTypeRepo::instance()[to]) {
    ioFactory::instance().message(L"The " + from.name() + L" seems different.");
    return true;
  }

  auto &holder = from.holder();
  item &rtn = createItem(to);

  std::array<const slot *, 2> slots;

  equippable* pE;
  if ((pE = dynamic_cast<equippable*>(&holder)))
    slots = pE->forceUnequip(from);
  else
    slots[0] = slots[1] = nullptr;

  if (!holder.replaceItem(from, rtn))
    return false;

  int d;
  for (auto dt : damageRepo::instance())
    if ((d = from.damageOfType(dt.first)) != 0)
      rtn.forceDamageOnly(dt.first, d);

  if (from.enchantment() != rtn.enchantment())
    rtn.enchant(rtn.enchantment() - from.enchantment());

  if (slots[0] != nullptr || slots[1] != nullptr)
    pE->equip(rtn, slots);
  
  from.destroy();
  return true;
}

