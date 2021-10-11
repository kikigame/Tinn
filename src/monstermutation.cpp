/* License and copyright go here*/

// define mutated types of monsters

#include "monstermutation.hpp"
#include "damage.hpp"
#include "monsterType.hpp"
#include "renderable.hpp"
#include "terrain.hpp"

class mutationImpl {
private:
  const mutationType t_;
public:
  mutationImpl(mutationType t) : t_(t) {};
  virtual ~mutationImpl() {}
  bool spreadsOnAttack(const monsterType &t) const {return false;}
  bool spreadsOnKill(const monsterType &t) const  {return false;}
  bool spreadsOnCorpse(const monsterType &t) const  {return false;}
  virtual const wchar_t *prefix() const = 0;
  bool operator ==(const mutation &o) {
    return t_ == o.type();
  }
  bool operator <(const mutation &o) {
    return t_ < o.type();
  }
  bool operator >(const mutation &o) {
    return t_ > o.type();
  }
  const mutationType &type() const {
    return t_;
  }
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const = 0;
  virtual bool appliesTo(const monsterType &t) const {
    return true;
  }
};

class delegatingAbilities : public monsterAbilities {
 private:
  std::shared_ptr<monsterAbilities> d_;
 public:
  delegatingAbilities(std::shared_ptr<monsterAbilities> &delegate) : d_(delegate) {}
  virtual ~delegatingAbilities() {}
  virtual void proof(const damage & type, const bool isProof) { d_->proof(type, isProof); }
  virtual const bool proof(const damage & type) const { return d_->proof(type);  }
  virtual void eatVeggie(const bonus & isBonus) { d_->eatVeggie(isBonus);  }
  virtual const bonus eatVeggie() const {  return d_->eatVeggie();  }
  virtual void dblAttack(const bonus & isDblAttack) {  d_->dblAttack(isDblAttack);  }
  virtual const bonus dblAttack() const { return d_->dblAttack();  }
  virtual void resist(const damage * type, char level) { d_->resist(type, level);  }
  virtual const char resist(const damage & type) const { return d_->resist(type);  }
  virtual void extraDamage(const damage * type, char level) { d_->extraDamage(type, level);  }
  virtual const char extraDamage(const damage & type) const { return d_->extraDamage(type); }
  virtual void move(const terrain & type, const bool isMove) { d_->move(type, isMove); }
  virtual const bool move(const terrain & type) const { return d_->move(type); }
  virtual void sense(const sense::sense &t, bool s) { d_->sense(t,s); }
  virtual const bool hasSense(const sense::sense &t) const { return d_->hasSense(t); };
  virtual void fly(const bool canFly) { d_->fly(canFly); }
  virtual const bool fly() const { return d_->fly(); }
  virtual void fearless(const bonus &fearless) { d_->fearless(fearless); }
  virtual const bonus fearless() const { return d_->fearless(); }
  virtual void entrap(const int ticksToEscape) { d_->entrap(ticksToEscape); }
  virtual const bool entrapped() const { return d_->entrapped(); }
  virtual void climb(const bonus & canClimb) { d_->climb(canClimb); }
  virtual const bonus climb() const { return d_->climb(); }
  virtual void speedy(const bonus & fast) { d_->speedy(fast); }
  virtual const bonus speedy() const { return d_->speedy(); }
  virtual speed adjust(const speed & fastness) { return d_->adjust(fastness); }
  virtual void throws(const bool lobs) { d_->throws(lobs); }
  virtual const bool throws() const { return d_->throws(); }
  virtual void zap(const bool useWands) { d_->zap(useWands); }
  virtual const bool zap() const { return d_->zap(); }
  virtual void sleeps(const bool snoozy) { d_->sleeps(snoozy); }
  virtual const bool sleeps() const { return d_->sleeps(); }
  virtual void carryWeightN(const int n) { d_->carryWeightN(n); } // NB: Mutator is permanent.
  virtual const int carryWeightN() const { return d_->carryWeightN(); }
};

