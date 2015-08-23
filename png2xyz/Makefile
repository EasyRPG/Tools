SHELL = /bin/sh
TARGET = png2xyz
CC = g++
SRCS =  src/png2xyz.cpp
CFLAGS = -O2 -s -Wall -Wextra -ansi -pedantic $(shell pkg-config --cflags libpng zlib)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

LIBS = $(shell pkg-config --libs libpng zlib)

all:
	$(CC) -o $(TARGET) $(SRCS) $(CXXFLAGS) $(LIBS)
clean:
	rm -f $(TARGET)
