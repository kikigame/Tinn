/* License and copyright go here*/

// special levels

#ifndef LEVELFACTORY_HPP__
#define LEVELFACTORY_HPP__

class levelImpl;
class level;
class levelGen;

enum class specialLevelKey {
  WATER,
    SPACE,
    END
};

levelGen *newGen(specialLevelKey, levelImpl *, level *, bool addDownRamp);

#endif // ndef LEVELFACTORY_HPP__
