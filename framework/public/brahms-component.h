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

	//	engine version (INT16 flags) Without subversion numbers, there's no reason not to hard code these.
        #define VERSION_ENGINE_MAJ ( 0 )
	#define VERSION_ENGINE_MIN ( 7 )
	#define VERSION_ENGINE_REL ( 3 )
	#define VERSION_ENGINE_REV ( 2696 )

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

	#define TYPE_UNSPECIFIED         ( 0x00000000 )

	//  container width
	#define TYPE_WIDTH_UNSPECIFIED   ( 0x00000000 )
	#define TYPE_WIDTH_8BIT          ( 0x00000001 )
	#define TYPE_WIDTH_16BIT         ( 0x00000002 )
	#define TYPE_WIDTH_32BIT         ( 0x00000003 )
	#define TYPE_WIDTH_64BIT         ( 0x00000004 )
	#define TYPE_WIDTH_MASK          ( 0x0000000F )

	//  storage format
	#define TYPE_FORMAT_UNSPECIFIED  ( 0x00000000 )
	#define TYPE_FORMAT_FLOAT        ( 0x00000010 )
	#define TYPE_FORMAT_FIXED        ( 0x00000020 )
	#define TYPE_FORMAT_INT          ( 0x00000030 )
	#define TYPE_FORMAT_UINT         ( 0x00000040 )
	#define TYPE_FORMAT_BOOL         ( 0x00000050 )
	#define TYPE_FORMAT_CHAR         ( 0x00000060 )
	#define TYPE_FORMAT_STRUCT       ( 0x000000A0 )
	#define TYPE_FORMAT_CELL         ( 0x000000B0 )
	#define TYPE_FORMAT_MASK         ( 0x000000F0 )

	//  these bits describe the "element"
	#define TYPE_ELEMENT_MASK        ( TYPE_WIDTH_MASK | TYPE_FORMAT_MASK )

	//  complexity flags
	#define TYPE_COMPLEX_UNSPECIFIED ( 0x00000000 )
	#define TYPE_REAL                ( 0x00000100 )
	#define TYPE_COMPLEX             ( 0x00000200 )
	#define TYPE_COMPLEX_MASK        ( 0x00000300 )

	//	complexity storage format
	#define TYPE_CPXFMT_UNSPECIFIED ( 0x00000000 )
	#define TYPE_CPXFMT_ADJACENT    ( 0x00000400 )
	#define TYPE_CPXFMT_INTERLEAVED ( 0x00000800 )
	#define TYPE_CPXFMT_MASK        ( 0x00000C00 )

	//	ordering flags
	#define TYPE_ORDER_UNSPECIFIED  ( 0x00000000 )
	#define TYPE_ORDER_COLUMN_MAJOR ( 0x00001000 )
	#define TYPE_ORDER_ROW_MAJOR    ( 0x00002000 )
	#define TYPE_ORDER_MASK         ( 0x00003000 )

	//	these bits describe the "array"
	#define TYPE_ARRAY_MASK          ( TYPE_ELEMENT_MASK | TYPE_COMPLEX_MASK | TYPE_CPXFMT_MASK | TYPE_ORDER_MASK )

	//  derived constants are the ones you would actually use (some do not specify bit-width, for obvious reasons)
	#define TYPE_FLOAT32             ( TYPE_FORMAT_FLOAT | TYPE_WIDTH_32BIT )
	#define TYPE_FLOAT64             ( TYPE_FORMAT_FLOAT | TYPE_WIDTH_64BIT )
	#define TYPE_INT8                ( TYPE_FORMAT_INT   | TYPE_WIDTH_8BIT )
	#define TYPE_INT16               ( TYPE_FORMAT_INT   | TYPE_WIDTH_16BIT )
	#define TYPE_INT32               ( TYPE_FORMAT_INT   | TYPE_WIDTH_32BIT )
	#define TYPE_INT64               ( TYPE_FORMAT_INT   | TYPE_WIDTH_64BIT )
	#define TYPE_UINT8               ( TYPE_FORMAT_UINT  | TYPE_WIDTH_8BIT )
	#define TYPE_UINT16              ( TYPE_FORMAT_UINT  | TYPE_WIDTH_16BIT )
	#define TYPE_UINT32              ( TYPE_FORMAT_UINT  | TYPE_WIDTH_32BIT )
	#define TYPE_UINT64              ( TYPE_FORMAT_UINT  | TYPE_WIDTH_64BIT )
	#define TYPE_BOOL8               ( TYPE_FORMAT_BOOL  | TYPE_WIDTH_8BIT )
	#define TYPE_BOOL16              ( TYPE_FORMAT_BOOL  | TYPE_WIDTH_16BIT )
	#define TYPE_BOOL32              ( TYPE_FORMAT_BOOL  | TYPE_WIDTH_32BIT )
	#define TYPE_BOOL64              ( TYPE_FORMAT_BOOL  | TYPE_WIDTH_64BIT )
	#define TYPE_CHAR8               ( TYPE_FORMAT_CHAR  | TYPE_WIDTH_8BIT )
	#define TYPE_CHAR16              ( TYPE_FORMAT_CHAR  | TYPE_WIDTH_16BIT )
	#define TYPE_CHAR32              ( TYPE_FORMAT_CHAR  | TYPE_WIDTH_32BIT )
	#define TYPE_CHAR64              ( TYPE_FORMAT_CHAR  | TYPE_WIDTH_64BIT )
	#define TYPE_STRUCT              ( TYPE_FORMAT_STRUCT )
	#define TYPE_CELL                ( TYPE_FORMAT_CELL )

	//	synonyms
	#define TYPE_SINGLE              ( TYPE_FLOAT32 )
	#define TYPE_DOUBLE              ( TYPE_FLOAT64 )
	#define TYPE_BYTE                ( TYPE_UINT8 )
	#define TYPE_BOOL                ( TYPE_BOOL8 )



