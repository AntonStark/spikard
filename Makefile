CFLAGS = -std=c++11 -g
LFLAGS = -ldl -lpthread

main: core.cpp
	g++ -c $(CFLAGS) main.cpp
	g++ -o p main.o $(LFLAGS)
clear: 
	rm *.o p
