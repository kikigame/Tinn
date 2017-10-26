/* License and copyright go here*/

// Debugging bits and bobs; not compiled into the main game

#include <iostream>

void dumpReligion() {

  std::vector<Element> allE = {
    Element::earth,
    Element::air,
    Element::fire,
    Element::water,
    Element::plant,
    Element::time,
    Element::none };
  std::vector<Domination> allD = {  Domination::concentration, Domination::aggression, Domination::none };
  std::vector<Outlook> allO = {  Outlook::kind, Outlook::cruel, Outlook::none };


  for (Element e : allE)
    for (Domination dn : allD)
      for (Outlook o : allO) {
	auto &d = deityRepo::instance().getExact(e, dn, o);

  switch (d.element()) {
  case Element::earth: std::cout << "earth"; break;
  case Element::air: std::cout << "air"; break;
  case Element::fire: std::cout << "fire"; break;
  case Element::water: std::cout << "water"; break;
  case Element::plant: std::cout << "plant"; break;
  case Element::time: std::cout << "time"; break;
  case Element::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << "\t";

  switch (d.domination()) {
  case Domination::concentration: std::cout << "concentration"; break;
  case Domination::aggression: std::cout << "aggression"; break;
  case Domination::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << "\t";

  switch (d.outlook()) {
  case Outlook::kind: std::cout << "kindness"; break;
  case Outlook::cruel: std::cout << "cruelty"; break;
  case Outlook::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << std::endl
	    << d.render() << " " 
	    << d.name() << ":\t"
	    << d.description()
	    << std::endl;
  
  std::cout << std::endl << std::endl;
      }
}


#include "role.hpp"
#include "player.hpp"

void dumpStartStats() {
  std::vector<Element> allE = {
    Element::earth,
    Element::air,
    Element::fire,
    Element::water,
    Element::plant,
    Element::time,
    Element::none };
  std::vector<Domination> allD = {  Domination::concentration, Domination::aggression, Domination::none };
  std::vector<Outlook> allO = {  Outlook::kind, Outlook::cruel, Outlook::none };
  std::vector<monsterTypeKey> allM = { monsterTypeKey::troll, monsterTypeKey::human };
  std::vector<roleType> allR = { roleType::shopkeeper, };//roleType::warrior };

  auto o = ::std::shared_ptr<io>(NULL);

  for (Element e : allE)
    for (Domination dn : allD)
      for (Outlook o : allO) {
	auto &d = deityRepo::instance().getExact(e, dn, o);

	for (auto type : allM)
	  for (auto role : allR) {
	    
	    playerBuilder bldr;
	    bldr.name("Timmy the tested");
	    bldr.align(d);
	    bldr.male(0);
	    bldr.female(0);
	    bldr.type(monsterTypeRepo::instance()[type]);
	    bldr.job(roleRepo::instance()[role]);

	    player p(NULL, bldr);

  switch (d.element()) {
  case Element::earth: std::cout << "earth"; break;
  case Element::air: std::cout << "air"; break;
  case Element::fire: std::cout << "fire"; break;
  case Element::water: std::cout << "water"; break;
  case Element::plant: std::cout << "plant"; break;
  case Element::time: std::cout << "time"; break;
  case Element::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << "\t";

  switch (d.domination()) {
  case Domination::concentration: std::cout << "concentration"; break;
  case Domination::aggression: std::cout << "aggression"; break;
  case Domination::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << "\t";

  switch (d.outlook()) {
  case Outlook::kind: std::cout << "kindness"; break;
  case Outlook::cruel: std::cout << "cruelty"; break;
  case Outlook::none: std::cout << "none"; break;
  default: std::cout << "ERROR"; break;
  }

  std::cout << "\t";

  switch (type) {
  case monsterTypeKey::troll: std::cout << "troll"; break;
  case monsterTypeKey::human: std::cout << "human"; break;
  default: std::cout << "ERROR"; break;
  }

  /*
  std::cout << "\t";

  switch (role) {
  case roleType::shopkeeper: std::cout << "shopkeeper"; break;
  case roleType::warrior: std::cout << "warrior"; break;
  default: std::cout << "ERROR"; break;
  }*/

  std::cout << "\n"
	    << "ST:" << p.strength()
	    << " AP:" << p.appearance()
	    << " FI:" << p.fighting()
	    << " DO:" << p.dodge()
	    << " DA:" << p.damage()
	    << "\n" << std::endl;

	  }
      }
}
