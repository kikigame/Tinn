/* License and copyright go here*/

// dungeon functions

#include "dungeon.hpp"
#include "terrain.hpp"
#include "output.hpp"
#include "appraise.hpp"
#include "role.hpp"

#include "chargen.hpp"
#include <sstream>

const int NUM_LEVELS = 100;

// initialise the dungeon:
dungeon::dungeon() 
  : alive_(true),
    cur_level_(1) {
  levelFactory factory(*this, NUM_LEVELS);
  for (auto l : factory)
    level_.emplace_back(l);
  /*
  for (int l = 0; l <= NUM_LEVELS; ++l) {
    // the last level has no down ramp
    bool downRamp = l < NUM_LEVELS;
    level_.emplace_back(new level(*this, ios, l, downRamp));
    }*/
  playerBuilder pb = chargen();
  level &start = *(level_[cur_level_]);
  pb.startOn(start);
  player_ = std::shared_ptr<player> (new player(pb));
  // player starts on level 1, NOT the first level:
  start.addMonster(player_, start.findTerrain(terrainType::UP));

  // welcome message
  ioFactory::instance().message(player_->job().startGameMessage());
}

dungeon::~dungeon() {
}

void dungeon::draw () const{
  ioFactory::instance().draw(*this);
}

std::shared_ptr<player> dungeon::pc() {
  return player_;
}

const std::shared_ptr<player>  dungeon::pc() const {
  return player_;
}

int dungeon::maxLevel() const {
  return level_.size() -1;
}

void dungeon::upLevel() {
  if (cur_level_ < 0) throw L"at top"; // already checked in level{}
  cur_level_--;
  level_[cur_level_]->addMonster(pc(), level_[cur_level_]->findTerrain(terrainType::DOWN));
}

void dungeon::downLevel() {
  if (cur_level_ >= maxLevel()) throw L"at bottom"; // already checked in level{}
  cur_level_++;
  level_[cur_level_]->addMonster(pc(), level_[cur_level_]->findTerrain(terrainType::UP));
}

void dungeon::quit() {
  alive_ = false;
}
void dungeon::playerDeath() {
  ioFactory::instance().longMsg(L"You are no longer alive.");
  alive_ = false;
}

bool dungeon::alive() const {
  return alive_;
};

void dungeon::interrogate() const {
  auto &l = *level_[cur_level_];
  ioFactory::instance().interrogate(l, l.pcPos());
}

std::wstring dungeon::score() const {
  std::wstringstream rtn;
  double sumValue =0, sumWeight =0;
  player_->forEachItem([this, &rtn, &sumValue, &sumWeight](item &i, std::wstring name) {
      double value = appraise(*player_, i);
      double weight = i.weight();
      rtn << name << L"\t, scoring:" << value << L"\t, weight" << weight << L"N\n";
      sumValue += value, sumWeight += weight;
    });
  rtn << L"Total score of " << sumValue << L"; you finished with " << sumWeight << L"N of loot";
  return rtn.str();
}
