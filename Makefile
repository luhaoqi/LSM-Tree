
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++14 -Wall

all: correctness persistence main

correctness: kvstore.o correctness.o index.cpp buffer.cpp

persistence: kvstore.o persistence.o index.cpp buffer.cpp

main: kvstore.o main.cpp index.cpp buffer.cpp

clean:
	-rm -f correctness persistence *.o
