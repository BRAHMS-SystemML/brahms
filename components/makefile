
# include makefile.brahms
include ../makefile.brahms

default :
	@make -s silent_all

std :
	@make -s silent_std

silent_std :
	make -C std.h -s
	make -C std -s

silent_all :
	make -C std.h -s
	make -C sets -s
	make -C std -s
	make -C test -s
	make -C bench -s
	make -C language -s
	make -C example -s
	make -C dev -s
ifeq ($(NOWX),)
	make -C gui -s
endif



