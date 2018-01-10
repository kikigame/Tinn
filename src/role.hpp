/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#ifndef __ROLE_HPP
#define __ROLE_HPP

#include <memory>

enum class roleType {
  warrior,
  shopkeeper
};

class level;
class levelImpl;
class levelGen;
class roleImpl;

class role {
private:
  std::unique_ptr<roleImpl> pImpl_;
public:
  role(roleImpl *);
  // name of this role
  const wchar_t * const name() const;  
  // message to user when starting the game
  const wchar_t * const startGameMessage() const;
  // per-role quest level:
  levelGen * newQuestLevelGen(levelImpl &, level &) const; // returns new instance
  roleType type() const;
};

class roleRepoImpl;

class roleRepo {
private:
  std::unique_ptr<roleRepoImpl> pImpl_;
  static std::unique_ptr<roleRepo> instance_;
  roleRepo();
public:
  const role & operator[](const roleType &) const;
  static roleRepo& instance();
  static void close();
};



#endif // ndef  __ROLE_HPP
