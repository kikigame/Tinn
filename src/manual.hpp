/* License and copyright go here*/

// Text for the manuals
#ifndef MANUAL_HPP__
#define MANUAL_HPP__

namespace manual {
  enum class chapter {
		      NONE, // this page intentionally left blank!
		      strength,
		      fight,
		      dodge,
		      damage,
		      appearance,
		      melee,
		      ranged,
		      religion,
		      END
  };

  // retrieve the chapter name (may be nullptr)
  const wchar_t *const name(const chapter &c);
  // retrieve the chapter text (may be nullptr)
  const wchar_t *const chap(const chapter &c);
  // retrieve the advanced manual text, if any (else nullptr)
  const wchar_t *const advanced(const chapter &c);
}

#endif //ndef MANUAL_HPP__
