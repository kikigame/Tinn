/* License and copyright go here*/

// render a dreamspace "level", separate from the main dunguen.
// The player is always centred, but the world moves around them. Only the area around them is shown, in an infinite, ever-changing maze.

#include "dungeon.hpp"
#include "output.hpp"
#include "player.hpp"
#include "terrain.hpp"
#include "religion.hpp"
#include "random.hpp"
#include "items.hpp"

class holyBook; // defined in items.cpp

namespace morpheus {

  const int PLAYER_X = 35;
  const int PLAYER_Y = 11;


  class awaken{}; // thrown to wake up
  
  /*
   * coordinate direction flags
   */
  class nesw : private std::bitset<4> {
  public:
    nesw(bool n, bool s, bool e, bool w) :
      std::bitset<4>() {
      (*this)[3] = n;
      (*this)[2] = e;
      (*this)[1] = s;
      (*this)[0] = w; // lsb
    }
    nesw(bool n, bool s, bool e, bool w, std::size_t minExits) :
      std::bitset<4>() {
      (*this)[3] = n;
      (*this)[2] = e;
      (*this)[1] = s;
      (*this)[0] = w; // lsb
      int toAdd = rndPickI(minExits,static_cast<std::size_t>(4)) - count();
      for (int i = 0; i < toAdd; ++i) {
	while (true) {
	  int j = rndPickI(0, 3);
	  if (!(*this)[j]) (*this)[j] = true;
	  break; // stop on first match
	}
      }
    }
    bool n() const { return (*this)[3]; }
    void n(bool b) { (*this)[3] = b; }
    bool s() const { return (*this)[2]; }
    void s(bool b) { (*this)[2] = b; }
    bool e() const { return (*this)[1]; }
    void e(bool b) { (*this)[1] = b; }
    bool w() const { return (*this)[0]; }
    void w(bool b) { (*this)[0] = b; }
  private:
    static int rnd() {
      return rand() & 0b1111;
    }
  };

  class renderChar : public renderable {
    wchar_t c_;
  public:
    renderChar(const wchar_t c) : c_(c) {}
    renderChar(const renderChar &r) : c_(r.render()) {}
    virtual const wchar_t render() const { return c_; }
    virtual std::wstring name() const { return std::wstring() + c_; }
    virtual std::wstring description() const  { return std::wstring() + c_; }
    renderChar& operator=(const renderChar &r) { c_ = r.render(); return *this; }
    bool operator == (const wchar_t &r) const { return c_ == r; };
  };


  /*
   * We deal out a 3x3 array of different types of card to form the dungeon.
   */
  class dreamCard : public nesw {
  protected:
    terrainType pass_, wall_;
    dreamCard(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w, std::size_t minExits) :
      nesw(n,e,s,w, minExits),
      pass_(pass), wall_(wall) {}
    dreamCard(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w) :
      nesw(n,e,s,w),
      pass_(pass), wall_(wall) {}
  public:
    virtual ~dreamCard() {}
    virtual bool passable(const coord &c) const = 0;
    virtual optionalRef<item> itemAt(const coord &c) const {
      // default is no item
      return optionalRef<item>();
    }
    virtual terrainType terrainAt(const coord &c) const {
      // default is plain terrain
      return passable(c) ? pass_ : wall_;
    }
    virtual const renderable &render(const coord &c) const {
      // default is to show the item if any, else terrain
      auto item = itemAt(c);
      if (item) return item.value();
      return tFactory.get(terrainAt(c));
    }
    // called on moving from or within this zone; c is dest coord
    virtual void onMove(player &p, const coord &c) {}
  };


  /*
   * draw a corridor, like this:
   *
   * 1111           1101
   *   ###   ###     ###   ###
   *   ###   ###     ###   ###
   *   ##     ##     ##     ##
   *                       ###
   *       @             @ ###
   *                       ###
   *   ##     ##     ##     ##
   *   ###   ###     ###   ###
   *   ###   ###     ###   ###
   */
  class dreamCorridor : public dreamCard {
  public:
    dreamCorridor(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w, std::size_t minExits) :
      dreamCard(pass, wall, n, e, s, w, minExits) {}
    virtual ~dreamCorridor() {}
    virtual bool passable(const coord &c) const {
      if ((c.first == 2 || c.first == 6) && (c.second == 2 || c.second == 6))
	  return true; // make dead ends more obvious and corridor more ornate
      if (c.second < 3) return c.first >= 3 && c.first < 6 && n();
      if (c.first >= 6) return c.second >= 3 && c.second < 6 && e();
      if (c.second >= 6) return c.first >= 3 && c.first < 6 && s();
      if (c.first < 3) return c.second >= 3 && c.second < 6 && w();
      return true;
    }
  };

