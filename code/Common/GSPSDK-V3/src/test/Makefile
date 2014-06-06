include ../Makefile.def

#输出目标名称
TARGET = test$(EXT_NAME)
#编译的子目录
SUBDIR = 
#清理的目录
CLEAN_PATH = 

CHANGE_DEPS =  Makefile ../Makefile.def *.h  ../../../lib/GSP/*.a

INSTALL_HEADER_FILE = 
INSTALL_PATH = ../../test/

all : $(TARGET)
rebuild : clean all

INC += -I../../../interface/GSP

LIBS += -L../../../lib/CommonLibs -L../../../lib/NetService  -L../../../lib/GSP -lGSPClient  -lGSPCommon  -lGSPServer -lCommonLibs -lNetServiceLib -pthread



OBJS = \
	BTreeTest.o  ClientTest.o  MempoolTest.o  ServerTest.o  StringTest.o  ThreadPoolTest.o  cmdline.o  test.o
	
	

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


$(TARGET) : $(OBJS) $(CHANGE_DEPS)
	@ echo "Create library $@"
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS) $(LIBS)
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
		
