/* License and copyright go here*/

// Object supplying a reference to another object; suitable for use in containers.

#ifndef REF__
#define REF__

// basically the same as in C++17, but I don't want to up the standard for one template class
// this one only ever holds a reference
template <typename T>
class ref {
private:
  T* value_;
public:
  ref(T & t) : value_(&t) {};
  ref<T> &operator =(const ref<T> &other) {
    value_ = other.value_;
    return *this;
  }
  T & value() {return *value_; }
  const T & value() const { return *value_; }
  bool operator ==(const ref<T> &other) {
    return value_ == other.value_;
  }
  bool operator !=(const ref<T> &other) {
    return !(*this == other);
  }
};

#endif // ndef REF__
