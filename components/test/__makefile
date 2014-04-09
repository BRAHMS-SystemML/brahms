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
#	$Id:: makefile 2449 2010-01-25 16:02:52Z benjmitch         $
#	$Rev:: 2449                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2010-01-25 16:02:52 +0000 (Mon, 25 Jan 2010)       $
#________________________________________________________________
#

include ../../makefile.brahms

# path
BASEPATH=client/brahms

# silent default
default :
	@make -s silent

# default is to build everything
silent:
	$(EMPTY_LINE)
	$(HEADER_LINE) test
	$(EMPTY_LINE)
	make -s -f ../makefile-m COMPTYPE=process NAME=rand SUBPATH=$(BASEPATH)/test/rand
	make -s -f ../makefile-cpp COMPTYPE=process NAME=except SUBPATH=$(BASEPATH)/test/except
	make -s -f ../makefile-cpp COMPTYPE=process NAME=abort SUBPATH=$(BASEPATH)/test/abort
	make -s -f ../makefile-cpp COMPTYPE=process NAME=hang SUBPATH=$(BASEPATH)/test/hang
	make -s -f ../makefile-cpp COMPTYPE=process NAME=sequence SUBPATH=$(BASEPATH)/test/sequence
	make -s -f ../makefile-cpp COMPTYPE=process NAME=busy SUBPATH=$(BASEPATH)/test/busy
	make -s -f ../makefile-cpp COMPTYPE=process NAME=listened SUBPATH=$(BASEPATH)/test/listened
	make -s -f ../makefile-cpp COMPTYPE=process NAME=complex SUBPATH=$(BASEPATH)/test/complex
	make -s -f ../makefile-cpp COMPTYPE=process NAME=complexr SUBPATH=$(BASEPATH)/test/complexr
	make -s -f ../makefile-py COMPTYPE=process NAME=complexp SUBPATH=$(BASEPATH)/test/complexp

test :
	make -s -f ../makefile-py COMPTYPE=process NAME=complexp SUBPATH=$(BASEPATH)/test/complexp
