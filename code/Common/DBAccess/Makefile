SRCDIR = .

LIB_OBJS = $(SRCDIR)/src/InnerRealize/CConnection.o \
	$(SRCDIR)/src/InnerRealize/CDBConnectionPool.o \
	$(SRCDIR)/src/InnerRealize/COracleConnection.o \
	$(SRCDIR)/src/InnerRealize/CConcreteConnectionFactory.o \
	$(SRCDIR)/src/InnerRealize/COracleRecordSet.o \
	$(SRCDIR)/src/InnerRealize/CRecordSet.o \
	$(SRCDIR)/src/InnerRealize/IConnectionFactory.o \
	$(SRCDIR)/src/InnerRealize/CSqlServerConnection.o \
	$(SRCDIR)/src/InnerRealize/CSqlServerRecordSet.o \
	$(SRCDIR)/src/InnerRealize/CDBFieldInfo.o \
	$(SRCDIR)/src/OuterInterface/DBAccessModule.o

LIB_NAME = DBAccess
LIB_DIR = ../lib/DBAccess

CC = gcc
CPP = g++
AR = ar
AR2 = ranlib

CFLAGS = -O3 -Wall -D__linux__ -D_LINUX
DEFINES =
INC = -I./ -I../include

ARFLAGS = cru

all: lib update

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


update:
	cp $(SRCDIR)/src/OuterInterface/IDBAccessModule.h  ../interface/DBAccess/

clean:
	rm -f $(LIB_OBJS)
	rm -f lib$(LIB_NAME).a

rebuild: clean all