  /*
   * draw a room, like this:
   *
   * 1111           1101
   *  ###   ###   ###   ###    
   *  ##     ##   ##     ##
   *  #       #   #       #
   *                          
   *      @           @  
   *                        
   *  #       #   #       #
   *  ##     ##   ##     ##
   *  ###   ###   #########
   */
  class dreamRoom : public dreamCard {
    player &p_;
    std::map<coord, std::reference_wrapper<item>> items_;
  private:
    bool addHighlightedItem();
    bool addHighValueItem();
    bool addWeapon();
    bool addClothing();
    bool addTool();
    bool addCoalignedItem();
  public:
    dreamRoom(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w, std::size_t minExits, player &p) :
      dreamCard(pass, wall, n, e, s, w, minExits), p_(p), items_() {
      // dream items
      int roll = dPc();
      if (roll == 0 && addHighlightedItem()) return;
      if (roll < 25 && addHighValueItem()) return;
      switch (p.align().outlook()) {
      case Outlook::kind:
	if (roll < 50 && addWeapon()) return;
	break;
      case Outlook::cruel:
	if (roll < 50 && addClothing()) return;
	break;
      default:
	if (roll < 50 && addTool()) return;
      }
      if (roll < 75 && addCoalignedItem()) return;
    }
    virtual ~dreamRoom() {}
    virtual bool passable(const coord &c) const {
      if ((c.first == 1 || c.first == 7) && (c.second == 1 || c.second == 7))
	  return false; // round the corners because we can
      if (c.second == 0) return c.first >= 3 && c.first < 6 && n();
      if (c.first == 8) return c.second >= 3 && c.second < 6 && e();
      if (c.second == 8) return c.first >= 3 && c.first < 6 && s();
      if (c.first == 0) return c.second >= 3 && c.second < 6 && w();
      return true;
    }
    virtual const renderable &render(const coord &c) const {
      auto it = items_.find(c);
      if (it == items_.end()) return dreamCard::render(c);
      return it->second;
    }
    virtual void onMove(player &p, const coord &c) {
      auto it = items_.find(c);
      if (it == items_.end()) return;
      item &item = it->second;
      // try to pick up item
      auto name = item.name();
      auto desc = item.description();
      auto &io = ioFactory::instance();
      io.message(L"You dream of something...");
      std::wstring prefix;
      if (p_.addItem(item)) {
	prefix = L"You hold: ";
      } else {
	prefix = L"You see: ";
      }
      io.message(prefix + name);
      io.longMsg(desc);
      items_.erase(it);
    }
  private:
    bool addItem(std::function<bool(const item&)> filter) {
      auto item = itemHolderMap::instance().rndIf([this, filter](const ::item &i) { return !p_.contains(i) && filter(i); });
      if (!item) return false;
      coord pos(0,0);
      do {
	pos = coord(rndPickI(0, 9), rndPickI(0, 9));
      } while (!passable(pos));
      items_.emplace(pos, item.value());
      return true;
    }
  };
  bool dreamRoom::addHighlightedItem() {
    return addItem([](const item &i) { return i.highlight(); });
  }
  bool dreamRoom::addHighValueItem() {
    return addItem([](const item &i) { return i.render() == L'*' && i.weight() > 10; });
  }
  bool dreamRoom::addWeapon() {
    return addItem([](const item &i) { return i.render() == L'!' || i.render() == L'¬'; });
  }
  bool dreamRoom::addClothing() {
    return addItem([](const item &i) { return i.render() == L'['; });
  }
  bool dreamRoom::addTool() {
    return addItem([](const item &i) {
	static std::wstring chars(L"♪8=");
	return chars.find(i.render()) != chars.npos;
      });
  }
  bool dreamRoom::addCoalignedItem() {
    return addItem([this](const item &i) {
	auto pAligned = dynamic_cast<const hasAlign*>(&i);
	return pAligned && pAligned->align() == p_.align();
      });
  }

