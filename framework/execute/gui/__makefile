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
#	$Id:: makefile 2446 2010-01-12 16:57:41Z benjmitch         $
#	$Rev:: 2446                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2010-01-12 16:57:41 +0000 (Tue, 12 Jan 2010)       $
#________________________________________________________________
#





# common file
EASY_BUILD=1
include ../../../makefile.brahms



################################################################
#	Architecture-specific settings

ifeq ($(ARCH_NIX), 1)

	# local (compiler switches)
	COPT=$(FLAG_SHARED)
	LOPT=

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT=-LD
	LOPT=

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT)
LOPTS=$(LOPT_BASE) $(LOPT)




################################################################
#	Targets

ifeq ($(ARCH_NIX),1)
default : header $(PATH_TMP)/gui.$(EXTOBJ) $(PATH_TMP)/Gauge.$(EXTOBJ)
else
default : header $(PATH_TMP)/gui.$(EXTOBJ)
endif

header :
	$(call MKDIR, $(PATH_TMP))

$(PATH_TMP)/%.$(EXTOBJ) : %.cpp %.h makefile ../../../makefile.brahms *.h *.cpp
	@$(CXX) $(COPTS) -c $< $(NAMEOBJ)${call ONEARG,$@}
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))

$(PATH_TMP)/%.$(EXTOBJ) : %.c %.h makefile ../../../makefile.brahms
	$(SHOW_MAKING) $<
	@$(CC) -c $(COPTS) $< $(NAMEOBJ)${call ONEARG,$@}
	$(SHOW_MAKING) $(notdir $(subst $(SPACE),\$(SPACE),$@))


