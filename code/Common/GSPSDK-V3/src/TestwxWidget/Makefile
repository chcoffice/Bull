include ../Makefile.def

#输出目标名称
TARGET = TestwxWidget$(EXT_NAME)
#编译的子目录
SUBDIR = 
#清理的目录
CLEAN_PATH = 

CHANGE_DEPS =  Makefile ../Makefile.def *.h  ../../test/*.a

INSTALL_HEADER_FILE = 
INSTALL_PATH = ../../test/

all : $(TARGET)
rebuild : clean all

INC += $(INTERNAL_INTERFACE_INC)
INC += $(INTERNAL_COMMON_INC)
INC += -I../ExtLibrary/wxWidgets/linux/include/wx-2.9  
INC += -I../ExtLibrary/wxWidgets/linux/lib/wx/include/gtk2-unicode-2.9

LIBS += -L../../../lib/CommonLibs -L$(LIBRARY_PATH)  -L$(LIBRARY_FFMPEG_PATH)
LIBS += -L../ExtLibrary/wxWidgets/linux/lib


LIBS += -lGSPServer$(EXT_NAME)  -lGSPClient$(EXT_NAME) 
LIBS += -lProGSPServer$(EXT_NAME) -lProGSPClient$(EXT_NAME) -lProGSPCommon$(EXT_NAME)  
LIBS += -lProRTSPServer$(EXT_NAME) -lProRTSPCommon$(EXT_NAME)
LIBS += -lGSPCommon$(EXT_NAME)
LIBS += -lCommonLibs
#wxWidgets
#LIBS += -lwx_gtk2u_qa-2.9 -lwx_gtk2u_core-2.9
LIBS += -lwx_gtk2u_xrc-2.9 -lwx_gtk2u_html-2.9 -lwx_gtk2u_qa-2.9 -lwx_gtk2u_adv-2.9 -lwx_gtk2u_core-2.9 -lwx_baseu_xml-2.9 -lwx_baseu_net-2.9 -lwx_baseu-2.9 
#ffmpeg
LIBS += -lavformat  -lavcodec  -lavutil 
#system 
LIBS +=  -pthread -ldl -lm -lz -lbz2

CFLAGS += -DWXUSINGDLL -D__WXGTK__

OBJS = \
	MainApp.o  TestClient.o  TestServer.o  test_wdr.o
		

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
	@for path in $(INSTALL_PATH); \
    do \
		rm -f  $$path/$(TARGET); \
		echo "remove $$path/$(TARGET)"; \
    done
	@for file in $(INSTALL_HEADER_FILE); \
    do \
		rm -f  $(INSTALL_HEADER_PATH)/$$file; \
		echo "remove $(INSTALL_HEADER_PATH)/$$file"; \
    done
		
