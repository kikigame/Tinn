# This is an auto-generated file. Please make changes in build.pl

CXX ?= c++

WINCXX ?= /usr/bin/x86_64-w64-mingw32-c++ -I../mingw-64/ncurses/include -unicode -L../mingw-64/ncurses/lib/ -Wl,--enable-auto-import

CXXLINK ?= -lncursesw

WINCXXLINK = -lncursesw -lpsapi -static

tinn : Makefile ofiles 
ofiles : src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dreamscape.o src/dungeon.o src/encyclopedia.o src/equippable.o src/geometry.o src/item.o src/itemholder.o src/items.o src/itemType.o src/level.o src/levelFactory.o src/main.o src/mobile.o src/monster.o src/monsterFactory.o src/monsterIntrinsics.o src/monstermutation.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/polymorph.o src/quest.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/spaceZone.o src/target.o src/terrain.o src/time.o src/transport.o 

	$(CXX)  src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dreamscape.o src/dungeon.o src/encyclopedia.o src/equippable.o src/geometry.o src/item.o src/itemholder.o src/items.o src/itemType.o src/level.o src/levelFactory.o src/main.o src/mobile.o src/monster.o src/monsterFactory.o src/monsterIntrinsics.o src/monstermutation.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/polymorph.o src/quest.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/spaceZone.o src/target.o src/terrain.o src/time.o src/transport.o  -Wall -g -std=c++11 $(CXXLINK) -o tinn

# Windown port 
tinn.exe : Makefile clean 
	CXX="$(WINCXX)" make -k ofiles && \
	$(WINCXX)  src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dreamscape.o src/dungeon.o src/encyclopedia.o src/equippable.o src/geometry.o src/item.o src/itemholder.o src/items.o src/itemType.o src/level.o src/levelFactory.o src/main.o src/mobile.o src/monster.o src/monsterFactory.o src/monsterIntrinsics.o src/monstermutation.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/polymorph.o src/quest.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/spaceZone.o src/target.o src/terrain.o src/time.o src/transport.o  -Wall -g -std=c++11 $(CXXLINK) -o tinn.exe

Makefile: build.pl
	./build.pl > Makefile

lint:
	cppcheck --enable=performance --enable=warning --enable=portability src

clean:
	rm -f   src/action.o src/adjectives.o src/appraise.o src/bonus.o src/characteristic.o src/chargen.o src/coord.o src/damage.o src/dreamscape.o src/dungeon.o src/encyclopedia.o src/equippable.o src/geometry.o src/item.o src/itemholder.o src/items.o src/itemType.o src/level.o src/levelFactory.o src/main.o src/mobile.o src/monster.o src/monsterFactory.o src/monsterIntrinsics.o src/monstermutation.o src/monsterType.o src/output.o src/output_curses.o src/output_term.o src/player.o src/polymorph.o src/quest.o src/religion.o src/role.o src/shop.o src/shopkeeper.o src/shrine.o src/slots.o src/spaceZone.o src/target.o src/terrain.o src/time.o src/transport.o 

src/action.o : src/action.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/random.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/action.cpp -c -Wall -g -std=c++11 -o src/action.o -finput-charset=utf8 -fexec-charset=utf8

src/adjectives.o : src/adjectives.cpp 
	$(CXX) src/adjectives.cpp -c -Wall -g -std=c++11 -o src/adjectives.o -finput-charset=utf8 -fexec-charset=utf8

src/appraise.o : src/appraise.cpp src/action.hpp src/appraise.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/appraise.cpp -c -Wall -g -std=c++11 -o src/appraise.o -finput-charset=utf8 -fexec-charset=utf8

src/bonus.o : src/bonus.cpp src/bonus.hpp 
	$(CXX) src/bonus.cpp -c -Wall -g -std=c++11 -o src/bonus.o -finput-charset=utf8 -fexec-charset=utf8

src/characteristic.o : src/characteristic.cpp src/characteristic.hpp 
	$(CXX) src/characteristic.cpp -c -Wall -g -std=c++11 -o src/characteristic.o -finput-charset=utf8 -fexec-charset=utf8

src/chargen.o : src/chargen.cpp src/bonus.hpp src/characteristic.hpp src/chargen.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/chargen.cpp -c -Wall -g -std=c++11 -o src/chargen.o -finput-charset=utf8 -fexec-charset=utf8

src/coord.o : src/coord.cpp src/coord.hpp 
	$(CXX) src/coord.cpp -c -Wall -g -std=c++11 -o src/coord.o -finput-charset=utf8 -fexec-charset=utf8

src/damage.o : src/damage.cpp src/damage.hpp src/materialType.hpp 
	$(CXX) src/damage.cpp -c -Wall -g -std=c++11 -o src/damage.o -finput-charset=utf8 -fexec-charset=utf8

