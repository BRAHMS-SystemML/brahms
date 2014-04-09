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

	$Id:: identifier.h 2098 2009-09-13 12:43:07Z benjmitch     $
	$Rev:: 2098                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-09-13 13:43:07 +0100 (Sun, 13 Sep 2009)       $
________________________________________________________________

*/





namespace brahms
{
	namespace systemml
	{



	////////////////	SAMPLE RATE SUPPORT FUNCTIONS

		bool validSampleRate(SampleRate sampleRate);
		SampleRate toLowestTerms(SampleRate sampleRate);
		SampleRate parseSampleRate(string s_sampleRate);
		



	////////////////	IDENTIFIER SUPPORT FUNCTIONS

		string prettySet(const string& name);
		bool validSetName(const char* part);
		bool validPortName(const char* part);
		string validateSetName(const char* part, const char* fullspec);
		string validatePortName(const char* part, const char* fullspec);
		string validateProcessName(const char* part, const char* fullspec);



	////////////////	IDENTIFIER

		enum IdentifierType
		{
			ST_NULL = 0,
			ST_PROCESS,
			ST_INPUT_SET,
			ST_OUTPUT_SET,
			ST_INPUT_PORT,
			ST_OUTPUT_PORT
		};

		class Identifier
		{

		public:

			Identifier();
			Identifier(const Identifier& src);
			Identifier& operator=(const Identifier& src);
			~Identifier();

			void parse(const char* fullspec, IdentifierType expectedType, const char* name);
			operator string() const;

			bool hasPath;					//	true if identifier had path component asd/asd/...
			string processName;				//	process name
			string setName;					//	set name
			string portName;				//	port name

			IdentifierType type;			//	identifier type

		};






	}
}

