all : lib mathlang.o
	gcc -shared -g -Wl,-soname,mathlang.so -o mathlang.so *.o
	mv mathlang.so ../../mathlang.so
lib : logic.o signature.o parser.o
	
logic.o : ../logic.cpp ../logic.hpp
	g++ -fPIC -c -g -std=c++11 ../logic.cpp
signature.o : ../signature.cpp ../signature.hpp
	g++ -fPIC -c -g -std=c++11 ../signature.cpp
parser.o : ../parser.cpp ../parser.hpp
	g++ -fPIC -c -g -std=c++11 ../parser.cpp
mathlang.o : ../mathlang.cpp
	g++ -fPIC -c -g -std=c++11 ../mathlang.cpp
