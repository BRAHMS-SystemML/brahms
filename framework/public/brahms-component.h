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

	$Id:: brahms-component.h 2439 2009-12-13 19:59:42Z benjmitch    $
	$Rev:: 2439                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-12-13 19:59:42 +0000 (Sun, 13 Dec 2009)       $
________________________________________________________________

*/




#ifndef INCLUDED_BRAHMS_COMPONENT_INTERFACE
#define INCLUDED_BRAHMS_COMPONENT_INTERFACE



////////////////////////////////////////////////////////////////////////////////
//
//	"brahms-component.h" defines the Component Interface, which is the
//	interface to the Engine used by Components. Thus, it is included in builds
//	of Components, Language Bindings (which are implemented as Components), and
//	the Engine itself.
//
//	Note that this file is parsed, rather than copied, to "brahms-component.h"
//	in the include folder, with version data and constant syntax set on the way.
//
////////////////////////////////////////////////////////////////////////////////



////////////////    PRE-AMBLE

	//	engine version
	__UINT16 VERSION_ENGINE_MAJ __TOK_ __MAJ__ _TOK__
	__UINT16 VERSION_ENGINE_MIN __TOK_ __MIN__ _TOK__
	__UINT16 VERSION_ENGINE_REL __TOK_ __REL__ _TOK__
	__UINT16 VERSION_ENGINE_REV __TOK_ __REV__ _TOK__

	//	GCC section
	#ifdef __GNUC__

		//	version
		#if __GNUC__ < 4
		#error Compiler version too old (__GNUC__ < 4)
		#endif

		//	bit-width (pre-define in makefile if this does not work for your platform)
		#ifndef ARCH_BITS
		#ifdef __i386__
		#define ARCH_BITS 32
		#endif
		#ifdef __x86_64
		#define ARCH_BITS 64
		#endif
		#endif

		//	visibility
		#define BRAHMS_DLL_EXPORT_CPP __attribute__ ((visibility("default")))
		#define BRAHMS_DLL_IMPORT_CPP __attribute__ ((visibility("default")))
		#define BRAHMS_DLL_EXPORT extern "C" __attribute__ ((visibility("default")))
		#define BRAHMS_DLL_IMPORT extern "C" __attribute__ ((visibility("default")))

	#endif

	//	MSC section
	#ifdef _MSC_VER

		//	version
		#if _MSC_VER < 1300
		#error Compiler version too old (_MSC_VER < 1300)
		#endif
		
		//	bit-width (pre-define in makefile if this does not work for your platform)
		#ifndef ARCH_BITS
		#ifdef _M_IX86
		#define ARCH_BITS 32
		#endif
		#ifdef _M_X64
		#define ARCH_BITS 64
		#endif
		#endif

		//	visibility
		#define BRAHMS_DLL_EXPORT_CPP __declspec(dllexport)
		#define BRAHMS_DLL_IMPORT_CPP __declspec(dllimport)
		#define BRAHMS_DLL_EXPORT extern "C" __declspec(dllexport)
		#define BRAHMS_DLL_IMPORT extern "C" __declspec(dllimport)

	#endif

	//	check success
	#ifndef ARCH_BITS
	#error Compiler or architecture unrecognised - on OSX, use flag "--arch" - or, add a block for your compiler to the top of brahms-component.h
	#endif

	//	define __NIX__ for those platforms
	#ifdef __GLN__
	#define __NIX__
	#endif
	#ifdef __OSX__
	#define __NIX__
	#endif

	//	check success
	#ifndef __NIX__
	#ifndef __WIN__
	#error Platform unrecognised - define __WIN__, __GLN__ or __OSX__
	#endif
	#endif

	//  BRAHMS_BUILD_TARGET must be defined
	#ifndef BRAHMS_BUILD_TARGET
	#error Cannot include the Component Interface directly (include an overlay/binding, instead)
	#endif
	
	//  BRAHMS_ENGINE_VIS is import, unless otherwise specified
	#ifdef __cplusplus
	#ifdef BRAHMS_BUILDING_ENGINE
	#define BRAHMS_ENGINE_VIS BRAHMS_DLL_EXPORT
	#define BRAHMS_ENGINE_VIS_CPP BRAHMS_DLL_EXPORT_CPP
	#endif
	#ifndef BRAHMS_ENGINE_VIS
	#define BRAHMS_ENGINE_VIS BRAHMS_DLL_IMPORT
	#define BRAHMS_ENGINE_VIS_CPP BRAHMS_DLL_IMPORT_CPP
	#endif
	#else
	#define BRAHMS_ENGINE_VIS
	#define BRAHMS_ENGINE_VIS_CPP
	#endif



////////////////    ENVIRONMENT DETECTION

	//	Microsoft Visual C++ defines __int* in all cases

	//	GNU C does not (the __int* here are taken from "GNU C, ISO C99: 7.18 Integer types <stdint.h>")
	#ifdef __GNUC__
	#define __int8 char
	#define __int16 short int
	#define __int32 int
	#if ARCH_BITS == 64
	#define __int64 long int
	#else
	#define __int64 long long int
	#endif
	#endif



////////////////    STANDARD NUMERIC TYPES

	//	if using some other compiler, it will probably fall over right about now...
	//	if so, please nudge us to add support for your compiler
	typedef float                   FLOAT32;
	typedef double                  FLOAT64;
	typedef signed __int8           INT8;
	typedef signed __int16          INT16;
	typedef signed __int32          INT32;
	typedef signed __int64          INT64;
	typedef unsigned __int8         UINT8;
	typedef unsigned __int16        UINT16;
	typedef unsigned __int32        UINT32;
	typedef unsigned __int64        UINT64;
	typedef unsigned __int8         BOOL8;
	typedef unsigned __int16        BOOL16;
	typedef unsigned __int32        BOOL32;
	typedef unsigned __int64        BOOL64;
	typedef unsigned __int8         CHAR8;
	typedef unsigned __int16        CHAR16;
	typedef unsigned __int32        CHAR32;
	typedef unsigned __int64        CHAR64;

	//	synonyms
	typedef float					SINGLE;
	typedef double					DOUBLE;



////////////////    START NAMESPACE / EXTERN C

#ifdef __cplusplus
namespace brahms { extern "C" {
#endif



////////////////    NUMERIC TYPE CONSTANTS

	typedef UINT32 TYPE;

