/* License and copyright go here*/

// Holds a refernce to an object. May be empty.

#ifndef OPTIONAL_REF__
#define OPTIONAL_REF__

// basically the same as in C++17, but I don't want to up the standard for one template class
// this one only ever holds a reference
template <typename T>
class optionalRef {
private:
  bool has_value_;
  T* value_;
public:
  optionalRef() : has_value_(false), value_(0) {}
  optionalRef(T & t) : has_value_(true), value_(&t) {};
  optionalRef<T> &operator =(const optionalRef<T> &other) {
    has_value_ = other.has_value_;
    value_ = other.value_;
    return *this;
  }
  operator bool() const { return has_value_; };
  T & value() {return *value_; }
  const T & value() const { return *value_; }
  void reset() { has_value_ = false; value_ = 0; }
};

#endif // ndef OPTIONAL_REF__
