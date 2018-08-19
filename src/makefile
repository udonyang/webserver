INCS=-I.
GCC_FLAGS=-std=gnu++11 -Wall -O2 -g $(INCS)
GCC=g++ $(GCC_FLAGS)
IP=$(shell ifconfig en0 inet | egrep '[0-9]+(\.[0-9]+)+' -o | head -n 1)

BIN=webserver.bin

$(BIN): util.o main.o
	$(GCC) -o $@ $^

test_util: util.o test_util.cpp
	$(GCC) -o $@ $^
 
main.o: main.cpp

util.o: konst.h util.cpp

clean:
	rm -rf *.o *.bin *.dSYM

kill:
	killall $(BIN) 

run:
	./$(BIN) $(IP) 8080 1

test:
	curl http://$(IP):8080

%.o: %.cpp
	$(GCC) -c $<
