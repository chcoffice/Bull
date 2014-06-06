SRCDIR = .

LIB_OBJS = $(SRCDIR)/NetCommonStack/BaseChannel.o \
	$(SRCDIR)/NetCommonStack/BaseProtocol.o \
	$(SRCDIR)/NetCommonStack/BufferArray.o \
	$(SRCDIR)/NetCommonStack/BufferObj.o \
	$(SRCDIR)/NetCommonStack/BufMgr.o \
	$(SRCDIR)/NetCommonStack/NetChannel.o \
	$(SRCDIR)/NetCommonStack/NetClient.o \
	$(SRCDIR)/NetCommonStack/NetCommuBase.o \
	$(SRCDIR)/NetCommonStack/NetCommuStack.o \
	$(SRCDIR)/NetCommonStack/NetConfig.o \
	$(SRCDIR)/NetCommonStack/NetMessage.o \
	$(SRCDIR)/NetCommonStack/NetServer.o \
	$(SRCDIR)/NetCommonStack/SendTask.o \
	$(SRCDIR)/NetCommonStack/ThreadObj.o \
	$(SRCDIR)/NetCommonStack/UserProtocol.o	 \

LIB_NAME = NetCommonStack
LIB_DIR = ../lib/NetCommonStack

CC = gcc
CPP = g++
LD = g++

CFLAGS = -O3 -Wall -D__linux__ -D_LINUX
DEFINES = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_ISOC9X_SOURCE
INC = -I./ -I../include -I../interface/NetService

all: sharelib

sharelib: lib$(LIB_NAME).so

lib$(LIB_NAME).so: $(LIB_OBJS)
	$(RM) lib$(LIB_NAME).so
	$(LD) -shared $(LIB_OBJS) -o $(LIB_DIR)/lib$(LIB_NAME).so
	cp $(SRCDIR)/NetCommonStack/NetCommonStack.h ../interface/NetCommonStack/

%.o: %.S
	$(CC) $(CFLAGS) $(INC) $(DEFINES) -c -o $@ $<
	
%.o:%.c
	$(CC) $(CFLAGS) $(INC) $(DEFINES) -c $< -o $@

%.o:%.cpp
	$(CPP) $(CFLAGS) $(INC) $(DEFINES) -c $< -o $@

clean:
	rm -f $(LIB_OBJS)
	rm -f lib$(LIB_NAME).so

rebuild: clean all

