/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#include "role.hpp"
#include "level.hpp"
#include "levelGen.hpp"
#include "monster.hpp"
#include <map>

class roleImpl {
private:
  const roleType type_;
  const wchar_t * const name_;
  const wchar_t * const startGameMessage_;
  const std::function<levelGen*(levelImpl &, level &)> lg_;
public:
  roleImpl(const roleType &type, 
	   const wchar_t* const name,
	   const wchar_t* const startGameMessage,
	   const std::function<levelGen*(levelImpl &, level &)> lg)
    : type_(type), name_(name),
      startGameMessage_(startGameMessage),lg_(lg) {}
  const roleType type() const {
    return type_;
  }
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const startGameMessage() const {
    return startGameMessage_;
  }
  levelGen * newQuestLevelGen(levelImpl & li, level &l) const {
    return lg_(li, l);
  }
};


role::role(roleImpl *pImpl)
  : pImpl_(pImpl) {};

const wchar_t * const role::name() const {
  return pImpl_->name();
}
const wchar_t * const role::startGameMessage() const {
  return pImpl_->startGameMessage();
}
levelGen * role::newQuestLevelGen(levelImpl & li, level &l) const {
  return pImpl_->newQuestLevelGen(li,l);
}

// ref: Many, many slay-the-dragon myths. St George arguably, although
// he was a real Turkish Christian who never slew a dragon (martyred
// in 303AD for refusing to convert to the Roman religion).
class warriorQuestLevelGen : public levelGen {
public:
  warriorQuestLevelGen(levelImpl &li, level &l) :
    levelGen(&li,l) {}
  virtual ~warriorQuestLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual coord upRampPos() { return coord(2,10); }
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::GROUND);
    place(upRampPos(), terrainType::UP);
    //    place(coord(level::MAX_WIDTH-2,10), terrainType::DOWN);
    addMonster(monsterTypeKey::dragon, coord(level::MAX_WIDTH-2, 10));
  }
};

class shopQuestLevelGen : public levelGen {
public:
  shopQuestLevelGen(levelImpl &li, level &l) :
    levelGen(&li,l) {}
  virtual ~shopQuestLevelGen() {}
  virtual void negotiateRamps(optionalRef<levelGen> next) {}
  virtual coord upRampPos() { return coord(1,10); }
  virtual void build() {
    for (int x=1; x<level::MAX_WIDTH; ++x)
      for (int y=1; y<level::MAX_HEIGHT; ++y)
	place(coord(x,y), terrainType::GROUND);
    place(upRampPos(), terrainType::UP);
    //    place(coord(30,10), terrainType::DOWN);
    // TODO: This should be a difficult monster to trade with somehow.
    addMonster(monsterTypeKey::troll, coord(level::MAX_WIDTH-2, 10));
  }
};


class roleRepoImpl {
private:
  std::map<roleType,role> repo_;
public:
  roleRepoImpl() {
    emplace(new roleImpl(roleType::warrior, L"warrior",
			 L"Contender Ready!", // ref: Gladiators (TV gameshow)
			 [](levelImpl &li, level &l) { return new warriorQuestLevelGen(li, l); }
			 ));
    emplace(new roleImpl(roleType::shopkeeper, L"shopkeeper",
			 L"As if by magic, the shopkeeper appeared.", // Ref: Mr Benn
			 [](levelImpl &li, level &l) { return new shopQuestLevelGen(li, l); }
			 ));
  }
  void emplace(roleImpl *r) {
    repo_.emplace(r->type(), r);
  }
  const role & get(const roleType &r) const {
    return repo_.at(r);
  }
};

std::unique_ptr<roleRepo> roleRepo::instance_;


roleRepo::roleRepo()
  : pImpl_(new roleRepoImpl()) {};

  
const role & roleRepo::operator[](const roleType &r) const {
  return pImpl_->get(r);
}

roleRepo& roleRepo::instance() {
  if (!instance_)
    instance_.reset(new roleRepo()); // was empty
  return *instance_;
}

void roleRepo::close() {
  instance_.reset(NULL);
}
