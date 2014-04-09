#________________________________________________________________
#
#	This is the platform-independent makefile for this
#	module. You will need to have UnxUtils installed to
#	make it work on Windoze:
#
#		http://unxutils.sourceforge.net
#________________________________________________________________
#
#	This file is part of BRAHMS
#	Copyright (C) 2007 Ben Mitchinson
#	URL: http://brahms.sourceforge.net
#
#	This program is free software; you can redistribute it and/or
#	modify it under the terms of the GNU General Public License
#	as published by the Free Software Foundation; either version 2
#	of the License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#________________________________________________________________
#
#	Subversion Repository Information (automatically updated on commit)
#
#	$Id:: makefile 2405 2009-11-19 13:46:34Z benjmitch         $
#	$Rev:: 2405                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-19 13:46:34 +0000 (Thu, 19 Nov 2009)       $
#________________________________________________________________
#

default :
	@make -s silent_default




# common file
include ../../../makefile.brahms

# version
REL=0
REV=$(VERSION_BRAHMS_REV)
DVER=-D__REL__=$(REL) -D__REV__=$(REV)

# paths under install path
PATH_GRP=$(PATH_BDG_CMP_MAT)

ifeq ($(SYSTEMML_MATLAB_PATH),)
${error set SYSTEMML_MATLAB_PATH}
endif
PATH_MATLAB=${subst $(SPACE),\$(SPACE),${subst \,/,$(SYSTEMML_MATLAB_PATH)}}



################################################################
#	Architecture-specific settings

ifeq ($(ARCH_OS), gln)
	
	ifeq ($(ARCH_BITS), 64)
		MATARCH=glnxa64
	else
		MATARCH=glnx86
	endif

	COPT = \
		-I$(PATH_MATLAB)/extern/include -shared \

	LOPT = \
		-lbrahms-engine -L$(PATH_MATLAB)/bin/$(MATARCH) -leng -lmx -Wl,-soname=lib$(BASENAME).$(EXTCMP)

	SETLIB=export LIBRARY_PATH=$(PATH_BIN)
	SETINC=export CPATH=$(PATH_INC):$(PATH_SML_NSP)

endif

ifeq ($(ARCH_OS), osx)
	
	ifeq ($(ARCH_BITS), 64)
		MATARCH=what_is_this_supposed_to_be
	else
		MATARCH=maci
	endif

	COPT = \
		-I$(PATH_MATLAB)/extern/include -dynamiclib \

	LOPT = \
		-lbrahms-engine -L$(PATH_MATLAB)/bin/$(MATARCH) -leng -lmx

	SETLIB=export LIBRARY_PATH=$(PATH_BIN)
	SETINC=export CPATH=$(PATH_INC):$(PATH_SML_NSP)

endif

ifeq ($(ARCH_OS), win)

	COPT = \
		-LD

	LOPT = \
		libbrahms-engine.lib \
		-EXPORT:EventHandler \
		libeng.lib libmx.lib

	SETLIB=set LIB=%%LIB%%;${subst \ , ,$(PATH_MATLAB)/extern/lib/win32/microsoft};
	SETINC=set INCLUDE=%%INCLUDE%%;${subst \ , ,$(PATH_MATLAB)/extern/include};

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) -I../../engine
LOPTS=$(LOPT_BASE) $(LOPT)



################################################################
#
#	Targets
#
################################################################

DEPS=$(IF_BOTH) $(IF_1199) makefile ../../../makefile.brahms

silent_default : header $(PATH_GRP)/libbrahms-1258.$(EXTCMP)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/bindings/1258
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_GRP))

$(PATH_GRP)/libbrahms-1258.$(EXTCMP) : matlab.cpp matlab-support.cpp common.h timer.h $(DEPS)
ifeq ($(ARCH_OS),win)
	@echo @echo off > cmdfile.bat
else
	@echo \#!/bin/sh > cmdfile.bat
endif
	@echo $(SETLIB) >> cmdfile.bat
	@echo $(SETINC) >> cmdfile.bat
	@echo $(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS) >> cmdfile.bat
ifeq ($(ARCH_NIX),1)
	chmod u+x cmdfile.bat
	./cmdfile.bat
else
	@cmdfile
	@del *.obj
	@del ${call ONEARG,$(PATH_GRP)}\*.exp
	@del ${call ONEARG,$(PATH_GRP)}\*.lib
	@del ${call ONEARG,$(PATH_GRP)}\*.manifest
endif
	@rm cmdfile.bat
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))



