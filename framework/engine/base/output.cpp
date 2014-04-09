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

	$Id:: output.cpp 2380 2009-11-16 02:00:10Z benjmitch       $
	$Rev:: 2380                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-16 02:00:10 +0000 (Mon, 16 Nov 2009)       $
________________________________________________________________

*/



////////////////	DECORATION

#define MSG_INDENT "       "

//	final routing is to stdout (which may have been redirected by the caller)
#define final(s) { if (logfile.is_open()) logfile << (s) << endl; else cout << (s) << endl; }

////////////////	TIME ELAPSED STRING

#include <cmath>

inline string Elapsed(DOUBLE t)
{
	stringstream ss;
	double s = floor(t);
	double ms = floor((t - s) * 1000000);
	ss << s << ".";
	ss.width(6);
	ss.fill('0');
	ss << ms;
	return ss.str();
}

inline UINT32 base_unitIndex(UINT32 i)
{
	return i + 1;
}




namespace brahms
{
	namespace output
	{



////////////////	OUTPUT SOURCE

		Source::Source()
		{
			sink = NULL;
		}

		Source::Source(const Source& rhs)
		{
			sink = rhs.sink;
			threadIdentifier = rhs.threadIdentifier;
			prefix = rhs.prefix;
		}

		Source& Source::operator=(const Source& rhs)
		{
			//	handle self-assignment
			if (&rhs == this) return *this;

			//	copy as necessary
			sink = rhs.sink;
			threadIdentifier = rhs.threadIdentifier;
			prefix = rhs.prefix;
			return *this;
		}

		Source::~Source()
		{
		}

		void Source::disconnect()
		{
			this->sink = NULL;
		}

		void Source::connect(Sink* sink, const char* threadIdentifier)
		{
			this->sink = sink;
			this->threadIdentifier = threadIdentifier;

			//	make prefix of fixed length
			UINT32 L = 6;
			prefix = threadIdentifier;
			while(prefix.length() < L) prefix += " ";

			//	announce connection
			sink->connect(this);
		}

		Source& Source::operator<<(const EnumDetailLevel& p_level)
		{
			//	send output to sink
			if (sink)
			{
				//	if D_INFO_UNHIDEABLE, send anyway as D_INFO
				if (p_level == D_INFO_UNHIDEABLE)
					sink->message(this, D_INFO);

				//	else, send only if within level
				else if (p_level <= level)
					sink->message(this, p_level);
			}

			//	or warn
			else
			{
				//	warn
				____WARN("MESSAGE LOST: " << buffer.str());
			}

			//	clear
			buffer.str("");
			buffer.flags(ios::dec | ios::skipws);

			//	ok
			return *this;
		}



////////////////	OUTPUT SINK

		Sink::Sink()
		{
			format = FMT_TEXT;
			debugtrace = false;
			messageCount = 0;
			//indent = 0;
		}

		Sink::~Sink()
		{
			if (logfile.is_open()) logfile.close();
		}

		EnumDetailLevel translateLogLevel(char c)
		{
			switch(c)
			{
				case 'n': return (D_NONE);
				case 'w': return (D_WARN);
				case 'i': return (D_INFO);
				case 'v': return (D_VERB);
				case 'l': return (D_LOQU);
				case 'f': return (D_FULL);
				case '0': return (D_NONE);
				case '1': return (D_WARN);
				case '2': return (D_INFO);
				case '3': return (D_VERB);
				case '4': return (D_LOQU);
				case '5': return (D_FULL);
			}

			ferr << E_INVOCATION << "invalid log level \"" << c << "\"";
			return D_INFO;
		}

