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

src/action.o : src/action.cpp src/iterable.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/movement.hpp src/items.hpp src/slots.hpp src/renderable.hpp src/action.hpp src/monster.hpp src/zone.hpp src/coord.hpp src/random.hpp src/output.hpp src/bonus.hpp src/damage.hpp src/time.hpp 
	$(CXX) src/action.cpp -c -Wall -g -std=c++11 -o src/action.o -finput-charset=utf8 -fexec-charset=utf8

src/adjectives.o : src/adjectives.cpp 
	$(CXX) src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/time.hpp src/bonus.hpp src/damage.hpp src/zone.hpp src/coord.hpp src/monster.hpp src/renderable.hpp src/items.hpp src/slots.hpp src/action.hpp src/movement.hpp src/itemholder.hpp src/characteristic.hpp src/level.hpp src/appraise.hpp src/monsterType.hpp src/iterable.hpp src/monsterIntrinsics.hpp src/materialType.hpp src/itemTypes.hpp src/optionalRef.hpp 
	$(CXX) src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	$(CXX) src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	$(CXX) src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/monsterIntrinsics.hpp src/iterable.hpp src/monsterType.hpp src/optionalRef.hpp src/materialType.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/movement.hpp src/slots.hpp src/renderable.hpp src/religion.hpp src/dungeon.hpp src/chargen.hpp src/monster.hpp src/coord.hpp src/zone.hpp src/player.hpp src/output.hpp src/role.hpp src/bonus.hpp src/time.hpp 
	$(CXX) src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	$(CXX) src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/materialType.hpp src/damage.hpp 
	$(CXX) src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/dungeon.hpp src/monster.hpp src/chargen.hpp src/zone.hpp src/coord.hpp src/player.hpp src/output.hpp src/role.hpp src/damage.hpp src/bonus.hpp src/time.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/iterable.hpp src/appraise.hpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/terrain.hpp src/movement.hpp src/items.hpp src/renderable.hpp src/slots.hpp src/action.hpp 
	$(CXX) src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/slots.hpp src/renderable.hpp src/items.hpp src/action.hpp src/movement.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/iterable.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/time.hpp src/bonus.hpp src/damage.hpp src/output.hpp src/religion.hpp src/encyclopedia.hpp src/monster.hpp src/zone.hpp src/coord.hpp 
	$(CXX) src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/time.hpp src/damage.hpp src/bonus.hpp src/output.hpp src/encyclopedia.hpp src/zone.hpp src/coord.hpp src/random.hpp src/monster.hpp src/renderable.hpp src/slots.hpp src/items.hpp src/action.hpp src/movement.hpp src/shop.hpp src/itemholder.hpp src/characteristic.hpp src/level.hpp src/monsterIntrinsics.hpp src/iterable.hpp src/monsterType.hpp src/materialType.hpp src/itemTypes.hpp src/optionalRef.hpp 
	$(CXX) src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/iterable.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/shop.hpp src/movement.hpp src/action.hpp src/slots.hpp src/items.hpp src/renderable.hpp src/monster.hpp src/random.hpp src/coord.hpp src/zone.hpp src/religion.hpp src/encyclopedia.hpp src/output.hpp src/damage.hpp src/bonus.hpp src/time.hpp 
	$(CXX) src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/renderable.hpp src/itemTypes.hpp src/materialType.hpp src/damage.hpp src/random.hpp 
	$(CXX) src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/monsterIntrinsics.hpp src/monsterType.hpp src/iterable.hpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/shrine.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/terrain.hpp src/levelGen.hpp src/movement.hpp src/renderable.hpp src/slots.hpp src/items.hpp src/action.hpp src/religion.hpp src/dungeon.hpp src/monster.hpp src/coord.hpp src/random.hpp src/zone.hpp src/player.hpp src/output.hpp src/role.hpp src/bonus.hpp src/damage.hpp src/time.hpp src/labyrinth.hpp 
	$(CXX) src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/movement.hpp src/renderable.hpp src/slots.hpp src/materialType.hpp src/optionalRef.hpp src/itemTypes.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/iterable.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/damage.hpp src/bonus.hpp src/role.hpp src/xo.hpp src/time.hpp src/zone.hpp src/coord.hpp src/monster.hpp src/dungeon.hpp src/religion.hpp src/output.hpp src/player.hpp 
	$(CXX) src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/terrain.hpp src/monsterIntrinsics.hpp src/iterable.hpp src/monsterType.hpp src/itemTypes.hpp src/optionalRef.hpp src/materialType.hpp src/items.hpp src/renderable.hpp src/slots.hpp src/action.hpp src/graphsearch.hpp src/movement.hpp src/player.hpp src/output.hpp src/religion.hpp src/dungeon.hpp src/monster.hpp src/random.hpp src/coord.hpp src/zone.hpp src/pathfinder.hpp src/time.hpp src/bonus.hpp src/damage.hpp 
	$(CXX) src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/damage.hpp src/bonus.hpp src/renderable.hpp src/movement.hpp src/terrain.hpp src/monsterIntrinsics.hpp src/materialType.hpp 
	$(CXX) src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/religion.hpp src/monster.hpp src/coord.hpp src/zone.hpp src/random.hpp src/role.hpp src/bonus.hpp src/time.hpp src/iterable.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/optionalRef.hpp src/materialType.hpp src/characteristic.hpp src/itemholder.hpp src/level.hpp src/movement.hpp src/renderable.hpp src/slots.hpp 
	$(CXX) src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/movement.hpp src/renderable.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/iterable.hpp src/monsterType.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/bonus.hpp src/role.hpp src/time.hpp src/monster.hpp src/zone.hpp src/coord.hpp src/religion.hpp src/dungeon.hpp src/output.hpp src/player.hpp 
	$(CXX) src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/renderable.hpp src/slots.hpp src/movement.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/materialType.hpp src/optionalRef.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/iterable.hpp src/time.hpp src/bonus.hpp src/output.hpp src/player.hpp src/coord.hpp src/zone.hpp src/monster.hpp src/dungeon.hpp 
	$(CXX) src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/movement.hpp src/renderable.hpp src/slots.hpp src/optionalRef.hpp src/materialType.hpp src/monsterType.hpp src/iterable.hpp src/monsterIntrinsics.hpp src/level.hpp src/characteristic.hpp src/itemholder.hpp src/bonus.hpp src/time.hpp src/monster.hpp src/coord.hpp src/zone.hpp src/dungeon.hpp src/output.hpp src/player.hpp 
	$(CXX) src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/optionalRef.hpp src/itemTypes.hpp src/materialType.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/iterable.hpp src/action.hpp src/items.hpp src/renderable.hpp src/slots.hpp src/movement.hpp src/output.hpp src/player.hpp src/monster.hpp src/zone.hpp src/coord.hpp src/religion.hpp src/dungeon.hpp src/time.hpp src/damage.hpp src/bonus.hpp 
	$(CXX) src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/renderable.hpp src/religion.hpp 
	$(CXX) src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/levelGen.hpp src/movement.hpp src/slots.hpp src/renderable.hpp src/optionalRef.hpp src/materialType.hpp src/iterable.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/level.hpp src/terrain.hpp src/itemholder.hpp src/characteristic.hpp src/bonus.hpp src/role.hpp src/time.hpp src/monster.hpp src/coord.hpp src/zone.hpp 
	$(CXX) src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/movement.hpp src/action.hpp src/renderable.hpp src/slots.hpp src/items.hpp src/materialType.hpp src/itemTypes.hpp src/optionalRef.hpp src/appraise.hpp src/monsterType.hpp src/monsterIntrinsics.hpp src/iterable.hpp src/level.hpp src/shop.hpp src/characteristic.hpp src/itemholder.hpp src/bonus.hpp src/damage.hpp src/time.hpp src/coord.hpp src/zone.hpp src/random.hpp src/monster.hpp src/religion.hpp src/output.hpp 
	$(CXX) src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	$(CXX) src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/damage.hpp src/bonus.hpp src/time.hpp src/monster.hpp src/coord.hpp src/random.hpp src/zone.hpp src/religion.hpp src/output.hpp src/player.hpp src/movement.hpp src/action.hpp src/items.hpp src/renderable.hpp src/slots.hpp src/optionalRef.hpp src/itemTypes.hpp src/shrine.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/iterable.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp 
	$(CXX) src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/monsterType.hpp src/materialType.hpp src/movement.hpp src/slots.hpp 
	$(CXX) src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/time.hpp src/bonus.hpp src/zone.hpp src/coord.hpp src/monster.hpp src/slots.hpp src/renderable.hpp src/movement.hpp src/terrain.hpp src/level.hpp src/itemholder.hpp src/characteristic.hpp src/materialType.hpp src/optionalRef.hpp src/monsterType.hpp src/iterable.hpp src/monsterIntrinsics.hpp 
	$(CXX) src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	$(CXX) src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