class vampireAbilities : public delegatingAbilities {
 public:
  vampireAbilities(std::shared_ptr<monsterAbilities> &delegate) : delegatingAbilities(delegate) {}
  virtual ~vampireAbilities() {}
  virtual const bool proof(const damage & type) const {
    if (type.type() == damageType::hot) return false;
    if (type.type() == damageType::cold) return true;
    return delegatingAbilities::proof(type);
  }
  virtual const bonus eatVeggie() const { return bonus(false); }
  virtual const char resist(const damage & type) const {
    if (type.type() == damageType::hot) return -50;
    if (type.type() == damageType::cold) return 50;
    return delegatingAbilities::resist(type);
  }
  virtual const char extraDamage(const damage & type) const {
    char rtn = delegatingAbilities::extraDamage(type);
    if (type.type() == damageType::bashing) return rtn < 0 ? 50 : rtn * 2;
    return rtn;
  }
  virtual const bonus speedy() const {
    return bonus(true);
  }
  virtual speed adjust(const speed & fastness) {
    speed f = delegatingAbilities::adjust(fastness);
    return f == speed::stop || f == speed::slow3 || f == speed::slow2  || f == speed::perturn ?
      speed::turn2 : f;
  }
  virtual bool appliesTo(const monsterType &t) const {
    // can't turn the dead:
    return !t.undead();;
  }
};

class cyberAbilities : public delegatingAbilities {
 public:
  cyberAbilities(std::shared_ptr<monsterAbilities> &delegate) : delegatingAbilities(delegate) {}
  virtual ~cyberAbilities() {}
  virtual const bool fearless() { return true; }
  virtual const bool swim() { return true; }
  virtual const bool fly() { return true; }

  virtual const bool proof(const damage & type) const {
    if (type.type() == damageType::wet) return false;
    if (type.type() == damageType::electric) return true;
    if (type.type() == damageType::disintegration) return true;
    return delegatingAbilities::proof(type);
  }
  virtual const bonus eatVeggie() const { return bonus(false); } // does not eat
  virtual const char resist(const damage & type) const {
    if (type.type() == damageType::wet) return -50;
    if (type.type() == damageType::electric) return 50;
    if (type.type() == damageType::disintegration) return 50;
    return delegatingAbilities::resist(type);
  }
  virtual const char extraDamage(const damage & type) const {
    char rtn = delegatingAbilities::extraDamage(type);
    if (type.type() == damageType::electric) return rtn < 0 ? 50 : rtn * 2;
    return rtn;
  }
  virtual const bonus speedy() const {
    return bonus(true);
  }
  virtual speed adjust(const speed & fastness) {
    speed f = delegatingAbilities::adjust(fastness);
    return f == speed::stop || f == speed::slow3 || f == speed::slow2  || f == speed::perturn ?
      speed::turn2 : f;
  }
};


class wereAbilities : public delegatingAbilities {
 public:
  wereAbilities(std::shared_ptr<monsterAbilities> &delegate) : delegatingAbilities(delegate) {}
  virtual ~wereAbilities() {}
  virtual const bonus fearless() const { return true; }
  virtual const bool swim() const { return false; } // cannot cross water
  virtual const bonus eatVeggie() const { return bonus(false); } // eat meat
  virtual const char resist(const damage & type) const {
    if (type.type() == damageType::hot) return -50;
    if (type.type() == damageType::cold) return 50;
    return delegatingAbilities::resist(type);
  }
  virtual const char extraDamage(const damage & type) const {
    char rtn = delegatingAbilities::extraDamage(type);
    if (type.type() == damageType::bashing) return rtn < 0 ? 50 : rtn * 2;
    return rtn;
  }
  virtual const bonus speedy() const {
    return bonus(true);
  }
  virtual speed adjust(const speed & fastness) {
    speed f = delegatingAbilities::adjust(fastness);
    return f == speed::stop || f == speed::slow3 || f == speed::slow2  || f == speed::perturn ?
      speed::perturn : f;
  }
};

class spaceAbilities : public delegatingAbilities {
public:
  spaceAbilities(std::shared_ptr<monsterAbilities> &delegate) : delegatingAbilities(delegate) {}
  virtual ~spaceAbilities() {}
  virtual const bool fly() const { return true; }
  virtual const bool move(const terrain & type) const {
    if (type.type() == terrainType::SPACE) return true;
    return delegatingAbilities::move(type);
  }
};