  /*
   * draw a basic shrine, like this: (all shrines have 1 exit)
   *
   * *********
   * *********
   * *********
   * ***   ***
   *      _***
   * ***   ***
   * *********
   * *********
   * *********
   */
  class dreamShrine : public dreamCard {
  private:
    coord shrinePos_;
  public:
    dreamShrine(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w) :
      dreamCard(pass, wall, n, e, s, w),
      shrinePos_(
		 n ? coord(4,5) :
		 e ? coord(3,4) :
		 s ? coord(4,3) :
		 coord(5,4) /*m*/
		 ) {
    }
    virtual ~dreamShrine() {}
  public:
    virtual bool passable(const coord &c) const {
      if (c == shrinePos_) return false;
      if ((c.first >=3 && c.first < 6) &&
	  (c.second >=3 && c.second < 6))
	return true;
      if (n()) return c.first == 4 && c.second < 3;
      if (e()) return c.second == 4 && c.first >= 6;
      if (s()) return c.first == 4 && c.second >= 6;
      if (w()) return c.second == 4 && c.first < 3;
      return false;
    }
    virtual const renderable &render(const coord &c) const {
      if (c == shrinePos_) return tFactory.get(terrainType::ALTAR);
      return dreamCard::render(c);
    }
  private:
    // everyone who sets foot in their dream temple gets the souvenir book.
    void presentWelcomePresent(player &p) const {
      if (0 == p.countIf([&p](item &i) {
	    auto pAlign = dynamic_cast<hasAlign*>(&i);
	    auto pBook = dynamic_cast<item*>(&i);
	    if (!pAlign || !pBook) return false;
	    // if they have any aligned books, bless or enchant them...
	    if (p.align() == pAlign->align()) {
	      if (pBook->isBlessed())
		pBook->enchant(1);
	      else pBook->bless(true);
	    }{
	      pBook->enchant(-1);
	      pBook->curse(true);
	    }
	    return true;
	  })) {
	// ... otherwise, they get a blessed book.
	auto &book = createHolyBook(p.align());
	book.bless(true);
	book.enchant(1);
	p.addItem(book);
      }
    }
    bool uncurseAll(player &p) {
      auto count = p.countIf([](item &i) {
	  auto matched = i.isCursed() && !dynamic_cast<hasAlign*>(&i);
	  if (matched) i.curse(false);
	  return matched;
	});
      return count > 0;
    }
    bool blessAll(player &p) {
      auto count = p.countIf([](item &i) {
	  auto matched = !i.isBlessed() && !dynamic_cast<hasAlign*>(&i);
	  if (matched) i.bless(true);
	  return matched;
	});
      return count > 0;      
    }
    bool enchantAll(player &p) {
      auto count = p.countIf([](item &i) {
	  auto e = i.enchantment();
	  i.enchant(1);
	  return e < i.enchantment();
	});
      return count > 0;      
    }
  public:
    virtual void onMove(player &p, const coord &c) {
      const io &io = ioFactory::instance();
      if (c.first == c.second && c.first == 4) {
	io.message(L"You stand by the altar in the dream shrine of " + p.align().name());
	std::wstring feel;
	switch (p.align().domination()) {
	case Domination::aggression: feel = L"fired up"; break;
	default:
	case Domination::concentration: feel = L"at peace"; break;
	}
	io.message(L"You feel " + feel + L" in the Sacred House of " + p.align().house());

	presentWelcomePresent(p);
	
	if (p.align().nonaligned()) {
	  io.longMsg(L"This dream is making less sense than usual.");
	  return;
	}
	std::wstring salute;
	switch (p.align().outlook()) {
	case Outlook::cruel: salute = L"\"My fearsome acolyte"; break;
	case Outlook::kind: salute = L"\"My brave child"; break;
	default: salute = L"\"My loyal follower"; break;
	}
	// some obvious Nethack-like temple effects
	if (uncurseAll(p) ||
	    blessAll(p) ||
	    enchantAll(p)) {
	  io.message(L"A vision of " + p.align().name() + L" appears in your dream.");
	  std::wstring instruct;
	  switch (p.align().outlook()) {
	  case Outlook::cruel: instruct = L"receive "; break;
	  case Outlook::kind: instruct = L"please accept "; break;
	  default: instruct = L"\"; here is "; break;
	  }
	  io.message(salute + L", " + instruct + L"my gift to aid you in your quest.");
	} else
	  io.message(salute + L"; you are indeed on the correct path.");
	io.message(L"\"Awaken now, and continue to do my work.\"");
	throw awaken();
      }
    }
  };

