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
#	$Id:: makefile 2406 2009-11-19 20:53:19Z benjmitch         $
#	$Rev:: 2406                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
#________________________________________________________________
#

# common file
include ../../../../makefile.brahms

# type
LANGUAGE=py

# directory
DIR=$(PATH_SUP_TEM)/process/1262

default :
	@make -s silent

silent : $(DIR)/component.$(LANGUAGE)

$(DIR)/component.$(LANGUAGE) : component.$(LANGUAGE) makefile ../test.m
	echo $(LANGUAGE)...
	$(call MKDIR, $(DIR))
	cp ../test.m ${call ONEARG,$(DIR)}
	cp $< ${call ONEARG,$(DIR)}
	cp ../../../../shared/node.process.xml $(DIR)/node.xml
	cp ../../../../shared/release.1262.xml $(DIR)/release.xml
