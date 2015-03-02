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

	$Id:: text.h 2278 2009-11-01 21:23:08Z benjmitch           $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/

#ifndef INCLUDED_BRAHMS_BASE_TEXT
#define INCLUDED_BRAHMS_BASE_TEXT

#include <string>
using std::string;
#include <vector>
using std::vector;

namespace brahms
{
	inline bool isunderscore(char c)
	{
		return c == '_';
	}

	inline bool istokenstart(char c)
	{
		return isalpha(c) || isunderscore(c);
	}

	inline bool istokencontinue(char c)
	{
		return isalpha(c) || isunderscore(c) || isdigit(c);
	}

	inline DOUBLE sampleRateToRate(SampleRate& sr)
	{
		return ((DOUBLE)sr.num) / ((DOUBLE)sr.den);
	}

	inline DOUBLE sampleRateToPeriod(SampleRate& sr)
	{
		return ((DOUBLE)sr.den) / ((DOUBLE)sr.num);
	}

	namespace text
	{
		void grep(string& str, const string what, const string with);
		vector<string> explode(const string delim, const string& data);
		void trim(string& str);
		const char* type2string(TYPE type);
		string precision2string(INT32 prec);
		string toString(FrameworkVersion v);
		string toString(ComponentVersion v);
		string info2string(const ComponentInfo* info);
		INT32 strimatch(const string& s1, const string& s2);
		string slash2underscore(string cls);
		string sampleRateToString(SampleRate& sr);
		string n2s(DOUBLE val, UINT32 width = 0);
		string h2s(UINT64 val, UINT32 width = 0);
		const DOUBLE S2N_FAILED = 1.23456789e308;
		DOUBLE s2n(const string &s);
	}
}

#endif // INCLUDED_BRAHMS_BASE_TEXT
