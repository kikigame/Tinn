/* License and copyright go here*/

// Spirituality of Tinn.

#include "religion.hpp"
#include <map>
#include <vector>
#include <memory>
#include <sstream>

// most spiritual paths are represented by a deity.
// all combinations exist.
// none/none/none is special, as it is for an nonaligned creature; there is a placeholder object for the "nonaligned" deity
// deities with two "

class deityImpl {
private:
  const Element element_;
  const Domination domination_;
  const Outlook outlook_;
  const wchar_t * const name_;
  const wchar_t * const house_;
  const wchar_t * const teachings_;
  std::wstring description_;
public:
  deityImpl(const Element element, const Domination domination, const Outlook outlook,
	    const wchar_t * const name, const wchar_t * const house, const wchar_t * const teachings) :
    element_(element),
    domination_(domination),
    outlook_(outlook),
    name_(name),
    house_(house),
    teachings_(teachings) {
    std::wstringstream desc;
    desc << 
      name_ << L" of the house of " << house_ 
	<< L"\n    Ascribes to ";

    switch (element_) {
    case Element::earth: desc << L"earth"; break;
    case Element::air: desc << L"air"; break;
    case Element::fire: desc << L"fire"; break;
    case Element::water: desc << L"water"; break;
    case Element::plant: desc << L"flora"; break;
    case Element::time: desc << L"time"; break;
    case Element::none: break;
    default: desc << L"ERROR"; break;
    }
    
    if (element_ == Element::none && domination_ == Domination::none && outlook_ == Outlook::none)
      desc << L"naught";
    else if (element_ == Element::none)
      desc << L"the ";
    else if (domination_ != Domination::none) 
      desc << L" with the " ;
    else if (outlook_ != Outlook::none)
      desc << L" with a ";
    switch (outlook_) {
    case Outlook::kind: desc << L"kindly "; break;
    case Outlook::cruel: desc << L"harsh "; break;
    case Outlook::none: break;
    default: desc << L"ERROR"; break;
    }

    if (domination != Domination::none || outlook_ != Outlook::none)
      desc << L"aspect";

    switch (domination_) {
    case Domination::concentration: desc << L" of concentration"; break;
    case Domination::aggression: desc << L" of aggression"; break;
    case Domination::none: break;
    default: desc << L"ERROR"; break;
    }


    desc << L".\n    Teaches "
	 << teachings_; // no full-stop; we may want a semicolon along with further details eg during chargen
    description_ = desc.str();
  }
  Element element() const { return element_; }
  Domination domination() const { return domination_; }
  Outlook outlook() const { return outlook_; }
  const wchar_t render() const { return 'G'; }
  const wchar_t * const name() const { return name_; };
  const wchar_t * const description() const { 
    return description_.c_str();
  }
  const wchar_t * const house() const {
    return house_;
  }
};
  
// consumer of the newly allocated pointer.
deity::deity(deityImpl* impl) :
  pImpl_ (std::unique_ptr<deityImpl>(impl)) {}

deity::~deity(){}

Element deity::element() const { return pImpl_->element(); }
Domination deity::domination() const { return pImpl_->domination(); }
Outlook deity::outlook() const { return pImpl_->outlook(); }
// deity::retrieve a reference to the nonaligned deity:
const wchar_t deity::render() const { return pImpl_->render(); }
std::wstring deity::name() const { return pImpl_->name(); }
const wchar_t * const deity::description() const { return pImpl_->description(); }
const wchar_t * const deity::house() const { return pImpl_->house(); }


// concevably used if the vector expands
deity::deity(deity && rhs) : pImpl_(std::move(rhs.pImpl_)) {
}

bool deity::nonaligned() const {
  return (element() == Element::none &&
	  outlook() == Outlook::none &&
	  domination() == Domination::none);
}

unsigned char deity::coalignment(const deity & path) const {
  if (nonaligned() || path.nonaligned()) return 0; // nonaligned things never align, even with each other.
  int counter=0;
  if (element() == path.element()) ++counter;
  if (outlook() == path.outlook()) ++counter;
  if (domination() == path.domination()) ++counter;
  return counter;
}

