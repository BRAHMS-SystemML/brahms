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

ifeq ($(ARCH_OS), gln)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=${call ONEARG,$(PATH_TMP)/base.$(EXTOBJ)}

	# MPI compiler
	MCXX=mpicxx $(CPPFLAGS)

endif

ifeq ($(ARCH_OS), osx)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=-lbrahms-engine ${call ONEARG,$(PATH_TMP)/base.$(EXTOBJ)}

	# MPI compiler
	MCXX=mpicxx $(CPPFLAGS)

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT=-LD
	LOPT=wsock32.lib user32.lib shell32.lib libbrahms-engine.lib ${call ONEARG,$(PATH_TMP)/base.$(EXTOBJ)}
	LOPTMPI=mpi.lib cxx.lib /NODEFAULTLIB:LIBCMT	

	# MPI compiler
	MCXX=cl $(CPPFLAGS)

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) $(DVER) -I.. -I../engine
LOPTS=$(LOPT_BASE) $(LOPT)



################################################################
#	Targets

ifeq ($(NOMPI),)
default : sockets-all mpi-all
else
default : sockets-all
endif

sockets-all : header-sockets sockets-layer
mpi-all : header-mpi mpi-layer

header-sockets :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/channel/sockets
	$(EMPTY_LINE)

header-mpi :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/channel/mpi
	$(EMPTY_LINE)

sockets-layer : $(PATH_BIN)/libbrahms-channel-sockets.$(EXTDLL)

mpi-layer : $(PATH_BIN)/libbrahms-channel-mpich2.$(EXTDLL)

DEPS=channel.cpp deliverer.cpp fifo.cpp *.h $(IF_BOTH) ../engine/base/*.h ../engine/support/*.h makefile ../../makefile.brahms $(PATH_TMP)/base.$(EXTOBJ)

$(PATH_BIN)/libbrahms-channel-sockets.$(EXTDLL) : $(DEPS) sockets*.cpp 
	@$(CXX) $(COPTS) -D__SOCKETS__ $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS)
	@rm -f \*.$(EXTOBJ)
ifeq ($(ARCH_NIX), 1)
	@cd $(PATH_BIN); ln -sf libbrahms-channel-sockets.$(EXTDLL) libbrahms-channel-sockets.$(EXTDLL_1)
	@cd $(PATH_BIN); ln -sf libbrahms-channel-sockets.$(EXTDLL_1) libbrahms-channel-sockets.$(EXTDLL_2)
endif
ifeq ($(ARCH_OS),win)
	@del ${call ONEARG,$(PATH_BIN)}\*.exp
	@del ${call ONEARG,$(PATH_BIN)}\*sockets.lib
endif
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))

$(PATH_BIN)/libbrahms-channel-mpich2.$(EXTDLL) : $(DEPS) mpi*.cpp
	@$(MCXX) $(COPTS) -D__MPI__ $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS) $(LOPTMPI)
	@rm -f \*.$(EXTOBJ)
ifeq ($(ARCH_NIX), 1)
	@cd $(PATH_BIN); ln -sf libbrahms-channel-mpich2.$(EXTDLL) libbrahms-channel-mpich2.$(EXTDLL_1)
	@cd $(PATH_BIN); ln -sf libbrahms-channel-mpich2.$(EXTDLL_1) libbrahms-channel-mpich2.$(EXTDLL_2)
endif
ifeq ($(ARCH_OS),win)
	@del ${call ONEARG,$(PATH_BIN)}\*.exp
	@del ${call ONEARG,$(PATH_BIN)}\*mpich2.lib
endif
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))