  /*
   *    E           W         N          S
   * *********  ********* ***   ***  *********
   * **   ****  ****   ** **** ****  ** *** **
   * *     ***  ***     * ***   ***  *   *   *
   * ** ┌─┐ *    * ┌─┐ ** ** ┌─┐ **  *  ┌─┐  *
   * ***│&│        │&│*** *  │&│  *  *  │&│  *
   * ** └─┘ *    * └─┘ ** *  └─┘  *  ** └─┘ **
   * *     ***  ***     * *   *   *  ***   ***
   * **   ****  ****   ** ** *** **  **** ****
   * *********  ********* *********  ***   ***
   *                      012345678
   */
  class boudoir : public dreamCard {
  private:
    std::shared_ptr<monster> bus_;
    static std::array<renderChar, 6> renders;
    bool warned_;
  public:
    boudoir(terrainType pass, terrainType wall, bool n, bool e, bool s, bool w, std::shared_ptr<monster> &bus) :
      dreamCard(pass, wall, n, e, s, w),
      bus_(bus),
      warned_(false) {}
    virtual ~boudoir() {}
    virtual bool passable(const coord &c) const {
      coord tc = nesw::n() ? c :
	nesw::s() ? coord(c.first, 8 - c.second) :
	nesw::e() ? coord(c.second, 8 - c.first) :
	/*w*/ coord(c.second, c.first);
      auto x = tc.first;
      switch (tc.second) {
      case 0: return x >= 3 && x <= 5;
      case 1: return x == 4;
      case 2: return x >= 3 && x <= 5;
      case 3: return x >= 2 && x <= 6;
      case 4:
      case 5: return x >= 1 && x <= 7; 
      case 6: return x != 0 && x != 4 && x != 8;
      case 7: return x == 2 || x == 6;
      default:
      case 8: return false;
      }
    }
    virtual const renderable &render(const coord &c) const {
      if (c.first < 3 || c.second < 3 || c.first >5 || c.second > 5)
	return dreamCard::render(c);
      int x = c.first;
      switch (c.second) {
      case 3: return x == 3 ? renders[0] : x == 4 ? renders[1] : renders[2];
      case 4: return x == 4 ? bus_ ? static_cast<renderable &>(*bus_) :
	static_cast<const renderable &>(tFactory.get(wall_)) : static_cast<renderable &>(renders[3]);
      default:
      case 5: return x == 3 ? renders[4] : x == 4 ? renders[1] : renders[5];
      }
      return dreamCard::render(c);
    }
    virtual void onMove(player &p, const coord &c) {
      if (!bus_) return;
      const io &io = ioFactory::instance();
      bool m = bus_->isMale();
      if (!warned_) {
	io.message(L"You enter an opulant bedchamber.");
	io.message(m ? L"A bewitching image of masculine charm lies alluringly on the sheets." :
		   L"A pulchritudinous image of feminine charm is draping herself desirably ower\nthe bed.");
	io.longMsg(L"WARNING: A " + bus_->name() + L" can be incredibly dangerous, even in dreams.\n"
		   "Proceed only with extreme caution.");
	warned_ = true;
      }
      if (c == coord(4,4)) {
	auto &action = m ? incubusAction() : succubusAction();
	// TODO: should blessed and cursed be passed through from the original action?
	if (action(false, false, *bus_, p)) {
	  io.message(L"The vividness of this dream will stay with you for a long time.");
	  throw awaken();
	} else {
	  bus_.reset();
	  io.message(L"You are alone with your dreams...");
	}
      }
    }
  };

