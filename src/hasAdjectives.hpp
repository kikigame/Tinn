/* License and copyright go here*/

// Things and stuff

#ifndef HAS_ADJECTIVES_HPP__
#define HAS_ADJECTIVES_HPP__

#include <vector>
#include <string>
#include <algorithm>

/*
 * pure base class for things that supply adjectives.
 *
 * items have adjectives that are mostly generated.
 * monsters have adjectives, due to magical effects or perhaps as a special ability in future.
 * corpses are items made from monsters; they keep their monstrous adjectives.
 *
 */
class hasAdjectives {
public:
  virtual std::vector<std::wstring> adjectives() const = 0;
  bool hasAdjective(const std::wstring &adj) const {
    auto a = adjectives();
    return std::find(a.begin(), a.end(), adj) != a.end();
  }
  virtual ~hasAdjectives() {}
};

#endif //ndef HAS_ADJECTIVES_HPP__
