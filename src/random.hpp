/* License and copyright go here*/

// Random numbers

#include <ctime>
#include <random>
#include <iterator>
#include <functional>
#include <bitset>

// we're a game, not a crypto, so seeding with the time should be okay.
//static std::default_random_engine generator(1473628262);
constexpr std::hash<std::bitset<sizeof(time_t)>> hasher;
// NB: I'm hashing the time to make it more random; otherwise on Linux/g++, we always get the first few high bits in the first result, leading to the same options being picked.
static std::default_random_engine generator(hasher(std::bitset<sizeof(time_t)>(time(nullptr))));
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
    auto distance = dis(generator);
    std::advance(start, distance);
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

// as rndPick, but iterate the full list, and allow skipping
template <typename I>
I rndPick(const I &start, const I end, bool (*f)(const I&)) {
  if (start == end) return start;
  I rtn = start, cur = start;
  for (size_t counter=1; cur != end; ++cur) {
    if (f(cur)) {
	std::uniform_int_distribution<int> dis(0, counter);
	if (dis(generator) == 1) rtn = cur;
	counter++;
      }
  }
  return rtn;
}

// as rndPick, but specialized for enum types:
template <typename E>
E rndPickEnum(const E start, const E end) {
  size_t begin = static_cast<size_t>(start),
    stop = static_cast<size_t>(end);
  size_t max = stop - begin;
  if (max > 1) {
    std::uniform_int_distribution<int> dis(0, max - 1);
    begin += dis(generator);
  }
  return static_cast<E>(begin);
}

// random value from type E
template <typename E>
E rndPick() {
  size_t begin = 0,
    stop = static_cast<size_t>(E::END);
  size_t max = stop - begin;
  if (max > 1) {
    std::uniform_int_distribution<int> dis(0, max - 1);
    begin += dis(generator);
  }
  return static_cast<E>(begin);
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
 * ((int getLevelFactor() : dungeon level factor (F) - not used; specifies power of monster))
 * int getLevelOffset() : dungeon level offset (O)
 * int getMinSpawn() : min number (Mn)
 * int getMaxSpawn() : max number (Mx)
 * T must be a copyable pointer type.
 *
 * No T may be spawned if the dungeon level, L < O.
 * If spawned, number spawned is uniform between Mn--Wx
 *
 * Of the eligable Ts, a number of T will be spawned between R/2 and R,
 * where "R" is the number of rooms in the level.
 *
 * Params:
 * from, to => options for generation
 * l => current level (L)
 * r => number of rooms (R) in current level
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
    auto o = (*i)->getLevelOffset(); // TODO: unsure if this is being used consistently
    if (l >= o)
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
