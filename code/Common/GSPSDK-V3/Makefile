

#编译的子目录
SUBDIR = ./src
#清理的目录
CLEAN_PATH = ./bin ./test



all clean rebuild : 
	@for path in $(SUBDIR); \
    do \
		$(MAKE) -C $$path $@; \
    done
	


