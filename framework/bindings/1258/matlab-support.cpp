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

	$Id:: matlab-support.cpp 2428 2009-12-11 15:10:37Z benjmit#$
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

	string n2s(UINT32 n)
	{
		ostringstream s;
		s << n;
		return s.str();
	}

	string h2s(UINT32 n)
	{
		ostringstream s;
		s << hex;
		s << n;
		string ss = s.str();
		while(ss.length() < 8)
			ss = "0" + ss;
		return ss;
	}

	inline void clearAndNullArray(mxArray*& array)
	{
		if (array)
		{
			mxDestroyArray(array);
			array = NULL;
		}
	}

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
//	NEW FIELD SHORTCUTS
//
////////////////////////////////////////////////////////////////

	bool mxIsUintA(mxArray* array)
	{
#if ARCH_BITS == 64
		 return mxIsUint64(array);
#endif
#if ARCH_BITS == 32
		 return mxIsUint32(array);
#endif
	}
	
	bool mxIsSymbol(mxArray* array)
	{
		return mxIsUintA(array) && (mxGetNumberOfElements(array) == 1);
	}

	Symbol mxGetHandle(mxArray* array)
	{
		if (!mxIsSymbol(array)) return S_NULL;
		Symbol handle = *((UINTA*)mxGetData(array));
		if (S_NUMBER(handle) || S_ERROR(handle)) return S_NULL;
		return handle;
	}
	
	mxArray* addEmptyStruct(mxArray* array, const char* name)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateStructMatrix(1, 1, 0, 0);
		mxSetFieldByNumber(array, 0, field, mxField);
		return mxField;
	}

	mxArray* addNonScalarStruct(mxArray* array, const char* name, UINT32 M)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateStructMatrix(M, 1, 0, 0);
		mxSetFieldByNumber(array, 0, field, mxField);
		return mxField;
	}

	mxArray* addEmptyCell(mxArray* array, const char* name, UINT32 M = 0)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateCellMatrix(M, 1);
		mxSetFieldByNumber(array, 0, field, mxField);
		return mxField;
	}

	void addString(mxArray* array, const char* name, string value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateString(value.c_str());
		mxSetFieldByNumber(array, 0, field, mxField);
	}

	BOOL8* addScalarBOOL8(mxArray* array, const char* name, BOOL8 value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
		* ((BOOL8*) mxGetData(mxField)) = value;
		mxSetFieldByNumber(array, 0, field, mxField);
		return (BOOL8*) mxGetData(mxField);
	}

	UINTA* addScalarUINTA(mxArray* array, const char* name, UINTA value)
	{
		INT32 field = mxAddField(array, name);

#if ARCH_BITS == 32
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
		* ((UINT32*) mxGetData(mxField)) = value;
#endif
#if ARCH_BITS == 64
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
		* ((UINT64*) mxGetData(mxField)) = value;
#endif
		
		mxSetFieldByNumber(array, 0, field, mxField);
		return (UINTA*) mxGetData(mxField);
	}

	UINT32* addScalarUINT32(mxArray* array, const char* name, UINT32 value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
		* ((UINT32*) mxGetData(mxField)) = value;
		mxSetFieldByNumber(array, 0, field, mxField);
		return (UINT32*) mxGetData(mxField);
	}

	UINT64* addScalarUINT64(mxArray* array, const char* name, UINT32 value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
		* ((UINT64*) mxGetData(mxField)) = value;
		mxSetFieldByNumber(array, 0, field, mxField);
		return (UINT64*) mxGetData(mxField);
	}

	UINT16* addScalarUINT16(mxArray* array, const char* name, UINT16 value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxUINT16_CLASS, mxREAL);
		* ((UINT16*) mxGetData(mxField)) = value;
		mxSetFieldByNumber(array, 0, field, mxField);
		return (UINT16*) mxGetData(mxField);
	}

	DOUBLE* addScalarDOUBLE(mxArray* array, const char* name, DOUBLE value)
	{
		INT32 field = mxAddField(array, name);
		mxArray* mxField = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
		* ((DOUBLE*) mxGetData(mxField)) = value;
		mxSetFieldByNumber(array, 0, field, mxField);
		return (DOUBLE*) mxGetData(mxField);
	}





