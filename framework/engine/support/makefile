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
include ../../../makefile.brahms



################################################################
#	Architecture-specific settings

# local (compiler switches)
COPT=$(FLAG_SHARED) # is this needed since we're only building obj file?
LOPT=

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT) -I.. -I../..
LOPTS=$(LOPT_BASE) $(LOPT)

# build to one object file
F_OBJ=$(PATH_TMP)/support.$(EXTOBJ)



################################################################
#	Targets

default : header $(F_OBJ)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/engine/support
	$(EMPTY_LINE)

$(F_OBJ) : support.cpp *.cpp *.h makefile ../../../makefile.brahms $(IF_BOTH) ../base/*.h ../../channel/channel.h
	$(call MKDIR, $(PATH_TMP))
ifeq ($(BUILD), DEBUG)
	@echo $(CXX) $(COPTS) -c $< $(NAMEOBJ)${call ONEARG,$@}
endif
	$(CXX) $(COPTS) -c $< $(NAMEOBJ)${call ONEARG,$@}
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))


