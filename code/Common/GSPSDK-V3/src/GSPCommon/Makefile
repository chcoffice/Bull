include ../Makefile.def

#输出目标名称
TARGET = libGSPCommon$(EXT_NAME).a
#编译的子目录
SUBDIR = 
#清理的目录
CLEAN_PATH = 

CHANGE_DEPS =  Makefile ../Makefile.def *.h

INSTALL_HEADER_FILE = *.h


all : $(TARGET)
rebuild : clean all



OBJS = \
	crc.o \
	Analyer.o \
	BTree.o \
	GSPLog.o \
	GSPSocket.o \
	List.o \
	MemPool.o  \
	RefBuffer.o  \
	ThreadPool.o \
	AsyncQueue.o \
	Frame.o  \
	GSPObject.o \
	GSPString.o \
	MainLoop.o \
	RefPtrObject.o  \
	Uri.o \
	md5.o \
	Atomic.o  \
	GSPProDebug.o \
	MediaInfo.o \
	Packet.o  \
	SocketEventContext.o \
	
	

OBJS := $(OBJS:%.o=$(BIN_DIR)%.o)

$(BIN_DIR)%.o: %.S 
	@ echo "Compiling $@"
	$(C) $(INC) $(CFLAGS) -o $@ -c $<


$(BIN_DIR)%.o : %.c $(CHANGE_DEPS)
	@ echo "Compiling $@"
	$(C) $(INC) $(CFLAGS) -o $@ -c $<

$(BIN_DIR)%.o : %.cpp  $(CHANGE_DEPS)
	@ echo "Compiling $@"
	$(CC) $(INC) $(CFLAGS) -o $@ -c $<

LIBS = -L../../../lib/CommonLibs -L../../../lib/NetService -lCommonLibs -lNetServiceLib -pthread

$(TARGET) : $(OBJS)
	@ echo "Create library $@"
	$(AR) ru $@ $^
#	$(LD) -o $@ $^ -L../ 
	@for path in $(INSTALL_PATH); \
    do \
		cp -f $@ $$path; \
		echo "Install $@ to $$path"; \
    done
	@for file in $(INSTALL_HEADER_FILE); \
    do \
		cp -f $$file $(INSTALL_HEADER_PATH); \
		echo "Install $$file to $(INSTALL_HEADER_PATH)"; \
    done

	
 clean :
	rm -f $(TARGET)
	rm -f $(OBJS)
		
