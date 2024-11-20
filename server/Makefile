CPP_FLAGS=-ggdb -Wall -Werror -Wextra -Iinclude
THREAD_FLAGS=-pthread

# main cpp + all .o files
all: bin/main.o bin/server.o
	g++ $(CPP_FLAGS) -o bin/server bin/main.o bin/server.o $(THREAD_FLAGS)

bin/server.o: src/server.cpp include/server.hpp
	g++ $(CPP_FLAGS) -c src/server.cpp ${THREAD_FLAGS} -o bin/server.o

bin/main.o: main.cpp
	mkdir -p bin/
	g++ $(CPP_FLAGS) -c main.cpp $(THREAD_FLAGS) -o bin/main.o

run:
	./bin/server

clean:
	rm -f bin/*.o
	rm -f bin/server