	__TYPE TYPE_UNSPECIFIED         __TOK_ 0x00000000 _TOK__

	//  container width
	__TYPE TYPE_WIDTH_UNSPECIFIED   __TOK_ 0x00000000 _TOK__
	__TYPE TYPE_WIDTH_8BIT          __TOK_ 0x00000001 _TOK__
	__TYPE TYPE_WIDTH_16BIT         __TOK_ 0x00000002 _TOK__
	__TYPE TYPE_WIDTH_32BIT         __TOK_ 0x00000003 _TOK__
	__TYPE TYPE_WIDTH_64BIT         __TOK_ 0x00000004 _TOK__
	__TYPE TYPE_WIDTH_MASK          __TOK_ 0x0000000F _TOK__

	//  storage format
	__TYPE TYPE_FORMAT_UNSPECIFIED  __TOK_ 0x00000000 _TOK__
	__TYPE TYPE_FORMAT_FLOAT        __TOK_ 0x00000010 _TOK__
	__TYPE TYPE_FORMAT_FIXED        __TOK_ 0x00000020 _TOK__
	__TYPE TYPE_FORMAT_INT          __TOK_ 0x00000030 _TOK__
	__TYPE TYPE_FORMAT_UINT         __TOK_ 0x00000040 _TOK__
	__TYPE TYPE_FORMAT_BOOL         __TOK_ 0x00000050 _TOK__
	__TYPE TYPE_FORMAT_CHAR         __TOK_ 0x00000060 _TOK__
	__TYPE TYPE_FORMAT_STRUCT       __TOK_ 0x000000A0 _TOK__
	__TYPE TYPE_FORMAT_CELL         __TOK_ 0x000000B0 _TOK__
	__TYPE TYPE_FORMAT_MASK         __TOK_ 0x000000F0 _TOK__

	//  these bits describe the "element"
	__TYPE TYPE_ELEMENT_MASK        __TOK_ TYPE_WIDTH_MASK | TYPE_FORMAT_MASK _TOK__

	//  complexity flags
	__TYPE TYPE_COMPLEX_UNSPECIFIED __TOK_ 0x00000000 _TOK__
	__TYPE TYPE_REAL                __TOK_ 0x00000100 _TOK__
	__TYPE TYPE_COMPLEX             __TOK_ 0x00000200 _TOK__
	__TYPE TYPE_COMPLEX_MASK        __TOK_ 0x00000300 _TOK__

	//	complexity storage format
	__TYPE TYPE_CPXFMT_UNSPECIFIED __TOK_ 0x00000000 _TOK__
	__TYPE TYPE_CPXFMT_ADJACENT    __TOK_ 0x00000400 _TOK__
	__TYPE TYPE_CPXFMT_INTERLEAVED __TOK_ 0x00000800 _TOK__
	__TYPE TYPE_CPXFMT_MASK        __TOK_ 0x00000C00 _TOK__
	
	//	ordering flags
	__TYPE TYPE_ORDER_UNSPECIFIED  __TOK_ 0x00000000 _TOK__
	__TYPE TYPE_ORDER_COLUMN_MAJOR __TOK_ 0x00001000 _TOK__
	__TYPE TYPE_ORDER_ROW_MAJOR    __TOK_ 0x00002000 _TOK__
	__TYPE TYPE_ORDER_MASK         __TOK_ 0x00003000 _TOK__

	//	these bits describe the "array"
	__TYPE TYPE_ARRAY_MASK          __TOK_ TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK | TYPE_CPXFMT_MASK | TYPE_ORDER_MASK _TOK__

	//  derived constants are the ones you would actually use (some do not specify bit-width, for obvious reasons)
	__TYPE TYPE_FLOAT32             __TOK_ TYPE_FORMAT_FLOAT | TYPE_WIDTH_32BIT _TOK__
	__TYPE TYPE_FLOAT64             __TOK_ TYPE_FORMAT_FLOAT | TYPE_WIDTH_64BIT _TOK__
	__TYPE TYPE_INT8                __TOK_ TYPE_FORMAT_INT   | TYPE_WIDTH_8BIT _TOK__
	__TYPE TYPE_INT16               __TOK_ TYPE_FORMAT_INT   | TYPE_WIDTH_16BIT _TOK__
	__TYPE TYPE_INT32               __TOK_ TYPE_FORMAT_INT   | TYPE_WIDTH_32BIT _TOK__
	__TYPE TYPE_INT64               __TOK_ TYPE_FORMAT_INT   | TYPE_WIDTH_64BIT _TOK__
	__TYPE TYPE_UINT8               __TOK_ TYPE_FORMAT_UINT  | TYPE_WIDTH_8BIT _TOK__
	__TYPE TYPE_UINT16              __TOK_ TYPE_FORMAT_UINT  | TYPE_WIDTH_16BIT _TOK__
	__TYPE TYPE_UINT32              __TOK_ TYPE_FORMAT_UINT  | TYPE_WIDTH_32BIT _TOK__
	__TYPE TYPE_UINT64              __TOK_ TYPE_FORMAT_UINT  | TYPE_WIDTH_64BIT _TOK__
	__TYPE TYPE_BOOL8               __TOK_ TYPE_FORMAT_BOOL  | TYPE_WIDTH_8BIT _TOK__
	__TYPE TYPE_BOOL16              __TOK_ TYPE_FORMAT_BOOL  | TYPE_WIDTH_16BIT _TOK__
	__TYPE TYPE_BOOL32              __TOK_ TYPE_FORMAT_BOOL  | TYPE_WIDTH_32BIT _TOK__
	__TYPE TYPE_BOOL64              __TOK_ TYPE_FORMAT_BOOL  | TYPE_WIDTH_64BIT _TOK__
	__TYPE TYPE_CHAR8               __TOK_ TYPE_FORMAT_CHAR  | TYPE_WIDTH_8BIT _TOK__
	__TYPE TYPE_CHAR16              __TOK_ TYPE_FORMAT_CHAR  | TYPE_WIDTH_16BIT _TOK__
	__TYPE TYPE_CHAR32              __TOK_ TYPE_FORMAT_CHAR  | TYPE_WIDTH_32BIT _TOK__
	__TYPE TYPE_CHAR64              __TOK_ TYPE_FORMAT_CHAR  | TYPE_WIDTH_64BIT _TOK__
	__TYPE TYPE_STRUCT              __TOK_ TYPE_FORMAT_STRUCT _TOK__
	__TYPE TYPE_CELL                __TOK_ TYPE_FORMAT_CELL _TOK__

