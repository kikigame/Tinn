/* License and copyright go here*/

// define the player in the dungeon

#ifndef PLAYER_HPP__
#define PLAYER_HPP__

#include "monster.hpp"
#include "coord.hpp"

#include <string>

class level;
class playerBuilder;
class role;

class player : public monster {
private:
  const std::wstring name_;
  role &role_;
public:
  player(playerBuilder &builder);
  virtual ~player();
  virtual const wchar_t render() const ;
  virtual std::wstring name() const ;
  virtual std::wstring description() const ;
  virtual bool highlight() const { return true; };

  // is this monster computer-controlled?
  virtual bool isPlayer() const { return true; }

  void takeInventory();
  void equip();
  void drop(level &lvl);
  void use();

  const role &job() const { return role_; }
  role &job() { return role_; }

protected:
  virtual void death(bool allowCorpse);
private:
  // used internally to recurse inventory:
  void containerInventory(std::wstringstream &inv,
			  const std::wstring indent,
			  const item &i) const; 
};

class playerBuilder : public monsterBuilder {
  friend class player;
private:
  std::wstring name_;
  role * role_;
public:
  playerBuilder();
  virtual ~playerBuilder();
  void name(const std::wstring &n);
  void job(role & r);
  role & job();
  const role & job() const;
};

#endif // ndef PLAYER_HPP__
