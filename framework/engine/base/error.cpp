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

	$Id:: error.cpp 2309 2009-11-06 00:49:32Z benjmitch        $
	$Rev:: 2309                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-06 00:49:32 +0000 (Fri, 06 Nov 2009)       $
________________________________________________________________

*/


#include "base.h"


#define ERROR_INDENT "    "


////////////////	NAMESPACE

namespace brahms
{
	namespace error
	{

		

////////////////	ERROR CLASS

		Error& Error::at(const char* cfile, int line)
		{
			//	strip path from filename
			string file = cfile;
			size_t p = file.rfind("/");
			if (p == string::npos) p = file.rfind("\\");
			if (p != string::npos) file = file.substr(p+1);

			stringstream ss;
			ss << "£at " << file << ":" << line;
			traces.push_back(ss.str());
			return (*this);
		}

		Error& Error::trace(const string& msg)
		{
			traces.push_back(msg);
			return (*this);
		}

		Error& Error::debugtrace(const string& msg)
		{
			traces.push_back("£" + msg);
			return (*this);
		}

		void inline xmlsafe(string& dst, const string& src)
		{
			//	search for dodgy characters
			const char* p = src.c_str();
			UINT32 l=0;
			UINT32 r=0;

			while(p[r])
			{
				switch(p[r])
				{
				case '<': dst += src.substr(l, r-l); dst += "&lt;";  l = r = r + 1; break;
				case '>': dst += src.substr(l, r-l); dst += "&gt;";  l = r = r + 1; break;
				case '&': dst += src.substr(l, r-l); dst += "&amp;";  l = r = r + 1; break;
				case '\'': dst += src.substr(l, r-l); dst += "&apos;";  l = r = r + 1; break;
				case '"': dst += src.substr(l, r-l); dst += "&quot;";  l = r = r + 1; break;
				default: r++;
				}
			}

			//	add the last
			dst += src.substr(l, r-l);
		}

		string Error::format(TextFormat ef, bool debugtrace)
		{
			string err;

			switch(ef)
			{

/*
			//	special case returned by the python bindings
			if (msg.length() > 6 && msg.substr(0,6) == "<code>")
			{
				size_t pos = msg.find("</code>");
				if (pos != string::npos)
				{
					string val = msg.substr(9, pos - 9);
					UINT32 u = brahms::text::s2n(val);
					ret = format(u);
				}
			}
*/

				case FMT_XML:
				{
					err = "<error>";
					err += "<code>" + string(brahms::base::symbol2string(code)) + "</code>";
					err += "<msg>";
					xmlsafe(err, msg);
					err += "</msg>";
					for (unsigned int t=0; t<traces.size(); t++)
					{
						string tr = traces[t];
						bool isdev = tr.length() && tr.substr(0,1) == "£";
						if (isdev) tr = tr.substr(1);

						if (debugtrace || !isdev)
						{
							err += "<trace>";
							xmlsafe(err, tr);
							err += "</trace>";
						}
					}
					err += "</error>";
					break;
				}

				case FMT_TEXT:
				{
					err = brahms::base::symbol2string(code);
					err += ": " + msg;
					for (unsigned int t=0; t<traces.size(); t++)
					{
						string tr = traces[t];
						bool isdev = tr.length() && tr.substr(0,1) == "£";
						if (isdev) tr = tr.substr(1);

						if (debugtrace || !isdev)
						{
							err += "\n" ERROR_INDENT + tr;
						}
					}
					break;
				}
				
				default:
				{
					err = "<no format specified in Error::format()>";
				}
			}

			return err;
		}



////////////////	NAMESPACE

	}
}


