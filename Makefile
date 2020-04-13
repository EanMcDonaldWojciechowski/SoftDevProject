build:
	g++ -pthread ./test/testLinus.cpp

run:
	./a.out -nodeIndex 0 -nodes 3 &
	./a.out -nodeIndex 1 -nodes 3 &
	./a.out -nodeIndex 2 -nodes 3
