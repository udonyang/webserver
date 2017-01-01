INCS=-I.
GCC_FLAGS=-lc -std=gnu++11 -Wall -O2 -g $(INCS)
GCC=g++ $(GCC_FLAGS)

BIN=webserver.bin

$(BIN): util.o main.o
	$(GCC) -o $@ $^
 
main.o: main.cpp

util.o: util.cpp

clean:
	rm -rf *.o *.bin *.dSYM

run:
	./$(BIN) 127.0.0.1 8080

%.o: %.cpp
	$(GCC) -c $<
