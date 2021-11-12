/* License and copyright go here*/

// If wishes were dishes, we'd all be eating steak. -Jane Cobb, Firefly

#ifndef WISH_HPP__
#define WISH_HPP__

#include "items.hpp"
#include "monster.hpp"

/*
 * mon - monster who's doing the wishing (currently always the player)
 * it - item used to OBTAIN the wish
 */
void wish(monster &mon, item &it);

#endif //ndef WISH_HPP__
