/* License and copyright go here*/

// types of senses that a monster or player may have; filters for messages
// formatted messages based on player's sense (may also hook into monster senses in future)

#ifndef SENSE_HPP__
#define SENSE_HPP__

#include <memory>
#include <cstdint> // for uint8_t
#include <string>
#include "optionalRef.hpp"
class coord;

namespace sense {

  enum sense : unsigned uint8_t
    {
     NONE = 0x0,
     SIGHT = 0x1, /*you see it*/
     SOUND = 0x2, /*you hear it*/
     TOUCH = 0x4, /*you feel it (physical contact) */
     TASTE = 0x8, /*you taste it*/
     SMELL = 0x10, /*you smell it*/
     SIXTH = 0x20, /* you intuit it with confidence (in this game, religious) */
     TELE = 0x40, /* you sense it by telepathy*/
     MAG = 0x80, /* you sense a magnetic field (magnetosensation) */
     ANY = 0xFF
    };

  constexpr bool directional(const sense &s) {
    return (s & (SIGHT | SOUND | TELE | MAG)) != NONE;
  }
} // namespace sense


/*
 * We mant to be able to say any of these, filling in values for %s at some point: 
 * "(3,4) %s sneezes" (you can see and hear it)
 * "(3,4) You see %s sneeze" (deaf)
 * "(3,4) You hear %s sneeze" (blind)
 * "You sense %s sneezing" (sixth sense)
 *
 * the coords will be filled in by the output, which could use other forms of display as needed. 
 * The syntax could be:
 * d.msg() << (sense::SIGHT | sense::SOUND | sense::SIXTH) << L"%s sneezes"
 *         << sense::SIGHT << L"You see %s sneeze"
 *         << sense::SOUND << L"You hear %s szeeze"
 *         << sense::SIXTH << L"You sense %s sneezing"
 *         << monster.name()
 *         << msg::endl;
 */

class formatterImpl;
class dungeon;
class longMsg {};
class stop {};

class formatter {
private:
  std::shared_ptr<formatterImpl> impl_;
public:
  const stop end;
  formatter(const dungeon &);
  ~formatter();
  formatter& operator << (const coord &); // location the message relates to
  formatter& operator << (const sense::sense &); // filter message type for next string
  formatter& operator << (const std::wstring &); // format or argument
  formatter& operator << (const long &); // pause after output; use bigger dialog
  formatter& operator << (const stop &); // do the output
};


#endif //ndef SENSE_HPP__
