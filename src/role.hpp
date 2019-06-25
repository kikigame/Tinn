/* License and copyright go here*/

// specific features for player roles.
// I've kept this as the usual enum-key-and-repository pattern, as we may want monster roles in futurre.

#ifndef __ROLE_HPP
#define __ROLE_HPP

#include "quest.hpp"
#include "religion.hpp"
#include <memory>

class level;
class levelImpl;
class levelGen;
class roleImpl;
class player;

// roleType enum defined in quest.hpp to avoid cyclic dependency

class role {
private:
  std::unique_ptr<roleImpl> pImpl_;
public:
  role(roleImpl *);
  // name of this role
  const wchar_t * const name() const;  
  // message to user when starting the game
  const wchar_t * const startGameMessage() const;
  // per-role quest levels:
  std::vector<quest>::iterator questsBegin();
  std::vector<quest>::const_iterator questsBegin() const;
  std::vector<quest>::iterator questsEnd();
  std::vector<quest>::const_iterator questsEnd() const;
  roleType type() const;
  // called on on the starting player:
  void setupPlayer(player &);
  // inform the user about their quests
  std::wstring questText() const;
};

class roleRepoImpl;

class roleRepo {
private:
  std::unique_ptr<roleRepoImpl> pImpl_;
  static std::unique_ptr<roleRepo> instance_;
  roleRepo(const deity &align);
public:
  role & operator[](const roleType &);
  static roleRepo& instance(const deity &deity);
  static void close();
};



#endif // ndef  __ROLE_HPP
