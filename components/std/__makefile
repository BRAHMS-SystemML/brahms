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
#	$Id:: makefile 2427 2009-12-11 04:57:31Z benjmitch         $
#	$Rev:: 2427                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-12-11 04:57:31 +0000 (Fri, 11 Dec 2009)       $
#________________________________________________________________
#

include ../../makefile.brahms

# path
BASEPATH=std/2009

# silent default
default :
	@make -s silent

# default is to build everything
silent:
	$(EMPTY_LINE)
	$(HEADER_LINE) std
	$(EMPTY_LINE)
	make -s -f ../makefile-cpp COMPTYPE=data NAME=data_numeric SUBPATH=$(BASEPATH)/data/numeric
	make -s -f ../makefile-cpp COMPTYPE=data NAME=data_spikes SUBPATH=$(BASEPATH)/data/spikes
	make -s -f ../makefile-cpp COMPTYPE=utility NAME=util_rng SUBPATH=$(BASEPATH)/util/rng
	make -s -f ../makefile-cpp COMPTYPE=process NAME=random_numeric SUBPATH=$(BASEPATH)/random/numeric
	make -s -f ../makefile-cpp COMPTYPE=process NAME=random_spikes SUBPATH=$(BASEPATH)/random/spikes
	make -s -f ../makefile-cpp COMPTYPE=process NAME=math_esum SUBPATH=$(BASEPATH)/math/esum
	make -s -f ../makefile-cpp COMPTYPE=process NAME=math_eproduct SUBPATH=$(BASEPATH)/math/eproduct
	make -s -f ../makefile-cpp COMPTYPE=process NAME=source_numeric SUBPATH=$(BASEPATH)/source/numeric
	make -s -f ../makefile-cpp COMPTYPE=process NAME=source_spikes SUBPATH=$(BASEPATH)/source/spikes
	make -s -f ../makefile-cpp COMPTYPE=process NAME=resample_numeric SUBPATH=$(BASEPATH)/resample/numeric
	make -s -f ../makefile-cpp COMPTYPE=process NAME=resample_spikes SUBPATH=$(BASEPATH)/resample/spikes

test:
	make -s -f ../makefile-cpp COMPTYPE=data NAME=data_numeric SUBPATH=$(BASEPATH)/data/numeric
