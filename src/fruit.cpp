/* License and copyright go here*/

#include "output.hpp"
#include "renderable.hpp"
#include "monsterType.hpp"
#include "items.hpp"
#include "random.hpp"
#include "monster.hpp"

namespace gamble {

  class renderChar : public renderable {
  private:
    wchar_t ch_;
    bool hi_;
  public:
    renderChar() : ch_(L'\0'), hi_(false) {} // required by container, but otherwise not used
    renderChar(wchar_t ch, bool highlight) : ch_(ch), hi_(highlight) {}
    renderChar(wchar_t ch) : ch_(ch), hi_(false) {}
    renderChar(const renderChar &c) = default;
    virtual ~renderChar() {}
    virtual const wchar_t render() const { return ch_; }
    virtual std::wstring name() const { return std::wstring({ch_, L'\0'}); }
    virtual std::wstring description() const { return std::wstring({ch_, '\0'}); }
    virtual bool highlight() const { return hi_; }
    renderChar &operator=(const renderChar &other) {
      ch_ = other.ch_;
      hi_ = other.hi_;
      return *this;
    }
  };
  
  class fruitMachine : public renderByCoord, private itemHolder {
  private:
    mutable std::map<wchar_t, renderChar> chars_; // mutable as cache of constant wrappers
    const bool blessed_, cursed_;
    enum class mode { ready, spin, spin_alt, result, finished };
    mode mode_;
    optionalRef<renderable> a_,b_,c_;
    monster &player_;
  public:
    fruitMachine(bool blessed, bool cursed, monster &player) :
      chars_(), blessed_(blessed), cursed_(cursed),
      mode_(mode::ready), a_(), b_(), c_(), player_(player) {}// = L"._|\/:="
  private:
    const renderChar &render(wchar_t c) const {
      return chars_[c] = renderChar(c);
    };
  public:
    void spin() {
      mode_ = mode_ == mode::spin ? mode::spin_alt : mode::spin;
    }
    void roll() {
      roll(a_);
      roll(b_);
      roll(c_);
      mode_ = mode::result;
    }
/*0
  1                                                                 ___
  2       .______________________________________________________. /   \2
          |......................................................| |   |3
          |......................................................| \___/4
          |......................................................|  \ / 5
          |......................................................|  | | 6
          |......................................................|  | | 7
          |................../=\......./=\......./=\.............|  | | 8
          |..................|:|.......|:|.......|:|.............|  | | 9
          |..................   .......   .......   .............|  | |10
          |..................---.......---.......---.............|  | |11
          |..................   .......   .......   .............|  | |12
          |.................. * ....... & ....... $ .............|  | |13
          |..................   .......   .......   .............|  | |14
          |..................---.......---.......---.............|--| |15
          |..................   .......   .......   .............|  : /16
          |..................|:|.......|:|.......|:|.............|---/ 17
          |..................\=/.......\=/.......\=/.............|
          |......................................................|
          |......................................................|__
          |.........................................................\
          |..................._____________________..................\
          |................../ : : : : : : : : : : \ .................|
          |.................|_______________________|.................|
*/
    // interface renderByCoord:
    virtual const renderable & renderableAt(const coord &p) const {
      const auto x = p.first;
      auto y = p.second+1;

      if ((x == 30 || x == 40 || x == 50) && y == 13)
	switch (mode_) {
	case mode::ready:
	  return render(L':');
	case mode::spin:
	  return render(L'/');
	case mode::spin_alt:
	  return render(L'\\');
	case mode::result:
	  if (x == 30) return a_.value();
	  if (x == 40) return b_.value();
	  if (x == 50) return c_.value();
	default:
	  return render(L'.');
	};

      if (mode_ == mode::result && y == 19) {
	const std::wstring n = a_.value().name();
	int len = n.length();
	int xStart = 30 - len/2;
	int xEnd = xStart + len;
	if (x >= xStart && x < xEnd)
	  return render(n[x-xStart]);
      }
      if (mode_ == mode::result && y == 20) {
	const std::wstring n = b_.value().name();
	int len = n.length();
	int xStart = 40 - len/2;
	int xEnd = xStart + len;
	if (x >= xStart && x < xEnd)
	  return render(n[x-xStart]);
      }
      if (mode_ == mode::result && y == 21) {
	const std::wstring n = c_.value().name();
	int len = n.length();
	auto xStart = 50 - len/2;
	int xEnd = xStart + len;
	if (x >= xStart && x < xEnd)
	  return render(n[x-xStart]);
      }
      
      if (x < 10) return render(L' ');
      if (x == 10 && y == 2) return render(L'.');
      //      if (x < 65 && y == 8) return render(L'.');
      if (x == 10 && y > 2) return render(L'|');
      if (x < 65 && y == 2) return render(L'_');
      if (x == 65 && y == 2) return render(L'.');
      if (x < 29) return render(L'.');
      if ((x == 28 || x == 52) && y == 24) return render(L'|');
      if (x < 61 && y < 8) return render(L'.');
      if ((x == 29 || x == 39 || x == 49) && y == 8) return render(L'/');
      if ((x == 30 || x == 40 || x == 50) && y == 8) return render(L'=');
      if ((x == 31 || x == 41 || x == 51) && y == 8) return render(L'\\');
      if ((x == 29 || x == 39 || x == 49) && (y == 9 || y == 17)) return render(L'|');
      if ((x == 30 || x == 40 || x == 50) && (y == 9 || y == 17)) return render(L':');
      if ((x == 31 || x == 41 || x == 51) && (y == 9 || y == 17)) return render(L'|');
      if ((x == 29 || x == 39 || x == 49) && y == 10) return render(L' ');
      if ((x == 30 || x == 40 || x == 50) && y == 10) return render(L' ');
      if ((x == 31 || x == 41 || x == 51) && y == 10) return render(L' ');
      if ((x == 29 || x == 39 || x == 49) && (y == 11 || y == 15)) return render(L'-');
      if ((x == 30 || x == 40 || x == 50) && (y == 11 || y == 15)) return render(L'-');
      if ((x == 31 || x == 41 || x == 51) && (y == 11 || y == 15)) return render(L'-');
      if ((x == 29 || x == 39 || x == 49) && (y == 10 || y == 12 || y == 14 || y == 16)) return render(L' ');
      if ((x == 30 || x == 40 || x == 50) && (y == 10 || y == 12 || y == 14 || y == 16)) return render(L' ');
      if ((x == 31 || x == 41 || x == 51) && (y == 10 || y == 12 || y == 14 || y == 16)) return render(L' ');
      if ((x == 29 || x == 39 || x == 49) && y == 18) return render(L'\\');
      if ((x == 30 || x == 40 || x == 50) && y == 18) return render(L'=');
      if ((x == 31 || x == 41 || x == 51) && y == 18) return render(L'/');
      if ((x == 29 || x == 31 || x == 39 || x == 41 || x == 49 || x == 51) && (y == 13)) return render(L' ');
      if (x >= 30 && x <= 50 && y == 22) return render(L'_');
      if (x >= 30 && x <= 50 && y == 23 && x % 2) return render(L':');
      if (x == 29 && y == 23) return render(L'/');
      if (x == 51 && y == 23) return render(L'\\');
      if (x >= 29 && x <= 51 && y == 23) return render(L' ');
      if (x >= 29 && x <= 51 && y == 24) return render(L'_');
      if ((x == 66 || x == 67) && y == 20) return render(L'_');
      if (x < 65) return render(L'.');
      if (x == 65 && y <= 20) return render(L'|');
      if (x < 67 && y == 20) return render(L'.');
      if (x == 68 && y == 21) return render(L'\\');
      if (x < 69 && y == 22) return render(L'.');
      if (x == 69 && y == 22) return render(L'\\');
      if (x < 69 && y > 20) return render(L'.');
      if (x < 70 && y > 21) return render(L'.');
      if (x == 70 && y > 22) return render(L'|');

      // NB: I've moved the knob down by 1 square, to avoid it being wiped
      // out by ynPrompt under the ncurses output.
      y--;
      // arm
      if (x >= 68 && x <= 70 && (y == 1 || y == 4)) return render(L'_');
      if (x == 67 && y == 2) return render(L'/');
      if (x == 71 && y == 2) return render(L'\\');
      if ((x == 67 || x == 71) && y == 3) return render(L'|');
      if (x == 67 && y == 4) return render(L'\\');
      if (x == 71 && y == 4) return render(L'/');
      if (x == 68 && y == 5) return render(L'\\');
      if (x == 70 && y == 5) return render(L'/');
      if ((x == 68 || x == 70) && y >= 6 && y <= 15) return render(L'|');
      if ((x == 66 || x == 67) && (y == 15 || y == 17)) return render(L'-');
      if (x == 68 && y == 16) return render(L':');
      if (x == 70 && y == 16) return render(L'/');
      if (x == 69 && y == 17) return render(L'/');
      if (x >= 66 && x <= 68 && y == 17) return render(L'-');

      return render(L' ');
    }
    virtual drawIter drawBegin() const {
      return drawIter(*this, coord(0,0), 72, 24);
    }
    virtual drawIter drawEnd() const {
      return drawIter(*this, coord(-1,-1), 72, 24);
    }
    // interface renderByCoord ends
  private:
    void roll(optionalRef<renderable> &x) {
      auto roll = dPc() / 5; // 20 slots per reel, as per traditional mechanical 1-armed bandit.
      switch (roll) {
      case 0: setItem(itemTypeKey::gpl_brick, x); break;
      case 1: setMonster(monsterTypeKey::incubus, x); break; // same chance as succubus
      case 2: setItem(itemTypeKey::shop_card, x); break;
      case 3: setItem(itemTypeKey::necklace, x); break;
      case 4: setItem(itemTypeKey::codex, x); break;
      case 5: setItem(itemTypeKey::poke, x); break;
      case 6: setItem(itemTypeKey::pop, x); break;
      case 7: setItem(itemTypeKey::bottle, x); break;
      case 8: setItem(itemTypeKey::lily, x); break;
      case 9: setItem(itemTypeKey::bow, x); break;
      case 10: setItem(itemTypeKey::lotus, x); break;
      case 11: setItem(itemTypeKey::conch, x); break;
      case 12: setMonster(monsterTypeKey::ferret, x); break;
      case 13: setMonster(monsterTypeKey::bull, x); break;
      case 14: setMonster(monsterTypeKey::kelpie, x); break;
      case 15: setMonster(monsterTypeKey::fox, x); break;
      case 16: setMonster(monsterTypeKey::hound, x); break;
      case 17: setMonster(monsterTypeKey::goblin, x); break;
      case 18: setMonster(monsterTypeKey::siren, x); break;
      case 19: setMonster(monsterTypeKey::succubus, x); break; // same chance as incubus
      default:
      case 20: setMonster(monsterTypeKey::dragon, x); break;
      }
    }

