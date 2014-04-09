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
#	$Id:: makefile 2372 2009-11-15 01:52:05Z benjmitch         $
#	$Rev:: 2372                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-15 01:52:05 +0000 (Sun, 15 Nov 2009)       $
#________________________________________________________________
#

# common file
include ../../../../makefile.brahms

# directory
DIR=$(PATH_SUP_TUT)/system/995

default :
	@make -s silent

silent : \
	header \
	$(DIR)/README \
	$(DIR)/developing_systems.m \
	$(DIR)/tutorial_1.m \
	$(DIR)/tutorial_2.m \
	$(DIR)/tutorial_3.m \
	$(DIR)/tutorial_4.m

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) tutorial/system/995
	$(EMPTY_LINE)

$(DIR)/README : README
	echo Making directory
	echo Copying $<
	$(call MKDIR, $(DIR))
	cp $< '$@'

$(DIR)/developing_systems.m : developing_systems.m
	echo Copying $<
	cp $< '$@'

$(DIR)/tutorial_1.m : tutorial_1.m
	echo Copying $<
	cp $< '$@'

$(DIR)/tutorial_2.m : tutorial_2.m
	echo Copying $<
	cp $< '$@'

$(DIR)/tutorial_3.m : tutorial_3.m
	echo Copying $<
	cp $< '$@'

$(DIR)/tutorial_4.m : tutorial_4.m
	echo Copying $<
	cp $< '$@'
