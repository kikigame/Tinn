/* License and copyright go here*/

// formatted messages based on player's sense

#include "coord.hpp"
#include "dungeon.hpp"
#include "optionalRef.hpp"
#include "output.hpp"
#include "random.hpp"
#include "religion.hpp"
#include "sense.hpp"

#include <vector>

class formatterImpl {
private:
  dungeon &dungeon_;
  sense::sense nextSense_;
  bool waitingSenseMsg_;
  optionalRef<const coord> loc_;
  unsigned char dtv_; // divination test value (lazy-evaluated)
  std::vector<sense::sense> sense_;
  std::vector<std::wstring> msg_;
  std::vector<std::wstring> args_;
public:
  formatterImpl(dungeon &d) :
    dungeon_(d),
    nextSense_(sense::ANY),
    waitingSenseMsg_(false),
    loc_(),
    dtv_(101) {}
  
  void nextSense(const sense::sense &s) {
    if (waitingSenseMsg_) throw "Sense message missing";
    nextSense_ = s;
    waitingSenseMsg_ = true;
  }

  void nextString(const std::wstring &s) {
    if (waitingSenseMsg_) {
      sense_.push_back(nextSense_);
      msg_.push_back(s);
    }
    else args_.push_back(s);
    waitingSenseMsg_ = false;
  }

  void loc(const coord &c) {
    loc_ = optionalRef<const coord>(c);
  }

  void publish() {
    auto pc = dungeon_.pc();
    auto a = pc->abilities();
    // first find the format message
    std::wstring buffer = L"";
    bool hasDir = false;
    if (loc_)
      for (auto s : sense_) {
	if (hasSense(s, a)) { hasDir = true; break; }
      }
    size_t id=0;
    for (auto s : sense_) {
      std::wstring &f = msg_[id++];
      if (hasSense(s, a)) {
	buffer = f;
	break;
      }
    }

    if (buffer != L"") {
      size_t idx=0;
      for (auto s : args_) {
	idx = buffer.find(L"%s", idx);
	if (idx == std::string::npos) throw std::wstring(L"Not enough arguments to ") + buffer;
	buffer.replace(idx, 2, s);
	idx += s.length();
      }
      // TODO: flags for longMessage, location (filtered by directional on matched type)
      if (hasDir)
	ioFactory::instance().message(loc_.value(), buffer);
      else
	ioFactory::instance().message(buffer);
    }
    
    sense_ = std::vector<sense::sense>();
    msg_ = std::vector<std::wstring>();
    args_ = std::vector<std::wstring>();
    waitingSenseMsg_ = false;
    dtv_ = 101;
    loc_ = optionalRef<const coord>();
  }

private:
  unsigned char divTest() {
    if (dtv_ > 100) dtv_ = dPc();
    return dtv_;
  }
  bool hasSense(sense::sense &s, const std::shared_ptr<monsterAbilities> &a) {
    return (s & sense::SIGHT && a->hasSense(sense::SIGHT)) ||
	  (s & sense::SOUND && a->hasSense(sense::SOUND)) ||
	  (s & sense::TOUCH && a->hasSense(sense::TOUCH)) || // NB: this filters for no gloves etc; see monsterAbilites.cpp
	  (s & sense::TASTE && a->hasSense(sense::TASTE)) ||
	  (s & sense::SMELL && a->hasSense(sense::SMELL)) ||
	  (s & sense::SIXTH && divTest() < deityRepo::instance().divinity()) ||
	  (s & sense::TELE && a->hasSense(sense::TELE)) ||
      (s & sense::MAG && a->hasSense(sense::MAG));
  }
};

formatter::formatter(dungeon &d) : impl_(new formatterImpl(d)), end() {}
formatter::~formatter() { delete impl_; }

formatter& formatter::operator << (const sense::sense &s) { // filter message type for next string
  impl_->nextSense(s); return *this;
}
formatter& formatter::operator << (const std::wstring &s) { // format or argument
  impl_->nextString(s); return *this;
}
formatter& formatter::operator << (const coord &c) { // coordinate of message
  impl_->loc(c); return *this;
}
formatter& formatter::operator << (const stop &s) { // do the output
  impl_->publish(); return *this;
}
