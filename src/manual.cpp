/* License and copyright go here*/

// Text for the manuals
#include "manual.hpp"

const wchar_t *const manual::name(const manual::chapter &c) {
  switch (c) {
  case manual::chapter::strength: return L"Strength";
  case manual::chapter::fight: return L"Fight";
  case manual::chapter::dodge: return L"Dodge";
  case manual::chapter::damage: return L"Damage";
  case manual::chapter::appearance: return L"Appearance";
  case manual::chapter::melee: return L"Melee";
  case manual::chapter::ranged: return L"Ranged";
  case manual::chapter::religion: return L"Spirituality";
  default: return nullptr;
  }
}

const wchar_t *const manual::chap(const manual::chapter &c) {
  switch (c) {
  case manual::chapter::strength:
    return L"Characteristic: Strength\n\n"
"Stregth has a number of effects during play.\n\n"
"All characteristics ranges from 0 (--) to 100 (**). If below its current max\n"
"value, the percentage reduction is shown, eg 75%48.\n\n"
"The main effects of strength are: the damage dealt in melee, and how much\n"
"weight you can carry before becoming slowed.\n"
      "Monsters with high strength are best attacked at range.";
  case manual::chapter::fight:
    return L"Characteristic: Fight\n\n"
"Fight is your ability to hit your opponent in melee combat.\n\n"
"All characteristics ranges from 0 (--) to 100 (**). If below its current max\n"
"value, the percentage reduction is shown, eg 75%48.\n\n"
"Specifically, your fight characteristic is your effectiveness at hitting a\n"
"non-moving target with an average weapon.";
  case manual::chapter::dodge:
    return L"Characteristic: Dodge\n\n"
"Dodge is your ability to avoid various blows in combat.\n\n"
"All characteristics ranges from 0 (--) to 100 (**). If below its current max\n"
"value, the percentage reduction is shown, eg 75%48.\n\n"
"When a blow is on target, a higher dodge score will reduce the chance of\n"
"contact. Dodge:-- has no effect; Dodge:** will half the chance of injury.";
  case manual::chapter::damage:
    return L"Characteristic: Damage\n\n"
"Damage relates to your level of injury during the game.\n\n"
"All characteristics ranges from 0 (--) to 100 (**). If below its current max\n"
"value, the percentage reduction is shown, eg 75%48.\n\n"
"Your damage characteristic shows how much damage you can take during play,\n"
"so it's a good idea to keep this as high as possible. Your current damage,\n"
"on the other hand, shows the damage you have taken. This starts at 0%\n"
"and can be reduced by eating.";
  case manual::chapter::appearance:
    return L"Characteristic: Appearance\n\n"
"Your outward appearance affects various actions during the game.\n\n"
"All characteristics ranges from 0 (--) to 100 (**). If below its current max\n"
"value, the percentage reduction is shown, eg 75%48.\n\n";
  case manual::chapter::melee:
    return L"Melee\n\n"
"Remember: the other party usually gets to go first. To fight an opponent,"
"simply move into them.\n"
"Monsters will attack any other species, unless they share an alignment.\n"
"To get the best chance of hitting, wield a large, heavy or enchanted weapon.\n"
"Wear the best armour you can find, and change it as it becomes damaged, thus\n"
"less effective. Withdraw, drop your damaged armour, then don fresh armour. A\n"
"repair kit is usually effective the first time it's used on damaged armour.\n"
"Don't forget that bucklers etc are weapons and armour: saving weight but\n"
"wears out faster. Weapons wear out too; the same applies.";
  case manual::chapter::ranged:
    return L"Ranged\n\n"
"Ranged combat lets you keep your distance, but your ammunition is limited.\n"
"Some projectiles are more effective if you wield the right weapon: a\n"
"crossbow for bolts, a sling for rocks. Drop a weapon to change.\n"
"To use amunition, simply use it, then select the target. Concentrate your\n"
"fire on one target at a time; many monsters can't move through each other.\n"
"As with all weapons, enchantment increases the damage dealt.\n"
"Most missiles require line of sight, but monsters can throw things at you too.";
  case manual::chapter::religion:
    return L"Spirituality\n\n"
"There are, of course, 62 religions, and we can't cover them all. Just think\n"
"of the ten main axioms:-\n"
" * Dominion: concentration or aggression;\n"
" * Outlook : kindness or power;\n"
" * Element : earth, air, plant, water, time or fire.\n"
"All paths follow one or more of these axioms; when you meet another path, it\n"
"may be aligned on each axis - or not. The more axes in common, the greater\n"
"the alignment.\n"
"Offering (leaving) normal food in a shrine increases your alignment with an\n"
"axiom of that path. Offerings are for the needy, so taking food reduces your\n"
"alignment unless you are starving.\n"
"Improving your alignment can ease your entry into shrines and affect monster\n"
"reactions.\n"
"Alignment can be changed with a wand of conversion: this is unpredictable.\n"
"Visiting a dreamscape shrine can bless and enchant various items.\n"
"An elemental alignment axis can affect flowers in gardens.\n"
"Angels help and hinder differently depending on the domination axis.\n";
  default:
    return nullptr;
  }
}

const wchar_t *const manual::advanced(const manual::chapter &c) {
  switch (c) {
  case manual::chapter::strength:
    return L"\n"
"Strength is also important for pushing through the barriers on a shrine to a\n"
"deitiy for whose path you are only slightly aligned.\n"
"While certain item enchantments are the main way of affecting your strength,\n"
"laying with an incubus or succubus can also have an effect.";
  case manual::chapter::fight:
    return L"\n"
"A small bonus to hitting your target applies if your weapon is magnetic.";
  case manual::chapter::dodge:
    return L"\n"
"While certain item enchantments are the main way of affecting your dodge,\n"
"laying with an incubus or succubus can also have an effect.\n";
  case manual::chapter::damage:
    return L"\n"
"Note that seasoning food (by applying a salt or pepper cellar) will increase\n"
"its the nutritional value. Larger meals decrease more of your injury, and\n"
"consuming fluids will have a smaller effect. Sleeping - while dangerous -\n"
"is also restorative, as is entering a shine of a deity who shines upon you.\n"
"Injured snakes are shorter.";
  case manual::chapter::ranged:
    return L"\n"
"Wearing a bow does not help with ranged combat.";
  default: return nullptr;
  }
}
