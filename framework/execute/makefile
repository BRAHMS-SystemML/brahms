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
#	$Id:: makefile 2371 2009-11-15 01:28:27Z benjmitch         $
#	$Rev:: 2371                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-15 01:28:27 +0000 (Sun, 15 Nov 2009)       $
#________________________________________________________________
#



# common file
include ../../makefile.brahms

# version
MAJ=$(VERSION_BRAHMS_MAJ)
MIN=$(VERSION_BRAHMS_MIN)
REL=$(VERSION_BRAHMS_REL)
REV=$(VERSION_BRAHMS_REV)
DVER=-D__MAJ__=$(MAJ) -D__MIN__=$(MIN) -D__REL__=$(REL) -D__REV__=$(REV)


ifeq ($(ARCH_OS), osx)
BUILD_PLATFORM=osx
else
# build platform
UNAME = \
	${shell uname -n} \
	(${shell uname -m}, \
	${shell uname -s}-${shell uname -v}.${shell uname -r})
ifeq ($(PROCESSOR_ARCHITEW6432), )
BUILD_PLATFORM=$(UNAME)
else
BUILD_PLATFORM=$(UNAME) ($(PROCESSOR_ARCHITEW6432))
endif
endif


################################################################
#	Architecture-specific settings

ifeq ($(ARCH_OS), gln)

	# local (compiler switches)
	COPT="-DBUILD_PLATFORM=\"$(BUILD_PLATFORM)\""

	#-ldl -lXmu -lSM -lICE -lXext -lX11

	# link to gui stuff (later, the gui module will do this)
	ifeq ($(with-x),)
		with-x=/usr/X11R6
	endif
	ifeq ($(ARCH_BITS),64)
		LOPTGUI=-L$(with-x)/lib64 -lXt -lXaw -lrt
	else
		LOPTGUI=-L$(with-x)/lib -lXt -lXaw -lrt
	endif

	# link to other stuff
	LOPT=$(LOPTGUI) ${call ONEARG,$(PATH_TMP)/Gauge.$(EXTOBJ)} -lbrahms-engine

	# compiler
	MCXX=mpic++

endif

ifeq ($(ARCH_OS), osx)

	# local (compiler switches)
	COPT="-DBUILD_PLATFORM=\"$(BUILD_PLATFORM)\""

	#-ldl -lXmu -lSM -lICE -lXext -lX11

	# link to gui stuff (later, the gui module will do this)
	ifeq ($(with-x),)
		with-x=/usr/lib/x86_64-linux-gnu/
	endif
	ifeq ($(ARCH_BITS),64)
		LOPTGUI=-L$(with-x)/lib64 -lX11 -lXt -lXaw
	else
		LOPTGUI=-L$(with-x)/lib -lXmu -lX11 -lXt -lXaw
	endif

	# link to other stuff
	LOPT=$(LOPTGUI) ${call ONEARG,$(PATH_TMP)/Gauge.$(EXTOBJ)} -lbrahms-engine -bind_at_load

	# compiler
	MCXX=mpic++

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT="-DBUILD_PLATFORM=\"$(BUILD_PLATFORM)\""
	LOPT=shell32.lib user32.lib gdi32.lib comctl32.lib \
		${call ONEARG,$(PATH_BIN)/libbrahms-engine.lib}

		#\
		#$(PATH_BIN)/libbrahms-base.0.lib \
		#$(PATH_BIN)/libbrahms-gui.0.lib

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) $(DVER)
LOPTS=$(LOPT_BASE) $(LOPT) ${call ONEARG,$(PATH_TMP)/gui.$(EXTOBJ)}



################################################################
#	Targets

default : header $(PATH_BIN)/brahms-execute$(EXTEXE)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/execute
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_BIN))
	make -C gui -s

$(PATH_BIN)/brahms-execute$(EXTEXE) : main.cpp *.cpp *.h $(IF_BOTH) makefile ../../makefile.brahms $(PATH_TMP)/gui.$(EXTOBJ)
	$(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS) 
ifeq ($(ARCH_OS), win)
	@rm main.obj
endif
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))
