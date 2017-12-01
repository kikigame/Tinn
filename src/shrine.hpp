/* License and copyright go here*/

// room of a level assigned to a religious deity

#ifndef SHRINE_HPP__
#define SHRINE_HPP__

#include "zone.hpp"

class deity;
class monster;
class item;
class io;

/*
 * Shrines work by limiting entry based on religious alignment,
 * and accepting donations of consumables (%) to improve stats.
 *
 * A player may also take food from a shrine; this costs in religious alignment
 * but supplies food (which is assumed to be useful).
 */
class shrine : public zoneArea<monster>, public zoneArea<item> {
private:
  const coord ul_;
  const coord lr_;
  const deity &align_;
  const io &io_;
public:
  // construct a new shrine with the given rectangular coordinates
  // with random alignment
  shrine(const coord &ul, const coord &lr, const io &);
  // construct a new shrine with the given rectangular coordinates:
  shrine(const coord &ul, const coord &lr, const deity &, const io &);

  virtual ~shrine();

  // "Shrine of <foo>"
  std::wstring name() const;

  // determine if a given square contains this zone
  virtual bool contains(coord area);

  // NB "moving" means moving for a monster/player, or
  // being fired, thrown, kicked, etc for an item

  // called whenever entering the zone
// handles limitations on entering the zone
  virtual bool onEnter(std::shared_ptr<monster>, itemHolder &pev);
// handles dropping food from the zone
  virtual bool onEnter(std::shared_ptr<item>, itemHolder &pev);

// handles taking food from the zone
  virtual bool onExit(std::shared_ptr<item>, itemHolder &prev);

};


#endif //ndef SHRINE_HPP__
