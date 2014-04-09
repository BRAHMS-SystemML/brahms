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
#	$Id:: makefile 2403 2009-11-19 12:45:50Z benjmitch         $
#	$Rev:: 2403                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-19 12:45:50 +0000 (Thu, 19 Nov 2009)       $
#________________________________________________________________
#

# common file
include ../../makefile.brahms

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

	# local (compiler switches)
	COPT=-I$(PATH_MATLAB)/extern/include -D__NIX__
	LOPT=-L$(PATH_MATLAB)/bin/$(MATARCH) -leng -lmx -lut -lmat -lMTwister -lstdc++ -licudata -licuuc -licui18n -licuio

endif

ifeq ($(ARCH_OS), osx)

	ifeq ($(ARCH_BITS), 64)
		MATARCH=what_is_this_supposed_to_be
	else
		MATARCH=maci
	endif

	# local (compiler switches)
	COPT=-I$(PATH_MATLAB)/extern/include -D__NIX__
	LOPT=-L$(PATH_MATLAB)/bin/$(MATARCH) -leng -lmx -lut -lmat -lMTwister -lstdc++ -licudata -licuuc -licui18n -licuio

endif

ifeq ($(ARCH_OS), win)

	# local (compiler switches)
	COPT=-I${call ONEARG,$(PATH_MATLAB)}/extern/include
	LOPT=-LIBPATH:${call ONEARG,$(PATH_MATLAB)}/extern/lib/win32/microsoft libeng.lib

endif

# compiler switches for all builds
COPTS=$(COPT_BASE) $(COPT)
LOPTS=$(LOPT_BASE) $(LOPT)

DEPS=checkengine.cpp *.h *.cpp makefile  ../../makefile.brahms

default :
	@make -s silent

silent : header $(PATH_SUP_CHK_ENG)/README $(PATH_SUP_CHK_ENG)/check_simultaneous_use.bat $(PATH_SUP_CHK_ENG)/checkengine$(EXTEXE)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) support/checkengine
	$(EMPTY_LINE)

$(PATH_SUP_CHK_ENG)/README : README
	$(SHOW_MAKING) $<
	$(call MKDIR, $(PATH_SUP_CHK_ENG))
	cp README $(PATH_SUP_CHK_ENG)/README

$(PATH_SUP_CHK_ENG)/check_simultaneous_use.bat : check_simultaneous_use.bat
	$(SHOW_MAKING) $<
	$(call MKDIR, $(PATH_SUP_CHK_ENG))
	cp check_simultaneous_use.bat $(PATH_SUP_CHK_ENG)/check_simultaneous_use.bat

$(PATH_SUP_CHK_ENG)/checkengine$(EXTEXE) : $(DEPS)
	@$(CXX) $(COPTS) $< $(NAMEEXE)${call ONEARG,$@} $(LOPTS)
ifeq ($(ARCH_OS), win)
	@rm *.obj
endif

	
