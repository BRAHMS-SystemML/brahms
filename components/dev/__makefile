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

include ../../makefile.brahms

# path
BASEPATH=dev/std

# silent default
default :
	@make -s silent

# default is to build everything
silent:
	$(EMPTY_LINE)
	$(HEADER_LINE) dev
	$(EMPTY_LINE)
	make -s -f ../makefile-cpp COMPTYPE=process NAME=cat SUBPATH=$(BASEPATH)/cat/numeric
	make -s -f ../makefile-cpp COMPTYPE=process NAME=index SUBPATH=$(BASEPATH)/index/numeric
	make -s -f ../makefile-py COMPTYPE=process NAME=eval SUBPATH=$(BASEPATH)/eval/python
# this one is only for testing
#make -s -f ../makefile-cpp COMPTYPE=process NAME=modlin_activation_leaky SUBPATH=dev/modlin/leaky
