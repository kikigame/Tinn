/* License and copyright go here*/

// define an individual characteristic

#include "characteristic.hpp"

characteristic::characteristic(unsigned char current, unsigned char max) : 
  current_(current), max_(max) {
}
characteristic::characteristic(unsigned char value) : 
  current_(value), max_(value) {
}
characteristic::~characteristic() {}

unsigned char characteristic::max() const {
  return max_;
}
unsigned char characteristic::cur() const {
  return current_;
}
characteristic & characteristic::operator = (unsigned char newVal) {
  current_ = (newVal > max_) ? max_ : newVal;
  return *this;
}
characteristic & characteristic::operator = (unsigned int newVal) {
  current_ = (newVal > max_) ? max_ : newVal;
  return *this;
}
characteristic characteristic::operator + (unsigned char newVal) const {
  unsigned int newV = ((current_ + newVal) > max_) ? max_ : current_ + newVal;
  return characteristic(newV, max_);
}
characteristic characteristic::operator + (unsigned int newVal) const {
  unsigned long newV = ((current_ + newVal) > max_) ? max_ : current_ + newVal;
  return characteristic(newV, max_);
}
characteristic characteristic::operator + (char newVal) const {
  int newV = (current_ + newVal);
  if (newV < 0) newV = 0;
  if (newV > max_) newV = max_;
  return characteristic(newV, max_);
}
characteristic characteristic::operator + (int newVal) const {
  long newV = (current_ + newVal);
  if (newV < 0) newV = 0;
  if (newV > max_) newV = max_;
  return characteristic(newV, max_);
}
characteristic & characteristic::operator += (unsigned char newVal) {
  current_ = ((*this) + newVal).current_;
  return *this;
}
characteristic & characteristic::operator += (unsigned int newVal) {
  current_ = ((*this) + newVal).current_;
  return *this;
}
characteristic & characteristic::operator += (char newVal) {
  current_ = ((*this) + newVal).current_;
  return *this;
}
characteristic & characteristic::operator += (int newVal) {
  current_ = ((*this) + newVal).current_;
  return *this;
}



characteristic characteristic::operator - (unsigned char newVal) const {
  unsigned int newV = (current_ < newVal) ? 0 : current_ - newVal;
  return characteristic(newV, max_);
}
characteristic characteristic::operator - (unsigned int newVal) const {
  unsigned long newV = (current_ < newVal) ? 0 : current_ - newVal;
  return characteristic(static_cast<unsigned char>(newV), max_);
}
characteristic characteristic::operator - (char newVal) const {
  int newV = (current_ - newVal);
  if (newV < 0) newV = 0;
  if (newV > max_) newV = max_;
  return characteristic(newV, max_);
}
characteristic characteristic::operator - (int newVal) const {
  long newV = (current_ - newVal);
  if (newV < 0) newV = 0;
  if (newV > max_) newV = max_;
  return characteristic(newV, max_);
}
characteristic & characteristic::operator -= (unsigned char newVal) {
  current_ = ((*this) - newVal).current_;
  return *this;
}
characteristic & characteristic::operator -= (unsigned int newVal) {
  current_ = ((*this) - newVal).current_;
  return *this;
}
characteristic & characteristic::operator -= (char newVal) {
  current_ = ((*this) - newVal).current_;
  return *this;
}
characteristic & characteristic::operator -= (int newVal) {
  current_ = ((*this) - newVal).current_;
  return *this;
}


void characteristic::bonus(unsigned char amount) {
  current_ += amount;
  max_ += amount;
  if (current_ > 100) current_ = 100;
  if (max_ > 100) max_ = 100;
}

void characteristic::cripple(unsigned char amount) {
  current_ -= amount;
  max_ -= amount;
  if (current_ < 0) current_ = 0;
  if (max_ < 0) max_ = 0;
}

void characteristic::adjustBy(int amount) {
  if (amount < 0)
    (*this) -= amount;
  else if (amount > 0)
    bonus(static_cast<unsigned char>(std::min(100,amount)));
}

bool characteristic::isFull() const {
  return current_ == max_;
}

std::ostream & operator << (std::ostream & out, const characteristic & c) {
  unsigned int cur = c.cur();
  if (c.isFull() && cur == 100) {
    out << "**";
  } else if (c.isFull()) {
    out << cur;
  } else if (cur == 100) {
    out << cur << '%';
  } else if (c.max() == 0) {
    out << cur << "--";
  } else {
    out << ((100*static_cast<unsigned int>(cur))/c.max())
	<< '%'
	<< static_cast<unsigned int>(c.max());
  }
  return out;
}
std::wostream & operator << (std::wostream & out, const characteristic & c) {
  unsigned int cur = c.cur();
  if (c.isFull() && cur == 100) {
    out << L"**";
  } else if (c.isFull()) {
    out << cur;
  } else if (cur == 100) {
    out << cur << L'%';
  } else if (c.max() == 0) {
    out << cur << L"--";
  } else {
    out << ((100*static_cast<unsigned int>(cur))/c.max())
	<< L'%'
	<< static_cast<unsigned int>(c.max());
  }
  return out;
}