		void Sink::init(const CreateEngine& createEngine)
		{
			format = createEngine.logFormat;
			debugtrace = createEngine.engineFlags & F_MAXERR;

			//	log file
			if (createEngine.logFilename)
			{
				logfile.open(createEngine.logFilename, ios::binary | ios::trunc);
				if (!logfile) ferr << E_ERROR << "failed open logfile \"" << createEngine.logFilename << "\"";
			}

			//	default
			defaultLevel = D_INFO;

			if (createEngine.logLevels)
			{
				string s = createEngine.logLevels;

				//	if this happens when we are already connected, we'll have to
				//	fill in the "sources" field of the switches as we create them
				if (sources.size())
					ferr << E_INTERNAL;

				while(s.length())
				{
					//	peel off one option
					size_t sc = s.find(";");
					if (sc == string::npos)
						ferr << E_INVOCATION << "invalid log level string \"" << s << "\"";
					string ll = s.substr(0, sc);
					s = s.substr(sc + 1);

					//	process log option, which has the form
					//	l : starting log level for all threads (c, w, i, v, 1-5)
					//	l-t : starting log level for thread with identifier "t"
					//	l-t-s : new log level for thread t when section s is entered
					//	l--s : new log level for all threads when section s is entered (i.e. empty thread ID string means all threads)

					size_t da = ll.find("-");
					if (da == string::npos)
					{
						//	starting log level for all threads (must be exactly one char)
						if (ll.length() != 1)
							ferr << E_INVOCATION << "invalid log level string \"" << ll << "\"";
						defaultLevel = translateLogLevel(ll.at(0));
					}

					else
					{
						//	level part (L) must be exactly one char
						if (da != 1)
							ferr << E_INVOCATION << "invalid log level string \"" << ll << "\"";
						EnumDetailLevel level = translateLogLevel(ll.at(0));

						//	of rest, is it split into T-S?
						string l = ll.substr(da+1);
						da = l.find("-");
						string threadIdentifier;

						//	no
						if (da == string::npos)
						{
							//	T
							threadIdentifier = l;
							l = "";
						}

						//	yes
						else
						{
							//	T-S
							threadIdentifier = l.substr(0, da);
							l = l.substr(da + 1);
						}

						//	empty-empty is no go
						if (threadIdentifier.length() == 0 && l.length() == 0)
							ferr << E_INVOCATION << "invalid log level string \"" << ll << "\"";

						//	replace any underscores in section name with spaces
						for (unsigned int c=0; c<l.length(); c++)
							if (l[c] == '_') l[c] = ' ';

						//	store
						LogLevelSwitch sw;
						sw.threadIdentifier = threadIdentifier;
						sw.section = l;
						sw.level = level;
						switches.push_back(sw);
					}

				}
			}

			//	write header
			if (format == FMT_XML)
			{
				string s = "<?xml version=\"1.0\"?>\n<?xml-stylesheet href=\"log.xslt\" type=\"text/xsl\"?>\n<log>";
				final(s.c_str());
			}
		}

		UINT32 Sink::term(const DataBurst& dataBurst, vector<brahms::error::Error>& errors)
		{
			if (format == FMT_XML)
			{
				stringstream ss;
				ss << "<data>\n";
				ss << "<voiceindex>" << base_unitIndex(dataBurst.voiceIndex) << "</voiceindex>\n";
				ss << "<voicecount>" << dataBurst.voiceCount << "</voicecount>\n";
				ss << "<workercount>" << dataBurst.workerCount << "</workercount>\n";
				ss << "</data>";
				final(ss.str().c_str());
			}

			//	write errors
			if (format == FMT_XML) final("<errors>");
			//else if (errors.size()) final("________________________________________________________________\n");
			for (unsigned int e=0; e<errors.size(); e++)
			{
				string s = errors[e].format(format, debugtrace);
				if (e) s = "\n" + s;
				final(s.c_str());
			}
			if (format == FMT_XML) final("</errors>");

			//	write footer
			if (format == FMT_XML)
				final("</log>");

			//	ok
			return messageCount;
		}