  std::array<renderChar, 6> boudoir::renders = {
    renderChar(L'┌'), renderChar(L'─'), renderChar(L'┐'),
    renderChar(L'│'), renderChar(L'└'), renderChar(L'┘')
  };

 
  terrainType impassable(const deity &r) {
    switch (r.element()) {
    case Element::earth:
      return terrainType::ROCK;
    case Element::air:
    case Element::time:
      return terrainType::SPACE;
    case Element::fire:
      return terrainType::FIRE;
    case Element::water:
      return terrainType::WATER;
    case Element::plant:
    default:
      return terrainType::ROCK; // TODO: better to return a tree?
    }
  }


  struct cardPos {
    std::shared_ptr<dreamCard> card_;
    coord cardPos_;
  };
  
  class dreamscape : public renderByCoord {
  private:
    player &p_;
    std::shared_ptr<dreamCard> cards_[3][3]; 
    int offsetX_, offsetY_; // coords of the top-left of the first card
    terrainType pass_; terrainType wall_;
    void newCard(std::size_t x, std::size_t y) {
      auto roll = dPc();
      if (roll <= 10) {
	cards_[x][y] = std::make_shared<dreamShrine>
	  (pass_, wall_,y == 2, x == 0, y == 0, x == 2);
	return ;
      }
      if (roll <= 55)
	cards_[x][y] = std::make_shared<dreamCorridor>
	  (pass_, wall_,y == 2, x == 0, y == 0, x == 2, 2);
      if (roll >= 90) {
	auto &level = p_.curLevel().dung()[roll];
	auto &mt = monsterTypeRepo::instance()[roll %2 == 0 ? monsterTypeKey::incubus : monsterTypeKey::succubus];
	auto monster = mt.spawn(level);
	cards_[x][y] = std::make_shared<boudoir>
	  (pass_, wall_,y == 2, x == 0, y == 0, x == 2, monster);
      } else
	cards_[x][y] = std::make_shared<dreamRoom>
	  (pass_, wall_,y == 2, x == 0, y == 0, x == 2, 2, p_);
    }
  public:
    dreamscape (player &p) :
      p_(p),
      cards_(),
      offsetX_(PLAYER_X - 13), offsetY_(PLAYER_Y - 13),
      pass_(terrainType::GROUND),
      wall_(impassable(p.align())) {
      cards_[1][1] = std::make_shared<dreamCorridor>(pass_, wall_, true, true, true, true, 4); // center
      // start with corridors
      cards_[1][0] = std::make_shared<dreamCorridor>(pass_, wall_, false, false, true, false, 2); // n
      cards_[2][1] = std::make_shared<dreamCorridor>(pass_, wall_, false, false, false, true, 2); // e
      cards_[1][2] = std::make_shared<dreamCorridor>(pass_, wall_, true, false, false, false, 2); // s
      cards_[0][1] = std::make_shared<dreamCorridor>(pass_, wall_, false, true, false, false, 2); // w
    }
  private:
    cardPos at(const coord &p) const {
      int cardX, cardY, cardOffsetX, cardOffsetY;
      if (p.first >= offsetX_ && p.first < offsetX_ + 9) {
	cardX = 0; cardOffsetX = offsetX_;
      } else if (p.first >= offsetX_ + 9 && p.first < offsetX_ + 18) {
	cardX = 1; cardOffsetX = offsetX_ + 9;
      } else if (p.first >= offsetX_ + 18 && p.first <= offsetX_ + 27) {
	cardX = 2; cardOffsetX = offsetX_ + 18;
      } else {
	cardX = cardOffsetX = -1;
      }
      if (p.second >= offsetY_ && p.second < offsetY_ + 9) {
	cardY = 0; cardOffsetY = offsetY_;
      } else if (p.second >= offsetY_ + 9 && p.second < offsetY_ + 18) {
	cardY = 1; cardOffsetY = offsetY_ + 9;
      } else if (p.second >= offsetY_ + 18 && p.second <= offsetY_ + 27) {
	cardY = 2; cardOffsetY = offsetY_ + 18;
      } else {
	cardY = cardOffsetY = -1;
      }

      std::shared_ptr<dreamCard> card =	cardX >= 0 && cardY >= 0 ?
	cards_[cardX][cardY] : std::shared_ptr<dreamCard>();

      coord cardLoc = coord(p.first - cardOffsetX, p.second - cardOffsetY);

      return cardPos{card, cardLoc};
    }
  public:
    // interface renderByCoord:
    virtual const renderable & renderableAt(const coord &p) const {
      if (p.first == PLAYER_X && p.second == PLAYER_Y)
	return p_;
      cardPos cp(at(p));
      if (!cp.card_) return tFactory.get(wall_);
      
      auto item = cp.card_->itemAt(cp.cardPos_);
      if (item) return item.value();
      return cp.card_->render(cp.cardPos_);
    }
    virtual drawIter drawBegin() const {
      return drawIter(*this, coord(0,0), PLAYER_X*2, PLAYER_Y*2-2);
    }
    virtual drawIter drawEnd() const {
      return drawIter(*this, coord(-1,-1), PLAYER_X*2, PLAYER_Y*2-2);    
    }
    // interface renderByCoord ends

