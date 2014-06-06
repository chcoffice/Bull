SRCDIR = .

LIB_OBJS = $(SRCDIR)/src/BaseSocket.o \
	$(SRCDIR)/src/ClientChannel.o \
	$(SRCDIR)/src/CommunicationManager.o \
	$(SRCDIR)/src/IOCP.o \
	$(SRCDIR)/src/LinuxTcpSocket.o \
	$(SRCDIR)/src/LinuxUdpSocket.o \
	$(SRCDIR)/src/NetInterfaceCommData.o \
	$(SRCDIR)/src/NetService.o \
	$(SRCDIR)/src/ServerChannel.o \
	$(SRCDIR)/src/SocketChanel.o \
	$(SRCDIR)/src/ThreadIOCP.o \
	$(SRCDIR)/src/ThreadPoolModule.o \
	$(SRCDIR)/src/WinTcpSocket.o \
	$(SRCDIR)/src/WinUdpSocket.o \
	$(SRCDIR)/src/ThreadEpoll.o	 \
	$(SRCDIR)/src/ThreadSelect.o	 \
	$(SRCDIR)/src/LinuxEpoll.o	 

LIB_NAME = NetServiceLib
LIB_DIR = ../lib/NetService/

CC = gcc
CPP = g++
AR = ar
AR2 = ranlib

CFLAGS = -g -O3 -Wall -D__linux__ -D_LINUX
DEFINES =
INC = -I./ -I../include

ARFLAGS = cru

all: lib install

lib: lib$(LIB_NAME).a

lib$(LIB_NAME).a: $(LIB_OBJS)
	$(RM) lib$(LIB_NAME).a
	$(AR) $(ARFLAGS) $(LIB_DIR)/lib$(LIB_NAME).a $(LIB_OBJS)
	$(AR2) $(LIB_DIR)/lib$(LIB_NAME).a

%.o: %.S
	$(CC) $(CFLAGS) $(INC) $(DEFINES) -c -o $@ $<
	
%.o:%.c
	$(CC) $(CFLAGS) $(INC) $(DEFINES) -c $< -o $@

%.o:%.cpp
	$(CPP) $(CFLAGS) $(INC) $(DEFINES) -c $< -o $@

install:
	cp $(SRCDIR)/src/INetService.h  ../interface/NetService/
	cp $(SRCDIR)/src/DataType.h  ../interface/NetService/

clean:
	rm -f $(LIB_OBJS)
	rm -f lib$(LIB_NAME).a

rebuild: clean all

