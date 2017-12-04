# This is an auto-generated file. Please make changes in build.pl

tinn : Makefile src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 
	c++  src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 -lncursesw -o tinn

Makefile: build.pl
	./build.pl > Makefile

clean:
	rm -f   src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 

src/adjectives.o : src/adjectives.cpp 
	c++ src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/items.hpp src/time.hpp src/bonus.hpp src/monster.hpp src/level.hpp src/itemTypes.hpp src/iterable.hpp src/appraise.hpp src/monsterIntrinsics.hpp src/slots.hpp src/materialType.hpp src/damage.hpp src/renderable.hpp src/movement.hpp src/itemholder.hpp src/coord.hpp src/zone.hpp src/characteristic.hpp src/monsterType.hpp 
	c++ src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	c++ src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	c++ src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/iterable.hpp src/chargen.hpp src/level.hpp src/bonus.hpp src/time.hpp src/monster.hpp src/religion.hpp src/player.hpp src/role.hpp src/monsterType.hpp src/characteristic.hpp src/coord.hpp src/zone.hpp src/movement.hpp src/itemholder.hpp src/dungeon.hpp src/renderable.hpp src/materialType.hpp src/slots.hpp src/output.hpp src/monsterIntrinsics.hpp 
	c++ src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	c++ src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/materialType.hpp src/damage.hpp 
	c++ src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/monster.hpp src/bonus.hpp src/time.hpp src/level.hpp src/terrain.hpp src/chargen.hpp src/iterable.hpp src/dungeon.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/monsterIntrinsics.hpp src/slots.hpp src/output.hpp src/materialType.hpp src/characteristic.hpp src/monsterType.hpp src/player.hpp src/coord.hpp src/zone.hpp 
	c++ src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/itemholder.hpp src/movement.hpp src/damage.hpp src/renderable.hpp src/output.hpp src/materialType.hpp src/slots.hpp src/monsterIntrinsics.hpp src/religion.hpp src/characteristic.hpp src/monsterType.hpp src/coord.hpp src/zone.hpp src/level.hpp src/encyclopedia.hpp src/items.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/iterable.hpp src/itemTypes.hpp 
	c++ src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/itemTypes.hpp src/iterable.hpp src/items.hpp src/time.hpp src/monster.hpp src/bonus.hpp src/level.hpp src/encyclopedia.hpp src/coord.hpp src/random.hpp src/zone.hpp src/characteristic.hpp src/shop.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/output.hpp src/slots.hpp src/materialType.hpp src/renderable.hpp src/damage.hpp src/itemholder.hpp src/movement.hpp 
	c++ src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/itemholder.hpp src/movement.hpp src/damage.hpp src/renderable.hpp src/materialType.hpp src/output.hpp src/slots.hpp src/monsterIntrinsics.hpp src/shop.hpp src/monsterType.hpp src/characteristic.hpp src/random.hpp src/coord.hpp src/zone.hpp src/level.hpp src/encyclopedia.hpp src/items.hpp src/bonus.hpp src/time.hpp src/monster.hpp src/iterable.hpp src/itemTypes.hpp 
	c++ src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/materialType.hpp src/renderable.hpp src/damage.hpp src/random.hpp src/itemTypes.hpp 
	c++ src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/terrain.hpp src/level.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/items.hpp src/iterable.hpp src/itemTypes.hpp src/itemholder.hpp src/movement.hpp src/renderable.hpp src/damage.hpp src/dungeon.hpp src/materialType.hpp src/slots.hpp src/output.hpp src/monsterIntrinsics.hpp src/labyrinth.hpp src/player.hpp src/monsterType.hpp src/characteristic.hpp src/shrine.hpp src/zone.hpp src/random.hpp src/coord.hpp 
	c++ src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/religion.hpp src/monsterType.hpp src/characteristic.hpp src/role.hpp src/player.hpp src/zone.hpp src/coord.hpp src/itemholder.hpp src/movement.hpp src/damage.hpp src/renderable.hpp src/dungeon.hpp src/slots.hpp src/output.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/xo.hpp src/iterable.hpp src/itemTypes.hpp src/level.hpp src/debug.hpp src/monster.hpp src/time.hpp src/bonus.hpp 
	c++ src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/zone.hpp src/coord.hpp src/random.hpp src/religion.hpp src/monsterType.hpp src/characteristic.hpp src/slots.hpp src/output.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/movement.hpp src/itemholder.hpp src/damage.hpp src/renderable.hpp src/iterable.hpp src/itemTypes.hpp src/terrain.hpp src/level.hpp src/monster.hpp src/bonus.hpp src/time.hpp src/items.hpp src/graphsearch.hpp 
	c++ src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/materialType.hpp src/monsterIntrinsics.hpp src/movement.hpp src/renderable.hpp src/damage.hpp src/terrain.hpp src/bonus.hpp 
	c++ src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/bonus.hpp src/monster.hpp src/time.hpp src/level.hpp src/iterable.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/slots.hpp src/role.hpp src/characteristic.hpp src/monsterType.hpp src/religion.hpp src/zone.hpp src/random.hpp src/coord.hpp 
	c++ src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/iterable.hpp src/level.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/coord.hpp src/zone.hpp src/religion.hpp src/characteristic.hpp src/monsterType.hpp src/role.hpp src/player.hpp src/materialType.hpp src/slots.hpp src/output.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/movement.hpp src/dungeon.hpp src/renderable.hpp 
	c++ src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/iterable.hpp src/monster.hpp src/bonus.hpp src/time.hpp src/level.hpp src/characteristic.hpp src/monsterType.hpp src/player.hpp src/coord.hpp src/zone.hpp src/dungeon.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/monsterIntrinsics.hpp src/slots.hpp src/materialType.hpp src/output.hpp 
	c++ src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/iterable.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/level.hpp src/characteristic.hpp src/player.hpp src/monsterType.hpp src/coord.hpp src/zone.hpp src/dungeon.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/slots.hpp src/output.hpp 
	c++ src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/coord.hpp src/zone.hpp src/player.hpp src/monsterType.hpp src/characteristic.hpp src/religion.hpp src/monsterIntrinsics.hpp src/output.hpp src/slots.hpp src/materialType.hpp src/dungeon.hpp src/renderable.hpp src/damage.hpp src/movement.hpp src/itemholder.hpp src/itemTypes.hpp src/iterable.hpp src/items.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/level.hpp 
	c++ src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/renderable.hpp src/religion.hpp 
	c++ src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/role.hpp 
	c++ src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/monsterIntrinsics.hpp src/slots.hpp src/output.hpp src/materialType.hpp src/damage.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/zone.hpp src/random.hpp src/coord.hpp src/monsterType.hpp src/characteristic.hpp src/shop.hpp src/religion.hpp src/bonus.hpp src/monster.hpp src/time.hpp src/items.hpp src/level.hpp src/itemTypes.hpp src/iterable.hpp src/appraise.hpp 
	c++ src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	c++ src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/damage.hpp src/renderable.hpp src/itemholder.hpp src/movement.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/output.hpp src/slots.hpp src/monsterType.hpp src/player.hpp src/characteristic.hpp src/religion.hpp src/zone.hpp src/random.hpp src/coord.hpp src/shrine.hpp src/monster.hpp src/bonus.hpp src/time.hpp src/items.hpp src/level.hpp src/itemTypes.hpp src/iterable.hpp 
	c++ src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/materialType.hpp src/slots.hpp src/movement.hpp src/monsterType.hpp 
	c++ src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/terrain.hpp src/renderable.hpp 
	c++ src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	c++ src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

