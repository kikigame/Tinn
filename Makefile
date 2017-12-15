# This is an auto-generated file. Please make changes in build.pl

tinn : Makefile src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 
	c++  src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 -lncursesw -o tinn

Makefile: build.pl
	./build.pl > Makefile

lint:
	cppcheck --enable=performance --enable=warning --enable=portability src

clean:
	rm -f   src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 

src/action.o : src/action.cpp src/damage.hpp src/monsterType.hpp src/movement.hpp src/action.hpp src/monster.hpp src/optionalRef.hpp src/bonus.hpp src/output.hpp src/items.hpp src/monsterIntrinsics.hpp src/coord.hpp src/materialType.hpp src/itemholder.hpp src/zone.hpp src/level.hpp src/time.hpp src/itemTypes.hpp src/iterable.hpp src/renderable.hpp src/slots.hpp src/characteristic.hpp 
	c++ src/action.cpp -c -Wall -g -std=c++11 -o src/action.o -finput-charset=utf8 -fexec-charset=utf8

src/adjectives.o : src/adjectives.cpp 
	c++ src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/optionalRef.hpp src/monster.hpp src/monsterType.hpp src/movement.hpp src/damage.hpp src/coord.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/items.hpp src/bonus.hpp src/appraise.hpp src/time.hpp src/level.hpp src/itemholder.hpp src/zone.hpp src/slots.hpp src/characteristic.hpp src/iterable.hpp src/renderable.hpp src/itemTypes.hpp 
	c++ src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	c++ src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	c++ src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/monsterType.hpp src/movement.hpp src/religion.hpp src/monster.hpp src/role.hpp src/optionalRef.hpp src/bonus.hpp src/coord.hpp src/materialType.hpp src/output.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/zone.hpp src/time.hpp src/level.hpp src/dungeon.hpp src/chargen.hpp src/slots.hpp src/characteristic.hpp src/player.hpp src/iterable.hpp src/renderable.hpp 
	c++ src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	c++ src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/damage.hpp src/materialType.hpp 
	c++ src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/movement.hpp src/monsterType.hpp src/monster.hpp src/optionalRef.hpp src/bonus.hpp src/monsterIntrinsics.hpp src/output.hpp src/materialType.hpp src/coord.hpp src/terrain.hpp src/zone.hpp src/itemholder.hpp src/level.hpp src/time.hpp src/chargen.hpp src/dungeon.hpp src/renderable.hpp src/iterable.hpp src/player.hpp src/characteristic.hpp src/slots.hpp 
	c++ src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/itemTypes.hpp src/characteristic.hpp src/slots.hpp src/renderable.hpp src/iterable.hpp src/encyclopedia.hpp src/zone.hpp src/itemholder.hpp src/time.hpp src/level.hpp src/bonus.hpp src/materialType.hpp src/coord.hpp src/items.hpp src/monsterIntrinsics.hpp src/output.hpp src/religion.hpp src/movement.hpp src/monsterType.hpp src/damage.hpp src/monster.hpp src/optionalRef.hpp 
	c++ src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/encyclopedia.hpp src/itemholder.hpp src/zone.hpp src/time.hpp src/level.hpp src/itemTypes.hpp src/random.hpp src/slots.hpp src/characteristic.hpp src/iterable.hpp src/renderable.hpp src/monsterType.hpp src/movement.hpp src/damage.hpp src/optionalRef.hpp src/monster.hpp src/shop.hpp src/bonus.hpp src/coord.hpp src/materialType.hpp src/output.hpp src/items.hpp src/monsterIntrinsics.hpp 
	c++ src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/zone.hpp src/itemholder.hpp src/encyclopedia.hpp src/level.hpp src/time.hpp src/random.hpp src/itemTypes.hpp src/renderable.hpp src/iterable.hpp src/characteristic.hpp src/slots.hpp src/damage.hpp src/movement.hpp src/religion.hpp src/monsterType.hpp src/action.hpp src/optionalRef.hpp src/monster.hpp src/bonus.hpp src/shop.hpp src/monsterIntrinsics.hpp src/items.hpp src/output.hpp src/materialType.hpp src/coord.hpp 
	c++ src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/materialType.hpp src/damage.hpp src/renderable.hpp src/random.hpp src/itemTypes.hpp 
	c++ src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/characteristic.hpp src/slots.hpp src/renderable.hpp src/player.hpp src/iterable.hpp src/itemTypes.hpp src/random.hpp src/dungeon.hpp src/time.hpp src/level.hpp src/zone.hpp src/itemholder.hpp src/materialType.hpp src/coord.hpp src/terrain.hpp src/items.hpp src/monsterIntrinsics.hpp src/output.hpp src/bonus.hpp src/monster.hpp src/optionalRef.hpp src/labyrinth.hpp src/movement.hpp src/monsterType.hpp src/shrine.hpp src/damage.hpp 
	c++ src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/output.hpp src/monsterIntrinsics.hpp src/coord.hpp src/materialType.hpp src/bonus.hpp src/monster.hpp src/role.hpp src/optionalRef.hpp src/xo.hpp src/damage.hpp src/monsterType.hpp src/movement.hpp src/religion.hpp src/player.hpp src/iterable.hpp src/renderable.hpp src/slots.hpp src/characteristic.hpp src/dungeon.hpp src/itemTypes.hpp src/level.hpp src/time.hpp src/itemholder.hpp src/zone.hpp 
	c++ src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/level.hpp src/time.hpp src/zone.hpp src/itemholder.hpp src/renderable.hpp src/iterable.hpp src/characteristic.hpp src/slots.hpp src/random.hpp src/itemTypes.hpp src/pathfinder.hpp src/action.hpp src/optionalRef.hpp src/monster.hpp src/damage.hpp src/movement.hpp src/graphsearch.hpp src/religion.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/items.hpp src/output.hpp src/materialType.hpp src/terrain.hpp src/coord.hpp src/bonus.hpp 
	c++ src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/renderable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/terrain.hpp src/bonus.hpp src/damage.hpp src/movement.hpp 
	c++ src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/random.hpp src/slots.hpp src/characteristic.hpp src/iterable.hpp src/renderable.hpp src/itemholder.hpp src/zone.hpp src/time.hpp src/level.hpp src/bonus.hpp src/coord.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/religion.hpp src/movement.hpp src/optionalRef.hpp src/monster.hpp src/role.hpp 
	c++ src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/time.hpp src/level.hpp src/itemholder.hpp src/zone.hpp src/slots.hpp src/characteristic.hpp src/iterable.hpp src/player.hpp src/renderable.hpp src/dungeon.hpp src/role.hpp src/monster.hpp src/optionalRef.hpp src/monsterType.hpp src/movement.hpp src/religion.hpp src/coord.hpp src/materialType.hpp src/output.hpp src/monsterIntrinsics.hpp src/bonus.hpp 
	c++ src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/itemholder.hpp src/zone.hpp src/level.hpp src/time.hpp src/dungeon.hpp src/player.hpp src/iterable.hpp src/renderable.hpp src/slots.hpp src/characteristic.hpp src/monsterType.hpp src/movement.hpp src/monster.hpp src/optionalRef.hpp src/bonus.hpp src/output.hpp src/monsterIntrinsics.hpp src/coord.hpp src/materialType.hpp 
	c++ src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/monster.hpp src/bonus.hpp src/coord.hpp src/materialType.hpp src/output.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/zone.hpp src/time.hpp src/level.hpp src/dungeon.hpp src/slots.hpp src/characteristic.hpp src/player.hpp src/iterable.hpp src/renderable.hpp 
	c++ src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/monster.hpp src/optionalRef.hpp src/religion.hpp src/movement.hpp src/monsterType.hpp src/damage.hpp src/materialType.hpp src/coord.hpp src/items.hpp src/monsterIntrinsics.hpp src/output.hpp src/bonus.hpp src/time.hpp src/level.hpp src/zone.hpp src/itemholder.hpp src/characteristic.hpp src/slots.hpp src/renderable.hpp src/iterable.hpp src/player.hpp src/itemTypes.hpp src/dungeon.hpp 
	c++ src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/renderable.hpp src/religion.hpp 
	c++ src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/role.hpp 
	c++ src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/zone.hpp src/itemholder.hpp src/time.hpp src/level.hpp src/itemTypes.hpp src/random.hpp src/characteristic.hpp src/slots.hpp src/renderable.hpp src/iterable.hpp src/religion.hpp src/movement.hpp src/monsterType.hpp src/damage.hpp src/optionalRef.hpp src/monster.hpp src/bonus.hpp src/shop.hpp src/appraise.hpp src/materialType.hpp src/coord.hpp src/monsterIntrinsics.hpp src/items.hpp src/output.hpp 
	c++ src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	c++ src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/bonus.hpp src/coord.hpp src/materialType.hpp src/output.hpp src/monsterIntrinsics.hpp src/items.hpp src/monsterType.hpp src/shrine.hpp src/religion.hpp src/movement.hpp src/damage.hpp src/monster.hpp src/optionalRef.hpp src/itemTypes.hpp src/random.hpp src/slots.hpp src/characteristic.hpp src/iterable.hpp src/player.hpp src/renderable.hpp src/itemholder.hpp src/zone.hpp src/time.hpp src/level.hpp 
	c++ src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/materialType.hpp src/movement.hpp src/monsterType.hpp src/slots.hpp 
	c++ src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/renderable.hpp src/iterable.hpp src/characteristic.hpp src/slots.hpp src/zone.hpp src/itemholder.hpp src/level.hpp src/time.hpp src/bonus.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/coord.hpp src/terrain.hpp src/movement.hpp src/monsterType.hpp src/monster.hpp src/optionalRef.hpp 
	c++ src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	c++ src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