bool deity::operator ==(const deity &other) const {
  return this == &other;
}


class deityRepoImpl {
private:
  std::vector<deity> repo_; // to own the memory
  std::multimap<Element, deity*> byElement_;
  std::multimap<Domination, deity*> byDomination_;
  std::multimap<Outlook, deity*> byOutlook_;
public:
  deityRepoImpl() :
    repo_() {}
  deityRepoImpl(deityRepoImpl & rhs); // no definition; disallow copies
  void emplace(Element e, Domination d, Outlook o, const wchar_t * const name, const wchar_t * const house, const wchar_t * const teachings) {
    repo_.reserve(72); // There are 72 gods and deities in the map
    // deity constructor takes a reference to deityImpl.
    // it should probably be a std::unique_ptr, and use std::move to copy it into place, but this feels safer for now
    repo_.emplace_back(new deityImpl(e,d,o,name, house, teachings));
    deity &dy = *(repo_.rbegin());
    byElement_.emplace(std::pair<Element, deity*>(dy.element(), &dy));
    byDomination_.emplace(std::pair<Domination, deity*>(dy.domination(), &dy));
    byOutlook_.emplace(std::pair<Outlook, deity*>(dy.outlook(), &dy));
  }

  std::vector<deity>::iterator begin() {
    return repo_.begin();
  }
  std::vector<deity>::iterator end() {
    return repo_.end();
  }
};

std::unique_ptr<deityRepo> deityRepo::instance_;

deityRepo::deityRepo() :
  pImpl_(new deityRepoImpl()) {}

std::vector<deity>::iterator deityRepo::begin() {
  return pImpl_->begin();
}
std::vector<deity>::iterator deityRepo::end() {
  return pImpl_->end();
}

void deityRepo::close() {
  instance_.reset(NULL);
}

//#include <iostream>

// retrieve a reference to a specific deity:
deity& deityRepo::getExact(const Element element, const Domination domination, const Outlook outlook) const {
  for (auto &d : *pImpl_) {
    if (d.element() == element && d.domination() == domination && d.outlook() == outlook)
      return d;
  }
  throw "Thine deities hath deserted ye. This is probably a bug.";
}

