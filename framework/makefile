#________________________________________________________________
#
#	This is the platform-independent makefile for this
#	module. You will need to have UnxUtils installed to
#	make it work on Windoze:
#
#		http://unxutils.sourceforge.net
#
#	You can compile any of the flavours, using
#
#		make [gui=enable|disable] [with-x=DIR]
#
#	(default to gui=enable with-x=/usr/X11R6).
#	The with-x option is only used on Linux with gui=enable.
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
#	$Id:: makefile 2419 2009-11-30 18:33:48Z benjmitch         $
#	$Rev:: 2419                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
#________________________________________________________________
#



# SystemML Toolbox
PATH_STB_EXPECTED=../../../systemml/trunk
PATH_STB=${wildcard $(PATH_STB_EXPECTED)}

# common file
include ../makefile.brahms

# default
default :
	@make -s silent

silent :
ifeq ($(PATH_STB),)
else
	make -C $(PATH_STB) -s
endif
	make -C public -s
	make -C media -s
	make -C engine -s
	make -C channel -s
	make -C compress -s
	make -C execute -s
	make -C bindings -s



