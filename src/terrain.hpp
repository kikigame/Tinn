/* License and copyright go here*/

// terrain bits

#ifndef TERRAIN_HPP__
#define TERRAIN_HPP__

#include "renderable.hpp"
#include <memory> // shared_ptr

class monster;

enum class terrainType {
  ALTAR, // placed in shrines
  ROCK, // undiggable terrain.
  GROUND, // general movable areas. Currently, this means a room and is elegable to be replaced by a stairwell.
  UP, // up-going stairwell
  DOWN, // down-going stairwell
  // TRAP: as ground, but turns into a pit when non-flying monster encounters it
  PIT_HIDDEN,
  // TRAP: pit entraps non-flying players/monsters. Requires 4(ish) turns to escape, halved with fast-climb.
  // Falling into a pit instakills Zombies, else deals dPc()/10 bashing damage.
  PIT,
  // TRAP: a piano will fall on you the first time you step on it, then turns to ground
  PIANO_HIDDEN,
  FIRE, // fires of a campfire, fire-maze etc.
  WATER, // stream, river, moat, lake, sea
  SPACE, // vacuum, used with space zone. Chance of displaying as a L'*' for twinkling effect
  BULKHEAD,// outer hull of a ship or space station
  KNOTWEED, // impassible undergrowth.
  CRACK, // cracks in the wall; home to mokumokuren & portable holes won't stick
  // TRAP: as ground, but non-large monsters will be ensnared as pit, even if flying.
  WEB,
  // TRAP: monsters jumping onto a springboard will be flung forward by N squares
  SPRINGBOARD,
  SPRINGBOARD_HIDDEN,
  DECK
};

const wchar_t * const to_string(const terrainType &);

class terrain : public renderable {
private:
  friend class terrainFactoryImpl;
  const wchar_t render_;
  const wchar_t * const name_;
  const wchar_t * const description_;
  const terrainType type_;
  terrain(const wchar_t render, const wchar_t* const name,const wchar_t * const description, const terrainType type);
public:
  terrain(const terrain &other) = delete;
  terrain(terrain &&other) = delete;
  virtual ~terrain();
  virtual const wchar_t render() const;
  virtual std::wstring name() const;
  virtual std::wstring description() const;  

  // the type constant of the terrain
  terrainType type() const;

  // does the monster not want to move onte a square of this terrain type?
  bool entraps(const monster &m, bool includeHidden) const;

  bool operator ==(const terrain &other) const;
  bool operator !=(const terrain &other) const;
};

class terrainFactoryImpl;
class terrainFactory {
private:
  std::unique_ptr<terrainFactoryImpl> pImpl_;
public:
  terrainFactory();
  ~terrainFactory();
  const terrain &get(terrainType type) const;
};

static terrainFactory tFactory;

#endif //ndef TERRAIN_HPP__
