/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitch(inson)
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

	$Id:: python-support.cpp 2428 2009-12-11 15:10:37Z benjmit#$
	$Rev:: 2428                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)       $
________________________________________________________________

*/



////////////////////////////////////////////////////////////////
//
//	TEXT SUPPORT
//
////////////////////////////////////////////////////////////////	

	#ifdef __NIX__
	#define OS_PATH_SEPARATOR "/"
	#else
	#define OS_PATH_SEPARATOR "\\"
	#endif

	void grep(string& str, const string& what, const string& with)
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





////////////////////////////////////////////////////////////////
//
//	TEXT FUNCTIONS
//
////////////////////////////////////////////////////////////////

	bool validateID(const char* pid)
	{
		//	ids returned by the wrapped function must be valid matlab field names...
		const char* id = pid;
		if (isalpha(*id))
		{
			id++;
			while(*id)
			{
				if (!( isalnum(*id) || (*id == '_')))
					break;
				id++;
			}
			if (!*id) return true;
		}
		return false;
	}

	string stringToSampleRate(string s_sampleRate, SampleRate& sr)
	{
		//	sampleRate must either be an integer (all digits, and not longer than two_to_the_52) or
		//	a fraction of the form "N/D" with both N and D compliant with the above.
		int L = s_sampleRate.length();
		const char* s = s_sampleRate.c_str();
		int slashPosition = -1;
		for (int c=0; c<L; c++)
		{
			if (s[c] == '/')
			{
				if (slashPosition != -1) return "too many / characters in sample rate";
				slashPosition = c;
			}
			else if(s[c] >= 48 && s[c] <= 57)
			{
				//	ok, it's a digit
			}
			else if (s[c] == '.')
			{
				return "dot is an invalid character in sample rate: specify a fractional rate as a fraction, e.g. 513/100 for 51.3Hz";
			}
			else return string("invalid character in sample rate: ") + s[c];
		}
		if (slashPosition == 0 || slashPosition == (L-1)) return "/ character cannot begin or end sample rate string";
		
		//	overflow err msg
		const char* E_SAMPLE_RATE_OVERFLOW = "E_SAMPLE_RATE_OVERFLOW: a requested sample rate is too high";

		//	represent as fraction
		if (slashPosition != -1)
		{
			double d_tmp;
			if (!(istringstream(s_sampleRate) >> d_tmp))
				return "could not interpret sample rate of \"" + s_sampleRate + "\"";
			if (d_tmp <= 0.0) return "sample rate must be positive";
			if (d_tmp > TEN_TO_THE_FIFTEEN) return E_SAMPLE_RATE_OVERFLOW;
			sr.num = d_tmp;
			s_sampleRate = s_sampleRate.substr(slashPosition + 1);
			if (!(istringstream(s_sampleRate) >> d_tmp))
				return "could not interpret sample rate of \"" + s_sampleRate + "\"";
			if (d_tmp <= 0.0) return "sample rate must be positive";
			if (d_tmp > TEN_TO_THE_FIFTEEN) return E_SAMPLE_RATE_OVERFLOW;
			sr.den = d_tmp;
		}
		else
		{
			double d_tmp;
			if (!(istringstream(s_sampleRate) >> d_tmp))
				return "could not interpret sample rate of \"" + s_sampleRate + "\"";
			if (d_tmp <= 0.0) return "sample rate must be positive";
			if (d_tmp > TEN_TO_THE_FIFTEEN) return E_SAMPLE_RATE_OVERFLOW;
			sr.num = d_tmp;
			sr.den = 1;
		}

		//	ok
		return "";
	}


