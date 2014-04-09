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
#	$Id:: makefile 2375 2009-11-15 20:34:47Z benjmitch         $
#	$Rev:: 2375                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-15 20:34:47 +0000 (Sun, 15 Nov 2009)       $
#________________________________________________________________
#



# common file
include ../../makefile.brahms

# common dependencies
DEPS=makefile ../../makefile.brahms



################################################################
#	Targets

default : header \
	$(PATH_MED)/log.xslt \
	$(PATH_MED)/brahms-128.dat \
	$(PATH_MED)/johannes-128.dat \
	$(PATH_MED)/adjacent-128.dat \
	$(PATH_MED)/interleaved-128.dat \
	$(PATH_MED)/scratchbot.avi

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/media
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_MED))

$(PATH_MED)/% : %
	$(SHOW_MAKING) $<
	@cp $< $(PATH_MED)