	//	synonyms
	__TYPE TYPE_SINGLE              __TOK_ TYPE_FLOAT32 _TOK__
	__TYPE TYPE_DOUBLE              __TOK_ TYPE_FLOAT64 _TOK__
	__TYPE TYPE_BYTE                __TOK_ TYPE_UINT8 _TOK__
	__TYPE TYPE_BOOL                __TOK_ TYPE_BOOL8 _TOK__



////////////////    ARCHITECTURE DEPENDENT

	#if ARCH_BITS == 64
	__UINT32 ARCH_BYTES             __TOK_ 8 _TOK__
	__TYPE TYPE_INTA                __TOK_ TYPE_INT64 _TOK__
	__TYPE TYPE_UINTA               __TOK_ TYPE_UINT64 _TOK__
	typedef INT64                  INTA;
	typedef UINT64                 UINTA;
	#else
	__UINT32 ARCH_BYTES             __TOK_ 4 _TOK__
	__TYPE TYPE_INTA                __TOK_ TYPE_INT32 _TOK__
	__TYPE TYPE_UINTA               __TOK_ TYPE_UINT32 _TOK__
	typedef INT32                  INTA;
	typedef UINT32                 UINTA;
	#endif



////////////////    TYPE MACROS

	#define TYPE_BYTES(type) (1 << ((type & TYPE_WIDTH_MASK) - 1))
	#define TYPE_COMPLEX_MULT(type) ((type & TYPE_COMPLEX) ? 2 : 1)



////////////////    TIMING

	typedef UINT64 BaseSamples;

	struct SampleRate
	{
		UINT64 num;
		UINT64 den;
	};



////////////////    DIMENSIONS

	struct Dimensions
	{
		const INT64* dims;
		UINT32 count;
	};

	__INT64 DIM_ANY        __TOK_ -1 _TOK__
	__INT64 DIM_NONZERO    __TOK_ -2 _TOK__
	__INT64 DIM_ELLIPSIS   __TOK_ -3 _TOK__
	__INT64 DIM_ABSENT     __TOK_ -4 _TOK__



////////////////    VERSION

	struct FrameworkVersion
	{
		UINT16 major;          //  interface broken
		UINT16 minor;          //  interface extended
		UINT16 release;        //  interface unchanged (bug fixes, optimizations)
		UINT16 revision;       //  private developer revision
	};

	struct ComponentVersion
	{
		UINT16 release;        //  interface unchanged (bug fixes, optimizations)
		UINT16 revision;       //  private developer revision
	};



////////////////    DETAIL LEVEL

	typedef UINT32 DetailLevel;

	__UINT32 D_NONE __TOK_ 0x00 _TOK__ //  normal levels
	__UINT32 D_WARN __TOK_ 0x20 _TOK__
	__UINT32 D_INFO __TOK_ 0x40 _TOK__
	__UINT32 D_VERB __TOK_ 0x60 _TOK__



////////////////    COMPONENT INFO

	typedef UINT32 ComponentType;

	//	these must be OR-able for component register, so distinct bits for each
	__UINT32 CT_NULL      __TOK_ 0 _TOK__
	__UINT32 CT_DATA      __TOK_ 1 _TOK__
	__UINT32 CT_PROCESS   __TOK_ 2 _TOK__
	__UINT32 CT_UTILITY   __TOK_ 4 _TOK__

	struct ComponentTime
	{
		struct SampleRate baseSampleRate;       //  (PARAMETER) base sample rate (Hz)
		BaseSamples executionStop;              //  (PARAMETER) time of execution soft stop (base samples)
		struct SampleRate sampleRate;           //  (PARAMETER) sample rate of component (Hz)
		BaseSamples samplePeriod;               //  (PARAMETER) sample period of component (base samples)
		BaseSamples now;                        //  (VARIABLE) time now (base samples)
	};

	struct ComponentInfo
	{
		const char* cls;                                      //  SystemML class name of component
		ComponentType type;                                   //  component type
		const struct ComponentVersion* componentVersion;      //  component version
		UINT32 flags;                                         //  see flags below
		const char* additional;                               //  additional info, in the form "key=val\nkey=val\n..."
		const char* libraries;                                //  external libraries linked, in the form "name=version\nname=version\n..."
	};

	__UINT32 F_NEEDS_ALL_INPUTS          __TOK_ 0x00000001 _TOK__	//	component must only receive EVENT_INIT_CONNECT when all its inputs are present
	__UINT32 F_INPUTS_SAME_RATE          __TOK_ 0x00000002 _TOK__	//	component must only receive inputs that share its sample rate
	__UINT32 F_OUTPUTS_SAME_RATE         __TOK_ 0x00000004 _TOK__	//	component must only create outputs that share its sample rate
	__UINT32 F_NOT_RATE_CHANGER          __TOK_ F_INPUTS_SAME_RATE | F_OUTPUTS_SAME_RATE _TOK__
	__UINT32 M_COMPONENT_FLAGS           __TOK_ 0x00000007 _TOK__

	struct ComponentData
	{
		const char* name;
		struct ComponentTime* time;
	};



////////////////    SYMBOLS

	/*
		Symbol is the same width as the architecture (ARCH_BITS); it is a UINTA.

		The most significant nibble of a Symbol is its TYPE (tttt):

			[1111] NEGATIVE NUMBER
			[0000] NUMBER
			[0001] CONSTANT

			...    values reserved; currently used as object handles

			[1110] ERROR

		If the Symbol represents a NEGATIVE NUMBER or NUMBER, the whole thing can
		be interpreted as an INTA number. The highest absolute value that can be
		represented is therefore limited to 28/60 bits, with a range of -0x10000000
		to 0x0FFFFFFF on 32-bit.

		If the Symbol represents a CONSTANT, the remaining bits are a simple code
		indicating some constant object (i.e. the Symbol as a whole is a constant).
		All such constants are organised as follows.

			[0001] ... <reserved> ... [cccc] [vvvv vvvv] [vvvv vvvv]

			c: constant class (not meaningful, just used for organisation)
			v: value

		If the Symbol represents an object HANDLE (SET, PORT, COMPONENT, XML NODE),
		it can be used to reference one of these objects in interface functions. The Symbol
		is organised according to a scheme maintained privately by the framework, and
		cannot be interpreted. It can be passed to a framework function to obtain
		information about what it represents.

		If the Symbol represents an ERROR, the remaining bits are organised as follows.

			[1110] ... <reserved> ... [eeee eeee] [eeee eeee]

			e: testable error code (see S_ERROR)

			the remaining bits are reserved for use by the framework (these bits may
			be on in any error message)

		----

		One uncertain design decision in the Component Interface has been: "Should
		the element of communication between Component and Engine be a handle or a pointer?".
		The advantage of a handle is that we can generate an error when the calling Component
		passes an invalid handle, rather than aborting if the caller passes an invalid
		pointer. The advantage of a pointer is that it is as-fast-as-can-be. Our rationale
		for choosing a handle is: computers will get faster, but developers will always
		make mistakes. Until computers do development. Oh well, any rationale is better
		than none I suppose.
	*/

