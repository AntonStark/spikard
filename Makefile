CFLAGS = -std=c++11
LFLAGS = -ldl

main: core.cpp
	g++ -c $(CFLAGS) main.cpp
	g++ -o p main.o $(LFLAGS)
clear: 
	rm *.o p
