/* License and copyright go here*/

// Where aliens come from. Ref: Elite (BBC Micro)

#ifndef ALIEN_HPP
#define ALIEN_HPP

#include "religion.hpp"
#include "renderable.hpp"

#include <string>


/*
 * Elite uses 3x16-bit words to define galaxies.
 * I can't be bothered to make it the same.
 * Note that most of the Elite code, translated from BBC Assembler to C, is available at http://www.iancgbell.clara.net/elite/text/, without license.
 *
 * I've not upcased the strings, as I don't want to accidentally return Zog in upper-case (a far-right conspiracy theory I don't want to give credence to by referencing).
 */

namespace alien {

  class worldFactory;
  
  // an alien world, spaceship or other origin
  class world : public renderable, public hasAlign {
  private:
    const std::wstring desc_;
    const std::wstring name_;
    const deity &align_;
    world(worldFactory &); // use static factory methods
  public:
    virtual ~world() {};
    // renderable:
    virtual const wchar_t render() const;
    virtual std::wstring name() const;
    virtual std::wstring description() const;
    // hasAlign:
    virtual const deity& align() const;

    static world spawn();
    static world spawn(uint8_t w,uint8_t x,uint8_t y,uint8_t z);
  };
  
  


  
} // namespace alien

std::wstring wordWrap(const std::wstring &str); // TODO: find a better utility place

#endif  //ndef ALIEN_HPP
