/* License and copyright go here*/

// define the player in the dungeon (I/O bit)

#ifndef CHARGEN_HPP__
#define CHARGEN_HPP__

class playerBuilder;
class dungeon;
class io;

// generate the player character (interactively) and return a fully-populated builder.
playerBuilder chargen();

#endif // ndef CHARGEN_HPP__
