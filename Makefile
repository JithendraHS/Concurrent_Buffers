#!/bin/sh
CC = g++-11
CFLAGS = -Wall -Werror --std=c++20 -pthread -fsanitize=address -fno-omit-frame-pointer

SOURCES = concurrent_containers.cpp command_handling.cpp buffer.cpp parallelized_code.cpp
OBJS = $(SOURCES:.cpp=.o)
TARGET = container
RM_FILES = $(OBJS:.o=)

%.o : %.cpp
	$(CC) $(CFLAGS) -g -c -o $@ $<
	@echo *****Object file created $@*****

.PHONY: all
all: $(TARGET)

.PHONY: build
build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -g -o $@ $^

.PHONY:clean
clean:
	@echo *****Cleaning*****
	rm -f *.o $(RM_FILES)