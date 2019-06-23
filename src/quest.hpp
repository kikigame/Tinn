/* License and copyright go here*/

// individual quests. These are obtained from roles.  In the first
// version, each role will have one quest, but there may be other
// features in future.

#ifndef __QUEST_HPP
#define __QUEST_HPP

#include <memory>
#include <vector>

enum class roleType {
  warrior,
  thief,
  shopkeeper,
  crusader
};



class level;
class levelImpl;
class levelGen;
class player;
class questImpl;
class deity;

class quest {
private:
  std::shared_ptr<questImpl> pImpl_; // shared, not unique, to ease questsForRole() interface
public:
  quest(questImpl *);
  // name of this quest
  const wchar_t * const name() const;  
  // quest information for the user
  const wchar_t * const questData() const;
  // per-quest quest levels:
  bool isQuestLevel(int depth) const;
  levelGen * newQuestLevelGen(levelImpl &, level &) const; // returns new instance
  // called for each level:
  void setupLevel(levelGen &lg, level &l, int depth);
  // callon on the starting player:
  void setupPlayer(player &);
  // is the quest successful?
  bool isSuccessful() const;
  // Y/N confirm prompt used in case quest is incomplete.
  const wchar_t * const incompletePrompt() const;
  // quest complete message
  const wchar_t * const completeMsg() const;
};

// create the quests, used by role.cpp
std::vector<quest> questsForRole(const deity &, roleType);

#endif // ndef  __QUEST_HPP
