all clean cleanall rebuild install:
	@-for i in `ls` ;\
	do \
		if [ $$i = "Makefile" ] ;\
		then \
			continue;\
		fi ;\
		if [ ! -d $$i ] ;\
		then \
			#echo "`pwd`/$$i is not a directory.";\
			continue;\
		fi ;\
		if [ ! -f $$i/Makefile ] ; \
		then \
			echo "WARNING: no Makefile in directory `pwd`/$$i.";\
			continue;\
		fi ;\
		( cd $$i;\
		$(MAKE) -e $@ ) ;\
		echo "";\
	done