deityRepo& deityRepo::instance() {
  if (!instance_) {
    instance_.reset(new deityRepo());

    // Where do the gods live? Olympus?
    std::unique_ptr<deityRepoImpl> &olym = instance_->pImpl_;

    // Builder of the gods

    olym->emplace(Element::none, Domination::none, Outlook::none, L"Nonalignment", L"Agnosticism", L"\"If I could speak, I would say that I did not exist, if I existed.\"");
    olym->emplace(Element::earth, Domination::concentration, Outlook::kind, L"Lyrenna", L"the Hearth", L"that strong foundations make a sturdy house; a sturdy house makes a good home");
    olym->emplace(Element::air, Domination::concentration, Outlook::kind, L"Aenna", L"Community", L"that friendship and alligences make us stronger");
    olym->emplace(Element::fire, Domination::concentration, Outlook::kind, L"Rifasis", L"Warmth", L"of the safety and protection of the warding fire");
    olym->emplace(Element::water, Domination::concentration, Outlook::kind, L"Iarae", L"the Well", L"to bathe in the cooling, healing waters is nourishment for the soul");
    olym->emplace(Element::plant, Domination::concentration, Outlook::kind, L"Taqris", L"Trees", L"of bearing fruit for others when you yourself are weak");
    olym->emplace(Element::time, Domination::concentration, Outlook::kind, L"Imius", L"Contemplation", L"Patience");
    olym->emplace(Element::none, Domination::concentration, Outlook::kind, L"Ajl", L"the Teacher", L"Steadfastness");
    olym->emplace(Element::earth, Domination::aggression, Outlook::kind, L"Edes", L"Planting", L"Crop Sewing");
    olym->emplace(Element::air, Domination::aggression, Outlook::kind, L"Falco", L"Hunting", L"that the early bird catches the worm"); //       % NB: "Falco" is a genus; "Falcon" is any member species; ref:saying");
    olym->emplace(Element::fire, Domination::aggression, Outlook::kind, L"Ydite", L"Cautery", L"that which does not kill you makes you stronger");
    olym->emplace(Element::water, Domination::aggression, Outlook::kind, L"Baccal", L"Water of Life", L"never to start a barroom brawl, but always to end one"); // % ref:"Water of life" is the literal translation of many alcoholic spirits. ref:Babylon 5 (and probably elsewhere) for quote (was only "fighting")");
    olym->emplace(Element::plant, Domination::aggression, Outlook::kind, L"Subum", L"Brambles", L"that thorns deter predators, but leave berries for the truly needy");
    olym->emplace(Element::time, Domination::aggression, Outlook::kind, L"Etax", L"Saviours", L"that simple things are often the most helpful");
    olym->emplace(Element::none, Domination::aggression, Outlook::kind, L"Ejl", L"the Student", L"to do the right thing, whatever the cost");
    olym->emplace(Element::earth, Domination::none, Outlook::kind, L"Alorr", L"Escapology", L"Freedom to Build");
    olym->emplace(Element::air, Domination::none, Outlook::kind, L"Cyrena", L"Portents", L"Prophecy");
    olym->emplace(Element::fire, Domination::none, Outlook::kind, L"Fyana", L"Campfires", L"Mysteries of Parables");
    olym->emplace(Element::water, Domination::none, Outlook::kind, L"Nuagi", L"the Pool", L"the flow of compassion");
    olym->emplace(Element::plant, Domination::none, Outlook::kind, L"Edes", L"Apothecaries", L"Outward Beauty");
    olym->emplace(Element::time, Domination::none, Outlook::kind, L"Jibum", L"Rest", L"Practice");
    olym->emplace(Element::none, Domination::none, Outlook::kind, L"Kithus", L"Help", L"to do unto others as you would have them do unto you"); // %ref:saying";
    olym->emplace(Element::earth, Domination::concentration, Outlook::cruel, L"Myorr", L"Landslides", L"the Burial of Pain");
    olym->emplace(Element::air, Domination::concentration, Outlook::cruel, L"Kytix", L"Emu", L"Primal Screaming"); // % ref:Cuddles (from Orville & Cuddles)");
    olym->emplace(Element::fire, Domination::concentration, Outlook::cruel, L"Begesti", L"Weaponsmiths", L"the forging of metal");
    olym->emplace(Element::water, Domination::concentration, Outlook::cruel, L"Usses", L"Drowning", L"of big fish; Little fish"); // %ref:PJ Harvey, down by the water (song)");
    olym->emplace(Element::plant, Domination::concentration, Outlook::cruel, L"Qolios", L"Pitcher", L"of entrapment and lures");
    olym->emplace(Element::time, Domination::concentration, Outlook::cruel, L"Dun", L"Mistakes", L"to laugh first, to laugh longest"); // % ref:saying");
    olym->emplace(Element::none, Domination::concentration, Outlook::cruel, L"Batar", L"Torture", L"of pleasure and pain");
    olym->emplace(Element::earth, Domination::aggression, Outlook::cruel, L"Pandra", L"Rockfalls", L"to watch your footing; nothing is safe");
    olym->emplace(Element::air, Domination::aggression, Outlook::cruel, L"Byton", L"Harpy Eagle", L"victory for the strongest strike");
    olym->emplace(Element::fire, Domination::aggression, Outlook::cruel, L"Y'k", L"Inferno", L"War");
    olym->emplace(Element::water, Domination::aggression, Outlook::cruel, L"Rustia", L"Whirlpools", L"hidden strength");
    olym->emplace(Element::plant, Domination::aggression, Outlook::cruel, L"Zadon", L"Venus Trap", L"of the Food Chain");
    olym->emplace(Element::time, Domination::aggression, Outlook::cruel, L"Etix", L"Velocity", L"the best defence is a good offence"); // % ref:saying");
    olym->emplace(Element::none, Domination::aggression, Outlook::cruel, L"Vugdohr", L"Plunder", L"Rapacity");
    olym->emplace(Element::earth, Domination::none,  Outlook::cruel, L"Laqadrix", L"Dungeons", L"Slingshots and Imagination");
    olym->emplace(Element::air, Domination::none,  Outlook::cruel, L"Saeyr", L"Falling", L"Altitude and Attitude");
    olym->emplace(Element::fire, Domination::none,  Outlook::cruel, L"Kyrash", L"Immolation", L"Revenge");
    olym->emplace(Element::water, Domination::none,  Outlook::cruel, L"Vagjun", L"Drowning", L"Mysteries of the deep");
    olym->emplace(Element::plant, Domination::none,  Outlook::cruel, L"Aaos", L"Harvest", L"Reaping");
    olym->emplace(Element::time, Domination::none,  Outlook::cruel, L"Ynos", L"Age", L": it's later than you think"); // % ref:Enjoy yourself (1949 song, Sigman, Magidson)");
    olym->emplace(Element::none, Domination::none,  Outlook::cruel, L"Brutus", L"Greed", L"et tu"); // % ref:Shakespeare's "Et tu Brute" from /Julius Caesar/";
    olym->emplace(Element::earth, Domination::concentration, Outlook::none, L"Ildir", L"Strata", L"that firm foundations make a solid structure; a solid structure will stand forever");
    olym->emplace(Element::air, Domination::concentration, Outlook::none, L"Ibus", L"Gales", L"to scatter the ashes of your losses and move on");
    olym->emplace(Element::fire, Domination::concentration, Outlook::none, L"Kyreus", L"Crucible", L"Focus");
    olym->emplace(Element::water, Domination::concentration, Outlook::none, L"Byasis", L"Heavy Water", L"that bending prevents breaking; sometimes slower is stronger");
    olym->emplace(Element::plant, Domination::concentration, Outlook::none, L"Amlene", L"Undergrowth", L"of the strongest flowers which grow in the darkest forests");
    olym->emplace(Element::time, Domination::concentration, Outlook::none, L"Vibtrix", L"Enjoyment", L"that hedonism takes work, and lots of practise");
    olym->emplace(Element::none, Domination::concentration, Outlook::none, L"Wistus", L"Thought", L"to hold that thought"); // % ref:saying");
    olym->emplace(Element::earth, Domination::aggression, Outlook::none, L"Olene", L"Rock", L"that primitive methods can be just as effective");
    olym->emplace(Element::air, Domination::aggression, Outlook::none, L"Vedarr", L"Blowdarts", L"striking from afar");
    olym->emplace(Element::fire, Domination::aggression, Outlook::none, L"Fliash", L"Fireballs", L"concentrated blasts of power"); // %ref: AFF has "Filash", god of Fire");
    olym->emplace(Element::water, Domination::aggression, Outlook::none, L"Wobdar", L"Waterfalls", L"that the best ride is often the one closest to being dashed on the rocks");
    olym->emplace(Element::plant, Domination::aggression, Outlook::none, L"Xeusis", L"Clematis", L"that wealth comes from land");
    olym->emplace(Element::time, Domination::aggression, Outlook::none, L"Fivmer", L"the Moment", L"Opportunity");
    olym->emplace(Element::none, Domination::aggression, Outlook::none, L"Riotus", L"Beserkers", L"Meleé");
    olym->emplace(Element::earth, Domination::none, Outlook::none, L"Rockus", L"Firmament", L"Stability");
    olym->emplace(Element::air, Domination::none, Outlook::none, L"Brezus", L"Tornado", L"Sensation");
    olym->emplace(Element::fire, Domination::none, Outlook::none, L"Pyreus", L"Flame", L"Passion");
    olym->emplace(Element::water, Domination::none, Outlook::none, L"Hydrus", L"the 7+1 Seas", L"Flow"); // % ref: "the 7 seas" is an idiom; 7+1 is Pratchett's Discword");
    olym->emplace(Element::plant, Domination::none, Outlook::none, L"Florus", L"Growth", L"of the ecosystem");
    olym->emplace(Element::time, Domination::none, Outlook::none, L"Tempus", L"Eternity", L"to get your life in order"); // % ref: "in order" idiom, but also strict linear progression as time

  }
  return *instance_;
}