////////////////    ARCHITECTURE DEPENDENT

	#if ARCH_BITS == 64
	#define ARCH_BYTES             ( 8 )
	#define TYPE_INTA                ( TYPE_INT64 )
	#define TYPE_UINTA               ( TYPE_UINT64 )
	typedef INT64                  INTA;
	typedef UINT64                 UINTA;
	#else
	#define ARCH_BYTES             ( 4 )
	#define TYPE_INTA                ( TYPE_INT32 )
	#define TYPE_UINTA               ( TYPE_UINT32 )
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

        // INT64 flags.
	#define DIM_ANY        ( -1 )
	#define DIM_NONZERO    ( -2 )
	#define DIM_ELLIPSIS   ( -3 )
	#define DIM_ABSENT     ( -4 )



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

	#define D_NONE ( 0x00 ) //  normal levels
	#define D_WARN ( 0x20 )
	#define D_INFO ( 0x40 )
	#define D_VERB ( 0x60 )



////////////////    COMPONENT INFO

	typedef UINT32 ComponentType;

	//	these must be OR-able for component register, so distinct bits for each
	#define CT_NULL      ( 0 )
	#define CT_DATA      ( 1 )
	#define CT_PROCESS   ( 2 )
	#define CT_UTILITY   ( 4 )

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

	#define F_NEEDS_ALL_INPUTS          ( 0x00000001 )	//	component must only receive EVENT_INIT_CONNECT when all its inputs are present
	#define F_INPUTS_SAME_RATE          ( 0x00000002 )	//	component must only receive inputs that share its sample rate
	#define F_OUTPUTS_SAME_RATE         ( 0x00000004 )	//	component must only create outputs that share its sample rate
	#define F_NOT_RATE_CHANGER          ( F_INPUTS_SAME_RATE | F_OUTPUTS_SAME_RATE )
	#define M_COMPONENT_FLAGS           ( 0x00000007 )

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
	#define S_NULL                     ( 0 )

	//	32-bit symbols
	#if ARCH_BITS == 32
	#define S_TYPE_MASK                ( 0xF0000000 )
	#define S_TYPE_NUMBER              ( 0x00000000 )
	#define S_TYPE_NEG_NUMBER          ( 0xF0000000 )
	#define S_TYPE_CONSTANT            ( 0x10000000 )
	#define S_TYPE_ERROR               ( 0xE0000000 )
	#define S_TYPE_RESERVED_MIN        ( 0x20000000 )
	#define S_TYPE_RESERVED_MAX        ( 0xDFFFFFFF )
	#define S_ERROR_CODE_MASK          ( 0x0000FFFF )
	#endif

	//	64-bit symbols
	#if ARCH_BITS == 64
	#define S_TYPE_MASK                ( 0xF000000000000000 )
	#define S_TYPE_NUMBER              ( 0x0000000000000000 )
	#define S_TYPE_NEG_NUMBER          ( 0xF000000000000000 )
	#define S_TYPE_CONSTANT            ( 0x1000000000000000 )
	#define S_TYPE_ERROR               ( 0xE000000000000000 )
	#define S_TYPE_RESERVED_MIN        ( 0x2000000000000000 )
	#define S_TYPE_RESERVED_MAX        ( 0xDFFFFFFFFFFFFFFF )
	#define S_ERROR_CODE_MASK          ( 0x000000000000FFFF )
	#endif

	//	brahms interface code symbol
	#define N_BRAHMS_INTERFACE			( 0x00010001 )

	//  constant range symbols
	#define C_BASE_COMPONENT           ( S_TYPE_CONSTANT + 0x00000000 )
	#define C_BASE_COMPONENT_EVENT     ( S_TYPE_CONSTANT + 0x00010000 )
	#define C_BASE_ENGINE              ( S_TYPE_CONSTANT + 0x00020000 )
	#define C_BASE_ENGINE_EVENT        ( S_TYPE_CONSTANT + 0x00030000 )
	#define C_BASE_USER                ( S_TYPE_CONSTANT + 0x00040000 )
	#define C_BASE_USER_EVENT          ( S_TYPE_CONSTANT + 0x00050000 )

	//	use this range for private legacy constants that must not conflict with future interface constants
	#define C_BASE_LEGACY              ( S_TYPE_CONSTANT + 0x000F0000 )

	//  component interface constant symbols
	#define C_OK                       ( C_BASE_COMPONENT + 0x0000 )
	#define C_YES                      ( C_BASE_COMPONENT + 0x0001 )
	#define C_NO                       ( C_BASE_COMPONENT + 0x0002 )
	#define C_CANCEL                   ( C_BASE_COMPONENT + 0x0003 )
	#define C_STOP_USER                ( C_BASE_COMPONENT + 0x0011 )
	#define C_STOP_EXTERNAL            ( C_BASE_COMPONENT + 0x0012 )
	#define C_STOP_CONDITION           ( C_BASE_COMPONENT + 0x0013 )
	#define C_STOP_THEREFOREIAM        ( C_BASE_COMPONENT + 0x0014 )
	#define C_FORM_FIXED               ( C_BASE_COMPONENT + 0x0021 )
	#define C_FORM_BOUNDED             ( C_BASE_COMPONENT + 0x0022 )
	#define C_FORM_UNBOUNDED           ( C_BASE_COMPONENT + 0x0023 )
	#define C_FORM_FIXED_BUT_LAST      ( C_BASE_COMPONENT + 0x0024 )
	#define C_TRANSPORT_PERIODIC       ( C_BASE_COMPONENT + 0x0031 )
	#define C_BUFFERING_ONLY_DISK      ( C_BASE_COMPONENT + 0x0041 )
	#define C_BUFFERING_FAVOUR_DISK    ( C_BASE_COMPONENT + 0x0042 )
	#define C_BUFFERING_BALANCED       ( C_BASE_COMPONENT + 0x0043 )
	#define C_BUFFERING_FAVOUR_MEMORY  ( C_BASE_COMPONENT + 0x0044 )
	#define C_BUFFERING_ONLY_MEMORY    ( C_BASE_COMPONENT + 0x0045 )

	//	error range symbols
	#define E_BASE_COMPONENT           ( S_TYPE_ERROR + 0x0000 )
	#define E_BASE_ENGINE              ( S_TYPE_ERROR + 0x8000 )

	//	error symbols
	#define E_ERROR                    ( E_BASE_COMPONENT + 0x0000 )
	#define E_USER                     ( E_BASE_COMPONENT + 0x0001 )
	#define E_INTERNAL                 ( E_BASE_COMPONENT + 0x0002 )
	#define E_MEMORY                   ( E_BASE_COMPONENT + 0x0003 )
	#define E_NOT_FOUND                ( E_BASE_COMPONENT + 0x0004 )
	#define E_NOT_IMPLEMENTED          ( E_BASE_COMPONENT + 0x0005 )
	#define E_UNRECOGNISED_EXCEPTION   ( E_BASE_COMPONENT + 0x0006 )
	#define E_STD                      ( E_BASE_COMPONENT + 0x0007 )
	#define E_REALITY_INVERSION        ( E_BASE_COMPONENT + 0x0008 )
	#define E_DATAML                   ( E_BASE_COMPONENT + 0x0009 )
	#define E_NULL_ARG                 ( E_BASE_COMPONENT + 0x000A )
	#define E_INVALID_ARG              ( E_BASE_COMPONENT + 0x000B )
	#define E_INVALID_HANDLE           ( E_BASE_COMPONENT + 0x000C )
	#define E_WRONG_HANDLE_TYPE        ( E_BASE_COMPONENT + 0x000D )
	#define E_WRONG_NEW_HANDLE_TYPE    ( E_BASE_COMPONENT + 0x000E )
	#define E_NOT_AVAILABLE            ( E_BASE_COMPONENT + 0x000F )
	#define E_OVERFLOW                 ( E_BASE_COMPONENT + 0x0010 )
	#define E_INTERFACE_MISUSE         ( E_BASE_COMPONENT + 0x0011 )
	#define E_NO_INSTANCE              ( E_BASE_COMPONENT + 0x0012 )
	#define E_NOT_SERVICED             ( E_BASE_COMPONENT + 0x0013 )
	#define E_PORT_EMPTY               ( E_BASE_COMPONENT + 0x0014 )
	#define E_INVALID_INPUT            ( E_BASE_COMPONENT + 0x0015 )
	#define E_FUNC_NOT_FOUND           ( E_BASE_COMPONENT + 0x0016 )
	#define E_BAD_ARG_COUNT            ( E_BASE_COMPONENT + 0x0017 )
	#define E_BAD_ARG_TYPE             ( E_BASE_COMPONENT + 0x0018 )
	#define E_BAD_ARG_SIZE             ( E_BASE_COMPONENT + 0x0019 )



