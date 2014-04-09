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

	$Id:: identifier.cpp 2100 2009-09-13 15:08:26Z benjmitch   $
	$Rev:: 2100                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-09-13 16:08:26 +0100 (Sun, 13 Sep 2009)       $
________________________________________________________________

*/


#include "systemml.h"


using namespace brahms::math;



namespace brahms
{
	namespace systemml
	{
		bool validSampleRate(SampleRate sampleRate)
		{
			return (sampleRate.num && sampleRate.den);
		}

		//	To Lowest Terms (crazy made-up algorithm)
		//	by mitch, which is why it's so ugly

		//	this is done to Process rates as they are read in from the SystemML
		//	and to Data rates as they are instantiated, so later when we do
		//	the base rate search, we can assume all passed rates are already in
		//	lowest terms

		SampleRate toLowestTerms(SampleRate s)
		{
			UINT64 d = gcd(s.num, s.den);
			if (d > 1)
			{
				s.num /= d;
				s.den /= d;
			}
			return s;
		}

		SampleRate parseSampleRate(string s_sampleRate)
		{

			//	sampleRate must either be an integer (all digits, and not longer than two_to_the_52) or
			//	a fraction of the form "N/D" with both N and D compliant with the above.
			int L = s_sampleRate.length();
			if (!L)
				ferr << E_SYSTEM_FILE << "sample rate string is empty";
			const char* s = s_sampleRate.c_str();
			int slashPosition = -1;
			for (int c=0; c<L; c++)
			{
				if (s[c] == '/')
				{
					if (slashPosition != -1) ferr << E_SYSTEM_FILE << "too many / characters in sample rate string";
					slashPosition = c;
				}
				else if(s[c] >= 48 && s[c] <= 57)
				{
					//	ok, it's a digit
				}
				else if (s[c] == '.')
				{
					ferr << E_SYSTEM_FILE << "sample rate string invalid - use fractions not decimals, e.g. 513/100 for 51.3Hz";
				}
				else ferr << E_SYSTEM_FILE << "invalid character in sample rate string \"" << s[c] << "\"";
			}
			if (slashPosition == 0 || slashPosition == (L-1))
				ferr << E_SYSTEM_FILE << "/ character cannot begin or end sampleRate string";

			//	represent as fraction
			SampleRate sr;
			if (slashPosition != -1)
			{
				double d_tmp;
				if (!(istringstream(s_sampleRate) >> d_tmp))
					ferr << E_SYSTEM_FILE << "sample rate could not be interpreted \"" << s_sampleRate << "\"";
				if (d_tmp <= 0.0)
					ferr << E_SYSTEM_FILE << "sample rate must be positive \"" << s_sampleRate << "\"";
				if (d_tmp > TEN_TO_THE_FIFTEEN)
					ferr << E_UNREPRESENTABLE << "sample rate numerator too large \"" << s_sampleRate << "\"";
				sr.num = d_tmp;
				s_sampleRate = s_sampleRate.substr(slashPosition + 1);
				if (!(istringstream(s_sampleRate) >> d_tmp))
					ferr << E_SYSTEM_FILE << "sample rate could not be interpreted \"" << s_sampleRate << "\"";
				if (d_tmp <= 0.0)
					ferr << E_SYSTEM_FILE << "sample rate must be positive \"" << s_sampleRate << "\"";
				if (d_tmp > TEN_TO_THE_FIFTEEN)
					ferr << E_UNREPRESENTABLE << "sample rate denominator too large \"" << s_sampleRate << "\"";
				sr.den = d_tmp;
			}
			else
			{
				double d_tmp;
				if (!(istringstream(s_sampleRate) >> d_tmp))
					ferr << E_SYSTEM_FILE << "sample rate could not be interpreted \"" << s_sampleRate << "\"";
				if (d_tmp <= 0.0)
					ferr << E_SYSTEM_FILE << "sample rate must be positive \"" << s_sampleRate << "\"";
				if (d_tmp > TEN_TO_THE_FIFTEEN)
					ferr << E_UNREPRESENTABLE << "sample rate too large \"" << s_sampleRate << "\"";
				sr.num = d_tmp;
				sr.den = 1;
			}

			//	convert to lowest terms
			return toLowestTerms(sr);
		}


		string prettySet(const string& name)
		{
			if (name.length()) return name;
			else return "<default set>";
		}

		bool validSetName(const char* part)
		{
			const char* p = part;

			//	first char must be alpha
			if (*p && !isalpha(*p)) return false;

			while(*p)
			{
				if (!( isalpha(*p) || isdigit(*p) || isunderscore(*p) ))
					return false;

				p++;
			}

			return true;
		}

		bool validPortName(const char* part)
		{
			//	as validSetName, but cannot be empty
			if (!*part) return false;
			return validSetName(part);
		}

		string validateSetName(const char* part, const char* fullspec)
		{
			const char* p = part;

			//	first char must be alpha
			if (*p && !isalpha(*p))
				ferr << E_SYSTEM_FILE
					<< "invalid SystemML Identifier \"" << fullspec
					<< "\" (first character of each part must be alpha)";

			while(*p)
			{
				if (!( isalpha(*p) || isdigit(*p) || isunderscore(*p) ))
					ferr << E_SYSTEM_FILE
						<< "invalid SystemML Identifier \"" << fullspec
						<< "\" (illegal character \"" << *p << "\")";

				p++;
			}

			return part;
		}

