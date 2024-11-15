CPP_FLAGS=-ggdb -Wall -Werror -Wextra -Iinclude
SDL_FLAGS=-lSDL2

# main cpp + all .o files
all: bin/main.o bin/interface.o
	g++ $(CPP_FLAGS) -o bin/game bin/main.o bin/interface.o $(SDL_FLAGS)

# o files -> corresponding .cpp and .h
bin/interface.o: src/interface.cpp include/interface.hpp
	
	g++ $(CPP_FLAGS) -c src/interface.cpp $(SDL_FLAGS) -o bin/interface.o

bin/main.o: main.cpp 
	mkdir -p bin/
	g++ $(CPP_FLAGS) -c main.cpp $(SDL_FLAGS) -o bin/main.o

run:
	./bin/game
