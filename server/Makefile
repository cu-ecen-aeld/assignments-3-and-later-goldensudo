# CC ?= $(CROSS_COMPILE)gcc
# SRC := aesdsocket.c
# TARGET ?= aesdsocket
# OBJS := $(SRC:.c=.o)
# CFLAGs ?= -g -Wall -Werror
# LDFLAGS ?= -lpthread -lrt

# all: $(TARGET)

# default: $(TARGET)

# $(TARGET) : $(OBJS)
# 	$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# clean:
# 	-rm -f *.o $(TARGET) 

# Makefile for aesdsocket

CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
CROSS_COMPILE ?=
TARGET ?= aesdsocket
LDFLAGS ?= -lpthread -lrt

default: $(TARGET)

all: default

aesdsocket: aesdsocket.o
	$(CC) $(CFLAGS) -o $(TARGET) aesdsocket.o $(LDFLAGS)
    
aesdsocket.o: aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c -o aesdsocket.o
    
clean:
	rm -f $(TARGET) aesdsocket.o