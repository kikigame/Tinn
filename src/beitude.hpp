/* License and copyright go here*/

// blessed and cursed status for objects, actions etc.

#ifndef BEITUDE_HPP
#define BEITUDE_HPP

// blessed/cursed/uncursed data
class bcu {
private:
  bool blessed_;
  bool cursed_;
public:
  bcu() : blessed_(false), cursed_(false) {}
  bcu(bool blessed, bool cursed) :
    blessed_(blessed), cursed_(cursed) {}
  bcu(const bcu &) = default;
  bool blessed() const { return blessed_; }
  bool cursed() const { return cursed_; }
  bcu operator !() const {
    return bcu(!blessed_, !cursed_);
  }
};

#endif //ndef BEITUDE_HPP
