/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: types.h 2406 2009-11-19 20:53:19Z benjmitch          $
	$Rev:: 2406                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_BRAHMS_BASE_TYPES
#define INCLUDED_BRAHMS_BASE_TYPES



typedef UINT8 BYTE;

namespace brahms
{

	typedef vector<DOUBLE>			VDOUBLE;
	typedef vector<SINGLE>			VSINGLE;
	typedef vector<UINT64>			VUINT64;
	typedef vector<UINT32>			VUINT32;
	typedef vector<UINT16>			VUINT16;
	typedef vector<UINT8>			VUINT8;
	typedef vector<INT64>			VINT64;
	typedef vector<INT32>			VINT32;
	typedef vector<INT16>			VINT16;
	typedef vector<INT8>			VINT8;
	typedef vector<CHAR64>			VCHAR64;
	typedef vector<CHAR32>			VCHAR32;
	typedef vector<CHAR16>			VCHAR16;
	typedef vector<CHAR8>			VCHAR8;
	typedef vector<BOOL64>			VBOOL64;
	typedef vector<BOOL32>			VBOOL32;
	typedef vector<BOOL16>			VBOOL16;
	typedef vector<BOOL8>			VBOOL8;
	typedef vector<bool>			Vbool;
	typedef vector<BYTE>			VBYTE;
	typedef vector<string>			VSTRING;

	struct EventModuleCreateBindings : public EventModuleCreate
	{
		struct
		{
			const char* namespaceRootPath;
			const char* componentClass;
			const char* releasePath;
			const char* moduleFilename;
		}
		wrapped;
	};

};



#endif


