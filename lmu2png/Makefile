SHELL = /bin/sh
TARGET = lmu2png
CC = g++
SRCS =  src/*.cpp
CFLAGS = -O2 -s -Wall -Wextra -ansi -pedantic $(shell pkg-config --cflags liblcf SDL2_image)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

LIBS = $(shell pkg-config --libs liblcf SDL2_image)

all:
	$(CC) -o $(TARGET) $(SRCS) $(CXXFLAGS) $(LIBS)
clean:
	rm -f $(TARGET)
