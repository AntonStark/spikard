g++ -fPIC -c -g -std=c++11  $1.cpp
gcc -shared $1.o -o $1.so
rm $1.o
