/* License and copyright go here*/

// track time spent in the dungeon

#include "time.hpp"
#include <vector>
#include <algorithm>

temporal::callback::callback(bool everyTick, const std::function<void()> &fn) :
  callback_(fn), everyTick_(everyTick) {
  if (everyTick_) ::time::onTick(*this);
  else ::time::onPlayerMove(*this);
}

temporal::callback::~callback() {
  if (everyTick_) ::time::offTick(*this);
  else ::time::offPlayerMove(*this);
}

void temporal::callback::operator()() {
  // dereference the pointer-to-lambda and invoke the captured method:
  callback_();
}


bool temporal::callback::operator ==(const temporal::callback &rhs) {
  return &(callback_) == &(rhs.callback_);
}

class timeImpl {
  friend class time;
private:
  long long moveCount_;
  std::vector<time::callback*> playerMoveCallbacks_;
  std::vector<time::callback*> tickCallbacks_;
};

std::unique_ptr<timeImpl> time::instance_(new timeImpl());

unsigned long long time::moveCount() {
  return instance_->moveCount_;
}


void time::tick(bool isMove) {
  ++ (instance_->moveCount_);
  if (isMove) {
    auto copy = instance_->playerMoveCallbacks_;
    for (auto f : copy) { (*f)(); }
  }
  auto copy = instance_->tickCallbacks_;
  for (auto f : copy) { (*f)(); }
}
void time::onPlayerMove(time::callback &onMove) {
  instance_->playerMoveCallbacks_.push_back(&onMove);
}
void time::onTick(time::callback &onMove) {
  instance_->tickCallbacks_.push_back(&onMove);
}


// Ah, the erase/remove idiom. Score one for Java, where this is just a method in AbstractList().

void time::offPlayerMove(time::callback &onMove) {
  auto &v = instance_->playerMoveCallbacks_;
  v.erase(std::remove(v.begin(), v.end(), &onMove), v.end());
  //  std::vector<time::callback>::iterator newEnd = std::remove
  //    (v.begin(), v.end(), [callback](time::callback &c){return &c == &callback;});
  //  v.erase(newEnd, v.end());
}

void time::offTick(time::callback &onMove) {
  auto &v = instance_->tickCallbacks_;
  v.erase(std::remove(v.begin(), v.end(), &onMove), v.end());
}
