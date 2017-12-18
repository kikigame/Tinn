/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#include "role.hpp"
#include <map>

class roleImpl {
private:
  const roleType type_;
  const wchar_t * const name_;
  const wchar_t * const startGameMessage_;
public:
  roleImpl(const roleType &type, 
	   const wchar_t* const name,
	   const wchar_t* const startGameMessage)
    : type_(type), name_(name), startGameMessage_(startGameMessage) {}
  const roleType type() const {
    return type_;
  }
  const wchar_t * const name() const {
    return name_;
  }
  const wchar_t * const startGameMessage() const {
    return startGameMessage_;
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
