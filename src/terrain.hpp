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
  PIT // TODO: climbing out of pits. Speed-climbing.
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
  virtual ~terrain();
  virtual const wchar_t render() const;
  virtual const wchar_t * const name() const;
  virtual const wchar_t * const description() const;  

  // the type constant of the terrain
  terrainType type() const;

  // can the given monster move onto a square of this terrain type?
  bool movable(const std::shared_ptr<monster> m) const { return movable(*(m.get())); }
  bool movable(const monster &m) const;

  // does the monster not want to move onte a square of this terrain type?
  bool entraps(const monster &m, bool includeHidden) const;
};

class terrainFactoryImpl;
class terrainFactory {
private:
  std::unique_ptr<terrainFactoryImpl> pImpl_;
public:
  terrainFactory();
  ~terrainFactory();
  std::shared_ptr<terrain> get(terrainType type) const;
};

static terrainFactory tFactory;

#endif //ndef TERRAIN_HPP__