////////////////    SYMBOL MACROS

	#define S_NUMBER(symbol) (((symbol & S_TYPE_MASK) == S_TYPE_NUMBER) || ((symbol & S_TYPE_MASK) == S_TYPE_NEG_NUMBER))
	#define S_CONSTANT(symbol) ((symbol & S_TYPE_MASK) == S_TYPE_CONSTANT)
	#define S_ERROR(symbol) (((symbol & S_TYPE_MASK) == S_TYPE_ERROR) ? (symbol & (S_TYPE_MASK | S_ERROR_CODE_MASK)) : S_NULL)



////////////////    EVENT TYPES

	//	null event symbol
	#define EVENT_NULL                  ( C_BASE_COMPONENT_EVENT + 0x0000 )

	//	module level event symbols
	#define EVENT_MODULE_QUERY          ( C_BASE_COMPONENT_EVENT + 0x0001 )
	#define EVENT_MODULE_INIT           ( C_BASE_COMPONENT_EVENT + 0x0011 )
	#define EVENT_MODULE_TERM           ( C_BASE_COMPONENT_EVENT + 0x0012 )
	#define EVENT_MODULE_CREATE         ( C_BASE_COMPONENT_EVENT + 0x0013 )
	#define EVENT_MODULE_DUPLICATE      ( C_BASE_COMPONENT_EVENT + 0x0014 )
	#define EVENT_MODULE_DESTROY        ( C_BASE_COMPONENT_EVENT + 0x0015 )
	#define EVENT_MODULE_MAX            ( C_BASE_COMPONENT_EVENT + 0x00FF )  //  this helps an existing binding to decide whether to pass an unrecognised (new) event to the object, or just ignore it

	//	component event symbols
	#define EVENT_INIT_COMPLETE         ( C_BASE_COMPONENT_EVENT + 0x0101 )  //  final configuration information for component
	#define EVENT_STATE_SET             ( C_BASE_COMPONENT_EVENT + 0x0102 )  //  here's your state as an XML node
	#define EVENT_STATE_GET             ( C_BASE_COMPONENT_EVENT + 0x0103 )  //  please supply your state as an XML node

	//  process event symbols
	#define EVENT_INIT_PRECONNECT       ( C_BASE_COMPONENT_EVENT + 0x0111 )  //  the number of inputs in each set is now set
	#define EVENT_INIT_CONNECT          ( C_BASE_COMPONENT_EVENT + 0x0112 )  //  here are your inputs so far - create outputs if you can
	#define EVENT_INIT_POSTCONNECT      ( C_BASE_COMPONENT_EVENT + 0x0113 )  //  all links are now made (you can now safely take pointers to interface data* objects)
	#define EVENT_RUN_PLAY              ( C_BASE_COMPONENT_EVENT + 0x0121 )  //  STOP --> PAUSE
	#define EVENT_RUN_RESUME            ( C_BASE_COMPONENT_EVENT + 0x0122 )  //  PAUSE --> PLAY
	#define EVENT_RUN_SERVICE           ( C_BASE_COMPONENT_EVENT + 0x0123 )  //  service input/output interfaces
	#define EVENT_RUN_PAUSE             ( C_BASE_COMPONENT_EVENT + 0x0124 )  //  PLAY --> PAUSE
	#define EVENT_RUN_STOP              ( C_BASE_COMPONENT_EVENT + 0x0125 )  //  PAUSE --> STOP

	//  data event symbols
	#define EVENT_CONTENT_SET           ( C_BASE_COMPONENT_EVENT + 0x0131 )  //  here's your content as a binary stream
	#define EVENT_CONTENT_GET           ( C_BASE_COMPONENT_EVENT + 0x0132 )  //  please supply your content as a binary stream
	#define EVENT_LOG_INIT              ( C_BASE_COMPONENT_EVENT + 0x0141 )  //  initialise storage
	#define EVENT_LOG_SERVICE           ( C_BASE_COMPONENT_EVENT + 0x0142 )  //  service (increment) storage
	#define EVENT_LOG_TERM              ( C_BASE_COMPONENT_EVENT + 0x0143 )  //  terminate storage (and return it)

	//  generic event symbols
	#define EVENT_FUNCTION_GET          ( C_BASE_COMPONENT_EVENT + 0x0151 )  //  get handle to utility function (generic interface)
	#define EVENT_FUNCTION_CALL         ( C_BASE_COMPONENT_EVENT + 0x0152 )  //  call utility function (generic interface)
	#define EVENT_GENERIC_STRUCTURE_SET ( C_BASE_COMPONENT_EVENT + 0x0161 )
	#define EVENT_GENERIC_STRUCTURE_GET ( C_BASE_COMPONENT_EVENT + 0x0162 )
	#define EVENT_GENERIC_FORM_ADVANCE  ( C_BASE_COMPONENT_EVENT + 0x0163 )
	#define EVENT_GENERIC_FORM_CURRENT  ( C_BASE_COMPONENT_EVENT + 0x0164 )
	#define EVENT_GENERIC_CONTENT_SET   ( C_BASE_COMPONENT_EVENT + 0x0165 )
	#define EVENT_GENERIC_CONTENT_GET   ( C_BASE_COMPONENT_EVENT + 0x0166 )



