/* License and copyright go here*/

// Things and stuff

#ifndef HAS_ADJECTIVES_HPP__
#define HAS_ADJECTIVES_HPP__

#include <vector>
#include <string>

/*
 * pure base class for things that supply adjectives.
 *
 * items have adjectives that are mostly generated.
 * monsters have adjectives, due to magical effects or perhaps as a special ability in future.
 * corpses are items made from monsters; they keep their monstrous adjectives.
 *
 * This is all so a glittery vampire can become a glittery corpse; it has no other function. Yet.
 */
class hasAdjectives {
public:
  virtual std::vector<std::wstring> adjectives() const = 0;
  virtual ~hasAdjectives() {}
};

#endif //ndef HAS_ADJECTIVES_HPP__
