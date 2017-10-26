/* License and copyright go here*/

// define the dungeon

#include "level.hpp"
#include "player.hpp"
#include "time.hpp"

#include <ostream>
#include <vector>

class io;

// encapsulates a game of Tinn. The levels, and everything in them, are known in gaming terms as a "dungeon"
// because of D&D, a term which persists even for rogue-like games which do not in fact take place in dungeons.
class dungeon {
private:
  bool alive_;
  ::std::vector<std::unique_ptr<level> > level_; //[NUM_LEVELS+1]; // 0 not used for now; may choose to do something with it later
  int cur_level_;
  ::std::shared_ptr<player> player_; // the hero of the game
  ::std::shared_ptr<io> io_;
  //  ::time::callback refresher_; // redraw on player move -> done in main loop instead, in case something registered later changes the screen
public:
  // create the dungeon. Given IO object will be used for interaction.
  dungeon(::std::shared_ptr<io> io);
  dungeon(const dungeon &rhs) = delete;
  ~dungeon();
  dungeon & operator=(const dungeon &rhs) = delete;
  // is the game still going?
  bool alive() const;
  // output the dungeon to the interface
  void draw () const;
  // access the current level
  level & cur_level() { return *(level_[cur_level_]); }
  const level & cur_level() const { return *(level_[cur_level_]); }
  // access the player
  std::shared_ptr<player> pc();
  const std::shared_ptr<player> pc() const;
  int maxLevel() const;

  // travelling
  void upLevel();
  void downLevel();

  // user quits:
  void quit();

  // user dies: (or at least their character does):
  void playerDeath();
};
