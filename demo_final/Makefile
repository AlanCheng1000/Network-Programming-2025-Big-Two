CXX = g++
CXXFLAGS = -I ../../../lib -g -std=c++17 -O2 -D_REENTRANT -Wall
LIBS = ../../../libunp.a -lpthread

PROGS =	big_two_cli big_two_serv
OBJECTS = card.o combination.o hand.o deck.o player.o game.o

# database.o is ignored for now

all:	${PROGS}

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

big_two_cli:	big_two_cli.o $(OBJECTS)
		${CXX} ${CXXFLAGS} $^ ${LIBS} -o $@

big_two_serv:	big_two_serv.o $(OBJECTS)
		${CXX} ${CXXFLAGS} $^ ${LIBS} -o $@

clean:
	rm -f ${PROGS} ${OBJECTS} big_two_cli.o big_two_serv.o

		
