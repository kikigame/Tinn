# This is an auto-generated file. Please make changes in build.pl

CXX ?= c++

WINCXX ?= /usr/bin/x86_64-w64-mingw32-c++ -I../mingw-64/ncurses/include -unicode -L../mingw-64/ncurses/lib/ -Wl,--enable-auto-import

CXXLINK ?= -lncursesw

WINCXXLINK = -lncursesw -lpsapi -static

tinn : Makefile ofiles 
ofiles : src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 

	$(CXX)  src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 $(CXXLINK) -o tinn

# Windown port 
tinn.exe : Makefile clean 
	CXX="$(WINCXX)" make -k ofiles && \
	$(WINCXX)  src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o  -Wall -g -std=c++11 $(CXXLINK) -o tinn.exe

Makefile: build.pl
	./build.pl > Makefile

lint:
	cppcheck --enable=performance --enable=warning --enable=portability src

clean:
	rm -f   src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dungeon.o src/encyclopedia.o src/itemholder.o src/items.o src/itemType.o src/level.o src/main.o src/monster.o src/monsterIntrinsics.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/terrain.o src/time.o 

src/action.o : src/action.cpp src/itemTypes.hpp src/iterable.hpp src/level.hpp src/output.hpp src/monster.hpp src/zone.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/coord.hpp src/time.hpp src/renderable.hpp src/characteristic.hpp src/items.hpp src/itemholder.hpp src/damage.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/action.hpp src/bonus.hpp src/movement.hpp src/random.hpp 
	$(CXX) src/action.cpp -c -Wall -g -std=c++11 -o src/action.o -finput-charset=utf8 -fexec-charset=utf8

