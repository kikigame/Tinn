/* License and copyright go here*/

// define an encyclopedia of the dungeon

#ifndef ENCYCLOPEDIA_HPP
#define ENCYCLOPEDIA_HPP

#include "optionalRef.hpp"

class item;

// activate the Hitch-Hiker's Guide
// io - Input/output layer
// allowFreeSearch - true to allow free-search over everything; false to require a category.
// other - optional renderable to look up
void invokeGuide(const bool allowFreeSearch,
		 const optionalRef<item> other = optionalRef<item>());

#endif // ndef ENCYCLOPEDIA_HPP
