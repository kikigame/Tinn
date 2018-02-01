/* License and copyright go here*/

#ifndef __ARGS_HPP
#define __ARGS_HPP

// In the insane idea of reducing dependencies, I have rolled my own
// command-line-parsing library.
// I am probably mad.

// see https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html

/*
 *
 * -abc == -a -b -c (single-char options)
 * -foo (multi-char options)
 * -ofoo == -o foo (options with arguments)
 *
 * To extend, we allow
 * -o=foo
 *
 * -- == -
 */

#include <set>
#include <algorithm>

class args {
private:
  const int argc_;
  const char * const * const argv_;
  std::set<char> flags_; // single-char options
  std::set<const char *> owa_; // options with arguments
public:
  args(const int argc, const char * const * const argv) :
    argc_(argc), argv_(argv), flags_() {};
  args &flag(char c) {
    flags_.emplace(c);
    return *this;
  }
  args &optWithArg(const char * o) {
    owa_.emplace(o);
    return *this;
  }
  // determine if the given flag is set in any option
  bool isFlag(const char flag) const {
    for (iter it = begin(), e = end(); it != e; ++it) {
      if (it.typ() != type::FLAG_LIST) continue;
      for (const char *c = it.value(); *c != '\0'; ++c)
	if (*c == flag) return true;
    }
    return false;
  }
  // determine if the given option is set. If it is,
  // return its value. If it is set but does not have a value, return opt.
  // if it is unset, return nullptr.
  const char* const option(const char *const opt) const {
    for (iter it = begin(); it != end(); ++it) {
      const char *t = it.value();
      switch (it.typ()) {
      case type::FLAG_LIST: continue;
      case type::SIMPLE_OPT: {
	auto a=t, b=opt;
	for (; *a != '\0' && *b != '\0'; ++a, ++b)
	  if (*a != *b) break;
	if (*a != *b) continue; // different length strings or mismatch
	return opt;
      }
      case type::OPT_WITH_ARG: {
	auto a=t, b=opt;
	for (; *a != '\0' && *b != '\0'; ++a, ++b)
	  if (*a != *b) break;
	if (*b != '\0') continue; // substring or mismatch
	if (*a == '=') ++a;
	return a;
      }
      case type::OPT_FOR_ARG: {
	auto a=t, b=opt;
	for (; *a != '\0' && *b != '\0'; ++a, ++b)
	  if (*a != *b) break;
	if (*a != *b) continue; // different length strings or mismatch
	return it.nextVal();
      }
      }
   }
   return nullptr;
  }
private:
  enum class type {
    FLAG_LIST,
    SIMPLE_OPT,
      OPT_WITH_ARG,
      OPT_FOR_ARG
  };
  class iter {
  private:
    int c_;
    int maxC_;
    const char * const * v_;
    const std::set<char> &flags_; // single-char options
    const std::set<const char *> &owa_; // options with arguments
  public:
    // start at 1 to skip the executable filename
    iter(const char * const *argv, const int argc,
	 const std::set<char> &flags, const std::set<const char *> &owa) :
      c_(1), maxC_(argc), v_(argv+1), flags_(flags), owa_(owa) {}
    iter(const int argc,
	 const std::set<char> &flags, const std::set<const char *> &owa) :
      c_(argc), maxC_(argc), v_(nullptr), flags_(flags), owa_(owa) {};
    bool operator == (const iter &other) const {
      return c_ == other.c_;
    }
    bool operator != (const iter &other) const {
      return !(*this == other);
    }
    const char * const value() const {
      auto rtn = *v_;
      if (rtn[0] == '-') rtn++;
      if (rtn[0] == '-') rtn++;
      return rtn;
    }
    const char * const nextVal() const {
      int pos = c_+1;
      if (pos == maxC_) return nullptr;
      return *(v_+1);
    }
    type typ() const {
      auto val = *v_;;
      bool hasFlags = val[0] == '-';
      if (hasFlags) val++;
      if (val[0] == '-') val++;
      if (hasFlags) {
	for (auto c = val; *c != '\0'; ++c)
	  if (flags_.find(*c) == flags_.end()) {
	    hasFlags = false;
	    break;
	  }
	if (hasFlags) return type::FLAG_LIST;
      }
      auto c = val;
      for (; *c != '\0' && *c != '='; ++c);
      for (auto str : owa_) {
	const char *a = val;
	const char *b = str;
	for (;
	     a != c && *b != '\0';
	     ++a, ++b) {
	  if (*a != *b) break;
	}
	if (a == c && *b == '\0') 
	  return *c == '=' ? type::OPT_WITH_ARG : type::OPT_FOR_ARG;
      }
      return type::SIMPLE_OPT;
    }
    iter &operator ++() {
      ++c_, ++v_;
      if (c_ == maxC_) return *this;
      auto t = typ();
      if (t == type::OPT_FOR_ARG)
	++c_, ++v_;
      return *this;
    }
  };
  iter begin() const {
    return iter(argv_, argc_, flags_, owa_);
  }
  iter end() const {
    return iter(argc_, flags_, owa_);
  }
};

#endif //ndef __ARGS_HPP
