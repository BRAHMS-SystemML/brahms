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
#	$Id:: makefile 2403 2009-11-19 12:45:50Z benjmitch         $
#	$Rev:: 2403                                                $
#	$Author:: benjmitch                                        $
#	$Date:: 2009-11-19 12:45:50 +0000 (Thu, 19 Nov 2009)       $
#________________________________________________________________
#



# common file
include ../../makefile.brahms

# same version as BRAHMS
MAJ=$(VERSION_BRAHMS_MAJ)
MIN=$(VERSION_BRAHMS_MIN)
REL=$(VERSION_BRAHMS_REL)
REV=$(VERSION_BRAHMS_REV)

# shell script name
SHELL_SCRIPT=brahms$(EXTBAT)

# common dependencies
DEPS=makefile ../../makefile.brahms



################################################################
#	Targets

default : header \
	$(PATH_SML_NSP)/WARNING \
	$(PATH_BRM)/README \
	$(PATH_BRM)/RELEASE-NOTES \
	$(PATH_BRM)/LICENSE \
	$(PATH_BRM)/brahms.xml \
	$(PATH_BIN)/$(SHELL_SCRIPT) \
	$(PATH_INC)/brahms-component.h \
	$(PATH_INC)/brahms-client.h \
	$(PATH_INC)/brahms-1065.h \
	$(PATH_INC)/brahms-1199.h \
	$(PATH_INC)/brahms-1266.h

header :
	$(EMPTY_LINE)
	$(HEADER_LINE) framework/public
	$(EMPTY_LINE)
	$(call MKDIR, $(PATH_INC))
	$(call MKDIR, $(PATH_BIN))
	$(call MKDIR, $(PATH_SML_NSP))

$(PATH_INC)/brahms-%.h : brahms-%.h $(DEPS)
	$(SHOW_MAKING) $<
	cat $< \
		| sed s/__MAJ__/$(MAJ)/ \
		| sed s/__MIN__/$(MIN)/ \
		| sed s/__REL__/$(REL)/ \
		| sed s/__REV__/$(REV)/ \
		| sed s/__UINT16/\#define/ \
		| sed s/__UINT32/\#define/ \
		| sed s/__INT32/\#define/ \
		| sed s/__INT64/\#define/ \
		| sed s/__TYPE/\#define/ \
		| sed s/__Symbol/\#define/ \
		| sed s/__TOK_/\(/ \
		| sed s/_TOK__/\)/ \
		>	${call ONEARG,$@}

$(PATH_INC)/brahms-1065.h : brahms-1065.h brahms-c++-common.h brahms-c++-legacy.h $(DEPS)
	$(SHOW_MAKING) $<
	cat brahms-c++-common.h brahms-c++-legacy.h $< \
		| sed s/__BINDING__/1065/g \
		| sed s/__REV__/$(VERSION_BRAHMS_REV)/g \
		> ${call ONEARG,$@}

$(PATH_INC)/brahms-1199.h : brahms-1199.h brahms-c++-common.h brahms-c++-legacy.h $(DEPS)
	$(SHOW_MAKING) $<
	cat brahms-c++-common.h brahms-c++-legacy.h $< \
		| sed s/__BINDING__/1199/g \
		| sed s/__REV__/$(VERSION_BRAHMS_REV)/g \
		> ${call ONEARG,$@}

$(PATH_INC)/brahms-1266.h : brahms-1266.h $(DEPS)
	$(SHOW_MAKING) $<
	cat $< \
		| sed s/__BINDING__/1266/g \
		| sed s/__REV__/$(VERSION_BRAHMS_REV)/g \
		> ${call ONEARG,$@}

$(PATH_BRM)/% : % $(DEPS)
	$(SHOW_MAKING) $<
	@cp $< $(PATH_BRM)

$(PATH_SML_NSP)/% : % $(DEPS)
	$(SHOW_MAKING) $<
	@cp $< $(PATH_SML_NSP)

$(PATH_BIN)/$(SHELL_SCRIPT) : $(SHELL_SCRIPT) $(DEPS)
	$(SHOW_MAKING) $<
	@cp $< ${call ONEARG,$@}
ifeq ($(ARCH_OS), gln)
	@chmod 0775 ${call ONEARG,$@}
endif
ifeq ($(ARCH_OS), osx)
	@chmod 0775 ${call ONEARG,$@}
endif


