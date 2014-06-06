include ../Makefile.def

#输出目标名称
TARGET = libGSPClient$(EXT_NAME).a
#编译的子目录
SUBDIR = 
#清理的目录
CLEAN_PATH = 

CHANGE_DEPS =  Makefile ../Makefile.def *.h

INSTALL_HEADER_FILE = IGSPClient.h GSPClientDefs.h


all : $(TARGET)
rebuild : clean all

INC += -I../GSPCommon



OBJS = \
	ClientChannel.o  ClientSection.o  IGSPClient.o
	
	

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


$(TARGET) : $(OBJS)
	@ echo "Create library $@"
	$(AR) ru $@ $^
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
		