src/adjectives.o : src/adjectives.cpp 
	$(CXX) src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/iterable.hpp src/level.hpp src/itemTypes.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp src/renderable.hpp src/characteristic.hpp src/monsterIntrinsics.hpp src/coord.hpp src/time.hpp src/itemholder.hpp src/items.hpp src/damage.hpp src/appraise.hpp src/action.hpp src/bonus.hpp src/slots.hpp src/materialType.hpp src/optionalRef.hpp src/movement.hpp 
	$(CXX) src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	$(CXX) src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	$(CXX) src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/optionalRef.hpp src/materialType.hpp src/dungeon.hpp src/slots.hpp src/bonus.hpp src/chargen.hpp src/movement.hpp src/player.hpp src/level.hpp src/output.hpp src/iterable.hpp src/zone.hpp src/monster.hpp src/role.hpp src/monsterType.hpp src/time.hpp src/monsterIntrinsics.hpp src/coord.hpp src/characteristic.hpp src/renderable.hpp src/religion.hpp src/itemholder.hpp 
	$(CXX) src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	$(CXX) src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/damage.hpp src/materialType.hpp 
	$(CXX) src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/items.hpp src/itemholder.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/characteristic.hpp src/renderable.hpp src/zone.hpp src/monster.hpp src/role.hpp src/monsterType.hpp src/itemTypes.hpp src/output.hpp src/level.hpp src/iterable.hpp src/player.hpp src/terrain.hpp src/movement.hpp src/optionalRef.hpp src/materialType.hpp src/dungeon.hpp src/slots.hpp src/bonus.hpp src/action.hpp src/chargen.hpp src/appraise.hpp src/damage.hpp 
	$(CXX) src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/encyclopedia.hpp src/damage.hpp src/bonus.hpp src/action.hpp src/materialType.hpp src/optionalRef.hpp src/slots.hpp src/movement.hpp src/output.hpp src/level.hpp src/iterable.hpp src/itemTypes.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp src/religion.hpp src/characteristic.hpp src/renderable.hpp src/time.hpp src/monsterIntrinsics.hpp src/coord.hpp src/itemholder.hpp src/items.hpp 
	$(CXX) src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/itemTypes.hpp src/output.hpp src/level.hpp src/iterable.hpp src/monster.hpp src/zone.hpp src/monsterType.hpp src/time.hpp src/monsterIntrinsics.hpp src/coord.hpp src/renderable.hpp src/characteristic.hpp src/items.hpp src/itemholder.hpp src/encyclopedia.hpp src/damage.hpp src/materialType.hpp src/optionalRef.hpp src/slots.hpp src/shop.hpp src/bonus.hpp src/action.hpp src/movement.hpp src/random.hpp 
	$(CXX) src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/movement.hpp src/random.hpp src/encyclopedia.hpp src/damage.hpp src/action.hpp src/bonus.hpp src/shop.hpp src/slots.hpp src/materialType.hpp src/optionalRef.hpp src/renderable.hpp src/characteristic.hpp src/religion.hpp src/coord.hpp src/monsterIntrinsics.hpp src/time.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/output.hpp src/itemTypes.hpp src/monsterType.hpp src/monster.hpp src/zone.hpp 
	$(CXX) src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/materialType.hpp src/damage.hpp src/random.hpp src/itemTypes.hpp src/renderable.hpp 
	$(CXX) src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/shrine.hpp src/terrain.hpp src/movement.hpp src/player.hpp src/random.hpp src/damage.hpp src/optionalRef.hpp src/materialType.hpp src/labyrinth.hpp src/slots.hpp src/dungeon.hpp src/bonus.hpp src/action.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/levelGen.hpp src/religion.hpp src/characteristic.hpp src/renderable.hpp src/items.hpp src/itemholder.hpp src/itemTypes.hpp src/level.hpp src/output.hpp src/iterable.hpp src/zone.hpp src/monster.hpp src/role.hpp src/monsterType.hpp 
	$(CXX) src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/religion.hpp src/renderable.hpp src/characteristic.hpp src/itemholder.hpp src/itemTypes.hpp src/output.hpp src/level.hpp src/iterable.hpp src/zone.hpp src/monster.hpp src/role.hpp src/monsterType.hpp src/xo.hpp src/movement.hpp src/player.hpp src/damage.hpp src/materialType.hpp src/optionalRef.hpp src/dungeon.hpp src/slots.hpp src/bonus.hpp 
	$(CXX) src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/items.hpp src/itemholder.hpp src/monsterIntrinsics.hpp src/coord.hpp src/time.hpp src/religion.hpp src/renderable.hpp src/characteristic.hpp src/monster.hpp src/zone.hpp src/monsterType.hpp src/pathfinder.hpp src/itemTypes.hpp src/iterable.hpp src/level.hpp src/output.hpp src/random.hpp src/movement.hpp src/terrain.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/action.hpp src/bonus.hpp src/graphsearch.hpp src/damage.hpp 
	$(CXX) src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/movement.hpp src/terrain.hpp src/materialType.hpp src/bonus.hpp src/damage.hpp src/monsterIntrinsics.hpp src/renderable.hpp 
	$(CXX) src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/random.hpp src/movement.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/bonus.hpp src/itemholder.hpp src/monsterIntrinsics.hpp src/coord.hpp src/time.hpp src/religion.hpp src/renderable.hpp src/characteristic.hpp src/role.hpp src/zone.hpp src/monster.hpp src/monsterType.hpp src/iterable.hpp src/level.hpp 
	$(CXX) src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/output.hpp src/level.hpp src/iterable.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp src/role.hpp src/religion.hpp src/characteristic.hpp src/renderable.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/bonus.hpp src/materialType.hpp src/optionalRef.hpp src/dungeon.hpp src/slots.hpp src/movement.hpp src/player.hpp 
	$(CXX) src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/movement.hpp src/player.hpp src/bonus.hpp src/optionalRef.hpp src/materialType.hpp src/dungeon.hpp src/slots.hpp src/characteristic.hpp src/renderable.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/level.hpp src/output.hpp src/iterable.hpp src/monsterType.hpp src/monster.hpp src/zone.hpp 
	$(CXX) src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/movement.hpp src/player.hpp src/bonus.hpp src/optionalRef.hpp src/materialType.hpp src/slots.hpp src/dungeon.hpp src/characteristic.hpp src/renderable.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/itemholder.hpp src/output.hpp src/level.hpp src/iterable.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp 
	$(CXX) src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/damage.hpp src/optionalRef.hpp src/materialType.hpp src/slots.hpp src/dungeon.hpp src/bonus.hpp src/action.hpp src/movement.hpp src/player.hpp src/itemTypes.hpp src/level.hpp src/output.hpp src/iterable.hpp src/zone.hpp src/monster.hpp src/monsterType.hpp src/time.hpp src/coord.hpp src/monsterIntrinsics.hpp src/characteristic.hpp src/renderable.hpp src/religion.hpp src/items.hpp src/itemholder.hpp 
	$(CXX) src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/renderable.hpp src/religion.hpp 
	$(CXX) src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/itemholder.hpp src/renderable.hpp src/characteristic.hpp src/levelGen.hpp src/coord.hpp src/monsterIntrinsics.hpp src/time.hpp src/monsterType.hpp src/role.hpp src/zone.hpp src/monster.hpp src/iterable.hpp src/level.hpp src/movement.hpp src/terrain.hpp src/bonus.hpp src/slots.hpp src/materialType.hpp src/optionalRef.hpp 
	$(CXX) src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/output.hpp src/level.hpp src/iterable.hpp src/itemTypes.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp src/renderable.hpp src/characteristic.hpp src/religion.hpp src/time.hpp src/monsterIntrinsics.hpp src/coord.hpp src/itemholder.hpp src/items.hpp src/damage.hpp src/appraise.hpp src/shop.hpp src/bonus.hpp src/action.hpp src/optionalRef.hpp src/materialType.hpp src/slots.hpp src/movement.hpp src/random.hpp 
	$(CXX) src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	$(CXX) src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/renderable.hpp src/religion.hpp src/characteristic.hpp src/monsterIntrinsics.hpp src/coord.hpp src/time.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/output.hpp src/itemTypes.hpp src/monsterType.hpp src/zone.hpp src/monster.hpp src/movement.hpp src/shrine.hpp src/random.hpp src/player.hpp src/damage.hpp src/action.hpp src/bonus.hpp src/slots.hpp src/materialType.hpp src/optionalRef.hpp 
	$(CXX) src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/slots.hpp src/materialType.hpp src/movement.hpp src/monsterType.hpp 
	$(CXX) src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/iterable.hpp src/level.hpp src/monster.hpp src/zone.hpp src/monsterType.hpp src/coord.hpp src/monsterIntrinsics.hpp src/time.hpp src/renderable.hpp src/characteristic.hpp src/itemholder.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/bonus.hpp src/movement.hpp src/terrain.hpp 
	$(CXX) src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	$(CXX) src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

