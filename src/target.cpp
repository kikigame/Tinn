/* License and copyright go here*/

#include "target.hpp"
#include "level.hpp"
#include "monster.hpp"
#include "religion.hpp"
#include "random.hpp"
#include "output.hpp"

#include <sstream>

// NB: No targeting method will return a monster that the target will refuse to fight

// pick a viable target for mon to attack, for which f returns true.
monster *pickTarget(monster &mon, std::function<bool(const monster &)> f) {
  std::vector<monster*> targets;
  auto &level = mon.curLevel();
  const auto &malign = mon.align();
  level.forEachMonster([&mon, &targets, &malign, f](monster &en) {
      // TODO: Duplicate code in mobile.cpp
      if (&en == &mon) return; // monsters don't usually fight themselves
      if (en.align().coalignment(malign) >= 3) return; // monsters don't usually fight other creautures of the same alignment
      if (en.type() == mon.type()) return; // monsters don't usually fight other creatures of the same class
      // TODO: charmed?
      // TODO: zones
      if (!f(en)) return;

      targets.emplace_back(&en);
    });
  if (targets.empty())
    return nullptr;
  if (mon.isPlayer()) {
    auto &ios = ioFactory::instance();
    std::vector<std::pair<int, std::wstring> > choices;
    int i=0;
    for (auto t : targets) {
      auto tPos = level.posOf(*t);
      std::wstringstream opt;
      opt << t->render() << L" (" << tPos.first << L"," << tPos.second << L")";
      choices.emplace_back(i++, opt.str());
    }
    return targets.at
	     (ios.choice
	      (L"Please enter targetting vector:",
	       // ref: Nethack genocide action wording
	       L"This action requires a target. You may choose any monster in range of sight.",
	       choices));
  } else
    return *(rndPick(targets.begin(), targets.end()));
}

// pick a target the src could hit with a rock (ie line of sight that src could move through)
template <>
monster *pickTarget<true>(monster &src) {
  auto &level = src.curLevel();
  coord srcPos = level.posOf(src);
  return pickTarget(src, [&level, &src, &srcPos](const monster &en) {
      // could src move towards en in a straight line, were such moves allowed?
      coord enPos = level.posOf(en);
      unsigned char distance = srcPos.linearDistance(enPos);
      // for d : each square from en (exclusive) to pos (exclusive):
      for (auto i = distance-1; i > 0; --i) {
	coord d(
		(srcPos.first * i + enPos.first * (distance - i)) /distance,
		(srcPos.second * i + enPos.second * (distance - i)) /distance
		);
	// projectiles can't pass if the monster can't pass:
	if (!level.movable(srcPos, d, src, false, false))
	  return false;
      }
      return true;
    });
}

// pick any target on same level as monster
template <>
monster *pickTarget<false>(monster &mon) {
  return pickTarget(mon, [](const monster &){return true; });
}
