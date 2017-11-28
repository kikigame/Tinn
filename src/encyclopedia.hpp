/* License and copyright go here*/

// define an encyclopedia of the dungeon

#ifndef ENCYCLOPEDIA_HPP
#define ENCYCLOPEDIA_HPP

#include "output.hpp"

class item;

// activate the Hitch-Hiker's Guide
// io - Input/output layer
// allowFreeSearch - true to allow free-search over everything; false to require a category.
// other - optional renderable to look up
void invokeGuide(const io &, const bool allowFreeSearch,
		 const std::shared_ptr<item> other = std::shared_ptr<item>(0));

#endif // ndef ENCYCLOPEDIA_HPP
