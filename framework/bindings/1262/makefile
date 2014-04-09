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
#	Copyright (C) 2007 Ben Mitch(inson)
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
PATH_GRP=$(PATH_BDG_CMP_PYT)




################################################################
#	Architecture-specific settings

ifeq ($(ARCH_OS), gln)

	PYTHON_INC=`python -c 'from distutils.sysconfig import *; print get_python_inc()'`
	NUMPY_INC=`python -c 'from numpy import *; print get_include()'`
	PYTHON_LIB=`python -c 'from distutils.sysconfig import *; print get_python_lib(0, 1)'`
	PYTHON_VERSION=`python -c 'from distutils.sysconfig import *; print get_config_var("VERSION")'`
	PYTHON_LIBS=`python -c 'from distutils.sysconfig import *; print get_config_var("LIBS")'`
	PYTHON_LINKFORSHARED=`python -c 'from distutils.sysconfig import *; print get_config_var("LINKFORSHARED")'`

	COPT = \
		-D__CLASS_CPP__=$(CLASS_CPP) -D__SUBCLASS__=$(SUBCLASS) \
		-I$(PYTHON_INC) -I$(NUMPY_INC) -Wno-write-strings -shared

	# moved to static linking, so we have a consistent version of python (v2.5, all platforms, for repeatability)
	#
	# COULD DO THIS! trouble is, numpy, etc. are all separate libraries. static link with whole shebang? not really
	# a good idea since user code could be in separate modules we've never heard of
	#
	# dynamic link reads "-lpython$(PYTHON_VERSION)"
	# static link reads "/usr/lib/python2.5/config/libpython2.5.a"

	# used to read "-Wl,-soname=lib$(BASENAME).$(EXTCMP)"
	# is still needed?
	LOPT = \
		-lbrahms-engine -L$(PYTHON_LIB) -lpython$(PYTHON_VERSION) $(PYTHON_LIBS) $(PYTHON_LINKFORSHARED)

	SETLIB=export LIBRARY_PATH=$(PATH_BIN)
	SETINC=export CPATH=$(PATH_INC):$(PATH_SML_NSP)

endif

ifeq ($(ARCH_OS), osx)

	PYTHON_INC=`python -c 'from distutils.sysconfig import *; print get_python_inc()'`
	NUMPY_INC=`python -c 'from numpy import *; print get_include()'`
	PYTHON_LIB=`python -c 'from distutils.sysconfig import *; print get_python_lib(0, 1)'`
	PYTHON_VERSION=`python -c 'from distutils.sysconfig import *; print get_config_var("VERSION")'`
	PYTHON_LIBS=`python -c 'from distutils.sysconfig import *; print get_config_var("LIBS")'`

#	PYTHON_LINKFORSHARED=`python -c 'from distutils.sysconfig import *; print get_config_var("LINKFORSHARED")'`
# http://bugs.python.org/issue3588
# the last "LINKFORSHARED" may not work on OSX. so we use the workaround detailed on that page

	PYTHON_LINKFORSHARED=-u _PyMac_Error /Library/Frameworks/Python.framework/Versions/Current/Python

	COPT = \
		-D__CLASS_CPP__=$(CLASS_CPP) -D__SUBCLASS__=$(SUBCLASS) \
		-I$(PYTHON_INC) -I$(NUMPY_INC) -Wno-write-strings -dynamiclib

	LOPT = \
		-lbrahms-engine -L$(PYTHON_LIB) -lpython$(PYTHON_VERSION) $(PYTHON_LIBS) $(PYTHON_LINKFORSHARED)

	SETLIB=export LIBRARY_PATH=$(PATH_BIN)
	SETINC=export CPATH=$(PATH_INC):$(PATH_SML_NSP)

endif

ifeq ($(ARCH_OS), win)

ifeq ($(SYSTEMML_PYTHON_PATH),)
	PATH_PYTHON=C:\Program\ Files\Python
else
	PATH_PYTHON=${subst $(SPACE),\$(SPACE),${subst \,/,$(SYSTEMML_PYTHON_PATH)}}
endif

	COPT = \
		-D__CLASS_CPP__=$(CLASS_CPP) -D__SUBCLASS__=$(SUBCLASS) \
		-LD

	LOPT = \
		libbrahms-engine.lib \
		-EXPORT:EventHandler

	SETLIB=set LIB=%%LIB%%;${subst \ , ,$(PATH_PYTHON)/libs};
	SETINC=set INCLUDE=%%INCLUDE%%;${subst \ , ,$(PATH_PYTHON)/include};${subst \ , ,$(PATH_PYTHON)/lib/site-packages/numpy/core/include};

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

silent_default : header $(PATH_GRP)/libbrahms-1262.$(EXTCMP)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/bindings/1262
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_GRP))

$(PATH_GRP)/libbrahms-1262.$(EXTCMP) : python.cpp python-support.cpp ../1258/common.h ../1258/timer.h $(DEPS)
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


