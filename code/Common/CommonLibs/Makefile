SRCDIR = .

LIB_OBJS = $(SRCDIR)/ModuleManager/ModuleManager.o \
	$(SRCDIR)/ConfigFile/ConfigFile.o \
	$(SRCDIR)/SystemInterface/CGSAutoMutex.o \
	$(SRCDIR)/SystemInterface/CGSCond.o \
	$(SRCDIR)/SystemInterface/CGSDir.o \
	$(SRCDIR)/SystemInterface/CGSDisk.o \
	$(SRCDIR)/SystemInterface/CGSMutex.o \
	$(SRCDIR)/SystemInterface/CGSProcessMutex.o \
	$(SRCDIR)/SystemInterface/CGSSem.o \
	$(SRCDIR)/SystemInterface/CGSThread.o \
	$(SRCDIR)/SystemInterface/CGSTime.o \
	$(SRCDIR)/SystemInterface/CGSWRMutex.o \
	$(SRCDIR)/SystemInterface/ISystemLayInterface.o \
	$(SRCDIR)/SystemInterface/GSThreadPool.o \
	$(SRCDIR)/SystemInterface/GSUtility.o \
	$(SRCDIR)/SystemInterface/xmlParser.o \
	$(SRCDIR)/Log/LogLibrary.o \
	$(SRCDIR)/MemComm/MemCommBase.o \
	$(SRCDIR)/MemComm/MemComm.o \
	$(SRCDIR)/MemComm/MemCommLocal.o \
	$(SRCDIR)/MemComm/MemCommProcess.o

	

LIB_NAME = CommonLibs
LIB_DIR = ../lib/CommonLibs

CC = gcc
CPP = g++
AR = ar
AR2 = ranlib

CFLAGS = -g -O3 -Wall -D__linux__ -D_LINUX
DEFINES = -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_ISOC9X_SOURCE
INC = -I./ -I../include

ARFLAGS = cru

all: install lib

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
	@echo "hello -----------" $@
	$(CPP) $(CFLAGS) $(INC) $(DEFINES) -c $< -o $@

install:
	cp $(SRCDIR)/ModuleManager/*.h ../interface/CommonLibs/
	cp $(SRCDIR)/ConfigFile/*.h ../interface/CommonLibs/
	cp $(SRCDIR)/SystemInterface/*.h ../interface/CommonLibs/
	cp $(SRCDIR)/MemComm/IMemComm.h ../interface/CommonLibs/
	cp $(SRCDIR)/Log/ILogLibrary.h ../interface/CommonLibs/

clean:
	rm -f $(LIB_OBJS)
	rm -f lib$(LIB_NAME).a

rebuild: clean all