		string validatePortName(const char* part, const char* fullspec)
		{
			//	as set, but cannot be empty string
			if (!*part)
				ferr << E_SYSTEM_FILE
					<< "invalid SystemML Identifier \"" << fullspec
					<< "\" (port name cannot be empty)";
			return validateSetName(part, fullspec);
		}

		string validateProcessName(const char* part, const char* fullspec)
		{
			//	as set, but cannot be empty string
			if (!*part)
				ferr << E_SYSTEM_FILE
					<< "invalid SystemML Identifier \"" << fullspec
					<< "\" (process name cannot be empty)";
			return validateSetName(part, fullspec);
		}


		Identifier::Identifier()
		{
			type = ST_NULL;
			hasPath = false;
		}

		Identifier::Identifier(const Identifier& src)
		{
			hasPath = src.hasPath;
			processName = src.processName;
			setName = src.setName;
			portName = src.portName;
			type = src.type;
		}

		Identifier& Identifier::operator=(const Identifier& src)
		{
			if (this == &src) return *this; // handle self-assignment gracefully

			hasPath = src.hasPath;
			processName = src.processName;
			setName = src.setName;
			portName = src.portName;
			type = src.type;

			return *this;
		}

		Identifier::~Identifier()
		{
		}

		void Identifier::parse(const char* c_fullspec, IdentifierType expectedType, const char* name)
		{
			//	clear any leftover state
			hasPath = false;
			processName = "";
			setName = "";
			portName = "";
			type = ST_NULL;

			//	get fullspec as string
			string fullspec = c_fullspec;

			//	remove any path parts
			while(true)
			{
				//	find /
				string::size_type f = fullspec.find('/');
				if (f == string::npos) break;

				//	break it off
				processName += validateProcessName(fullspec.substr(0, f).c_str(), c_fullspec) + "/";
				fullspec = fullspec.substr(f+1);

				//	mark
				hasPath = true;
			}

			//	if empty, return
			if (!fullspec.length())
				ferr << E_SYSTEM_FILE << "identifier \"" + string(name) + "\" was read as \"" + string(c_fullspec) + "\" and was invalid (nothing specified)";

			//	failsafe loop
			do
			{
				//	find any chevrons
				string::size_type fi = fullspec.find('<');
				string::size_type fo = fullspec.find('>');
				if (fi != string::npos && fo != string::npos)
					ferr << E_SYSTEM_FILE << "identifier \"" + string(name) + "\" was read as \"" + string(c_fullspec) + "\" and was invalid (both types of chevron)";

				//	if no chevrons, it's a process
				if (fi == string::npos && fo == string::npos)
				{
					processName += validateProcessName(fullspec.c_str(), c_fullspec);
					type = ST_PROCESS;
					break;
				}

				//	see whether it's an input or an output
				char chevron = '>';
				if (fi != string::npos) chevron = '<';
				string::size_type f = fullspec.find(chevron);

				//	get process name
				processName += validateProcessName(fullspec.substr(0, f).c_str(), c_fullspec);
				fullspec = fullspec.substr(f);

				//	now count chevrons
				int count = 0;
				while(fullspec.length() && fullspec[0] == chevron)
				{
					fullspec = fullspec.substr(1);
					count++;
				}

				//	can be 1, 2 or 3
				switch (count)
				{
					case 1:
					case 3:
					{
						//	expect port name
						portName = validatePortName(fullspec.c_str(), c_fullspec);
						if (chevron == '>') type = ST_OUTPUT_PORT;
						else type = ST_INPUT_PORT;
						break;
					}

					case 2:
					{
						//	expect set name
						f = fullspec.find(chevron);
						if (f == string::npos)
						{
							setName = validateSetName(fullspec.c_str(), c_fullspec);
							if (chevron == '>') type = ST_OUTPUT_SET;
							else type = ST_INPUT_SET;
							break;
						}

						//	else expect set name and port name
						setName = validateSetName(fullspec.substr(0,f).c_str(), c_fullspec);
						portName = validatePortName(fullspec.substr(f+1).c_str(), c_fullspec);
						if (chevron == '>') type = ST_OUTPUT_PORT;
						else type = ST_INPUT_PORT;
						break;
					}

					default:
						ferr << E_SYSTEM_FILE << "identifier \"" + string(name) + "\" was read as \"" + string(c_fullspec) + "\" and was invalid (too many chevrons)";
				}
			}
			while(false);

			//	check expected type
			if (expectedType != ST_NULL && expectedType != type)
				ferr << E_SYSTEM_FILE << "identifier \"" + string(name) + "\" was read as \"" + string(c_fullspec) + "\" and was invalid (wrong identifier type)";

			//	check not NULL
			if (type == ST_NULL) ferr << E_INTERNAL << "type == ST_NULL after Identifier parse";
		}

		Identifier::operator string() const
		{
			if (type == ST_NULL) return "<ST_NULL>";

			string ret = processName;
			if (type == ST_PROCESS) return ret;

			char chevron = (type == ST_OUTPUT_SET || type == ST_OUTPUT_PORT) ? '>' : '<';

			ret += chevron;
			ret += chevron;
			ret += setName;
			if (type == ST_INPUT_SET || type == ST_OUTPUT_SET) return ret;

			ret += chevron;
			ret += portName;
			return ret;
		}




	}
}


