/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#include "role.hpp"
#include "quest.hpp"
#include <map>

class roleImpl {
private:
  const roleType type_;
  const wchar_t * const name_;
  const wchar_t * const startGameMessage_;
  std::vector<quest> quests_;
public:
  roleImpl(const roleType &type, 
	   const wchar_t* const name,
	   const wchar_t* const startGameMessage) :
    type_(type), name_(name),
    startGameMessage_(startGameMessage),quests_(questsForRole(type)) {}
  roleImpl(const roleImpl &) = delete;
  roleImpl(roleImpl &&) = delete;
  const roleType type() const {
    return type_;
  }
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const startGameMessage() const {
    return startGameMessage_;
  }
  std::vector<quest>::iterator questsBegin() { return quests_.begin(); }
  std::vector<quest>::const_iterator questsBegin() const { return quests_.begin();  }
  std::vector<quest>::iterator questsEnd() { return quests_.end();  }
  std::vector<quest>::const_iterator questsEnd() const { return quests_.end();  }
};


role::role(roleImpl *pImpl)
  : pImpl_(pImpl) {};

const wchar_t * const role::name() const {
  return pImpl_->name();
}
const wchar_t * const role::startGameMessage() const {
  return pImpl_->startGameMessage();
}
roleType role::type() const {
  return pImpl_->type();
}
std::vector<quest>::iterator role::questsBegin() { return pImpl_->questsBegin(); }
std::vector<quest>::const_iterator role::questsBegin() const { return pImpl_->questsBegin();  }
std::vector<quest>::iterator role::questsEnd() { return pImpl_->questsEnd();  }
std::vector<quest>::const_iterator role::questsEnd() const { return pImpl_->questsEnd();  }


class roleRepoImpl {
private:
  std::map<roleType,role> repo_;
public:
  roleRepoImpl() {
    emplace(new roleImpl(roleType::warrior, L"warrior",
			 L"Contender Ready!" // ref: Gladiators (TV gameshow)
			 ));
    emplace(new roleImpl(roleType::shopkeeper, L"shopkeeper",
			 L"As if by magic, the shopkeeper appeared." // Ref: Mr Benn
			 ));
  }
  void emplace(roleImpl *r) {
    repo_.emplace(r->type(), r);
  }
  role & get(const roleType &r) {
    return repo_.at(r);
  }
};

std::unique_ptr<roleRepo> roleRepo::instance_;


roleRepo::roleRepo()
  : pImpl_(new roleRepoImpl()) {};

  
role & roleRepo::operator[](const roleType &r) {
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