	typedef UINTA Symbol;

	//	special NULL code means "nothing", and can be tested with "if (!...)" idiom.
	//	note that it is *equal to* the representation of the NUMBER zero, therefore
	//	interface functions returning numbers cannot also return S_NULL.
	__Symbol S_NULL                     __TOK_ 0 _TOK__

	//	32-bit
	#if ARCH_BITS == 32
	__Symbol S_TYPE_MASK                __TOK_ 0xF0000000 _TOK__
	__Symbol S_TYPE_NUMBER              __TOK_ 0x00000000 _TOK__
	__Symbol S_TYPE_NEG_NUMBER          __TOK_ 0xF0000000 _TOK__
	__Symbol S_TYPE_CONSTANT            __TOK_ 0x10000000 _TOK__
	__Symbol S_TYPE_ERROR               __TOK_ 0xE0000000 _TOK__
	__Symbol S_TYPE_RESERVED_MIN        __TOK_ 0x20000000 _TOK__
	__Symbol S_TYPE_RESERVED_MAX        __TOK_ 0xDFFFFFFF _TOK__
	__Symbol S_ERROR_CODE_MASK          __TOK_ 0x0000FFFF _TOK__
	#endif

	//	64-bit
	#if ARCH_BITS == 64
	__Symbol S_TYPE_MASK                __TOK_ 0xF000000000000000 _TOK__
	__Symbol S_TYPE_NUMBER              __TOK_ 0x0000000000000000 _TOK__
	__Symbol S_TYPE_NEG_NUMBER          __TOK_ 0xF000000000000000 _TOK__
	__Symbol S_TYPE_CONSTANT            __TOK_ 0x1000000000000000 _TOK__
	__Symbol S_TYPE_ERROR               __TOK_ 0xE000000000000000 _TOK__
	__Symbol S_TYPE_RESERVED_MIN        __TOK_ 0x2000000000000000 _TOK__
	__Symbol S_TYPE_RESERVED_MAX        __TOK_ 0xDFFFFFFFFFFFFFFF _TOK__
	__Symbol S_ERROR_CODE_MASK          __TOK_ 0x000000000000FFFF _TOK__
	#endif

	//	brahms interface code
	__Symbol N_BRAHMS_INTERFACE			__TOK_ 0x00010001 _TOK__

	//  constant ranges
	__Symbol C_BASE_COMPONENT           __TOK_ S_TYPE_CONSTANT + 0x00000000 _TOK__
	__Symbol C_BASE_COMPONENT_EVENT     __TOK_ S_TYPE_CONSTANT + 0x00010000 _TOK__
	__Symbol C_BASE_ENGINE              __TOK_ S_TYPE_CONSTANT + 0x00020000 _TOK__
	__Symbol C_BASE_ENGINE_EVENT        __TOK_ S_TYPE_CONSTANT + 0x00030000 _TOK__
	__Symbol C_BASE_USER                __TOK_ S_TYPE_CONSTANT + 0x00040000 _TOK__
	__Symbol C_BASE_USER_EVENT          __TOK_ S_TYPE_CONSTANT + 0x00050000 _TOK__
	
	//	use this range for private legacy constants that must not conflict with future interface constants
	__Symbol C_BASE_LEGACY              __TOK_ S_TYPE_CONSTANT + 0x000F0000 _TOK__

	//  component interface constants
	__Symbol C_OK                       __TOK_ C_BASE_COMPONENT + 0x0000 _TOK__
	__Symbol C_YES                      __TOK_ C_BASE_COMPONENT + 0x0001 _TOK__
	__Symbol C_NO                       __TOK_ C_BASE_COMPONENT + 0x0002 _TOK__
	__Symbol C_CANCEL                   __TOK_ C_BASE_COMPONENT + 0x0003 _TOK__
	__Symbol C_STOP_USER                __TOK_ C_BASE_COMPONENT + 0x0011 _TOK__
	__Symbol C_STOP_EXTERNAL            __TOK_ C_BASE_COMPONENT + 0x0012 _TOK__
	__Symbol C_STOP_CONDITION           __TOK_ C_BASE_COMPONENT + 0x0013 _TOK__
	__Symbol C_STOP_THEREFOREIAM        __TOK_ C_BASE_COMPONENT + 0x0014 _TOK__
	__Symbol C_FORM_FIXED               __TOK_ C_BASE_COMPONENT + 0x0021 _TOK__
	__Symbol C_FORM_BOUNDED             __TOK_ C_BASE_COMPONENT + 0x0022 _TOK__
	__Symbol C_FORM_UNBOUNDED           __TOK_ C_BASE_COMPONENT + 0x0023 _TOK__
	__Symbol C_FORM_FIXED_BUT_LAST      __TOK_ C_BASE_COMPONENT + 0x0024 _TOK__
	__Symbol C_TRANSPORT_PERIODIC       __TOK_ C_BASE_COMPONENT + 0x0031 _TOK__
	__Symbol C_BUFFERING_ONLY_DISK      __TOK_ C_BASE_COMPONENT + 0x0041 _TOK__
	__Symbol C_BUFFERING_FAVOUR_DISK    __TOK_ C_BASE_COMPONENT + 0x0042 _TOK__
	__Symbol C_BUFFERING_BALANCED       __TOK_ C_BASE_COMPONENT + 0x0043 _TOK__
	__Symbol C_BUFFERING_FAVOUR_MEMORY  __TOK_ C_BASE_COMPONENT + 0x0044 _TOK__
	__Symbol C_BUFFERING_ONLY_MEMORY    __TOK_ C_BASE_COMPONENT + 0x0045 _TOK__

