# Makefile for TCP Chat Application (C++20)
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pthread

# Targets
TARGETS := server client

# Source files
SRCS := server.cpp client.cpp

all: $(TARGETS)

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp

run-server: server
	./server

run-client: client
	./client

clean:
	rm -f $(TARGETS)
