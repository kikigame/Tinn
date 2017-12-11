/* License and copyright go here*/

// define an individual characteristic

#include <ostream>

class characteristic {
private:
  // the current level actually used for gameplay (0-max)
  unsigned char current_;
  // the current max level for this monster/PC (0-100)
  unsigned char max_;
public:
  // copy constructor:
  characteristic(const characteristic &other) = default;
  // create a degraded characteristic:
  characteristic(unsigned char current, unsigned char max);
  // most start at max:
  characteristic(unsigned char val);
  virtual ~characteristic();

  // ordinary use: just use the current_ value, limiting it by 0..max_:
  unsigned char max() const;
  unsigned char cur() const;
  characteristic & operator = (const characteristic &other) = default;
  characteristic & operator = (unsigned char);
  characteristic & operator = (unsigned int);
  characteristic operator + (unsigned char) const;
  characteristic operator + (unsigned int) const;
  characteristic operator + (char) const;
  characteristic operator + (int) const;
  characteristic & operator += (unsigned char);
  characteristic & operator += (unsigned int);
  characteristic & operator += (char);
  characteristic & operator += (int);
  characteristic operator - (unsigned char) const;
  characteristic operator - (unsigned int) const;
  characteristic operator - (char) const;
  characteristic operator - (int) const;
  characteristic & operator -= (unsigned char);
  characteristic & operator -= (unsigned int);
  characteristic & operator -= (char);
  characteristic & operator -= (int);

  // gain experience etc somehow. Adds to both current and max:
  void bonus(unsigned char amout);
  // reduce the maximum stat in a more permanent way; will reduce current and max
  void cripple(unsigned char amout);

  void enhancement(int amount) { amount > 0 ? bonus(amount) : cripple(amount); };

  // returns true if current = max
  bool isFull() const;

  // returns damage as a percentage
  unsigned char pc() const { 
    if (max_ == 0) return 0;
    return static_cast<unsigned char>((100 * static_cast<int>(current_)) / max_); }

  // the maximum maximum value of any stat.
  static const unsigned char MAX_MAX = 100;
};

// for terminal output or debugging. Each call creates 1-5 chars.
// output is of the form (NN is 1 or 2 numerals):
// "--" (for cur==max==0)
// "**" (for cur=100)
// "NN" (for cur==max<100)
// "NN%" (for cur < max == 100)
// "NN%NN" (for cur < max < 100)
std::ostream & operator << (std::ostream & out, const characteristic & c);
std::wostream & operator << (std::wostream & out, const characteristic & c);