class ghostAbilities : public delegatingAbilities {
public:
  ghostAbilities(std::shared_ptr<monsterAbilities> &delegate) : delegatingAbilities(delegate) {}
  virtual ~ghostAbilities() {}
  virtual const bool proof(const damage & type) const {
    return true; // proof to all damage (except from blessed items, which happen separately)
  }
  virtual const bonus eatVeggie() const { return false; }
  virtual const char resist(const damage & type) const { return 100; }
  virtual const char extraDamage(const damage & type) const { return -100; }
  virtual const bool move(const terrain & type) const { return true; }
  virtual const bool hear() const { return false; }
  virtual const bool see() const { return false; }
  virtual const bool fly() const { return false; }
  virtual const bonus fearless() const { return true; }
  virtual const bool entrapped() const { return false; }
  virtual const bonus climb() const { return true; } // not entrapped in pits
  virtual const bonus speedy() const { return false; }
  virtual const bool zap() const { return false; }
  virtual const bool sleeps() const { return false; }
  virtual const int carryWeightN() const { return 0; }
  virtual speed adjust(const speed & fastness) {
    return speed::slow3;
  }
  virtual bool appliesTo(const monsterType &t) const {
    return !t.undead(); // unded can't come back as ghosts
  }
};

class vampire : public mutationImpl {
public:
  vampire() : mutationImpl(mutationType::VAMPIRE) {}
  virtual ~vampire() {}
  bool spreadsOnCorpse() const { return true; }
  virtual const wchar_t *prefix() const { return L"vampyric ";}  
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const {
    return std::make_shared<vampireAbilities>(a);
  };
};

class cyber : public mutationImpl {
public:
  cyber() : mutationImpl(mutationType::CYBER) {}
  virtual ~cyber() {}
  bool spreadsOnKill() const { return true; }
  virtual const wchar_t *prefix() const { return L"cyber";}
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const {
    return std::make_shared<cyberAbilities>(a);
  };
};

class were : public mutationImpl {
public:
  were() : mutationImpl(mutationType::WERE) {}
  virtual ~were() {}
  bool spreadsOnAttack() const { return true; }
  virtual const wchar_t *prefix() const { return L"were-";}
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const {
    return std::make_shared<wereAbilities>(a);
  };
};

class space : public mutationImpl {
public:
  space() : mutationImpl(mutationType::SPACE) {}
  virtual ~space() {}
  virtual const wchar_t *prefix() const { return L"space ";}
  // non-contagious
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const {
    return std::make_shared<spaceAbilities>(a);
  };  
};

class ghost : public mutationImpl {
public:
  ghost() : mutationImpl(mutationType::GHOST) {}
  virtual ~ghost() {};
  virtual const wchar_t *prefix() const { return L"ghostly "; }
  virtual std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const {
    return std::make_shared<ghostAbilities>(a);
  };
};

mutation::mutation(mutationImpl *other) :
  pImpl_(other) {}

mutation::~mutation() {}

bool mutation::spreadsOnAttack(const monsterType &t) const {
  return pImpl_->spreadsOnAttack(t);
}
bool mutation::spreadsOnKill(const monsterType &t) const  {
  return pImpl_->spreadsOnKill(t);
}
bool mutation::spreadsOnCorpse(const monsterType &t) const  {
  return pImpl_->spreadsOnCorpse(t);
}
bool mutation::operator ==(const mutation &o) const {
  return *pImpl_ == o;
}
bool mutation::operator <(const mutation &o) const {
  return *pImpl_ < o;
}
bool mutation::operator >(const mutation &o) const {
  return *pImpl_ > o;
}
const mutationType &mutation::type() const {
  return pImpl_->type();
}
const wchar_t *mutation::prefix() const {
  return pImpl_->prefix();
}
std::shared_ptr<monsterAbilities> mutation::wrap(std::shared_ptr<monsterAbilities> &a) const {
  return pImpl_->wrap(a);
}
bool mutation::appliesTo(const monsterType &t) const {
  return pImpl_->appliesTo(t);
}
mutationFactory &mutationFactory::instance() {
  static mutationFactory instance;
  return instance;
}


