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
#	$Id:: makefile 2411 2009-11-20 12:09:27Z benjmitch         $
#	$Rev:: 2411                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-20 12:09:27 +0000 (Fri, 20 Nov 2009)       $
#________________________________________________________________
#

include ../../makefile.brahms

# silent default
default :
	@make -s silent

# common file
include ../../makefile.brahms

# group parameters
BASEPATH=client/brahms/example
SCRIPTPATH=$(PATH_SUP_EXA)/995

silent : header $(SCRIPTPATH)/rabbitsandfoxes.m $(SCRIPTPATH)/brainandhead.m $(SCRIPTPATH)/simplegui.m
	make -s -f ../makefile-m COMPTYPE=process NAME=gui SUBPATH=$(BASEPATH)/1258/gui
	make -s -f ../makefile-m COMPTYPE=process NAME=rabbits SUBPATH=$(BASEPATH)/1258/rabbits
	make -s -f ../makefile-m COMPTYPE=process NAME=foxes SUBPATH=$(BASEPATH)/1258/foxes
	make -s -f ../makefile-m COMPTYPE=process NAME=brain SUBPATH=$(BASEPATH)/1258/brain
	make -s -f ../makefile-m COMPTYPE=process NAME=head SUBPATH=$(BASEPATH)/1258/head
	make -s -f ../makefile-py COMPTYPE=process NAME=rabbits SUBPATH=$(BASEPATH)/1262/rabbits
	make -s -f ../makefile-py COMPTYPE=process NAME=foxes SUBPATH=$(BASEPATH)/1262/foxes
	make -s -f ../makefile-py COMPTYPE=process NAME=brain SUBPATH=$(BASEPATH)/1262/brain
	make -s -f ../makefile-py COMPTYPE=process NAME=head SUBPATH=$(BASEPATH)/1262/head

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) example
	$(EMPTY_LINE)
	$(call MKDIR, $(SCRIPTPATH))

$(SCRIPTPATH)/%.m : %.m makefile
	$(SHOW_MAKING) $<
	cp $< ${call ONEARG, $@}
