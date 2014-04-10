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

	$Id:: text.cpp 2316 2009-11-07 20:38:30Z benjmitch         $
	$Rev:: 2316                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-07 20:38:30 +0000 (Sat, 07 Nov 2009)       $
________________________________________________________________

*/

#include "base.h"

namespace brahms
{
	namespace text
	{
		void grep(string& str, const string what, const string with)
		{
			//	don't bother if they are the same
			if (what == with) return;

			//	global replace, but with offset so we can't get an infinite loop
			INT32 offset = 0;
			while(true)
			{
				size_t pos = str.find(what, offset);
				if (pos == string::npos) return;
				str.replace(pos, what.length(), with);
				offset = pos + with.length();
			}
		}

		VSTRING explode(const string delim, const string& data)
		{
			//	return vector of strings
			vector<string> ret;
			UINT32 offset = 0;

			//	split data by delim
			while(offset <= data.length())
			{
				//	find occurrence of delim
				string::size_type p = data.find(delim, offset);

				//	if not found, just add what's left
				if (p == string::npos)
				{
					ret.push_back(data.substr(offset));
					break;
				}

				//	otherwise, add first bit and continue with other bit
				ret.push_back(data.substr(offset,p-offset));
				offset = p + 1;
			}

			//	ok
			return ret;
		}

		void trim(string& str)
		{
			//	trim back
			UINT32 N = str.length();
			UINT32 R = N;
			while(R)
			{
				if (!isspace(str.at(R-1))) break;
				R--;
			}

			//	trim front
			UINT32 L = 0;
			while(L<R)
			{
				if (!isspace(str.at(L))) break;
				L++;
			}

			//	trim
			str = str.substr(L, R-L);
		}

		string n2s(DOUBLE val, UINT32 width)
		{
			stringstream ss;
			ss << val;
			if (!width) return ss.str();
			string ret = ss.str();
			while (ret.length() < width) ret = "0" + ret;
			return ret;
		}

		string h2s(UINT64 val, UINT32 width)
		{
			stringstream ss;
			ss << hex << uppercase << val;
			if (!width) return ss.str();
			string ret = ss.str();
			while (ret.length() < width) ret = "0" + ret;
			return ret;
		}

		DOUBLE s2n(const string &s)
		{
			stringstream ss(s);
			DOUBLE n = S2N_FAILED;
			ss >> n;
			return n;
		}

		const char* type2string(TYPE type)
		{
			switch(type & TYPE_ELEMENT_MASK)
			{
				case TYPE_SINGLE: return "SINGLE";
				case TYPE_DOUBLE: return "DOUBLE";
				case TYPE_INT8: return "INT8";
				case TYPE_INT16: return "INT16";
				case TYPE_INT32: return "INT32";
				case TYPE_INT64: return "INT64";
				case TYPE_UINT8: return "UINT8";
				case TYPE_UINT16: return "UINT16";
				case TYPE_UINT32: return "UINT32";
				case TYPE_UINT64: return "UINT64";
				case TYPE_CHAR8: return "CHAR8";
				case TYPE_CHAR16: return "CHAR16";
				case TYPE_CHAR32: return "CHAR32";
				case TYPE_CHAR64: return "CHAR64";
				case TYPE_BOOL8: return "BOOL8";
				case TYPE_BOOL16: return "BOOL16";
				case TYPE_BOOL32: return "BOOL32";
				case TYPE_BOOL64: return "BOOL64";
				case TYPE_STRUCT: return "STRUCT";
				case TYPE_CELL: return "CELL";

				default:
				{
					cerr << "type2string(0x" << hex << type << ") was called (unrecognised)\n";
					return "[TYPE_UNRECOGNISED]";
				}
			}
		}

		string precision2string(INT32 prec)
		{
			switch (prec)
			{
				case PRECISION_DO_NOT_LOG:
					return "[PRECISION_DO_NOT_LOG]";

				case PRECISION_NOT_SET:
					return "[PRECISION_NOT_SET]";

				default:
				{
					stringstream ss;
					ss << prec;
					return ss.str();
				}
			}
		}

		inline void v2s(UINT16 version, ostringstream &ret)
		{
			if (version == 0xFFFF) ret << "?";
			else ret << version;
		}

		string toString(FrameworkVersion v)
		{
			ostringstream ret;
			v2s(v.major, ret);
			ret << ".";
			v2s(v.minor, ret);
			ret << ".";
			v2s(v.release, ret);
			ret << ".";
			v2s(v.revision, ret);
			return ret.str();
		}

		string toString(ComponentVersion v)
		{
			ostringstream ret;
			v2s(v.release, ret);
			ret << ".";
			v2s(v.revision, ret);
			return ret.str();
		}

		string info2string(const ComponentInfo* info)
		{
			//	just a string summarising the specification
			string ret;
			if (info->cls) ret += info->cls;
			else ret += "\?\?\?";

			ret += "-" + toString(*info->componentVersion);
			return ret;
		}

		INT32 strimatch(const string& s1, const string& s2)
		{
			string::const_iterator it1=s1.begin();
			string::const_iterator it2=s2.begin();

			//	stop when either string's end has been reached
			while ( (it1!=s1.end()) && (it2!=s2.end()) )
			{
				if(::toupper(*it1) != ::toupper(*it2)) //letters differ?
				// return -1 to indicate smaller than, 1 otherwise
				return (::toupper(*it1)  < ::toupper(*it2)) ? -1 : 1;
				//proceed to the next character in each string
				++it1;
				++it2;
			}
			size_t size1=s1.size(), size2=s2.size();// cache lengths

			//	return -1, 0 or 1 according to strings' lengths
			if (size1 == size2) return 0;
			return (size1<size2) ? -1 : 1;
		}

		string slash2underscore(string cls)
		{
			brahms::text::grep(cls, "/", "_");
			return cls;
		}

		string sampleRateToString(SampleRate& sr)
		{
			stringstream ss;
			ss << sr.num << "/" << sr.den;
			return ss.str();
		}

	}
}



