/* License and copyright go here*/

// room of a level assigned to a religious deity

#include "shrine.hpp"
#include "items.hpp"
#include "monster.hpp"
#include "player.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp"

shrine::shrine(const coord &ul, const coord &lr, const deity &align) :
  area_(rectarea::create(ul, lr)), align_(align) {};

deity &rndAlign() {
  auto start = deityRepo::instance().begin();
  start++; // skip over nonaligned (doesn't make sense for shrines)
  auto pDeity = rndPick(start, deityRepo::instance().end());
  return *pDeity;
}

shrine::shrine(const coord &ul, const coord &lr) :
  area_(rectarea::create(ul, lr)), align_(rndAlign()) {
};

shrine::~shrine() {}

// determine if a given square contains this zone
bool shrine::contains(coord a) {
  return area_->contains(a);
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
bool shrine::onEnter(monster  &monster, itemHolder &pev) {
  auto &ios = ioFactory::instance();
  auto alignment = monster.align().coalignment(align_);
  auto p = monster.isPlayer();
  if (p) alignment += monster.align().alignCounter();
  if (alignment == 0) {
    if (p) ios.message(L"You are prevented from entering the " + name() + L" by a mysterious barrier");
    return false; // opposed. You can't normally enter by your own power.
  }
  if (alignment == 1) {
    auto roll = dPc();
    bool success = roll < monster.strength().cur();
    if (p) {
      // monster is a player
      ios.message(success ?
		  L"You force your way through a mysterious barrier\nWelcome to the " + name() : 
		  L"You are prevented from entering the " + name() + L" by a mysterious barrier");
    }
    return success;
  }
  if (p && alignment < 6) {
    ios.message(L"You enter the " + name());
    if (align_ == monster.align()) ios.message(L"You feel very safe here.");
  }
  if (p && alignment >= 6) {
    if (monster.abilities().hear())
      ios.message(L"A heavenly chorous you into the " + name());
    else
      ios.message(L"You enter the " + name());
    if (align_ == monster.align()) ios.message(L"You are completely safe here.");
    monster.injury() -= 25;
  }
  return true;
}

// handles dropping food from the zone
bool shrine::onEnter(item &item, itemHolder &prev) {
  auto p = dynamic_cast<monster*>(&prev);
  if (!p) return true; // okay, but not an offering as not from a monster
  if (item.render() != L'%') // okay, but not an offering as most creatures can't eat it
    return true;
  // TODO: Should dropping bottles of water count as an offering?
  deityRepo::instance().countAlign(p->align(), true);
  auto &ios = ioFactory::instance();
  ios.message(L"Your offering is accepted");
  return true;
}

// handles taking food from the zone
bool shrine::onExit(item &item, itemHolder &prev) {
  auto p = dynamic_cast<monster*>(&prev);
  if (!p) return true; // okay; no penalty for non-monsters
  if (item.render() != L'%') // okay, not an offering as most creatures can't eat it
    return true;
  auto &ios = ioFactory::instance();
  if (p->injury().pc() > 10) {
    ios.message(L"You take an offering intended for the needy...");
    deityRepo::instance().countAlign(align_, false);
  } else
    ios.message(L"You accept the charity of " + align_.name());
  return true;
}

// players may never attack in shrines.
// monsters will never attack a fully coaligned monster in a shrine
bool shrine::onAttack(monster &aggressor, monster &target) {
  auto p = dynamic_cast<player*>(&aggressor);
  auto &ios = ioFactory::instance();
  if (p) {
    if (align_.domination() == Domination::aggression)
      ios.message(L"Even " + align_.name() + L" needs a place to rest.");
    else
      ios.message(align_.name() + L" hath decreed this sanctuary as a place of rest."); 
    return false;
  }
  auto coalign = align_.coalignment(target.align());
  auto pl = dynamic_cast<player*>(&target);
    if (coalign == 3) {
      if (pl)
	ios.message(aggressor.name() + L" seems unwilling to attack you in the house of " + align_.house());
      else
	ios.message(aggressor.name() + L" seems unwilling to attack "+ target.name() +L" in the house of " + align_.house());	  
      return false;
    }
  return true;
}

