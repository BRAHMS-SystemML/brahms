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
#	$Id:: makefile 2376 2009-11-15 23:34:44Z benjmitch         $
#	$Rev:: 2376                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
#________________________________________________________________
#





# common file
include ../../../makefile.brahms



################################################################
#	Versioning

# same version as BRAHMS
MAJ=$(VERSION_BRAHMS_MAJ)
MIN=$(VERSION_BRAHMS_MIN)
REV=$(VERSION_BRAHMS_REV)
REL=$(VERSION_BRAHMS_REL)




################################################################
#	Architecture-specific settings

ifeq ($(ARCH_OS), gln)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=-L$(PATH_TMP)

endif

ifeq ($(ARCH_OS), osx)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=-L$(PATH_TMP)

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT=-LD
	LOPT=shell32.lib user32.lib

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) -I.. -I../..

# get all obj files
FILES_OBJ=${call ONEARG,$(PATH_TMP)/base.$(EXTOBJ)} ${call ONEARG,$(PATH_TMP)/error.$(EXTOBJ)} ${call ONEARG,$(PATH_TMP)/support.$(EXTOBJ)} ${call ONEARG,$(PATH_TMP)/systemml.$(EXTOBJ)}
LOPTS=$(LOPT_BASE) $(LOPT) $(FILES_OBJ)




################################################################
#	Targets

default : header $(PATH_BIN)/libbrahms-engine.$(EXTDLL)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/engine/main
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_BIN))

F_OBJ=$(PATH_TMP)/base.$(EXTOBJ) $(PATH_TMP)/error.$(EXTOBJ) $(PATH_TMP)/support.$(EXTOBJ) $(PATH_TMP)/systemml.$(EXTOBJ)

$(PATH_BIN)/libbrahms-engine.$(EXTDLL) : engine.cpp *.cpp *.h makefile ../../../makefile.brahms $(F_OBJ) $(IF_BOTH) ../../channel/channel.h
ifeq ($(BUILD), DEBUG)
	echo $(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS)
endif
	$(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS)
	@rm -f \*.$(EXTOBJ)
ifeq ($(ARCH_NIX), 1)
	@cd $(PATH_BIN); ln -sf libbrahms-engine.$(EXTDLL) libbrahms-engine.$(EXTDLL_1)
	@cd $(PATH_BIN); ln -sf libbrahms-engine.$(EXTDLL_1) libbrahms-engine.$(EXTDLL_2)
endif
ifeq ($(ARCH_OS),win)
	@del ${call ONEARG,$(PATH_BIN)}\*.exp
endif
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))



