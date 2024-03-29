/* License and copyright go here*/

// Spirituality of Tinn.

#ifndef RELIGION_HPP__
#define RELIGION_HPP__

#include "renderable.hpp"
#include <memory>
#include <vector>

// element-like things. eg element::earth.
enum class Element {
    earth,
    air,
    fire,
    water,
    plant,
    time,
    none // none of the above
};

// fighting style
enum class Domination {
    // concentrate; know thy enemy; think before you make each move
    concentration,
    // fury; aggression; beserker attack
    aggression,
    none // of the above
};

// who comes first: you or others?
enum class Outlook {
    // do unto others. Generally be helpful, not damaging
    kind,
    // power and self-advancement over others
    cruel,
    none // of the above
};

class deityImpl;

// most spiritual paths are represented by a deity.
// all combinations exist.
// none/none/none is special, as it is for an nonaligned creature; there is a placeholder object for the "nonaligned" deity
// deities with two "
class deity : public renderable {
private:
  std::unique_ptr<deityImpl> pImpl_;
 public:
  deity(deityImpl* pImpl); // Calling code uses the deityRepo instead.
  // move constructor required by vector contract:
  deity(deity && rds);
  deity operator =(deity &); // no definition; disallow copying
  virtual ~deity();
  Element element() const;
  Domination domination() const;
  Outlook outlook() const;
  virtual const wchar_t render() const;
  virtual std::wstring name() const;
  virtual std::wstring description() const;
  const wchar_t * const house() const;

  /*
   * calculate and return the alignment factor with the given path.
   * 0 = unaligned; 3 = same path.
   * Always returns 0 for nonaligned.
   */
  unsigned char coalignment(const deity & path) const;
  /*
   * are we nonaligned?
   */
  bool nonaligned() const;
  /*
   * are these the same deity? (identity comparison)
   */
  bool operator ==(const deity &other) const;
  /*
   * how well respected is the player by this deity?
   */
  unsigned char alignCounter() const;
};

class deityRepoImpl;

// repository for deities.
class deityRepo {
private:
  std::unique_ptr<deityRepoImpl> pImpl_;
  static std::unique_ptr<deityRepo> instance_;
  deityRepo();
public:
  // retrieve a reference to the nonaligned deity:
  deity& nonaligned() const {return getExact(Element::none, Domination::none, Outlook::none);}

  // retrieve a reference to a specific deity:
  deity& getExact(const Element, const Domination, const Outlook) const;

  // returns an oppesed path.
  // Note that nonaligned opposes itself (nonaligned are never coaligned).
  // NB: non-aligned monsters will attack each other
  //  - so all monsters will attack an oppositely-aligned monster.
  deity& getOpposed(const deity &) const;
  
  // iterate over all deities:
  std::vector<deity>::iterator begin();
  std::vector<deity>::iterator end();

  static deityRepo& instance();

  static void close();

  unsigned char alignCounter(const deity &) const;
  void countAlign(const deity&, bool advance);

  // player's divinity score: how much do the gods like them (from 0 to 100)
  unsigned char divinity() const;
};

// used for religious items
class hasAlign {
public:
  virtual ~hasAlign() {};
  virtual const deity& align() const = 0;
};

deity &rndAlign(); // defined in shrine.cpp

#endif //ndef RELIGION_HPP__
