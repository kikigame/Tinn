/* License and copyright go here*/

// define mutated types of monsters

#include "monstermutation.hpp"
#include "damage.hpp"
#include "monsterType.hpp"

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
  virtual void hear(const bool hearing) { d_->hear(hearing); }
  virtual const bool hear() const { return d_->hear(); }
  virtual void see(const bool sight) { d_->see(sight); }
  virtual const bool see() const {return d_->see(); }
  virtual void swim(const bool canSwim) { d_->swim(canSwim); }
  virtual const bool swim() const { return d_->swim(); }
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
  virtual const bool fearless() { return true; }
  virtual const bool swim() { return false; } // cannot cross water
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

mutationFactory::mutationFactory() :
  map_() {
  map_.emplace(mutationType::VAMPIRE, std::unique_ptr<mutation>(new mutation(new vampire())));
  map_.emplace(mutationType::CYBER, std::unique_ptr<mutation>(new mutation(new cyber())));
  map_.emplace(mutationType::WERE, std::unique_ptr<mutation>(new mutation(new were())));
}

const mutation &mutationFactory::operator[](const mutationType &t) {
  return *map_[t];
};
