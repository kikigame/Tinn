/* License and copyright go here*/

// define the player in the dungeon (I/O bit)


#include "dungeon.hpp"
#include "chargen.hpp"
#include "player.hpp"
#include "output.hpp"
#include "religion.hpp"
#include "role.hpp"

#include <map>

// generate the player character (interactively) and return a fully-populated builder.
playerBuilder chargen() {
  playerBuilder b;
  const io &ios = ioFactory::instance();
  /*if (true) {
    b.name(L"ki");
    b.align(deityRepo::instance().nonaligned());
    b.type(monsterTypeRepo::instance()[monsterTypeKey::troll]);
    b.job(roleRepo::instance()[roleType::warrior]);
    return b;
    }*/
  std::wstring name;
  do {
    name = ios.linePrompt
      (L"Let's play Tinn!\nCHARACTER GENERATION[1/7]> What is your handle? ",
       L"  They say you should never use your real name on the Internet.\n" 
       "  The handle identifies your character; it is how others may name\n"
       "  you. No alias carries any specific advantage, although shorter\n"
       "  names fit more easily on the screen.\n"
       );
  } while (name.empty());
  b.name(name);

  { // alignment
    Domination domination = ios.choice<Domination>
      (L"CHARACTER GENERATION[2/7]> Welcome " + name,
       L"If an enemy were bent on causing you harm, would you:\n",
       {
	 {Domination::concentration, L"Study your enemy to learn their weaknesses" },
	 {Domination::aggression, L"Defend yourself with a good offense"},
	 {Domination::none, L"None of the above" }
       },
       L"Those minded to concentration will have a bonus to mind and defence,\n" //
       "   at a cost of speed, and being hurt more by fire.\n\n"
       "Those minded to aggression will have a bonus to attack and damage,\n"
       "   at a cost of outward beauty, and being struck more often in combat."
       );
    Outlook outlook = ios.choice<Outlook>
      (L"CHARACTER GENERATION[3/7]> Welcome " + name,
       L"A rich stranger lies dying in pain before you; would you:\n",
       {
	 {Outlook::kind, L"Do what you can to ease the stranger's suffering"},
	 {Outlook::cruel, L"Keep the stranger alive as long as you can, however much it hurts"},
	 {Outlook::cruel, L"Take what riches you can"}, // greed, of course, is still cruelty, however callous
	 {Outlook::none, L"None of the above" }
       },
       L"Those minded to compassion will have a bonus to outward beauty,\n"
       "   at a cost of stamina.\n"
       "Those of a callous mindset will have an intrinsic armour bonus,\n"
       "   at a cost of physical charisma.\n"
       );
    Element element = ios.choice<Element>
      (L"CHARACTER GENERATION[4/7]> Your spiritual path",
       L"Which natural aspect best describes your alignment?\n",
      {
	{Element::earth,
	    L"Earth: strong; prefer veggie diets; poor with edged weapons.\n"},
	{Element::air,
	    L"Air: fast; good hearing; sonic attack bonus; easily damaged.\n"},
        {Element::fire,
	    L"Fire: double attacks; take more damage from water attacks.\n"},
        {Element::water,
	    L"Water: swimming; good with bashing weapons; slow.\n"},
        {Element::plant,
	    L"Flora: fast climber; prefer veggie diets; hurt more by bashing.\n"},
        {Element::time,
	    L"Time: fast; double attacks; hurt more by disintegration.\n"},
	{Element::none, L"None of the above" }
	 });
    auto &repo = deityRepo::instance();
    deity & path = repo.getExact(element, domination, outlook);
    ios.clear();

    if (path.nonaligned()) {
      ios.longMsg(name + L", ye have neither deity nor spirituality.\n"
		 "This is a cautious choice, but may leave you with less "
		 "help during the game.");
    } else {
      ios.longMsg(name + L", you are on the path of " + path.name() + L"\n"
		 + path.description());
    }
    b.align(path);
  } // alignment

  { // gender

    auto gender = ios.genderPrompt
      (/*msg*/L"CHARACTER GENERATION[5/7]> Your character's gender ?",
       /*help*/L"Gender is a complex issue with limited affect in this game.\n"
       "Mixed genders get bonuses and penalties proportionately.\n"
       "Some in-game effects may affect your character's gender later on.\n"
       "\nKeys:\n"
       " WASD - change values\n"
       " M/F/H/N - toggle popular values\n"
       " [Enter] - next stage\n",
       /*F0*/
       L"Hint: non-females are protected from sucubii's worst effects",
       /*F%*/
       L"Hint: female characters may have a bonus from incubii",
       /*M0*/
       L"Hint: non-males are protected from incubii's worst effects",
       /*M%*/
       L"Hint: male characters may have a bonus from sucubii");

    b.male(gender.first);
    b.female(gender.second);
  } // chargen

  { // race
    const monsterTypeKey choice = ios.choice<monsterTypeKey>
      (L"CHARACTER GENERATION[6/7]> Your character'S race",
       L"   Options are:",
      {
	  { monsterTypeKey::troll, L"Troll: rock-eating stony beings, big and ugly" },
	    // most generic option is always last
	  { monsterTypeKey::human, L"Human: curious earthly bipeds" }
      },
       L"Your species defines most of your physical attributes, and may affect\n"
       "   how others you meet will interact with you.\n");
    b.type(monsterTypeRepo::instance()[choice]);
  }

  { // role
    const roleType choice = ios.choice<roleType>
      (L"CHARACTER GENERATION[7/7]> Your character'S role",
       L"   Options are:",
      {
	{ roleType::shopkeeper, L"Shopkeeper: has access to expensive equipment" },
	    // most generic option is always last
	{ roleType::warrior, L"Warrior: a good choice against hostile monsters" }
      },
       L"Your job defines most of your starting inventory and defines your\n"
       "   quest within the Areas of Adventure.\n");
    b.job(roleRepo::instance()[choice]);
  }

  return b;
}

