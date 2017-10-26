/* License and copyright go here*/

// track time spent in the dungeon

#ifndef TIME_HPP
#define TIME_HPP

#include <memory>
#include <functional>

class timeImpl;

namespace temporal {
  class callback {
  private:
    const std::function<void()> callback_;
    const bool everyTick_;
  public:
    // consume the given callback
    // everyTick - true to register on tick, falso on player move
    callback (bool everyTick, const std::function<void()> &fn);
    // avoid copies:
    callback (const callback &rhs) = delete;
    // deregsiter the callback
    ~callback();
    // invoke the function:
    void operator()();
    // avoid copies:
    callback & operator =(const callback &rhs) = delete;
    // comparison for adding and removing
    bool operator ==(const callback &rhs);
  };
};

class time {
private:
  static std::unique_ptr<timeImpl> instance_;
public:
  typedef ::temporal::callback callback; 
  /*
   * The number of ticks that have elapsed in the game
   */
  static unsigned long long moveCount();
  /*
   * Called whenever something happens to advance the clock.
   * true - a player movement/action was *attempted*. All monsters get to move
   * false - something else caused time to move on. All monsters get to move, but the player won't be able to recact.
   */
  static void tick(bool isMove);
  /*
   * Register callbacks for player movement
   * I/O routines generally call this.
   */
  static void onPlayerMove( time::callback &onMove); // register
  static void offPlayerMove( time::callback &onMove); // unregister
  /*
   * Register callbacks for all time events.
   * Monsters generally call this.
   */
  static void onTick( time::callback &onMove); // register
  static void offTick( time::callback &onMove); // unregister
};


#endif // ndef TIME_HPP
