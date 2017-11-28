# This is an auto-generated file. Please make changes in build.pl

tinn : Makefile src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/slots.o src/terrain.o src/time.o 
	c++ -g   src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 -lncursesw -o tinn

Makefile: build.pl
	./build.pl > Makefile

clean:
	rm -f   src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/slots.o src/terrain.o src/time.o 

src/adjectives.o : src/adjectives.cpp 
	c++ -g src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/coord.hpp src/itemTypes.hpp src/level.hpp src/slots.hpp src/itemholder.hpp src/movement.hpp src/monsterType.hpp src/materialType.hpp src/damage.hpp src/bonus.hpp src/appraise.hpp src/time.hpp src/items.hpp src/monster.hpp 
	c++ -g src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	c++ -g src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	c++ -g src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/monster.hpp src/output.hpp src/religion.hpp src/monsterType.hpp src/materialType.hpp src/bonus.hpp src/time.hpp src/role.hpp src/chargen.hpp src/slots.hpp src/movement.hpp src/player.hpp src/dungeon.hpp src/itemholder.hpp src/monsterIntrinsics.hpp src/renderable.hpp src/characteristic.hpp src/coord.hpp src/level.hpp 
	c++ -g src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	c++ -g src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/materialType.hpp src/damage.hpp 
	c++ -g src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/bonus.hpp src/chargen.hpp src/time.hpp src/terrain.hpp src/materialType.hpp src/monsterType.hpp src/output.hpp src/monster.hpp src/coord.hpp src/level.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/movement.hpp src/dungeon.hpp src/itemholder.hpp src/player.hpp src/slots.hpp 
	c++ -g src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/itemholder.hpp src/movement.hpp src/slots.hpp src/itemTypes.hpp src/coord.hpp src/level.hpp src/monsterIntrinsics.hpp src/characteristic.hpp src/renderable.hpp src/monster.hpp src/output.hpp src/items.hpp src/encyclopedia.hpp src/bonus.hpp src/time.hpp src/damage.hpp src/religion.hpp src/materialType.hpp src/monsterType.hpp 
	c++ -g src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/materialType.hpp src/monsterType.hpp src/damage.hpp src/time.hpp src/shop.hpp src/bonus.hpp src/items.hpp src/encyclopedia.hpp src/output.hpp src/random.hpp src/monster.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/level.hpp src/itemTypes.hpp src/coord.hpp src/slots.hpp src/movement.hpp src/itemholder.hpp 
	c++ -g src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/renderable.hpp src/itemTypes.hpp src/materialType.hpp src/damage.hpp src/random.hpp 
	c++ -g src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/monsterIntrinsics.hpp src/characteristic.hpp src/renderable.hpp src/level.hpp src/itemTypes.hpp src/coord.hpp src/slots.hpp src/itemholder.hpp src/movement.hpp src/dungeon.hpp src/player.hpp src/labyrinth.hpp src/damage.hpp src/materialType.hpp src/monsterType.hpp src/terrain.hpp src/time.hpp src/bonus.hpp src/items.hpp src/monster.hpp src/output.hpp src/random.hpp 
	c++ -g src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/coord.hpp src/itemTypes.hpp src/level.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/xo.hpp src/dungeon.hpp src/itemholder.hpp src/movement.hpp src/player.hpp src/slots.hpp src/bonus.hpp src/debug.hpp src/role.hpp src/time.hpp src/monsterType.hpp src/materialType.hpp src/religion.hpp src/damage.hpp src/output.hpp src/monster.hpp 
	c++ -g src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/monsterIntrinsics.hpp src/renderable.hpp src/characteristic.hpp src/coord.hpp src/itemTypes.hpp src/level.hpp src/slots.hpp src/movement.hpp src/itemholder.hpp src/damage.hpp src/religion.hpp src/terrain.hpp src/monsterType.hpp src/materialType.hpp src/bonus.hpp src/time.hpp src/items.hpp src/monster.hpp src/random.hpp src/graphsearch.hpp src/output.hpp 
	c++ -g src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/bonus.hpp src/terrain.hpp src/materialType.hpp src/damage.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/movement.hpp 
	c++ -g src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/level.hpp src/coord.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/movement.hpp src/itemholder.hpp src/slots.hpp src/time.hpp src/role.hpp src/bonus.hpp src/materialType.hpp src/monsterType.hpp src/religion.hpp src/random.hpp src/monster.hpp 
	c++ -g src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/monsterIntrinsics.hpp src/renderable.hpp src/characteristic.hpp src/coord.hpp src/level.hpp src/slots.hpp src/movement.hpp src/player.hpp src/itemholder.hpp src/dungeon.hpp src/religion.hpp src/monsterType.hpp src/materialType.hpp src/bonus.hpp src/role.hpp src/time.hpp src/monster.hpp src/output.hpp 
	c++ -g src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/coord.hpp src/level.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/dungeon.hpp src/itemholder.hpp src/player.hpp src/movement.hpp src/slots.hpp src/bonus.hpp src/time.hpp src/materialType.hpp src/monsterType.hpp src/output.hpp src/monster.hpp 
	c++ -g src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/output.hpp src/monster.hpp src/monsterType.hpp src/materialType.hpp src/bonus.hpp src/time.hpp src/slots.hpp src/movement.hpp src/player.hpp src/itemholder.hpp src/dungeon.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/coord.hpp src/level.hpp 
	c++ -g src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/time.hpp src/bonus.hpp src/materialType.hpp src/monsterType.hpp src/damage.hpp src/religion.hpp src/output.hpp src/monster.hpp src/items.hpp src/level.hpp src/coord.hpp src/itemTypes.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/player.hpp src/dungeon.hpp src/itemholder.hpp src/movement.hpp src/slots.hpp 
	c++ -g src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/renderable.hpp src/religion.hpp 
	c++ -g src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/role.hpp 
	c++ -g src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/shop.hpp src/bonus.hpp src/appraise.hpp src/time.hpp src/monsterType.hpp src/materialType.hpp src/damage.hpp src/religion.hpp src/random.hpp src/output.hpp src/monster.hpp src/items.hpp src/itemTypes.hpp src/coord.hpp src/level.hpp src/characteristic.hpp src/renderable.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/movement.hpp src/slots.hpp 
	c++ -g src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	c++ -g src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/materialType.hpp src/monsterType.hpp src/slots.hpp src/movement.hpp 
	c++ -g src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/renderable.hpp src/terrain.hpp 
	c++ -g src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	c++ -g src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

