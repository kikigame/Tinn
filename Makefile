# This is an auto-generated file. Please make changes in build.pl

tinn : Makefile src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 
	c++  src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 -lncursesw -o tinn

Makefile: build.pl
	./build.pl > Makefile

lint:
	cppcheck --enable=performance --enable=warning --enable=portability src

clean:
	rm -f   src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 

src/adjectives.o : src/adjectives.cpp 
	c++ src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/optionalRef.hpp src/time.hpp src/damage.hpp src/itemholder.hpp src/itemTypes.hpp src/renderable.hpp src/appraise.hpp src/movement.hpp src/monsterType.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/coord.hpp src/level.hpp src/zone.hpp src/slots.hpp src/monster.hpp src/characteristic.hpp src/iterable.hpp src/bonus.hpp src/items.hpp 
	c++ src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	c++ src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	c++ src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/monsterIntrinsics.hpp src/materialType.hpp src/renderable.hpp src/movement.hpp src/monsterType.hpp src/output.hpp src/dungeon.hpp src/itemholder.hpp src/time.hpp src/optionalRef.hpp src/iterable.hpp src/role.hpp src/characteristic.hpp src/monster.hpp src/bonus.hpp src/slots.hpp src/player.hpp src/coord.hpp src/chargen.hpp src/zone.hpp src/level.hpp src/religion.hpp 
	c++ src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	c++ src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/materialType.hpp src/damage.hpp 
	c++ src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/slots.hpp src/player.hpp src/iterable.hpp src/monster.hpp src/characteristic.hpp src/bonus.hpp src/terrain.hpp src/coord.hpp src/chargen.hpp src/level.hpp src/zone.hpp src/renderable.hpp src/monsterType.hpp src/movement.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/time.hpp src/optionalRef.hpp src/output.hpp src/dungeon.hpp src/itemholder.hpp 
	c++ src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/time.hpp src/optionalRef.hpp src/damage.hpp src/output.hpp src/itemTypes.hpp src/itemholder.hpp src/renderable.hpp src/encyclopedia.hpp src/monsterType.hpp src/movement.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/religion.hpp src/coord.hpp src/zone.hpp src/level.hpp src/slots.hpp src/monster.hpp src/characteristic.hpp src/iterable.hpp src/items.hpp src/bonus.hpp 
	c++ src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/optionalRef.hpp src/time.hpp src/damage.hpp src/output.hpp src/itemTypes.hpp src/itemholder.hpp src/renderable.hpp src/encyclopedia.hpp src/movement.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/random.hpp src/materialType.hpp src/coord.hpp src/level.hpp src/zone.hpp src/slots.hpp src/shop.hpp src/iterable.hpp src/monster.hpp src/characteristic.hpp src/items.hpp src/bonus.hpp 
	c++ src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/slots.hpp src/shop.hpp src/monster.hpp src/iterable.hpp src/characteristic.hpp src/bonus.hpp src/items.hpp src/coord.hpp src/zone.hpp src/level.hpp src/renderable.hpp src/movement.hpp src/monsterType.hpp src/encyclopedia.hpp src/materialType.hpp src/random.hpp src/monsterIntrinsics.hpp src/optionalRef.hpp src/time.hpp src/damage.hpp src/output.hpp src/itemholder.hpp src/itemTypes.hpp 
	c++ src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/itemTypes.hpp src/damage.hpp src/materialType.hpp src/random.hpp src/renderable.hpp 
	c++ src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/bonus.hpp src/items.hpp src/monster.hpp src/characteristic.hpp src/iterable.hpp src/player.hpp src/slots.hpp src/level.hpp src/zone.hpp src/coord.hpp src/shrine.hpp src/terrain.hpp src/random.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/movement.hpp src/monsterType.hpp src/labyrinth.hpp src/renderable.hpp src/itemholder.hpp src/itemTypes.hpp src/dungeon.hpp src/output.hpp src/damage.hpp src/time.hpp src/optionalRef.hpp 
	c++ src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/materialType.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/xo.hpp src/renderable.hpp src/itemholder.hpp src/itemTypes.hpp src/dungeon.hpp src/output.hpp src/damage.hpp src/optionalRef.hpp src/time.hpp src/bonus.hpp src/monster.hpp src/iterable.hpp src/role.hpp src/characteristic.hpp src/player.hpp src/slots.hpp src/level.hpp src/zone.hpp src/coord.hpp src/religion.hpp 
	c++ src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/characteristic.hpp src/pathfinder.hpp src/monster.hpp src/iterable.hpp src/bonus.hpp src/items.hpp src/slots.hpp src/coord.hpp src/graphsearch.hpp src/zone.hpp src/level.hpp src/religion.hpp src/terrain.hpp src/random.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/renderable.hpp src/monsterType.hpp src/movement.hpp src/output.hpp src/itemholder.hpp src/itemTypes.hpp src/time.hpp src/optionalRef.hpp src/damage.hpp 
	c++ src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/movement.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/damage.hpp src/bonus.hpp src/terrain.hpp 
	c++ src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/religion.hpp src/zone.hpp src/level.hpp src/coord.hpp src/slots.hpp src/bonus.hpp src/monster.hpp src/iterable.hpp src/role.hpp src/characteristic.hpp src/time.hpp src/optionalRef.hpp src/itemholder.hpp src/movement.hpp src/monsterType.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/random.hpp 
	c++ src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/output.hpp src/dungeon.hpp src/itemholder.hpp src/optionalRef.hpp src/time.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/renderable.hpp src/monsterType.hpp src/movement.hpp src/coord.hpp src/level.hpp src/zone.hpp src/religion.hpp src/characteristic.hpp src/iterable.hpp src/monster.hpp src/role.hpp src/bonus.hpp src/slots.hpp src/player.hpp 
	c++ src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/movement.hpp src/monsterType.hpp src/renderable.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/time.hpp src/optionalRef.hpp src/itemholder.hpp src/dungeon.hpp src/output.hpp src/player.hpp src/slots.hpp src/bonus.hpp src/monster.hpp src/characteristic.hpp src/iterable.hpp src/level.hpp src/zone.hpp src/coord.hpp 
	c++ src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/optionalRef.hpp src/time.hpp src/itemholder.hpp src/output.hpp src/dungeon.hpp src/monsterType.hpp src/movement.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/level.hpp src/zone.hpp src/coord.hpp src/player.hpp src/slots.hpp src/bonus.hpp src/monster.hpp src/iterable.hpp src/characteristic.hpp 
	c++ src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/damage.hpp src/time.hpp src/optionalRef.hpp src/itemTypes.hpp src/itemholder.hpp src/output.hpp src/dungeon.hpp src/monsterType.hpp src/movement.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/religion.hpp src/level.hpp src/zone.hpp src/coord.hpp src/player.hpp src/slots.hpp src/items.hpp src/bonus.hpp src/monster.hpp src/iterable.hpp src/characteristic.hpp 
	c++ src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/religion.hpp src/renderable.hpp 
	c++ src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/role.hpp 
	c++ src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/coord.hpp src/level.hpp src/zone.hpp src/religion.hpp src/iterable.hpp src/characteristic.hpp src/monster.hpp src/bonus.hpp src/items.hpp src/slots.hpp src/shop.hpp src/output.hpp src/itemholder.hpp src/itemTypes.hpp src/time.hpp src/optionalRef.hpp src/damage.hpp src/materialType.hpp src/random.hpp src/monsterIntrinsics.hpp src/renderable.hpp src/appraise.hpp src/movement.hpp src/monsterType.hpp 
	c++ src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	c++ src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/shrine.hpp src/religion.hpp src/level.hpp src/zone.hpp src/coord.hpp src/player.hpp src/slots.hpp src/items.hpp src/bonus.hpp src/characteristic.hpp src/iterable.hpp src/monster.hpp src/damage.hpp src/time.hpp src/optionalRef.hpp src/itemTypes.hpp src/itemholder.hpp src/output.hpp src/movement.hpp src/monsterType.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/random.hpp src/materialType.hpp 
	c++ src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/slots.hpp src/monsterType.hpp src/movement.hpp src/materialType.hpp 
	c++ src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/monsterIntrinsics.hpp src/materialType.hpp src/renderable.hpp src/movement.hpp src/monsterType.hpp src/itemholder.hpp src/optionalRef.hpp src/time.hpp src/iterable.hpp src/characteristic.hpp src/monster.hpp src/bonus.hpp src/slots.hpp src/coord.hpp src/zone.hpp src/level.hpp src/terrain.hpp 
	c++ src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	c++ src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