////////////////    EVENTS

	#define F_FIRST_CALL                ( 0x00000001 )    //  first call of this event type
	#define F_LAST_CALL                 ( 0x00000002 )    //  last call of this event type
	#define F_GLOBAL_ERROR              ( 0x00000004 )    //  an error condition has arisen
	#define F_LOCAL_ERROR               ( 0x00000008 )    //  the receiving process has previously thrown

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

	#define F_NO_CONCURRENCY            ( 0x00000001 )	//	multiple instances of the component must not be executed concurrently
	#define F_NO_CHANGE_THREAD          ( 0x00000002 )	//	component must remain in the thread it starts in
	#define F_NO_CHANGE_VOICE           ( 0x00000004 )	//	component must remain in the voice it starts in

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

	#define F_UNDEFINED					( 0x00000001 )   //  set state to "undefined", whatever that means to you
	#define F_ZERO						( 0x00000002 )   //  set state to "zero", whatever that means to you

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

	#define F_ENCAPSULATED				( 0x00000001 )
        // INT32 flag
        #define PRECISION_NOT_SET			( 0x40000000 )

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

	#define F_MODIFIED ( 0x00000001 )

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



////////////////    LEGACY ENGINE EVENT SYMBOLS

	#define ENGINE_EVENT_SET_PORT_NAME		( C_BASE_ENGINE_EVENT + 0x8001 )
	#define ENGINE_EVENT_SET_PORT_SAMPLE_RATE	( C_BASE_ENGINE_EVENT + 0x8002 )
	#define ENGINE_EVENT_DATA_FROM_PORT		( C_BASE_ENGINE_EVENT + 0x8003 )
	#define ENGINE_EVENT_ASSERT_COMPONENT_SPEC	( C_BASE_ENGINE_EVENT + 0x8004 )
	#define ENGINE_EVENT_FIRE_EVENT_ON_DATA		( C_BASE_ENGINE_EVENT + 0x8005 )
	#define ENGINE_EVENT_GET_PORT_ON_SET		( C_BASE_ENGINE_EVENT + 0x8006 )
	#define ENGINE_EVENT_HANDLE_UTILITY_EVENT	( C_BASE_ENGINE_EVENT + 0x8007 )



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

                // symbols
	#define ENGINE_EVENT_STILL_ACTIVE	( C_BASE_ENGINE_EVENT + 0x0001 )
	#define ENGINE_EVENT_CREATE_UTILITY	( C_BASE_ENGINE_EVENT + 0x0002 )
	#define ENGINE_EVENT_GET_RANDOM_SEED	( C_BASE_ENGINE_EVENT + 0x0003 )
	#define ENGINE_EVENT_ERROR_MESSAGE	( C_BASE_ENGINE_EVENT + 0x0004 )
	#define ENGINE_EVENT_OUTPUT_MESSAGE	( C_BASE_ENGINE_EVENT + 0x0005 )
	#define ENGINE_EVENT_GET_SYMBOL_STRING	( C_BASE_ENGINE_EVENT + 0x0006 )
	#define ENGINE_EVENT_GET_TYPE_STRING	( C_BASE_ENGINE_EVENT + 0x0007 )

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

	#define F_TRACE      ( 0x00000001 ) // add trace message rather than error message
	#define F_DEBUGTRACE ( 0x00000002 ) // add debug-level trace message rather than error message

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

                // symbols
	#define ENGINE_EVENT_GET_SET		( C_BASE_ENGINE_EVENT + 0x0011 )
	#define ENGINE_EVENT_ADD_PORT		( C_BASE_ENGINE_EVENT + 0x0012 )
	#define ENGINE_EVENT_GET_PORT		( C_BASE_ENGINE_EVENT + 0x0013 )
	#define ENGINE_EVENT_GET_SET_INFO	( C_BASE_ENGINE_EVENT + 0x0014 )
	#define ENGINE_EVENT_GET_PORT_INFO	( C_BASE_ENGINE_EVENT + 0x0015 )

	//	index undefined
	#define INDEX_UNDEFINED ( 0xFFFFFFFF )

	//	interface flags
	#define F_IIF ( 0x00000001 )
	#define F_OIF ( 0x00000002 )

	//	port flags
	#define F_NO_STORE ( 0x00000001 )

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
	#define F_LISTENED	( 0x00000001 ) // o/p only
	#define F_IMPLICIT_NAME	( 0x00000001 ) // i/p only (so can have the same value as F_LISTENED)

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
