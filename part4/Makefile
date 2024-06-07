# Joseph Erlinger (jerling2@uoregon.edu)
# Special thanks to Gabriel Parmer for the YouTube tutorial.
# YouTube Link: https://youtu.be/DtGrdB8wQ_8?si=ySdCCRTerYwsmuCU.

BINARY = bank
LOGDIR = ./output
SAVINGSDIR = ./savings
SRCDIRS = . ./lib
INCDIRS = ./inc

CC = gcc
DEPFLAGS = -MP -MD
CFLAGS = -Wall -Wextra -g $(VERBOSE_FLAG) $(foreach D,$(INCDIRS),-I$(D)) $(DEPFLAGS)

CFILES = $(foreach D,$(SRCDIRS),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(CFILES)) 
DEPFILES = $(patsubst %.c,%.d,$(CFILES)) 
LOGFILES = $(wildcard $(LOGDIR)/*)
SAVINGFILES = $(wildcard $(SAVINGSDIR)/*)

DEBUG ?= 1
INFO ?= 1

.PHONY : all clean

all : $(BINARY)

debug :
	@$(MAKE) all DEBUG=1 INFO=1
	
info :
	@$(MAKE) all DEBUG=0 INFO=1

silent :
	@$(MAKE) all DEBUG=0 INFO=0

ifeq ($(DEBUG),1)
    VERBOSE_FLAG += -DDEBUG_ENABLED
endif

ifeq ($(INFO),1)
    VERBOSE_FLAG += -DINFO_ENABLED
endif

$(BINARY) : $(OBJECTS)
	$(CC) -o $@ $^

%.o : %.c
	$(CC) -D PART4 $(CFLAGS) -c -o $@ $^

clean :
	@rm -f $(BINARY) $(OBJECTS) $(DEPFILES) $(LOGFILES) $(SAVINGFILES) 