	//	error ranges
	__Symbol E_BASE_COMPONENT           __TOK_ S_TYPE_ERROR + 0x0000 _TOK__
	__Symbol E_BASE_ENGINE              __TOK_ S_TYPE_ERROR + 0x8000 _TOK__

	//	errors
	__Symbol E_ERROR                    __TOK_ E_BASE_COMPONENT + 0x0000 _TOK__
	__Symbol E_USER                     __TOK_ E_BASE_COMPONENT + 0x0001 _TOK__
	__Symbol E_INTERNAL                 __TOK_ E_BASE_COMPONENT + 0x0002 _TOK__
	__Symbol E_MEMORY                   __TOK_ E_BASE_COMPONENT + 0x0003 _TOK__
	__Symbol E_NOT_FOUND                __TOK_ E_BASE_COMPONENT + 0x0004 _TOK__
	__Symbol E_NOT_IMPLEMENTED          __TOK_ E_BASE_COMPONENT + 0x0005 _TOK__
	__Symbol E_UNRECOGNISED_EXCEPTION   __TOK_ E_BASE_COMPONENT + 0x0006 _TOK__
	__Symbol E_STD                      __TOK_ E_BASE_COMPONENT + 0x0007 _TOK__
	__Symbol E_REALITY_INVERSION        __TOK_ E_BASE_COMPONENT + 0x0008 _TOK__
	__Symbol E_DATAML                   __TOK_ E_BASE_COMPONENT + 0x0009 _TOK__
	__Symbol E_NULL_ARG                 __TOK_ E_BASE_COMPONENT + 0x000A _TOK__
	__Symbol E_INVALID_ARG              __TOK_ E_BASE_COMPONENT + 0x000B _TOK__
	__Symbol E_INVALID_HANDLE           __TOK_ E_BASE_COMPONENT + 0x000C _TOK__
	__Symbol E_WRONG_HANDLE_TYPE        __TOK_ E_BASE_COMPONENT + 0x000D _TOK__
	__Symbol E_WRONG_NEW_HANDLE_TYPE    __TOK_ E_BASE_COMPONENT + 0x000E _TOK__
	__Symbol E_NOT_AVAILABLE            __TOK_ E_BASE_COMPONENT + 0x000F _TOK__
	__Symbol E_OVERFLOW                 __TOK_ E_BASE_COMPONENT + 0x0010 _TOK__
	__Symbol E_INTERFACE_MISUSE         __TOK_ E_BASE_COMPONENT + 0x0011 _TOK__
	__Symbol E_NO_INSTANCE              __TOK_ E_BASE_COMPONENT + 0x0012 _TOK__
	__Symbol E_NOT_SERVICED             __TOK_ E_BASE_COMPONENT + 0x0013 _TOK__
	__Symbol E_PORT_EMPTY               __TOK_ E_BASE_COMPONENT + 0x0014 _TOK__
	__Symbol E_INVALID_INPUT            __TOK_ E_BASE_COMPONENT + 0x0015 _TOK__
	__Symbol E_FUNC_NOT_FOUND           __TOK_ E_BASE_COMPONENT + 0x0016 _TOK__
	__Symbol E_BAD_ARG_COUNT            __TOK_ E_BASE_COMPONENT + 0x0017 _TOK__
	__Symbol E_BAD_ARG_TYPE             __TOK_ E_BASE_COMPONENT + 0x0018 _TOK__
	__Symbol E_BAD_ARG_SIZE             __TOK_ E_BASE_COMPONENT + 0x0019 _TOK__



////////////////    SYMBOL MACROS

	#define S_NUMBER(symbol) (((symbol & S_TYPE_MASK) == S_TYPE_NUMBER) || ((symbol & S_TYPE_MASK) == S_TYPE_NEG_NUMBER))
	#define S_CONSTANT(symbol) ((symbol & S_TYPE_MASK) == S_TYPE_CONSTANT)
	#define S_ERROR(symbol) (((symbol & S_TYPE_MASK) == S_TYPE_ERROR) ? (symbol & (S_TYPE_MASK | S_ERROR_CODE_MASK)) : S_NULL)



////////////////    EVENT TYPES

	//	null event
	__Symbol EVENT_NULL                  __TOK_ C_BASE_COMPONENT_EVENT + 0x0000 _TOK__

	//	module level events
	__Symbol EVENT_MODULE_QUERY          __TOK_ C_BASE_COMPONENT_EVENT + 0x0001 _TOK__
	__Symbol EVENT_MODULE_INIT           __TOK_ C_BASE_COMPONENT_EVENT + 0x0011 _TOK__
	__Symbol EVENT_MODULE_TERM           __TOK_ C_BASE_COMPONENT_EVENT + 0x0012 _TOK__
	__Symbol EVENT_MODULE_CREATE         __TOK_ C_BASE_COMPONENT_EVENT + 0x0013 _TOK__
	__Symbol EVENT_MODULE_DUPLICATE      __TOK_ C_BASE_COMPONENT_EVENT + 0x0014 _TOK__
	__Symbol EVENT_MODULE_DESTROY        __TOK_ C_BASE_COMPONENT_EVENT + 0x0015 _TOK__
	__Symbol EVENT_MODULE_MAX            __TOK_ C_BASE_COMPONENT_EVENT + 0x00FF _TOK__  //  this helps an existing binding to decide whether to pass an unrecognised (new) event to the object, or just ignore it

	//	component events
	__Symbol EVENT_INIT_COMPLETE         __TOK_ C_BASE_COMPONENT_EVENT + 0x0101 _TOK__  //  final configuration information for component
	__Symbol EVENT_STATE_SET             __TOK_ C_BASE_COMPONENT_EVENT + 0x0102 _TOK__  //  here's your state as an XML node
	__Symbol EVENT_STATE_GET             __TOK_ C_BASE_COMPONENT_EVENT + 0x0103 _TOK__  //  please supply your state as an XML node