src/dreamscape.o : src/dreamscape.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/dreamscape.cpp -c -Wall -g -std=c++11 -o src/dreamscape.o -finput-charset=utf8 -fexec-charset=utf8

src/dungeon.o : src/dungeon.cpp src/action.hpp src/appraise.hpp src/bonus.hpp src/characteristic.hpp src/chargen.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/dungeon.cpp -c -Wall -g -std=c++11 -o src/dungeon.o -finput-charset=utf8 -fexec-charset=utf8

src/encyclopedia.o : src/encyclopedia.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/encyclopedia.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/encyclopedia.cpp -c -Wall -g -std=c++11 -o src/encyclopedia.o -finput-charset=utf8 -fexec-charset=utf8

src/equippable.o : src/equippable.cpp src/action.hpp src/bonus.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/random.hpp src/renderable.hpp src/slots.hpp 
	$(CXX) src/equippable.cpp -c -Wall -g -std=c++11 -o src/equippable.o -finput-charset=utf8 -fexec-charset=utf8

src/geometry.o : src/geometry.cpp src/coord.hpp src/geometry.hpp 
	$(CXX) src/geometry.cpp -c -Wall -g -std=c++11 -o src/geometry.o -finput-charset=utf8 -fexec-charset=utf8

src/item.o : src/item.cpp src/action.hpp src/bonus.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/materialType.hpp src/optionalRef.hpp src/random.hpp src/renderable.hpp src/slots.hpp 
	$(CXX) src/item.cpp -c -Wall -g -std=c++11 -o src/item.o -finput-charset=utf8 -fexec-charset=utf8

src/itemholder.o : src/itemholder.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/encyclopedia.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/random.hpp src/ref.hpp src/renderable.hpp src/shop.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/itemholder.cpp -c -Wall -g -std=c++11 -o src/itemholder.o -finput-charset=utf8 -fexec-charset=utf8

src/items.o : src/items.cpp src/action.hpp src/appraise.hpp src/bonus.hpp src/characteristic.hpp src/combat.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/encyclopedia.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/shop.hpp src/slots.hpp src/target.hpp src/terrain.hpp src/time.hpp src/transport.hpp src/zone.hpp 
	$(CXX) src/items.cpp -c -Wall -g -std=c++11 -o src/items.o -finput-charset=utf8 -fexec-charset=utf8

src/itemType.o : src/itemType.cpp src/damage.hpp src/itemTypes.hpp src/materialType.hpp src/random.hpp src/renderable.hpp 
	$(CXX) src/itemType.cpp -c -Wall -g -std=c++11 -o src/itemType.o -finput-charset=utf8 -fexec-charset=utf8

src/level.o : src/level.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/combat.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/geometry.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/labyrinth.hpp src/level.hpp src/levelFactory.hpp src/levelGen.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/shrine.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/transport.hpp src/zone.hpp 
	$(CXX) src/level.cpp -c -Wall -g -std=c++11 -o src/level.o -finput-charset=utf8 -fexec-charset=utf8

src/levelFactory.o : src/levelFactory.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/geometry.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/levelFactory.hpp src/levelGen.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/slots.hpp src/spaceZone.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/levelFactory.cpp -c -Wall -g -std=c++11 -o src/levelFactory.o -finput-charset=utf8 -fexec-charset=utf8

src/main.o : src/main.cpp src/args.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/time.hpp src/xo.hpp src/zone.hpp 
	$(CXX) src/main.cpp -c -Wall -g -std=c++11 -o src/main.o -finput-charset=utf8 -fexec-charset=utf8

src/mobile.o : src/mobile.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/mobile.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/pathfinder.hpp src/player.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/slots.hpp src/target.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/mobile.cpp -c -Wall -g -std=c++11 -o src/mobile.o -finput-charset=utf8 -fexec-charset=utf8

src/monster.o : src/monster.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/random.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/monster.cpp -c -Wall -g -std=c++11 -o src/monster.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterFactory.o : src/monsterFactory.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/mobile.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/monsterFactory.cpp -c -Wall -g -std=c++11 -o src/monsterFactory.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterIntrinsics.o : src/monsterIntrinsics.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/monsterIntrinsics.cpp -c -Wall -g -std=c++11 -o src/monsterIntrinsics.o -finput-charset=utf8 -fexec-charset=utf8

src/monstermutation.o : src/monstermutation.cpp src/bonus.hpp src/damage.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/monstermutation.hpp src/movement.hpp 
	$(CXX) src/monstermutation.cpp -c -Wall -g -std=c++11 -o src/monstermutation.o -finput-charset=utf8 -fexec-charset=utf8

src/monsterType.o : src/monsterType.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/quest.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/monsterType.cpp -c -Wall -g -std=c++11 -o src/monsterType.o -finput-charset=utf8 -fexec-charset=utf8

