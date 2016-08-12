CFLAGS = -std=c++11
LFLAGS = -ldl

main: core.cpp
	g++ -c $(CFLAGS) core.cpp
	g++ -o p core.o $(LFLAGS)
clear: 
	rm *.o p
