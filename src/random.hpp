/* License and copyright go here*/

// Random numbers

#include <ctime>
#include <random>
#include <iterator>


// we're a game, not a crypto, so seeding with the time should be okay.
//static std::default_random_engine generator;
//static std::default_random_engine generator(1473628262);
static std::default_random_engine generator(time(NULL));
static std::uniform_int_distribution<int> distributionRoom(3,5);
static auto numRooms = std::bind ( distributionRoom, generator );
static std::uniform_int_distribution<int> distributionWidth(5,10);
static auto roomWidth = std::bind ( distributionWidth, generator );
static std::uniform_int_distribution<int> distributionHeight(3,6);
static auto roomHeight = std::bind ( distributionHeight, generator );
static std::uniform_int_distribution<int> distributionCoinFlip(0,1);
static auto coinFlip = std::bind ( distributionCoinFlip, generator );
static auto corridorDir = std::bind ( distributionCoinFlip, generator );
static std::uniform_int_distribution<int> distributionMonstersPerRoom(-2,2);
static auto numMonsters = std::bind ( distributionMonstersPerRoom, generator );
static std::uniform_int_distribution<int> distributionItemsPerRoom(-4,4);
static auto numItems = std::bind ( distributionItemsPerRoom, generator );

static std::uniform_int_distribution<int> distribution52(1,51);
static auto d51 = std::bind ( distribution52, generator );

/*
 * "Pick a card" method (uniform distribution)
 *
 * Given two iterators from the same random-access container,
 * return a uniformly-distributed random value from the container.
 */
template <typename Iter>
Iter rndPick(/*by value*/Iter start, const Iter &end) {
  const auto max = std::distance(start, end);
  if (max > 1) { // don't burn random numbers to pick from single lists; happens a lot.
    std::uniform_int_distribution<int> dis(0, max - 1);
    std::advance(start, dis(generator));
  }
  return start;
}

// as rndPick, but between 2 numbers:
template <typename I>
I rndPickI(/*by value*/I start, const I end) {
  const auto max = end - start;
  if (max > 1) { // don't burn random numbers to pick from single lists; happens a lot.
    std::uniform_int_distribution<int> dis(0, max - 1);
    start += dis(generator);
  }
  return start;
}

/*
 * "Roll the dice" method (normal curve)
 *
 * Roll 2D52-2, producing a value between 0(inclusive) to 100(inclusive)
 * with a normal distribution centred on 50.
 *
 * Called D% for brevity, but distinct from a traditional uniform percentile die.
 */
unsigned char dPc();

/*
 * Used for random generation on levels (items, monsters, etc)
 *
 * I concept is a const random-access iterator of T.
 * I must be assignable.
 *
 * T concept must provide:
 * int getLevelFactor() : dungeon level factor (F)
 * int getLevelOffset() : dungeon level offset (O)
 * int getMinSpawn() : min number (Mn)
 * int getMaxSpawn() : max number (Mx)
 * T must be a copyable pointer type.
 *
 * No T may be spawned if the dungeon level, L < L * F + O.
 * If spawned, number spawned is uniform between Mn--Wx
 *
 * Of the eligable Ts, a number of T will be spawned between R/2 and R,
 * where "R" is the number of rooms in the level.
 *
 * Params:
 * from, to => options for generation
 * l => current level (L)
 * l => number of rooms (R) in current level
 *
 * Returns a list of pairts of quantity and T
 */
template<typename T, typename I>
std::vector<std::pair<unsigned int, T>>
rndGen(const I &from, const I & to, 
       const unsigned char l, const unsigned char r) {
  // first work out which items are allowed:
  std::vector<T> buffer;
 
  for (auto i = from; i < to; ++i) {
    auto f = (*i)->getLevelFactor();
    auto o = (*i)->getLevelOffset();
    if (l >= (l * f + o))
      buffer.push_back(*i);
  }
  auto max = buffer.size();
  std::uniform_int_distribution<int> dis(0, max - 1);

  // now we work out how many T's we need
  int numGroups;
  if (r < 1) numGroups = 0; // nothing to emplace
  else if (r == 1) numGroups = 1; // always place a monster if we can
  else {
    std::uniform_int_distribution<int> ng(r / 2, r);
    numGroups = ng(generator);
  }
  std::vector<std::pair<unsigned int, T>> rtn;
  for (auto counter = 0; counter < numGroups; ++counter) {
    auto idx = dis(generator);
    auto &i = buffer[idx];
    std::uniform_int_distribution<int> ng((*i).getMinSpawn(), (*i).getMaxSpawn());
    rtn.emplace_back(ng(generator), i);
  }
  return rtn;
}