		void Sink::section(const string& key)
		{
			//	now we lock the mutex to protect access to stdout
			brahms::os::MutexLocker locker(mutex);

			//	output only in XML mode
			if (key.length())
			{
				//	push
				sections.push(key);

				//	enter
				if (format == FMT_XML)
				{
					stringstream ss;
					ss << "<sect lvl=\"" << sections.size() << "\" key=\"" << key << "\">";
					final(ss.str().c_str());
				}

				//	switch log levels
				for (UINT32 s=0; s<switches.size(); s++)
				{
					if (switches[s].section == key)
					{
						//	operate this switch
						LogLevelSwitch& sw = switches[s];
						for (UINT32 i=0; i<sw.sources.size(); i++)
						{
							Source* src = sw.sources[i];
							src->levelsStack.push(src->level);
							src->level = sw.level;
						}
					}
				}
			}

			else
			{
				//	switch log levels
				for (UINT32 s=0; s<switches.size(); s++)
				{
					if (switches[s].section == sections.top())
					{
						//	operate this switch
						LogLevelSwitch& sw = switches[s];
						for (UINT32 i=0; i<sw.sources.size(); i++)
						{
							Source* src = sw.sources[i];

							//	if a thread starts after a switch has turned on, we may end
							//	up here to turn off the switch on a thread that has no history.
							//	we handle this silently by not restoring from the stack in this case.
							if (src->levelsStack.size())
							{
								src->level = src->levelsStack.top();
								src->levelsStack.pop();
							}
						}
					}
				}

				//	leave
				if (format == FMT_XML)
				{
					//stringstream ss;
					//ss << "<endsect lvl=\"" << sections.size() << "\" key=\"" << sections.top() << "\"/>";
					//final(ss.str().c_str());
					final("</sect>");
				}

				//	pop
				if (!sections.size()) ferr << E_INTERNAL << "stack underflow";
				sections.pop();
			}
		}

		bool matchThreadID(const string& pattern, const string& id)
		{
			if (pattern == id) return true; // exact match
			if (pattern.length() == 1 && id.length() >= 1 && pattern.substr(0,1) == id.substr(0,1)) return true; // first character match, e.g. "W" matches "W1" and "W12"
			return false;
		}

		void Sink::connect(Source* source)
		{
			//	set default log level
			source->level = defaultLevel;

			//	store
			sources.push_back(source);

			//	add source to all matching switches
			for (UINT32 s=0; s<switches.size(); s++)
			{
				//	handle default level
				if (switches[s].section.length() == 0)
				{
					if (matchThreadID(switches[s].threadIdentifier, source->threadIdentifier))
						source->level = switches[s].level;
				}

				//	handle section level
				else
				{
					if (switches[s].threadIdentifier.length() == 0 || matchThreadID(switches[s].threadIdentifier, source->threadIdentifier))
						switches[s].sources.push_back(source);
				}
			}
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

		char translateLogLevelBack(EnumDetailLevel l)
		{
			switch(l)
			{
				case D_NONE: return '\0';
				case D_WARN: return 'w';
				case D_INFO: return '\0';
				case D_VERB_HILITE: return 'h';
				case D_VERB_ERROR: return 'e';
				case D_INFO_UNHIDEABLE: return '\0';
				case D_VERB: return '\0';
				case D_LOQU: return '\0';
				case D_FULL: return '\0';
			}

			return '?';
		}

		//	send source buffer to log (if detail level is sufficient)
		void Sink::message(Source* source, EnumDetailLevel level)
		{
			//	now we lock the mutex to protect access to stdout
			brahms::os::MutexLocker locker(mutex);

			//	msg
			char f = translateLogLevelBack(level);
			string msg = source->buffer.str();

			//	build
			string st = Elapsed(sinkTimer.elapsed());
			string out;

			//	output to XML
			if (format == FMT_XML)
			{
				out = "<msg";
				if (f) out += " f=\"" + string(1, f) + "\""; // (f)ormatting
				out += " i=\"" + source->threadIdentifier + "\" t=\"" + st + "\">"; // (i)dentifier and (t)ime
				xmlsafe(out, msg);
				out += "</msg>";
			}
			
			//	output to text
			else
			{
				out = source->prefix + st;

				//	switch on detail level for various different behaviours
				switch(level)
				{
					case D_WARN:
					{
						out += ("**** WARNING **** " MSG_INDENT) + msg;
						break;
					}

					case D_VERB_HILITE:
					case D_VERB_ERROR:
					{
						out += ("**** " MSG_INDENT) + msg;
						break;
					}

					default:
					{
						out += MSG_INDENT + msg;
						break;
					}
				}
			}

			//	out
			final(out.c_str());

			//	ok
			messageCount++;
		}



////////////////	NAMESPACE

	}
}




