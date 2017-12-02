/* License and copyright go here*/

// room of a level assigned to a religious deity

#include "shrine.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "player.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp"

shrine::shrine(const coord &ul, const coord &lr, const deity &align, const io &io) :
  ul_(ul), lr_(lr), align_(align), io_(io) {};

deity &rndAlign() {
  auto start = deityRepo::instance().begin();
  start++; // skip over nonaligned (doesn't make sense for shrines)
  auto pDeity = rndPick(start, deityRepo::instance().end());
  return *pDeity;
}

shrine::shrine(const coord &ul, const coord &lr, const io &io) :
  ul_(ul), lr_(lr), align_(rndAlign()), io_(io) {
};

shrine::~shrine() {}

// determine if a given square contains this zone
bool shrine::contains(coord a) {
  return a.first >= ul_.first && a.first <= lr_.first &&
    a.second >= ul_.second && a.second <= lr_.second;
}

/*
 * Alignment rules for shrines:
 ***
 * 3 axes - this is a shrine to your deity. You may enter and receive full sanctuary (nothing attacks you while inside).
 * 2 axes - a close relative of your deity. You may enter, but receive no especial protection or danger from wandering monsters.
 * 1 axis - a loose relative of your deity. You may only enter by pushing your way through the doorway's "mysterious barrier", requiring a stat check (attack?). No especial protection or danger from wandering monsters.
 * 0 axes - this alter opposes your deity. You may not enter through the doorway. All creatures will attack you while you are inside, even those who wouldn't normally. *You* still can't attack, meaning this is a bad place to be.
 */

std::wstring shrine::name() const {
  return std::wstring(L"Venerated House of ") + align_.house() + L"; shrine to " + align_.name();
}

// handles limitations on entering the zone
bool shrine::onEnter(std::shared_ptr<monster> monster, itemHolder &pev) {
  auto coalign = align_.coalignment(monster->align());
  auto p = std::dynamic_pointer_cast<player>(monster);
  if (coalign == 0) {
    if (p) io_.message(L"You are prevented from entering the " + name() + L" by a mysterious barrier");
    return false; // opposed. You can't normally enter by your own power.
  }
  if (coalign == 1) {
    auto roll = dPc();
    bool success = roll < monster->strength().cur();
    if (p) {
      // monster is a player
      io_.message(success ?
		  std::wstring(L"You force your way through a mysterious barrier\nWelcome to the ") + name() : 
		  std::wstring(L"You are prevented from entering the " + name() + L" by a mysterious barrier"));
    }
    return success;
  }
  if (p) io_.message(L"You enter the " + name());
  if (align_ == monster->align()) io_.message(L"You feel very safe here.");
  return true;
}

// handles dropping food from the zone
bool shrine::onEnter(std::shared_ptr<item> item, itemHolder &prev) {
  auto p = dynamic_cast<monster*>(&prev);
  if (!p) return true; // okay, but not an offering as not from a monster
  if (item->render() != L'%') // okay, but not an offering as most creatures can't eat it
    return true;
  // TODO: Should dropping bottles of water count as an offering?
  // TODO: alignment counters
  io_.message(L"Your offering is accepted");
  return true;
}

// handles taking food from the zone
bool shrine::onExit(std::shared_ptr<item> item, itemHolder &prev) {
  auto p = dynamic_cast<monster*>(&prev);
  if (!p) return true; // okay; no penalty for non-monsters
  if (item->render() != L'%') // okay, not an offering as most creatures can't eat it
    return true;
  // TODO: alignment counters
  if (p->injury().pc() > 10)
    io_.message(L"You steal an offering intended for the needy...");
  else
    io_.message(std::wstring(L"You accept the charity of ") + align_.name());
  return true;
}