////////////////////////////////////////////////////////////////
//
//	TYPE CONVERSIONS
//
////////////////////////////////////////////////////////////////

	mxClassID dataType2clsID(TYPE type)
	{
		switch(type & TYPE_ELEMENT_MASK)
		{
		case TYPE_BOOL8: return mxLOGICAL_CLASS;
		case TYPE_CHAR16: return mxCHAR_CLASS;
		case TYPE_UINT8: return mxUINT8_CLASS;
		case TYPE_UINT16: return mxUINT16_CLASS;
		case TYPE_UINT32: return mxUINT32_CLASS;
		case TYPE_UINT64: return mxUINT64_CLASS;
		case TYPE_INT8: return mxINT8_CLASS;
		case TYPE_INT16: return mxINT16_CLASS;
		case TYPE_INT32: return mxINT32_CLASS;
		case TYPE_INT64: return mxINT64_CLASS;
		case TYPE_SINGLE: return mxSINGLE_CLASS;
		case TYPE_DOUBLE: return mxDOUBLE_CLASS;
		default:
			{
				berr << "data type is unsupported in matlab (" << hex << (type & TYPE_ELEMENT_MASK) << ")";
				return mxDOUBLE_CLASS;
			}
		}
	}

	mxClassID genericNumericType2clsID(TYPE type)
	{
		switch(type & TYPE_ELEMENT_MASK)
		{
		case TYPE_BOOL8: return mxLOGICAL_CLASS;
		case TYPE_BOOL16: return mxUINT16_CLASS;
		case TYPE_BOOL32: return mxUINT32_CLASS;
		case TYPE_BOOL64: return mxUINT64_CLASS;
		case TYPE_CHAR8: return mxUINT8_CLASS;
		case TYPE_CHAR16: return mxUINT16_CLASS;
		case TYPE_CHAR32: return mxUINT32_CLASS;
		case TYPE_CHAR64: return mxUINT64_CLASS;
		case TYPE_UINT8: return mxUINT8_CLASS;
		case TYPE_UINT16: return mxUINT16_CLASS;
		case TYPE_UINT32: return mxUINT32_CLASS;
		case TYPE_UINT64: return mxUINT64_CLASS;
		case TYPE_INT8: return mxINT8_CLASS;
		case TYPE_INT16: return mxINT16_CLASS;
		case TYPE_INT32: return mxINT32_CLASS;
		case TYPE_INT64: return mxINT64_CLASS;
		case TYPE_SINGLE: return mxSINGLE_CLASS;
		case TYPE_DOUBLE: return mxDOUBLE_CLASS;
		default:
			{
				berr << "data type is unsupported in matlab (" + n2s(type) + ")";
				return mxDOUBLE_CLASS;
			}
		}
	}

	UINT32 clsID2dataType(mxClassID clsid)
	{
		switch(clsid)
		{
		case mxLOGICAL_CLASS: return TYPE_BOOL8;
		case mxUINT8_CLASS: return TYPE_UINT8;
		case mxUINT16_CLASS: return TYPE_UINT16;
		case mxUINT32_CLASS: return TYPE_UINT32;
		case mxUINT64_CLASS: return TYPE_UINT64;
		case mxINT8_CLASS: return TYPE_INT8;
		case mxINT16_CLASS: return TYPE_INT16;
		case mxINT32_CLASS: return TYPE_INT32;
		case mxINT64_CLASS: return TYPE_INT64;
		case mxSINGLE_CLASS: return TYPE_SINGLE;
		case mxDOUBLE_CLASS: return TYPE_DOUBLE;
		default:
			{
				berr << "internal error - uncoded case (" + n2s(clsid) + ")";
				return 0;
			}
		}
	}

	char type2string_buffer[256];

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
				std::ostringstream ss;
				ss << "[TYPE:0x" << hex << (type & TYPE_ELEMENT_MASK) << "]";
				memcpy(type2string_buffer, ss.str().c_str(), ss.str().length());
				return type2string_buffer;
		}
	}



////////////////////////////////////////////////////////////////
//
//	MX EXTRACTIONS
//
////////////////////////////////////////////////////////////////

	string COMPONENT_CLASS_CPP::mxToString(mxArray* arr, const char* name)
	{
		if (!arr) return "";
		char* c = mxArrayToString(arr);
		if (!c) berr << "\"" << name << "\" should have been a string";
		string ret = c;
		mxFree(c);
		return ret;
	}

	UINT32 COMPONENT_CLASS_CPP::mxToUINT32(mxArray* arr, const char* name, bool acceptDouble)
	{
		if (mxGetNumberOfElements(arr) == 1 && !mxIsComplex(arr))
		{
			if (mxIsUint32(arr))
				return *(UINT32*)mxGetData(arr);
			if (acceptDouble && mxIsDouble(arr))
			{
				double dval = *mxGetPr(arr);
				UINT32 uval = (UINT32) dval;
				if (dval == uval) return uval;
			}
		}
		berr << "\"" << name << "\" should have been a real scalar UINT32";
		return 0;
	}

	UINT16 COMPONENT_CLASS_CPP::mxToUINT16(mxArray* arr, const char* name, bool acceptDouble)
	{
		if (mxGetNumberOfElements(arr) == 1 && !mxIsComplex(arr))
		{
			if (mxIsUint16(arr))
				return *(UINT16*)mxGetData(arr);
			if (acceptDouble && mxIsDouble(arr))
			{
				double dval = *mxGetPr(arr);
				UINT16 uval = (UINT16) dval;
				if (dval == uval) return uval;
			}
		}
		berr << "\"" << name << "\" should have been a real scalar UINT16";
		return 0;
	}

	Dims mxGetDims(mxArray* arr)
	{
		UINT32 nDims = mxGetNumberOfDimensions(arr);
		const MX_DIM* pDims = mxGetDimensions(arr);
		Dims dims;
		for (UINT32 d=0; d<nDims; d++)
			dims.push_back(pDims[d]);
		return dims;
	}

	TYPE mxGetType(mxArray* arr)
	{
		TYPE type = clsID2dataType(mxGetClassID(arr));
		if (mxIsComplex(arr)) type |= TYPE_COMPLEX;
		else type |= TYPE_REAL;
		return type;
	}





