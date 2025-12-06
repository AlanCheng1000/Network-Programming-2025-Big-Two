CXX = g++
CXXFLAGS = -I../lib -g -std=c++11 -O2 -D_REENTRANT -Wall
LIBS = ../../libunp.a -lpthread

big_two_cli:	big_two_cli.o
		${CXX} ${CXXFLAGS} -o $@ big_two_cli.o ${LIBS}

big_two_serv:	big_two_serv.o
		${CXX} ${CXXFLAGS} -o $@ big_two_serv.o ${LIBS}
		
