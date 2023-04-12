CC = g++
CFLAGS = -std=c++20 -Wall -Wpedantic

SRC = main.cpp producer_consumer_utils.hpp producer_consumer_utils.cpp constants.hpp

all: clean build

build:
	$(CC) $(CFLAGS) -o producer_consumer.out $(SRC)

debug:
	$(CC) $(CFLAGS) -o producer_consumer.out $(SRC) -g

clean:
	@rm -rf *.out
