all: shell vowels

shell: shell.cpp Command.cpp Command.h
	g++ shell.cpp Command.cpp -o shell

vowels: vowels.cpp
	g++ vowels.cpp -lpthread -o vowels

clean:
	rm -f shell vowels
