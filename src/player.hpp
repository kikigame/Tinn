/* License and copyright go here*/

// define the player in the dungeon

#ifndef PLAYER_HPP__
#define PLAYER_HPP__

#include "monster.hpp"
#include "coord.hpp"

#include <string>

class level;
class playerBuilder;
class io;

class player : public monster {
private:
  const std::wstring name_;
  // how much food have we eaten?
  int foodLevel_;
  const io* io_;
public:
  player(playerBuilder &builder);
  virtual ~player();
  virtual const wchar_t render() const ;
  virtual const wchar_t * const name() const ;
  virtual const wchar_t * const description() const ;
  virtual bool highlight() const { return true; };

  // is this monster computer-controlled?
  virtual bool isPlayer() const { return true; }

  void takeInventory();
  void equip();
  void drop(level &lvl);
  void use();

  // overridden to inform the user:
  virtual const wchar_t* const fall(unsigned char reductionPc);

protected:
  virtual void death();
private:
  // used internally to recurse inventory:
  void containerInventory(std::wstringstream &inv,
			  const std::wstring indent,
			  const std::shared_ptr<item> &i) const; 
};

class playerBuilder : public monsterBuilder {
  friend class player;
private:
  std::wstring name_;
  const role * role_;
  const io * io_;
public:
  playerBuilder();
  virtual ~playerBuilder();
  void name(const std::wstring &n);
  void job(const role & r);
  const role & job();
  void ios(const io * ios);
};

#endif // ndef PLAYER_HPP__
