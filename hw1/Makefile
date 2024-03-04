sim: pa1.o CPUList.o
	g++ -o sim pa1.o CPUList.o
pa1.o: pa1.cpp  CPUList.h
	g++ -c pa1.cpp
CPUList.o: CPUList.cpp CPUList.h
	g++ -c CPUList.cpp