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
#	$Id:: makefile 2408 2009-11-19 22:45:06Z benjmitch         $
#	$Rev:: 2408                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-19 22:45:06 +0000 (Thu, 19 Nov 2009)       $
#________________________________________________________________
#





# common file
include ../../makefile.brahms

# same version as BRAHMS
MAJ=$(VERSION_BRAHMS_MAJ)
MIN=$(VERSION_BRAHMS_MIN)
REV=$(VERSION_BRAHMS_REV)
REL=$(VERSION_BRAHMS_REL)


################################################################
#	Architecture-specific settings

ifeq ($(ARCH_NIX), 1)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=-lz -lbrahms-engine

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT=-LD
	LOPT=${call ONEARG,$(PATH_BIN)/libbrahms-engine.lib} zlib-1.2.3/zdll.lib

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) -I../engine
LOPTS=$(LOPT_BASE) $(LOPT)



################################################################
#	Targets

default : header $(PATH_BIN)/libbrahms-compress.$(EXTDLL)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/compress
	$(EMPTY_LINE)

DEPS=compress.cpp *.h *.cpp $(IF_CMP) makefile  ../../makefile.brahms

$(PATH_BIN)/libbrahms-compress.$(EXTDLL) : $(DEPS)
	@$(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS)
	@rm -f \*.$(EXTOBJ)
ifeq ($(ARCH_NIX), 1)
	@cd $(PATH_BIN); ln -sf libbrahms-compress.$(EXTDLL) libbrahms-compress.$(EXTDLL_1)
	@cd $(PATH_BIN); ln -sf libbrahms-compress.$(EXTDLL_1) libbrahms-compress.$(EXTDLL_2)
endif
ifeq ($(ARCH_OS),win)
	@del ${call ONEARG,$(PATH_BIN)}\*.exp
	@del ${call ONEARG,$(PATH_BIN)}\*compress.lib
	@cp zlib-1.2.3/zlib1.dll $(PATH_BIN)
endif
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))



