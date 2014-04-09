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

	$Id:: compress.h 2357 2009-11-11 01:12:24Z benjmitch       $
	$Rev:: 2357                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-11 01:12:24 +0000 (Wed, 11 Nov 2009)       $
________________________________________________________________

*/




////////////////	INTER-PROCESS DATA COMPRESSION (CONCERTO ONLY)

#ifndef INCLUDED_BRAHMS_COMPRESS
#define INCLUDED_BRAHMS_COMPRESS



#ifdef BRAHMS_BUILDING_COMPRESS
#define BRAHMS_COMPRESS_VIS BRAHMS_DLL_EXPORT
#else
#define BRAHMS_COMPRESS_VIS BRAHMS_DLL_IMPORT
#endif



//	function prototypes
typedef const char* (CompressFunction)(void* src, UINT32 srcBytes, void* dst, UINT32* dstBytes, INT32 compress);

//	export declaration
BRAHMS_COMPRESS_VIS const char* Compress(void* src, UINT32 srcBytes, void* dst, UINT32* dstBytes, INT32 compress);



////////////////	INCLUSION GUARD

#endif




