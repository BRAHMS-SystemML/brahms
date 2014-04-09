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
#	$Id:: makefile 2397 2009-11-18 21:20:07Z benjmitch         $
#	$Rev:: 2397                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)       $
#________________________________________________________________
#



# common file
include ../../../makefile.brahms

# common dependencies
DEPS=makefile ../../../makefile.brahms



################################################################
#	Targets

FILES_M_SRC=$(shell ls brahms*.m sml*.m *.expected @brahms_execution/*.m @brahms_execution/private/*.m @sml_log/*.m @sml_system/*.m @sml_system/private/*.m)
FILES_M_DST=$(addprefix $(PATH_BDG_INV_MAT)/,$(FILES_M_SRC))

default :
	@make -s silent

silent : \
	header \
	$(FILES_M_DST)

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/bindings/995
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_BDG_INV_MAT)/@brahms_execution/private)
	$(call MKDIR, $(PATH_BDG_INV_MAT)/@sml_system/private)
	$(call MKDIR, $(PATH_BDG_INV_MAT)/@sml_log)

$(PATH_BDG_INV_MAT)/%.m : %.m $(DEPS)
	$(SHOW_MAKING) $<
	cp $< '$@'

$(PATH_BDG_INV_MAT)/%.expected : %.expected $(DEPS)
	$(SHOW_MAKING) $<
	cp $< '$@'



