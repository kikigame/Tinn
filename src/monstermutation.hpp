/* License and copyright go here*/

// mutations of monsters in Tinn

#ifndef MONSTERMUTATION_HPP
#define MONSTERMUTATION_HPP

#include <memory>
#include <map>
#include "monsterIntrinsics.hpp"

enum class mutationType {
  VAMPIRE,
  CYBER,
  WERE, // TODO: should swap to alternate form? reseach.
  SPACE,
  END
};

class mutationImpl;
class mutationFactory;

class monsterType; // defined in monsterType.hpp

class mutation {
  friend class mutationFactory;
private:
  std::unique_ptr<mutationImpl> pImpl_;
  mutation(mutationImpl *pImpl);
public:
  mutation(const mutation &other) = delete;
  ~mutation();
  mutation &operator =(const mutation &other) = delete;
  bool spreadsOnAttack(const monsterType &t) const;
  bool spreadsOnKill(const monsterType &t) const;
  bool spreadsOnCorpse(const monsterType &t) const;
  bool operator ==(const mutation &o) const;
  bool operator <(const mutation &o) const;
  bool operator >(const mutation &o) const;
  const mutationType &type() const;
  const wchar_t *prefix() const;
  // adjust the intrinsics of the mosnster:
  std::shared_ptr<monsterAbilities> wrap(std::shared_ptr<monsterAbilities> &a) const;
  bool appliesTo(const monsterType &t) const;
};

class mutationFactory {
private:
  std::map<mutationType, std::unique_ptr<mutation>> map_;
public:
  static mutationFactory &instance();
  mutationFactory();
  const mutation &operator [](const mutationType &t) const;
};

#endif //ndef MONSTERMUTATION_HPP
