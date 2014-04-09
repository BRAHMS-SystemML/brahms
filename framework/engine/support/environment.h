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

	$Id:: environment.h 2309 2009-11-06 00:49:32Z benjmitch    $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_ENVIRONMENT
#define INCLUDED_ENGINE_ENVIRONMENT



////////////////	NAMESPACE

namespace brahms
{


////////////////	ENVIRONMENT

	class Environment
	{

	public:

		//	tors
		Environment(brahms::base::Core& core);
		~Environment();

		//	set interface
		void add(string key);
		void set(string key, string value, brahms::output::Source& fout);
		void set(string key, DOUBLE value, brahms::output::Source& fout);
		void load(string path, bool insist, brahms::output::Source& fout);
		void load(const char* path, brahms::xml::XMLNode& node, brahms::output::Source& fout);

		//	get interface
		DOUBLE get(const char* key, DOUBLE min = -1.0e100, DOUBLE max = +1.0e100) const;
		UINT32 getu(const char* key, UINT32 max = 0xFFFFFFFF) const;
		INT32 geti(const char* key, INT32 min = -0x80000000, INT32 max = 0x7FFFFFFF) const;
		bool getb(const char* key) const;
		string gets(const char* key) const;
		VUINT32 getulist(const char* key, const UINT32 count) const;

		//	miscellaneous
		void assertType(const char* name, char type) const;
		void finalize(UINT32 voiceCount, UINT32 voiceIndex, brahms::output::Source& fout);

		//	execution parameters
		brahms::xml::XMLNode nodeExecPars;
		brahms::xml::XMLControl controlExecPars;

		//	keep core up-to-date (ugly, but will do for now TODO)
		brahms::base::Core& core;
	};

}



////////////////	INCLUSION GUARD

#endif


