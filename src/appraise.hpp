/* License and copyright go here*/

// how much is my thingy worth?

#ifndef APPRAISE_HPP__
#define APPRAISE_HPP__

#include "items.hpp"
class monster;

enum class transaction {
  buy, // raise prices when buying, based on appearance
    sell, // raise/lower prices when selling, based on appearance
    score // fair price (used for scoring)
};

double appraise(const monster &monster, const item &thing, transaction t);

#endif // ndef APPRAISE_HPP__