////////////////////////////////////////////////////////////////
//
//	PY EXTRACTIONS
//
////////////////////////////////////////////////////////////////

	bool pyToString(PyObject* arr, string& ret)
	{
		char* c = PyString_AsString(arr);
		if (!c) return false;
		ret = c;
		return true;
	}

	bool pyToUINT64(PyObject* arr, UINT64& uval)
	{
		if (!PyInt_Check(arr)) return false;
		uval = PyInt_AsUnsignedLongMask(arr);
		return true;
	}

	bool pyToUINT32(PyObject* arr, UINT32& uval)
	{
		if (!PyInt_Check(arr)) return false;
		uval = PyInt_AsUnsignedLongMask(arr);
		return true;
	}

	bool pyToUINT16(PyObject* arr, UINT16& uval)
	{
		if (PyInt_Check(arr))
		{
			unsigned long ival = PyInt_AsUnsignedLongMask(arr);
			uval = ival;
			if (ival == uval) return true;
		}
		return false;
	}




////////////////////////////////////////////////////////////////
//
//	NEW FIELD SHORTCUTS
//
////////////////////////////////////////////////////////////////

	PyObject* addItem(PyObject* p, const char* key, PyObject* val)
	{
		bool success = false;
		if (val)
		{
			success = !PyDict_SetItemString(p, key, val);
			Py_DECREF(val);
		}
		return success ? val : NULL;
	}


////////////////////////////////////////////////////////////////
//
//	ARRAY FUNCTIONS
//
////////////////////////////////////////////////////////////////

	void interleaveComplexData(BYTE* data, const BYTE* real, const BYTE* imag, UINT64 numberOfElements, UINT32 bytesPerElement)
	{
		while (numberOfElements--)
		{
			memcpy(data, real, bytesPerElement);
			data += bytesPerElement;
			real += bytesPerElement;
			memcpy(data, imag, bytesPerElement);
			data += bytesPerElement;
			imag += bytesPerElement;
		}
	}

	void deinterleaveComplexData(BYTE* real, BYTE* imag, const BYTE* data, UINT64 numberOfElements, UINT32 bytesPerElement)
	{
		while (numberOfElements--)
		{
			memcpy(real, data, bytesPerElement);
			real += bytesPerElement;
			data += bytesPerElement;
			memcpy(imag, data, bytesPerElement);
			imag += bytesPerElement;
			data += bytesPerElement;
		}
	}




