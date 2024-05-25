# Joseph Erlinger (jerling2@uoregon.edu)
# Special thanks to Gabriel Parmer for the YouTube tutorial.
# YouTube Link: https://youtu.be/DtGrdB8wQ_8?si=ySdCCRTerYwsmuCU.

BINARY = bank
SRCDIRS = . ./lib
INCDIRS = ./inc

CC = gcc
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(foreach D,$(INCDIRS),-I$(D)) $(DEPFLAGS)

CFILES = $(foreach D,$(SRCDIRS),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(CFILES)) 
DEPFILES = $(patsubst %.c,%.d,$(CFILES)) 

.PHONY : all clean

all : $(BINARY)

$(BINARY) : $(OBJECTS)
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean :
	@rm -f $(BINARY) $(OBJECTS) $(DEPFILES)