	//  process events
	__Symbol EVENT_INIT_PRECONNECT       __TOK_ C_BASE_COMPONENT_EVENT + 0x0111 _TOK__  //  the number of inputs in each set is now set
	__Symbol EVENT_INIT_CONNECT          __TOK_ C_BASE_COMPONENT_EVENT + 0x0112 _TOK__  //  here are your inputs so far - create outputs if you can
	__Symbol EVENT_INIT_POSTCONNECT      __TOK_ C_BASE_COMPONENT_EVENT + 0x0113 _TOK__  //  all links are now made (you can now safely take pointers to interface data* objects)
	__Symbol EVENT_RUN_PLAY              __TOK_ C_BASE_COMPONENT_EVENT + 0x0121 _TOK__  //  STOP --> PAUSE
	__Symbol EVENT_RUN_RESUME            __TOK_ C_BASE_COMPONENT_EVENT + 0x0122 _TOK__  //  PAUSE --> PLAY
	__Symbol EVENT_RUN_SERVICE           __TOK_ C_BASE_COMPONENT_EVENT + 0x0123 _TOK__  //  service input/output interfaces
	__Symbol EVENT_RUN_PAUSE             __TOK_ C_BASE_COMPONENT_EVENT + 0x0124 _TOK__  //  PLAY --> PAUSE
	__Symbol EVENT_RUN_STOP              __TOK_ C_BASE_COMPONENT_EVENT + 0x0125 _TOK__  //  PAUSE --> STOP

	//  data events
	__Symbol EVENT_CONTENT_SET           __TOK_ C_BASE_COMPONENT_EVENT + 0x0131 _TOK__  //  here's your content as a binary stream
	__Symbol EVENT_CONTENT_GET           __TOK_ C_BASE_COMPONENT_EVENT + 0x0132 _TOK__  //  please supply your content as a binary stream
	__Symbol EVENT_LOG_INIT              __TOK_ C_BASE_COMPONENT_EVENT + 0x0141 _TOK__  //  initialise storage
	__Symbol EVENT_LOG_SERVICE           __TOK_ C_BASE_COMPONENT_EVENT + 0x0142 _TOK__  //  service (increment) storage
	__Symbol EVENT_LOG_TERM              __TOK_ C_BASE_COMPONENT_EVENT + 0x0143 _TOK__  //  terminate storage (and return it)

	//  generic events
	__Symbol EVENT_FUNCTION_GET          __TOK_ C_BASE_COMPONENT_EVENT + 0x0151 _TOK__  //  get handle to utility function (generic interface)
	__Symbol EVENT_FUNCTION_CALL         __TOK_ C_BASE_COMPONENT_EVENT + 0x0152 _TOK__  //  call utility function (generic interface)
	__Symbol EVENT_GENERIC_STRUCTURE_SET __TOK_ C_BASE_COMPONENT_EVENT + 0x0161 _TOK__
	__Symbol EVENT_GENERIC_STRUCTURE_GET __TOK_ C_BASE_COMPONENT_EVENT + 0x0162 _TOK__
	__Symbol EVENT_GENERIC_FORM_ADVANCE  __TOK_ C_BASE_COMPONENT_EVENT + 0x0163 _TOK__
	__Symbol EVENT_GENERIC_FORM_CURRENT  __TOK_ C_BASE_COMPONENT_EVENT + 0x0164 _TOK__
	__Symbol EVENT_GENERIC_CONTENT_SET   __TOK_ C_BASE_COMPONENT_EVENT + 0x0165 _TOK__
	__Symbol EVENT_GENERIC_CONTENT_GET   __TOK_ C_BASE_COMPONENT_EVENT + 0x0166 _TOK__



////////////////    EVENTS

	__UINT32 F_FIRST_CALL                __TOK_ 0x00000001 _TOK__    //  first call of this event type
	__UINT32 F_LAST_CALL                 __TOK_ 0x00000002 _TOK__    //  last call of this event type
	__UINT32 F_GLOBAL_ERROR              __TOK_ 0x00000004 _TOK__    //  an error condition has arisen
	__UINT32 F_LOCAL_ERROR               __TOK_ 0x00000008 _TOK__    //  the receiving process has previously thrown

	struct Event
	{
		UINT32 flags;
		Symbol type;
		void* object;
		void* data;
	};

	typedef Symbol (EventHandlerFunction) (struct Event*);



////////////////    MODULE-LEVEL EVENTS

	struct EventModuleQuery
	{
		UINT32 interfaceID;
	};

	struct ExecutionInfo
	{
		struct FrameworkVersion engineVersion;
		Symbol executionParameters;
		Symbol BufferingPolicy;
	};

	__UINT32 F_NO_CONCURRENCY            __TOK_ 0x00000001 _TOK__	//	multiple instances of the component must not be executed concurrently
	__UINT32 F_NO_CHANGE_THREAD          __TOK_ 0x00000002 _TOK__	//	component must remain in the thread it starts in
	__UINT32 F_NO_CHANGE_VOICE           __TOK_ 0x00000004 _TOK__	//	component must remain in the voice it starts in

	struct ModuleInfo
	{
		const struct FrameworkVersion* engineVersion;   //  engine version this module was compiled against
		UINT32 archBits;								//	bit-width this module was compiled at
		UINT32 binding;									//  binding name (indicates language)
		UINT32 flags;									//	see flags above
	};

	struct EventModuleInit
	{
		const struct ExecutionInfo* executionInfo;
		const struct ModuleInfo* moduleInfo;
	};

	//	this must be defined, so that functions using it (e.g. moduleTerm() in 1199) can compile correctly
	//	it must therefore have some content, so we give it dummy content which will change if ever we fill it with something
	struct EventModuleTerm
	{
		UINT32 ____reserved;
	};

	struct EventModuleCreate
	{
		Symbol hComponent;
		const struct ComponentData* data;
		const struct ComponentInfo* info;
	};

	/*
	struct EventModuleDuplicate (currently, we use EventModuleCreate - this could change in future, spawning a new, extended, type)
	struct EventModuleDestroy
	*/



////////////////    COMPONENT EVENTS

	struct EventInitComplete
	{
		UINT32 contentHeaderBytes;		//	bytes to reserve at top of buffer for EVENT_CONTENT_GET, or zero if EVENT_CONTENT_GET will not be sent
	};

	__UINT32 F_UNDEFINED					__TOK_ 0x00000001 _TOK__   //  set state to "undefined", whatever that means to you
	__UINT32 F_ZERO						__TOK_ 0x00000002 _TOK__   //  set state to "zero", whatever that means to you

	struct EventStateSet
	{
		UINT32 flags;           //  flags listed above
		Symbol state;
	};

	struct EventStateGet
	{
		UINT32 flags;
		Symbol state;
		INT32 precision;
	};



////////////////    PROCESS EVENTS