    void move(const int x, const int y) {
      coord next(PLAYER_X - x, PLAYER_Y - y);
      cardPos cp(at(next));
      if (!cp.card_ || !cp.card_->passable(cp.cardPos_))
	return; // can't move that way
      offsetX_ += x; offsetY_ +=y;
      if (offsetY_ == PLAYER_Y - 8) { // player moves north
	for (int x=0; x < 3; ++x) {
	  for (int y=2; y > 0; --y) cards_[x][y] = cards_[x][y-1];
	  cards_[x][0].reset();
	}
	offsetY_ -= 9;	
      }
      if (offsetX_ == PLAYER_X - 18) { // player moves east
	// drop row 0, shift left, reroll row 2
	for (int y=0; y < 3; ++y) {
	  for (int x=0; x < 2; ++x) cards_[x][y] = cards_[x+1][y];
	  cards_[2][y].reset();
	}
	offsetX_ += 9;
      }
      if (offsetY_ == PLAYER_Y - 18) { // player moves south
	// drop row 0, shift left, reroll row 2
	for (int x=0; x < 3; ++x) {
	  for (int y=0; y < 2; ++y) cards_[x][y] = cards_[x][y+1];
	  cards_[x][2].reset();
	}
	offsetY_ += 9;
      }
      if (offsetX_ == PLAYER_X - 8) { // player moves west
	for (int y=0; y < 3; ++y) {
	  for (int x=2; x > 0; --x) cards_[x][y] = cards_[x-1][y];
	  cards_[0][y].reset();
	}
	offsetX_ -= 9;	
      }

      // redraw any missing cards
      if (cards_[1][1]->n() && !cards_[1][0]) newCard(1,0);
      if (cards_[1][1]->e() && !cards_[2][1]) newCard(2,1);
      if (cards_[1][1]->s() && !cards_[1][2]) newCard(1,2);
      if (cards_[1][1]->w() && !cards_[0][1]) newCard(0,1);

      cp.card_->onMove(p_, cp.cardPos_);

    }
  };

} // namespace morpheus

void dream(monster &m, bool blessed, bool cursed) {
  player &p = dynamic_cast<player&>(m);
  const io &io = ioFactory::instance();
  morpheus::dreamscape d(p);
  try {
    for (int i=0; i < 100; ++i) {
      io.draw(d);
      wchar_t dir = io.dirPrompt();
      switch (dir) {
      case L'w': case L'W':
	d.move(0,1); // move N
	break;
      case L'a': case L'A':
	d.move(1,0); // E
	break;
      case L's': case L'S':
	d.move(0,-1); // S
	break;
      case L'd': case L'D':
	d.move(-1,0); // W
	break;
      default:
	break;
      }
    }
  } catch (morpheus:: awaken a) {
    // break out of the loop
  }
  if (blessed && cursed)
    io.longMsg(L"You awaken from your slumber.");
  else if (blessed)
    io.longMsg(L"You awake feeling refreshed.");
  else if (cursed)
    io.longMsg(L"You feel groggy after your nap.");
  else
    io.longMsg(L"You wake up.");
  p.curLevel().dung().announceLevel();
}
