
#输出目标名称
TARGET = 
#编译的子目录
SUBDIR =   ./GSPCommon  ./GSPClient ./GSPServer ./test
#清理的目录
CLEAN_PATH = 


all clean rebuild : 
	@for path in $(SUBDIR); \
    do \
		$(MAKE) -C $$path $@; \
    done