	/*
	struct EventInitPreconnect
	struct EventInitConnect
	struct EventInitPostconnect
	struct EventRunPlay
	struct EventRunResume
	struct EventRunService
	struct EventRunPause
	struct EventRunStop
	*/



////////////////    DATA EVENTS

	__UINT32 F_ENCAPSULATED				__TOK_ 0x00000001 _TOK__
	__INT32 PRECISION_NOT_SET			__TOK_ 0x40000000 _TOK__

	struct EventContent
	{
		UINT8* stream;			//  pointer to head of stream
		UINT64 bytes;			//  number of bytes available in stream
	};

	struct EventLog
	{
		UINT32 flags;
		INT32 precision;
		const char* filename;
		UINT64 count;
		void* source;
		Symbol result;
	};



////////////////    GENERIC EVENTS

	__UINT32 F_MODIFIED __TOK_ 0x00000001 _TOK__

	struct Argument
	{
		void* real;
		void* imag;
		TYPE type;
		struct Dimensions dims;
		UINT32 flags;
	};

	struct EventFunctionGet
	{
		const char* name;				//  name of sought function (IN)
		Symbol handle;					//  handle to use to call this function (OUT)
		UINT32 argumentModifyCount;		//  number of arguments this function will modify (OUT)
	};

	struct EventFunctionCall
	{
		Symbol handle;					//  handle of function to call (IN)
		struct Argument** arguments;	//  arguments (IN)
		UINT32 argumentCount;			//  number of arguments (IN), doubles as "offending argument" and "expected number of arguments" (OUT)
	};

	struct EventGenericStructure
	{
		const char* structure;
		TYPE type;
	};

	struct EventGenericForm
	{
		Symbol form;
		TYPE type;
		struct Dimensions dims;
	};

	struct EventGenericContent
	{
		TYPE type;
		const void* real;
		const void* imag;
		UINT64 bytes;
	};



////////////////    ENGINE EVENT

	struct EngineEvent
	{
		Symbol hCaller;
		UINT32 flags;
		Symbol type;
		void* data;
	};

	BRAHMS_ENGINE_VIS Symbol brahms_engineEvent(struct EngineEvent* data);



////////////////    LEGACY ENGINE EVENTS

	__Symbol ENGINE_EVENT_SET_PORT_NAME				__TOK_ C_BASE_ENGINE_EVENT + 0x8001 _TOK__
	__Symbol ENGINE_EVENT_SET_PORT_SAMPLE_RATE		__TOK_ C_BASE_ENGINE_EVENT + 0x8002 _TOK__
	__Symbol ENGINE_EVENT_DATA_FROM_PORT			__TOK_ C_BASE_ENGINE_EVENT + 0x8003 _TOK__
	__Symbol ENGINE_EVENT_ASSERT_COMPONENT_SPEC		__TOK_ C_BASE_ENGINE_EVENT + 0x8004 _TOK__
	__Symbol ENGINE_EVENT_FIRE_EVENT_ON_DATA		__TOK_ C_BASE_ENGINE_EVENT + 0x8005 _TOK__
	__Symbol ENGINE_EVENT_GET_PORT_ON_SET			__TOK_ C_BASE_ENGINE_EVENT + 0x8006 _TOK__
	__Symbol ENGINE_EVENT_HANDLE_UTILITY_EVENT		__TOK_ C_BASE_ENGINE_EVENT + 0x8007 _TOK__



////////////////    TYPES USED ON CLIENT INTERFACES

	struct ComponentSpec
	{
		const char* cls;
		UINT16 release;
	};

	struct HandledEvent
	{
		EventHandlerFunction* handler;
		struct Event event;
	};

	struct TransportDataPeriodic
	{
		struct SampleRate sampleRate;
	};



////////////////    GENERAL INTERFACE

	__Symbol ENGINE_EVENT_STILL_ACTIVE				__TOK_ C_BASE_ENGINE_EVENT + 0x0001 _TOK__
	__Symbol ENGINE_EVENT_CREATE_UTILITY			__TOK_ C_BASE_ENGINE_EVENT + 0x0002 _TOK__
	__Symbol ENGINE_EVENT_GET_RANDOM_SEED			__TOK_ C_BASE_ENGINE_EVENT + 0x0003 _TOK__
	__Symbol ENGINE_EVENT_ERROR_MESSAGE				__TOK_ C_BASE_ENGINE_EVENT + 0x0004 _TOK__
	__Symbol ENGINE_EVENT_OUTPUT_MESSAGE			__TOK_ C_BASE_ENGINE_EVENT + 0x0005 _TOK__
	__Symbol ENGINE_EVENT_GET_SYMBOL_STRING			__TOK_ C_BASE_ENGINE_EVENT + 0x0006 _TOK__
	__Symbol ENGINE_EVENT_GET_TYPE_STRING			__TOK_ C_BASE_ENGINE_EVENT + 0x0007 _TOK__
	
	struct EventCreateUtility
	{
		UINT32 flags;
		Symbol hUtility;
		struct ComponentSpec spec;
		const char* name;
		struct HandledEvent* handledEvent;
	};

	struct EventGetRandomSeed
	{
		UINT32 flags;
		UINT32* seed;
		UINT32 count;
	};

	struct EventErrorMessage
	{
		UINT32 flags;
		Symbol error;
		const char* msg;
	};

	__UINT32 F_TRACE      __TOK_ 0x00000001 _TOK__ // add trace message rather than error message
	__UINT32 F_DEBUGTRACE __TOK_ 0x00000002 _TOK__ // add debug-level trace message rather than error message

	struct EventOutputMessage
	{
		UINT32 flags;
		DetailLevel level;
		const char* msg;
	};

	struct EventGetSymbolString
	{
		UINT32 flags;
		Symbol symbol;
		const char* result;
	};

	struct EventGetTypeString
	{
		UINT32 flags;
		TYPE type;
		const char* result;
	};



////////////////    SYSTEMML INTERFACE

	__Symbol ENGINE_EVENT_GET_SET					__TOK_ C_BASE_ENGINE_EVENT + 0x0011 _TOK__
	__Symbol ENGINE_EVENT_ADD_PORT					__TOK_ C_BASE_ENGINE_EVENT + 0x0012 _TOK__
	__Symbol ENGINE_EVENT_GET_PORT					__TOK_ C_BASE_ENGINE_EVENT + 0x0013 _TOK__
	__Symbol ENGINE_EVENT_GET_SET_INFO				__TOK_ C_BASE_ENGINE_EVENT + 0x0014 _TOK__
	__Symbol ENGINE_EVENT_GET_PORT_INFO				__TOK_ C_BASE_ENGINE_EVENT + 0x0015 _TOK__