////////////////////////////////////////////////////////////////
//
//	TYPE CONVERSIONS
//
//	We support ten array types: complex32, complex64, float32,
//	float64, int16, int32, int8, uint16, uint32, uint8
//
////////////////////////////////////////////////////////////////

	NPY_TYPES COMPONENT_CLASS_CPP::dataType2arrayTypes(TYPE type)
	{
		switch(type & (TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK))
		{
		case TYPE_UINT8 | TYPE_REAL: return NPY_UBYTE;
		case TYPE_UINT16 | TYPE_REAL: return NPY_USHORT;
		case TYPE_UINT32 | TYPE_REAL: return NPY_UINT;
		case TYPE_INT8 | TYPE_REAL: return NPY_BYTE;
		case TYPE_INT16 | TYPE_REAL: return NPY_SHORT;
		case TYPE_INT32 | TYPE_REAL: return NPY_INT;
		case TYPE_SINGLE | TYPE_REAL: return NPY_FLOAT;
		case TYPE_DOUBLE | TYPE_REAL: return NPY_DOUBLE;
		case TYPE_SINGLE | TYPE_COMPLEX: return NPY_CFLOAT;
		case TYPE_DOUBLE | TYPE_COMPLEX: return NPY_CDOUBLE;
		case TYPE_BOOL8 | TYPE_REAL: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL8\" (boolean) is unsupported in python and cannot be translated"; }
		case TYPE_BOOL8 | TYPE_COMPLEX: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL8\" (boolean) is unsupported in python and cannot be translated"; }
		case TYPE_BOOL16 | TYPE_REAL: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL16\" is unsupported in python and cannot be translated"; }
		case TYPE_BOOL16 | TYPE_COMPLEX: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL16\" is unsupported in python and cannot be translated"; }
		case TYPE_BOOL32 | TYPE_REAL: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL32\" is unsupported in python and cannot be translated"; }
		case TYPE_BOOL32 | TYPE_COMPLEX: { berr << E_DATA_TYPE_UNSUPPORTED << "data type \"BOOL32\" is unsupported in python and cannot be translated"; }
		default:
			{
				berr << E_DATA_TYPE_UNSUPPORTED << "data type \"0x" << hex << type << "\" is unsupported in python and cannot be translated";
				return NPY_CDOUBLE;
			}
		}
	}

	NPY_TYPES COMPONENT_CLASS_CPP::genericNumericType2arrayTypes(TYPE type)
	{
		switch(type & (TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK))
		{
		case TYPE_BOOL8 | TYPE_REAL: return NPY_UBYTE;
		case TYPE_BOOL16 | TYPE_REAL: return NPY_USHORT;
		case TYPE_BOOL32 | TYPE_REAL: return NPY_UINT;
		case TYPE_CHAR8 | TYPE_REAL: return NPY_UBYTE;
		case TYPE_CHAR16 | TYPE_REAL: return NPY_USHORT;
		case TYPE_CHAR32 | TYPE_REAL: return NPY_UINT;
		case TYPE_UINT8 | TYPE_REAL: return NPY_UBYTE;
		case TYPE_UINT16 | TYPE_REAL: return NPY_USHORT;
		case TYPE_UINT32 | TYPE_REAL: return NPY_UINT;
		case TYPE_INT8 | TYPE_REAL: return NPY_BYTE;
		case TYPE_INT16 | TYPE_REAL: return NPY_SHORT;
		case TYPE_INT32 | TYPE_REAL: return NPY_INT;
		case TYPE_SINGLE | TYPE_REAL: return NPY_FLOAT;
		case TYPE_DOUBLE | TYPE_REAL: return NPY_DOUBLE;
		case TYPE_SINGLE | TYPE_COMPLEX: return NPY_CFLOAT;
		case TYPE_DOUBLE | TYPE_COMPLEX: return NPY_CDOUBLE;
		default:
			{
				berr << E_DATA_TYPE_UNSUPPORTED << "data type is unsupported in python (" << type << ")";
				return NPY_CDOUBLE;
			}	
		}
	}

	TYPE COMPONENT_CLASS_CPP::arrayTypes2dataType(PyArrayObject* obj)
	{
		NPY_TYPES arrayType = (NPY_TYPES)obj->descr->type_num;
		UINT32 elsize = obj->descr->elsize;

		switch(arrayType)
		{
			case NPY_UBYTE:
			case NPY_USHORT:
			case NPY_ULONG:
			case NPY_ULONGLONG:
			case NPY_UINT:
				switch(elsize)
				{
					case 1: return TYPE_UINT8 | TYPE_REAL;
					case 2: return TYPE_UINT16 | TYPE_REAL;
					case 4: return TYPE_UINT32 | TYPE_REAL;
					case 8: return TYPE_UINT64 | TYPE_REAL;
				}

			case NPY_BYTE:
			case NPY_SHORT:
			case NPY_LONG:
			case NPY_LONGLONG:
			case NPY_INT:
				switch(elsize)
				{
					case 1: return TYPE_INT8 | TYPE_REAL;
					case 2: return TYPE_INT16 | TYPE_REAL;
					case 4: return TYPE_INT32 | TYPE_REAL;
					case 8: return TYPE_INT64 | TYPE_REAL;
				}

			
			case NPY_FLOAT: return TYPE_SINGLE | TYPE_REAL;
			case NPY_DOUBLE: return TYPE_DOUBLE | TYPE_REAL;
			case NPY_CFLOAT: return TYPE_SINGLE | TYPE_COMPLEX;
			case NPY_CDOUBLE: return TYPE_DOUBLE | TYPE_COMPLEX;

			default:
			{
				berr << E_INTERNAL << "uncoded case (" << arrayType << ")";
				return TYPE_UNSPECIFIED;
			}
		}
	}