class mutationEncyclopedium : public renderable {
 private:
  const wchar_t render_;
  const std::wstring name_;
  const std::wstring desc_;
 public:
  mutationEncyclopedium(const wchar_t render,
			const std::wstring name,
			const std::wstring desc) :
    render_(render),
    name_(name),
    desc_(desc) {}
  virtual ~mutationEncyclopedium() {}
  virtual const wchar_t render() const {
    return render_;
  }
  virtual std::wstring name() const {
    return name_;
  }
  virtual std::wstring description() const {
    return desc_;
  }
};


mutationFactory::mutationFactory() :
  map_() {
  map_.emplace(mutationType::VAMPIRE, std::unique_ptr<mutation>(new mutation(new vampire())));
  map_.emplace(mutationType::CYBER, std::unique_ptr<mutation>(new mutation(new cyber())));
  map_.emplace(mutationType::WERE, std::unique_ptr<mutation>(new mutation(new were())));
  map_.emplace(mutationType::SPACE, std::unique_ptr<mutation>(new mutation(new space())));
  map_.emplace(mutationType::GHOST, std::unique_ptr<mutation>(new mutation(new ghost())));
  static mutationEncyclopedium encyclopedia[] = {
    {L'V',L"Vampyre",L"Vampyres feed on blood, the vital force of the living.\n"
     "The term is a relatively modern nomenclanture given to supernatioral\n"
     "revanants - the animated remains of the dead - of suicide victims or\n"
     "other evildoers.\n"
     "The best form of defence is mass public executions as they can be very\n"
     "hard to detect. However, they may appear as per living beings, bloated\n"
     "or ruddish in appearance, without the usual signs of vitality. The\n"
     "disappearance of local livestock, neighbours, relatives or loved ones\n"
     "may also be an early warning sign.\n"
     "To destroy a vampire, pierce the skin with a stake with ash, hawthorne,\n"
     "oak or aspen. Then bury it with the decapitated head between its feet.\n"
    },
    {L'C',L"Cyber",L"Mondas calls whence they hail; the twin planet of Earth,\n"
      "driven hence to travel the Universe. The cyber folk are unique for\n"
      "their method of reproduction, in which they take an otherwise\n"
      "unremarkable living human and augment it with various technological\n"
      "improvements until it becomes like them. Their lack of emotion seems\n"
      "not to soften their militant silvery nature. Gold is known to affect\n"
      "the respiratory system of older models.\n"},
    {L'W',L"Lycanthrope",L"The lycanthrope, or werewolf, is the ultimate form\n"
     "of the skills of wolf-charming and wolf-riding. The ability to change\n"
     "ones' form into that of a lupine can be exceptionally useful.\n"
     "In Turkey, for instance, shamen in wolf-form are reveared.\n"
     "The symptoms of clinical lycanthropy provides lupine behavioural traits\n"
     "and a ravenous appitite, but it is not to be confused with the original\n"
     "meaning of werewolf.\n"
     "Other forms of werecreatures are seen, such as the Asian werecats.\n"
    },
    {L'S',L"Space",L"Space creatures are much like their gravity-bound\n"
     "counterparts, except that they seem to have developed a knack for\n"
     "flying, and seem quite at home in the inhospitable wasteland of space.\n"
    },
    {L' ',L"Shade",L"When a death occurs for a person or beast whose life is\n"
     "not in order, especially of a violent death or one in unfortunate\n"
     "circumstances, the spirit cannot rest. It remains in the mortal plane,\n"
     "until its work is complete, or it is otherwise exorcised to the next\n"
     "world.\n"
     "Shades can be vanquished temporarily with weapons blessed by at least\n"
     "one spiritual path. While they have no physical form, they may excrete\n"
     "a gooey ectoplasm that has many alchemical uses."}
  };
}

const mutation &mutationFactory::operator[](const mutationType &t) const {
  if (map_.find(t) == map_.end())
    throw t;
  return *map_.at(t);
};