src/output.o : src/output.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/output.cpp -c -Wall -g -std=c++11 -o src/output.o -finput-charset=utf8 -fexec-charset=utf8

src/output_curses.o : src/output_curses.cpp src/args.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/output_curses.cpp -c -Wall -g -std=c++11 -o src/output_curses.o -finput-charset=utf8 -fexec-charset=utf8

src/output_term.o : src/output_term.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/output_term.cpp -c -Wall -g -std=c++11 -o src/output_term.o -finput-charset=utf8 -fexec-charset=utf8

src/player.o : src/player.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/dungeon.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/role.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/player.cpp -c -Wall -g -std=c++11 -o src/player.o -finput-charset=utf8 -fexec-charset=utf8

src/polymorph.o : src/polymorph.cpp src/action.hpp src/bonus.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/materialType.hpp src/optionalRef.hpp src/output.hpp src/renderable.hpp src/slots.hpp 
	$(CXX) src/polymorph.cpp -c -Wall -g -std=c++11 -o src/polymorph.o -finput-charset=utf8 -fexec-charset=utf8

src/quest.o : src/quest.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/levelGen.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/quest.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/quest.cpp -c -Wall -g -std=c++11 -o src/quest.o -finput-charset=utf8 -fexec-charset=utf8

src/religion.o : src/religion.cpp src/religion.hpp src/renderable.hpp 
	$(CXX) src/religion.cpp -c -Wall -g -std=c++11 -o src/religion.o -finput-charset=utf8 -fexec-charset=utf8

src/role.o : src/role.cpp src/quest.hpp src/role.hpp 
	$(CXX) src/role.cpp -c -Wall -g -std=c++11 -o src/role.o -finput-charset=utf8 -fexec-charset=utf8

src/shop.o : src/shop.cpp src/action.hpp src/appraise.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/shop.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/shop.cpp -c -Wall -g -std=c++11 -o src/shop.o -finput-charset=utf8 -fexec-charset=utf8

src/shopkeeper.o : src/shopkeeper.cpp 
	$(CXX) src/shopkeeper.cpp -c -Wall -g -std=c++11 -o src/shopkeeper.o -finput-charset=utf8 -fexec-charset=utf8

src/shrine.o : src/shrine.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/geometry.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/player.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/shrine.hpp src/slots.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/shrine.cpp -c -Wall -g -std=c++11 -o src/shrine.o -finput-charset=utf8 -fexec-charset=utf8

src/slots.o : src/slots.cpp src/bonus.hpp src/materialType.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/slots.hpp 
	$(CXX) src/slots.cpp -c -Wall -g -std=c++11 -o src/slots.o -finput-charset=utf8 -fexec-charset=utf8

src/spaceZone.o : src/spaceZone.cpp src/action.hpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/geometry.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/spaceZone.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/spaceZone.cpp -c -Wall -g -std=c++11 -o src/spaceZone.o -finput-charset=utf8 -fexec-charset=utf8

src/target.o : src/target.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/output.hpp src/random.hpp src/ref.hpp src/religion.hpp src/renderable.hpp src/slots.hpp src/target.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/target.cpp -c -Wall -g -std=c++11 -o src/target.o -finput-charset=utf8 -fexec-charset=utf8

src/terrain.o : src/terrain.cpp src/bonus.hpp src/characteristic.hpp src/coord.hpp src/damage.hpp src/equippable.hpp src/graphsearch.hpp src/hasAdjectives.hpp src/itemholder.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/monster.hpp src/monsterIntrinsics.hpp src/monsterType.hpp src/movement.hpp src/optionalRef.hpp src/random.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/zone.hpp 
	$(CXX) src/terrain.cpp -c -Wall -g -std=c++11 -o src/terrain.o -finput-charset=utf8 -fexec-charset=utf8

src/time.o : src/time.cpp src/time.hpp 
	$(CXX) src/time.cpp -c -Wall -g -std=c++11 -o src/time.o -finput-charset=utf8 -fexec-charset=utf8

src/transport.o : src/transport.cpp src/action.hpp src/bonus.hpp src/coord.hpp src/damage.hpp src/hasAdjectives.hpp src/itemTypes.hpp src/itemholder.hpp src/items.hpp src/iterable.hpp src/level.hpp src/materialType.hpp src/movement.hpp src/optionalRef.hpp src/pathfinder.hpp src/random.hpp src/ref.hpp src/renderable.hpp src/slots.hpp src/terrain.hpp src/time.hpp src/transport.hpp src/zone.hpp 
	$(CXX) src/transport.cpp -c -Wall -g -std=c++11 -o src/transport.o -finput-charset=utf8 -fexec-charset=utf8

