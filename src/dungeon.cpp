/* License and copyright go here*/

// dungeon functions

#include "dungeon.hpp"
#include "terrain.hpp"
#include "output.hpp"

#include "chargen.hpp"

const int NUM_LEVELS = 100;

// initialise the dungeon:
dungeon::dungeon(::std::shared_ptr<io> ios) 
  : alive_(true),
    cur_level_(1),
    io_(ios) {
    //    refresher_(true, new std::function<void()>([this]() { draw(); })) { // http://en.cppreference.com/w/cpp/utility/functional/function
  for (int l = 0; l <= NUM_LEVELS; ++l) {
    // the last level has no down ramp
    bool downRamp = l < NUM_LEVELS;
    level_.emplace_back(new level(*this, ios, l, downRamp));
  }
  playerBuilder pb = chargen(*ios);
  level &start = *(level_[cur_level_]);
  pb.startOn(start);
  player_ = std::shared_ptr<player> (new player(pb));
  // player starts on level 1, NOT the first level:
  start.addMonster(player_, start.findTerrain(terrainType::UP));
}

dungeon::~dungeon() {
}

void dungeon::draw () const{
  io_->draw(*this);
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
  io_->longMsg(L"You are no longer alive.");
  alive_ = false;
}

bool dungeon::alive() const {
  return alive_;
};

void dungeon::interrogate() const {
  auto &l = *level_[cur_level_];
  io_->interrogate(l, l.pcPos());
}
