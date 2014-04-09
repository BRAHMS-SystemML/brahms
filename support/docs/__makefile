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
#	$Id:: makefile 2383 2009-11-17 00:36:34Z benjmitch         $
#	$Rev:: 2383                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-17 00:36:34 +0000 (Tue, 17 Nov 2009)       $
#________________________________________________________________
#

# common file
include ../../makefile.brahms

default :
	@make -s silent

silent : header $(PATH_SUP_DOC)/README

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) support/docs
	$(EMPTY_LINE)

$(PATH_SUP_DOC)/README : README
	$(SHOW_MAKING) $<
	$(call MKDIR, $(PATH_SUP_DOC))
	cp README $(PATH_SUP_DOC)/README
