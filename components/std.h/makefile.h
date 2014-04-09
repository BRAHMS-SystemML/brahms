
# include makefile.brahms
include ../../makefile.brahms

# get version data
REL=0
REV=$(VERSION_BRAHMS_REV)

# get node path
NODEPATH=$(PATH_SML_NSP)/$(SUBPATH)
BINPATH=$(NODEPATH)/brahms/0

# get us path
USPATH=${subst /,_,$(SUBPATH)}

# copy over header
$(BINPATH)/$(TGT) : $(NAME).h makefile.h
	$(call MKDIR, $(BINPATH))
	$(SHOW_MAKING) $<
	cat $(NAME).h | sed s/____CLASS_US____/$(USPATH)_$(REL)/ > ${call ONEARG,$(BINPATH)/$(TGT)}