    void clear(optionalRef<renderable> &x) {
      if (x) {
	auto pI = dynamic_cast<item*>(&(x.value()));
	if (pI) destroyItem(*pI);
	// we don't clear the monsters
	x.reset();
      }
    }
    
    void setItem(const itemTypeKey &itk, optionalRef<renderable> &x) {
      auto &item = createItem(itk);
      if (blessed_ && dPc() < 25)
	player_.addItem(item);
      else
	addItem(item);
      clear(x);	
      x = optionalRef<renderable>(item);
    }
    void setMonster(const monsterTypeKey &mtk, optionalRef<renderable> &x) {
      level &level = player_.curLevel();
      const monsterType &type = monsterTypeRepo::instance()[mtk];
      auto monster = type.spawn(level);
      level.addMonster(monster, level.posOf(player_)); // TODO: random location?
      clear(x);
      x = optionalRef<renderable>(*monster);
      if (cursed_)
	level.addMonster(type.spawn(level), level.posOf(player_));
    }
    bool winOrLose() {
      // each roll is int[(2d51-2)/20]
      // the chances of two reels being the same are 57.56% (24.85% for 2 specific reels)
      // the chances of three reels being the same are 6.17%
      // that's not enough to keep the player interested, so we'll throw in a free spin if we get 2 the same.
      // if the user takes all free spins, this gives a chance of winning of 23.82% per inital roll (I think).
      wchar_t a = a_.value().render(); // currently, the render chars are different per slot.
      wchar_t b = b_.value().render();
      wchar_t c = c_.value().render();

      auto &ios = ioFactory::instance();
      if (a == b && b == c) {
	auto pA = dynamic_cast<item*>(&(a_.value()));
	if (a_.value().name().find(L"brick") != std::wstring::npos) {
	  ios.message(L" -- ! JACKPOT ! --");
	  ios.message(L" -- ! JACKPOT ! --");
	  ios.message(L" -- ! JACKPOT ! --");
	  ios.longMsg(L" You win the gold prize! ");
	  player_.addItem(*pA);
	  if (blessed_) {
	    auto pB = dynamic_cast<item*>(&(b_.value()));
	    if (pB) player_.addItem(*pB);
	    auto pC = dynamic_cast<item*>(&(c_.value()));
	    if (pC) player_.addItem(*pC);
	  }
	  return true; // done
	}
	ios.message(L" -- JACKPOT -- ");
	ios.longMsg(L" You win a latinum strip! ");
	player_.addItem(createItem(itemTypeKey::gpl_strip));
	return true; // done
      }
      if (a == b || b == c || a == c) {
	if (ios.ynPrompt(L" 2 out of 3 ... free spin?"))
	  mode_ = mode::spin; // skip the coin prompt
	return false; // prompt again
      }
      ios.longMsg(L"Dun cursed that roll."); // ref: the in-game deity of mistakes
      return false; // prompt again
    }
  public:
    void play(monster &p) {
      auto &ios = ioFactory::instance();
      ios.draw(*this);
      bool consent;
      do {
	if (mode_ != mode::spin) {
	  consent = 
	    ios.ynPrompt(L"Insert coin?");// ref: all arcade games
	  if (!consent) return;
	  auto coin =
	    p.firstItem([&p](item &i){
		return !i.highlight() &&
		i.name().find(L"coin") != std::wstring::npos;
	      });
	  if (!coin) {
	    ios.longMsg(L"Sorry; nothing you have seems to fit the slot.");
	    return;
	  }
	  p.destroyItem(coin.value());
	  spin();
	}
	ios.draw(*this);
	ios.longMsg(L"Things spin...");
	spin();
	ios.draw(*this);
	ios.longMsg(L"Things spin...");
	roll();
	ios.draw(*this);
	if (winOrLose()) return;
      } while (consent && mode_ != mode::finished);
    }
  };

} // namespace gamble

void playSlots(monster &p, bool blessed, bool cursed) {
  gamble::fruitMachine m(blessed, cursed, p);
  m.play(p);
}


