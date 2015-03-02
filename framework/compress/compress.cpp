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

	$Id:: compress.cpp 2357 2009-11-11 01:12:24Z benjmitch     $
	$Rev:: 2357                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-11 01:12:24 +0000 (Wed, 11 Nov 2009)       $
________________________________________________________________

*/

////////////////	INCLUDE SUPPORT
#define BRAHMS_BUILDING_COMPRESS

//	includes
#include "brahms-client.h"
#include "compress.h"

#ifdef __WIN__
//	use version in this folder
#include "zlib-1.2.3/zlib.h"
#endif

#ifdef __NIX__
//	use version installed on build machine
#include "zlib.h"
#endif

#if ZLIB_VERNUM < 0x1230
#error must link to zlib 1.2.1 or greater
#endif

BRAHMS_COMPRESS_VIS const char* Compress(void* src, UINT32 srcBytes, void* dst, UINT32* dstBytes, INT32 compress)
{
	//	compress == -1 means decompress
	//	compress == 1 to 9 means compress (at that level of compression)
	if (compress == -1)
	{
		if (!srcBytes)
		{
			*dstBytes = 0;
			return 0;
		}

		unsigned uncompressedSize;
		z_stream strm;

		/* allocate inflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = srcBytes;
		strm.next_in = (Bytef*) src;
		if (inflateInit(&strm) != Z_OK) return "inflateInit()";

		//	inflate
		strm.avail_out = *dstBytes;
		strm.next_out = (Bytef*) dst;
		while (true)
		{
			int ret = inflate(&strm, Z_FINISH);
			if (ret == Z_STREAM_END) break;

			switch(ret)
			{
				case Z_OK: continue;
				case Z_NEED_DICT: { return "Z_NEED_DICT in inflate()"; }
				case Z_ERRNO: { return "Z_ERRNO in inflate()"; }
				case Z_STREAM_ERROR: { return "Z_STREAM_ERROR in inflate()"; }
				case Z_DATA_ERROR: { return "Z_DATA_ERROR in inflate()"; }
				case Z_MEM_ERROR: { return "Z_MEM_ERROR in inflate()"; }
				case Z_BUF_ERROR: { return "Z_BUF_ERROR in inflate()"; }
				case Z_VERSION_ERROR: { return "Z_VERSION_ERROR in inflate()"; }
			}

			return "error in inflate()";
		}

		//	ok
		uncompressedSize = *dstBytes - strm.avail_out;
		inflateEnd(&strm);

		//	return uncompressed size
		*dstBytes = uncompressedSize;

		//	ok
		return 0;
	}

	if (compress >= 1 && compress <= 9)
	{
		if (!srcBytes)
		{
			*dstBytes = 0;
			return 0;
		}

		UINT32 compressedSize;
		z_stream strm;

		/* allocate deflate state */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		if (deflateInit(&strm, compress) != Z_OK)
			return "failed init deflate (deflateInit())";

		//	compress in one go
		strm.avail_out = *dstBytes;
		strm.next_out = (Bytef*)dst;
		strm.avail_in = srcBytes;
		strm.next_in = (Bytef*)src;

		while (true)
		{
			int ret = deflate(&strm, Z_FINISH);
			if (ret == Z_OK) continue;
			if (ret == Z_STREAM_END) break;
			return "error in deflate()";
		}

		compressedSize = *dstBytes - strm.avail_out; // what is left over tells us how much we compressed
		if (strm.avail_in != 0) return "unexpected error from deflate (2)"; /* all input will be used */

		/* clean up and return */
		deflateEnd(&strm);

		//	return compressed size
		*dstBytes = compressedSize;

		//	ok
		return 0;
	}

	//	unrecognised
	return "unrecognised compress level";
}
