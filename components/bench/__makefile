
# include makefile.brahms
include ../../makefile.brahms

# path
BASEPATH=client/brahms

# target path
PATH_TGT=$(PATH_SUP_BCH)/elements
PATH_TGT_BIN=$(PATH_TGT)/elements_monolithic$(EXTEXE)

# silent default
default :
	@make -s silent

# default is to build everything
silent: header obj
	make -s -f ../makefile-cpp COMPTYPE=process NAME=elements SUBPATH=$(BASEPATH)/bench/elements XLOPT=\"${call ONEARG,$(PATH_TMP)/scalable.$(EXTOBJ)}\" "XDEPS=scalable.h scalable.cpp elements_monolithic.cpp"
	make -s -f ../makefile-cpp COMPTYPE=process NAME=operation SUBPATH=$(BASEPATH)/bench/operation
	make -s -f ../makefile-cpp COMPTYPE=process NAME=overhead SUBPATH=$(BASEPATH)/bench/overhead

obj:
ifeq ($(ARCH_NIX),1)
	g++ -c -fPIC -O3 $(FLAG_ARCH) $(CPPFLAGS) scalable.cpp $(NAMEOBJ)${call ONEARG,$(PATH_TMP)/scalable.$(EXTOBJ)}
	g++ -O3 $(DEFINES) $(FLAG_ARCH) $(CPPFLAGS) -D__NIX__ elements_monolithic.cpp -o ${call ONEARG,$(PATH_TGT_BIN)} ${call ONEARG,$(PATH_TMP)/scalable.$(EXTOBJ)}
endif
ifeq ($(ARCH_OS),win)
	cl scalable.cpp -MD -c -EHsc -nologo -Ox $(NAMEOBJ)$(PATH_TMP)/scalable.$(EXTOBJ)
	cl elements_monolithic.cpp $(DEFINES) -EHsc -nologo -Ox -MD ${call ONEARG,$(PATH_TMP)/scalable.$(EXTOBJ)} ${call ONEARG,-Fe$(PATH_TGT_BIN)}
	del elements_monolithic.obj
	del ${call ONEARG,$(PATH_TGT)\*.manifest}
endif

header:
	$(EMPTY_LINE)
	$(HEADER_LINE) bench
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_TGT))