	//	index undefined
	__UINT32 INDEX_UNDEFINED __TOK_ 0xFFFFFFFF _TOK__

	//	interface flags
	__UINT32 F_IIF __TOK_ 0x00000001 _TOK__
	__UINT32 F_OIF __TOK_ 0x00000002 _TOK__

	//	port flags
	__UINT32 F_NO_STORE __TOK_ 0x00000001 _TOK__

	struct EventGetSet
	{
		UINT32 flags;
		const char* name;
	};

	struct EventAddPort
	{
		UINT32 flags;
		Symbol hSet;
		struct ComponentSpec spec;
		struct HandledEvent* handledEvent;
		const char* name;
		UINT32 index;
		Symbol hPortToCopy;
		Symbol transportProtocol;
		void* transportData;
	};

	struct EventGetPort
	{
		UINT32 flags;
		Symbol hSet;
		struct ComponentSpec spec;
		struct HandledEvent* handledEvent;
		const char* name;
		UINT32 index;
	};

	struct EventGetSetInfo
	{
		UINT32 flags;
		Symbol hSet;
		const char* name;
		UINT32 portCount;
	};

	//	port info flags
	__UINT32 F_LISTENED								__TOK_ 0x00000001 _TOK__ // o/p only
	__UINT32 F_IMPLICIT_NAME						__TOK_ 0x00000001 _TOK__ // i/p only (so can have the same value as F_LISTENED)

	struct EventGetPortInfo
	{
		UINT32 flags;
		Symbol hPort;
		const char* name;
		const struct ComponentInfo* componentInfo;
	};



////////////////    XML INTERFACE

	/*

		W3C Merged Document & Node & Element interfaces (truncated)
		based on: http://www.w3.org/TR/DOM-Level-3-Core/core.html

		Element         createElement(in DOMString tagName)
		DOMString       nodeName;
		Node            parentNode;
		NodeList        childNodes;
		Node            firstChild;
		Node            lastChild;
		Node            previousSibling;
		Node            nextSibling;
		Node            insertBefore(in Node newChild, in Node refChild)
		Node            replaceChild(in Node newChild, in Node oldChild)
		Node            removeChild(in Node oldChild)
		Node            appendChild(in Node newChild)
		boolean         hasChildNodes();
		Node            cloneNode(in boolean deep);
		boolean         hasAttributes();
		boolean         isSameNode(in Node other);
		boolean         isEqualNode(in Node arg);
		DOMString       getAttribute(in DOMString name);
		void            setAttribute(in DOMString name, in DOMString value)
		void            removeAttribute(in DOMString name)
		NodeList        getElementsByTagName(in DOMString name);
		boolean         hasAttribute(in DOMString name);

		BRAHMS XML INTERFACE is intended to approach the W3C standard, though it isn't
		exactly the same. We don't distinguish node types (i.e. we do minimal XML) so
		we have "node text" as a property of a node, rather than a sub-node.

	*/

	/* W3C */

	BRAHMS_ENGINE_VIS Symbol xml_createElement(const char* name);
	BRAHMS_ENGINE_VIS Symbol xml_setNodeName(Symbol node, const char* name);
	BRAHMS_ENGINE_VIS Symbol xml_getNodeName(Symbol node, const char** name);
	BRAHMS_ENGINE_VIS Symbol xml_parentNode(Symbol node);
	BRAHMS_ENGINE_VIS Symbol xml_childNodes(Symbol node, Symbol* children, UINT32* count);

	BRAHMS_ENGINE_VIS Symbol xml_firstChild(Symbol node);
	BRAHMS_ENGINE_VIS Symbol xml_lastChild(Symbol node);
	BRAHMS_ENGINE_VIS Symbol xml_previousSibling(Symbol node);
	BRAHMS_ENGINE_VIS Symbol xml_nextSibling(Symbol node);

	BRAHMS_ENGINE_VIS Symbol xml_insertBefore(Symbol node, Symbol child, Symbol ref);
	BRAHMS_ENGINE_VIS Symbol xml_replaceChild(Symbol node, Symbol child, Symbol old);
	BRAHMS_ENGINE_VIS Symbol xml_removeChild(Symbol node, Symbol old);
	BRAHMS_ENGINE_VIS Symbol xml_appendChild(Symbol node, Symbol child);
	BRAHMS_ENGINE_VIS Symbol xml_hasChildNodes(Symbol node);
	BRAHMS_ENGINE_VIS Symbol xml_cloneNode(Symbol node, UINT32 deep);
	BRAHMS_ENGINE_VIS Symbol xml_hasAttributes(Symbol node);

	BRAHMS_ENGINE_VIS Symbol xml_isSameNode(Symbol node, Symbol other);
	BRAHMS_ENGINE_VIS Symbol xml_isEqualNode(Symbol node, Symbol other);

	BRAHMS_ENGINE_VIS Symbol xml_getAttribute(Symbol node, const char* name, const char** value);
	BRAHMS_ENGINE_VIS Symbol xml_setAttribute(Symbol node, const char* name, const char* value);
	BRAHMS_ENGINE_VIS Symbol xml_removeAttribute(Symbol node, const char* name);

	BRAHMS_ENGINE_VIS Symbol xml_getElementsByTagName(Symbol node, const char* name, Symbol* children, UINT32* count);
	BRAHMS_ENGINE_VIS Symbol xml_hasAttribute(Symbol node, const char* name);

	/* Extensions */

	BRAHMS_ENGINE_VIS Symbol xml_setNodeText(Symbol node, const char* text);				//	node text is a property
	BRAHMS_ENGINE_VIS Symbol xml_getNodeText(Symbol node, const char** text);				//	node text is a property
	BRAHMS_ENGINE_VIS Symbol xml_clearNode(Symbol node);									//	delete children, attributes, and set text to "" (name is unchanged)
	BRAHMS_ENGINE_VIS Symbol xml_getChild(Symbol node, const char* name, UINT32 index);		//	get index'th child with specified name (i.e. behave as associative array)



////////////////    END EXTERN C / NAMESPACE

#ifdef __cplusplus
} }
#endif



////////////////    INCLUSION GUARD

#endif



