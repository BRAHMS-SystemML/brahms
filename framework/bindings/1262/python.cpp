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

	$Id:: python.cpp 2446 2010-01-12 16:57:41Z benjmitch       $
	$Rev:: 2446                                                $
	$Author:: benjmitch                                        $
	$Date:: 2010-01-12 16:57:41 +0000 (Tue, 12 Jan 2010)       $
________________________________________________________________

*/



/*

	WARNING!

	This code not currently thread-safe: see use of "BrahmsInstances"

*/


#define COMPONENT_PROCESS
#define COMPONENT_CLASS_CPP bindings__python_1262
#define BRAHMS_BUILDING_BINDINGS



//	python engine
#include "Python.h"
#include "frameobject.h"
#include "numpy/arrayobject.h"

//	deprecated, replaced (R2288, began giving deprecation warnings)
#undef PyArray_FromDims
#define PyArray_FromDims PyArray_SimpleNew
#undef PyArray_FromDimsAndDataAndDescr
#define PyArray_FromDimsAndDataAndDescr PyArray_NewFromDescr

//	common
#define ENGINE_OBJECT PyObject*
#define DESTROY_ENGINE_OBJECT Py_DECREF
#define BRAHMS_NO_LEGACY_SUPPORT

//	include component interface
#include "base/base.h"

//	include binding
#include "brahms-1199.h"
using namespace std;
using namespace brahms;

const TYPE TYPE_PREFERRED_STORAGE_FORMAT = TYPE_CPXFMT_INTERLEAVED | TYPE_ORDER_COLUMN_MAJOR;

struct Port
{
	Port()
	{
		mxPorts = NULL;
		form = S_NULL;
		mxData = NULL;
		bytes = 0;
		portIndex = 0;
		presented = false;
		hPort = S_NULL;
		type = TYPE_UNSPECIFIED;
		____CLEAR(hev);
	}

	~Port()
	{
		//	this belongs to us
		if (mxData) { Py_DECREF(mxData); }
	}

	Symbol fireNoCheck(Symbol type, UINT32 flags, void* data)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		return hev.handler(&hev.event);
	}

	Symbol fireCheck(Symbol type, UINT32 flags, void* data)
	{
		hev.event.type = type;
		hev.event.flags = flags;
		hev.event.data = data;
		Symbol err = hev.handler(&hev.event);
		if (S_ERROR(err)) throw err;
		return err;
	}

	//	store the passed object in this Port object (INCREF)
	void set(ENGINE_OBJECT obj)
	{
		//	check valid
		if (!obj) berr << E_INTERNAL;

		//	if object is already held, clear it
		if (mxData)
		{
			Py_DECREF(mxData);
			mxData = NULL;
		}

		//	store the new one
		mxData = obj;
		Py_INCREF(obj);
	}

	//	take the passed object back from this Port object (DECREF)
	void unset()
	{
		//	check valid
		if (!mxData) berr << E_INTERNAL;

		//	unset
		Py_DECREF(mxData);
		mxData = NULL;
	}

	//	return a borrowed pointer to the object (no effect on REF COUNT)
	ENGINE_OBJECT borrow()
	{
		//	check valid
		if (!mxData) berr << E_INTERNAL;

		//	ok
		return mxData;
	}

	//	present the stored object to the component (on the mxPorts array) in the right place
	//	the ref-counting is taken care of by the PyDict, so we can ignore this call in terms of REF COUNT
	void present()
	{
		//	if nothing set, error
		if (!mxData)
			berr << E_INTERNAL;

		//	if no mxPorts, error
		if (!mxPorts)
			berr << E_INTERNAL;

		//	otherwise, present (this causes INCREF)
		PyObject* mxPorts_p = PyList_GetItem(mxPorts, portIndex);
		if (!mxPorts_p) berr << E_INTERNAL;
		PyDict_SetItemString(mxPorts_p, "data", mxData);

		//	ok
		presented = true;
	}

	//	unpresent (hide) the stored object from the component
	//	the ref-counting is taken care of by the PyDict, so we can ignore this call in terms of REF COUNT
	void unpresent()
	{
		//	if not presented, no action
		if (!presented) return;

		//	if no mxPorts, error
		if (!mxPorts)
			berr << E_INTERNAL;

		//	otherwise, unpresent (this causes DECREF)
		PyObject* mxPorts_p = PyList_GetItem(mxPorts, portIndex);
		if (!mxPorts_p) berr << E_INTERNAL;
		PyDict_SetItemString(mxPorts_p, "data", Py_None);

		//	ok
		presented = false;
	}

	ENGINE_OBJECT mxPorts;
	Symbol form;
	UINT64 bytes;

	string name;
	Symbol hPort;
	UINT32 portIndex;

	HandledEvent hev;

	TYPE type;

private:

	ENGINE_OBJECT mxData;
	bool presented;

};

#include "../1258/common.h"

//	see PEP 353
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif





////////////////	COMPONENT INFORMATION

//	see error tag W_PYTHON_ENGINE_NOT_THREAD_SAFE
#define MODULE_MODULE_FLAGS F_NO_CONCURRENCY | F_NO_CHANGE_THREAD

//	per-module version information
const FrameworkVersion MODULE_VERSION_ENGINE = VERSION_ENGINE;

ModuleInfo MODULE_MODULE_INFO =
{
	&MODULE_VERSION_ENGINE,
	ARCH_BITS,
	1262,
	MODULE_MODULE_FLAGS
};





const UINT32 OUTPUT_BUFFER_SIZE = 16384;



//	static buffer for exceptions while no object exists
//	(also used as an error buffer in brahms.operation)
string staticErrorBuffer;

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	constructor and destructor, only if required
	COMPONENT_CLASS_CPP(EventModuleCreateBindings* emc);
	~COMPONENT_CLASS_CPP();

	Symbol event(Event* event);
	ComponentInfo* getThisComponentInfo();

	void newInputSetAdded(const char* name);
	void newOutputSetAdded(const char* name);

////////////////	MEMBERS

	//	methods
	void			fireWrappedFunction();

	//	helpers
	PyObject*		dataMLToPy(const DataMLNode& node);
	NPY_TYPES		dataType2arrayTypes(TYPE type);
	NPY_TYPES		genericNumericType2arrayTypes(TYPE type);
	TYPE			arrayTypes2dataType(PyArrayObject* obj);
	void			makeAllInputsAvailable();

	//	wrapped class
	string			className;
	string			releaseFolder;
	string			moduleFilename;

	//	Python objects
	PyObject*		pyBrahmsProcess;

	//	state
	PyObject*		pyInput;
	PyObject*		pyOutput;
	PyObject*		pyPersist;

	//	sub-fields of pyInput
	PyObject*		pyEvent;
	PyObject*		pyTime;					//	Process::time
	PyObject*		pyBsr;					//	Process::time::baseSampleRate
	PyObject*		pyIif;					//	Process::iif

	//	other vars set in fireWrappedFunction()
	Symbol			response;

	//	iif/oif
	vector<Set> iifx;
	vector<Set> oifx;

	//	utility objects requested by scripts
	vector<Utility> utilities;
	vector<UtilityFunction> utilityFunctions;

	//	string to cache info from object
	string				additionalInfo;
	string				externalLibraries;

	//	per-instance version info
	ComponentVersion	versionComponent;

	ComponentInfo componentInfo;
};




#include "python-support.cpp"



////////////////////////////////////////////////////////////////
//
//	MISCELLANEOUS
//
////////////////////////////////////////////////////////////////

ComponentInfo* COMPONENT_CLASS_CPP::getThisComponentInfo()
{
	____CLEAR(componentInfo);

	//	set in event type and flags to pyEvent
	addItem(pyEvent, "type", PyInt_FromLong(EVENT_MODULE_INIT));
	addItem(pyEvent, "flags", PyInt_FromLong(0));

	//	call function to service EVENT_MODULE_INIT
	fireWrappedFunction();
	if (response != C_OK) berr << E_NOT_SERVICED << "EVENT_MODULE_INIT";

	//	look for info field in output
	PyObject* pyInfo = PyDict_GetItemString(pyOutput, "info");
	if (!pyInfo || !PyDict_Check(pyInfo))
		____NOT_COMPLIANT("EVENT_MODULE_INIT", "invalid: info");

	//	zero everything
	componentInfo.cls = className.c_str();
	componentInfo.type = CT_PROCESS;
	componentInfo.componentVersion = &versionComponent;
	componentInfo.flags = 0;
	additionalInfo = "";
	externalLibraries = "";

	//	get all fields
	Py_ssize_t pyN = 0;
	PyObject* pyKey;
	PyObject* pyField;

	while (PyDict_Next(pyInfo, &pyN, &pyKey, &pyField))
	{
		if (!PyString_Check(pyKey))
			____NOT_COMPLIANT("EVENT_INFO", "invalid: info");

		char* key = PyString_AsString(pyKey);

		if (!strcmp(key, "component"))
		{
			//	should be a two-element tuple
			if (!PyTuple_Check(pyField) || PyTuple_GET_SIZE(pyField) != 2)
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.component");

			//	... containing integers
			PyObject* pyFirst = PyTuple_GET_ITEM(pyField, 0);
			PyObject* pySecond = PyTuple_GET_ITEM(pyField, 1);
			if (!PyInt_Check(pyFirst) || !PyInt_Check(pySecond))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.component");
			versionComponent.release = PyInt_AsUnsignedLongMask(pyFirst); // silent truncation
			versionComponent.revision = PyInt_AsUnsignedLongMask(pySecond); // silent truncation
		}

		else if (!strcmp(key, "flags"))
		{
			//	should be integer
			if (!PyInt_Check(pyField))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.flags");
			componentInfo.flags |= PyInt_AsUnsignedLongMask(pyField); // silent truncation
		}

		else if (!strcmp(key, "additional"))
		{
			//	add to info array
			if (!PyString_Check(pyField))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.additional");
			additionalInfo = PyString_AsString(pyField);
		}

		else if (!strcmp(key, "libraries"))
		{
			//	add to info array
			if (!PyString_Check(pyField))
				____NOT_COMPLIANT("EVENT_INFO", "invalid: info.libraries");
			externalLibraries = PyString_AsString(pyField);
		}

		else
		{
			____NOT_COMPLIANT("EVENT_INFO", "unrecognised field \"info." << key << "\"");
		}
	}

	//	also get python version (see /usr/include/python2.5/patchlevel.h)
	externalLibraries += "PYTHON=" PY_VERSION "\n";

	//	set pointers
	if (additionalInfo.length()) componentInfo.additional = additionalInfo.c_str();
	else componentInfo.additional = "";
	if (externalLibraries.length()) componentInfo.libraries = externalLibraries.c_str();
	else componentInfo.libraries = NULL;

	//	ok
	return &componentInfo;
}






void COMPONENT_CLASS_CPP::newInputSetAdded(const char* name)
{
	//	create set
	Symbol hSet = iif.getSet(name);
	iifx.resize(iifx.size() + 1);
	Set& set = iifx.back();
	set.hSet = hSet;
	set.name = iif.getSetName(hSet);

	//	get field name
	string fieldName = set.name;
	if (!fieldName.length()) fieldName = "default";

	//	create set field, add index field, add ports field
	set.mxSet = addItem(pyIif, fieldName.c_str(), PyDict_New());
}

void COMPONENT_CLASS_CPP::newOutputSetAdded(const char* name)
{
	//	create set
	Symbol hSet = oif.getSet(name);
	oifx.resize(oifx.size() + 1);
	Set& set = oifx.back();
	set.hSet = hSet;
	set.name = oif.getSetName(hSet);
}


////////////////////////////////////////////////////////////////
//
//	MAKE ALL INPUTS AVAILABLE
//
////////////////////////////////////////////////////////////////

/*

	The procedure is as follows:

		foreach (port)
		{
			make unavailable using unpresent()

			if (due)
			{
				if (first time through function, that is port.form not set yet, port.form == S_NULL)
				{
					attach port
					call EVENT_GENERIC_FORM_ADVANCE
					if (C_FORM_FIXED)
					{
						create array and keep it for the whole execution (store ref in port using port.set())
						set port.bytes to fixed byte-size of data
					}
					else
					{
						don't create array (store nothing in port, don't use port.set())
						leave port.bytes at zero (no information - assume C_FORM_UNBOUNDED even if C_FORM_BOUNDED)
					}
					set port.class (and lay in to mxPorts, for the user code to see)
					set port.structure (and lay in to mxPorts, for the user code to see)
				}

				if (C_FORM_FIXED)
				{
					update bytes in already-created array
					make available using present()
				}

				else
				{
					create new array
					make available using set(), present(), unset() (set() and unset() are used only as a convenience)
				}
			}
		}

*/

void COMPONENT_CLASS_CPP::makeAllInputsAvailable()
{
	for (UINT32 s=0; s<iifx.size(); s++)
	{
		//	extract
		Set& set = iifx[s];

		//	foreach port on set
		for (UINT32 p=0; p<set.size(); p++)
		{
			//	resolve port
			Port& port = *set[p];

			//	make unavailable
			port.unpresent();

			//	ensure attached
			if (port.form == S_NULL)
			{
				//	try attach
				if (!port.hev.event.object) continue;

				//	ok, attached: get form
				EventGenericForm egf;
				port.fireCheck(EVENT_GENERIC_FORM_ADVANCE, 0, &egf);
				port.form = egf.form;
				port.type = egf.type;

				//	special case (see note below)
				if (port.form == C_FORM_FIXED_BUT_LAST)
					port.form = C_FORM_BOUNDED;

				/*

				C_FORM_FIXED

					In the C_FORM_FIXED case, we can create the PyObject
					and just set its content through the GENERIC interface.

				C_FORM_UNBOUNDED

					In the C_FORM_UNBOUNDED case, we have to create the
					array anew on each call to makeAllInputsAvailable() since
					we can't know in advance how big it will be.

				C_FORM_BOUNDED (NOT YET SUPPORTED, CURRENTLY SAME AS UNBOUNDED)

					In the C_FORM_BOUNDED case, slightly non-intuitively,
					we can also create the PyObject and set its content through
					the GENERIC interface. We have to be careful though to
					also set its dimensions.

				C_FORM_FIXED
				C_FORM_BOUNDED

					To protect against segfaulting if the data object sends too
					much data back, we keep track of the actual capacity of the
					PyObject we've created in userDataB (in real bytes).

				C_FORM_FIXED_BUT_LAST

					We don't give this case special handling, currently, instead
					treating it as C_FORM_BOUNDED.

				*/

				//	for fixed-ish forms, create data array
				if (port.form == C_FORM_FIXED)
				{
					/*

						For these cases, we create the array in advance (for other
						cases, we create it on-the-fly).

					*/

					Dims b_dims = egf.dims;
					UINT64 count = b_dims.getNumberOfElements();
					switch (count)
					{
						case 0:
						{
							//	is empty
							npy_intp dims = 0;
							port.set(PyArray_FromDims(1, &dims, genericNumericType2arrayTypes(egf.type)));
							break;
						}

						case 1:
						{
							//	is scalar
							npy_intp dims = 1;
							port.set(PyArray_FromDims(1, &dims, genericNumericType2arrayTypes(egf.type)));
							break;
						}

						default:
						{
							//	convert dims if necessary
							vector<npy_intp> dims(b_dims.size());
							if (b_dims.size()) reverse_copy(b_dims.begin(), b_dims.end(), dims.begin());
							port.set(PyArray_FromDims(
								b_dims.size(),
								b_dims.size() ? &dims[0] : NULL,
								genericNumericType2arrayTypes(egf.type)));
						}
					}

					//	get byte capacity of this array
					port.bytes = b_dims.getNumberOfElements() * (TYPE_BYTES(egf.type));
				}

				//	otherwise, in the C_FORM_UNBOUNDED case, we have to create
				//	the mxArray anew at every call to this function

				//	set class into ports array
				EventGetPortInfo portInfo;
				portInfo.hPort = port.hPort;
				portInfo.flags = 0;
				portInfo.name = NULL;
				portInfo.componentInfo = NULL;

				EngineEvent event;
				event.hCaller = hComponent;
				event.flags = 0;
				event.type = ENGINE_EVENT_GET_PORT_INFO;
				event.data = (void*) &portInfo;

				/*brahms::Symbol result = */brahms::brahms_engineEvent(&event);
				addItem(PyList_GET_ITEM(port.mxPorts, p), "class", PyString_FromString(portInfo.componentInfo->cls));

				//	get data structure and set it in the ports array
				EventGenericStructure eas = {0};
				eas.type = TYPE_PREFERRED_STORAGE_FORMAT;
				port.fireCheck(EVENT_GENERIC_STRUCTURE_GET, 0, &eas);
				addItem(PyList_GET_ITEM(port.mxPorts, p), "structure", PyString_FromString(eas.structure));
			}

			//	if available, make available
			if (port.hev.event.object)
			{
				//	switch on mode
				switch(port.form)
				{

					case C_FORM_FIXED:
					{
						//	get content through generic access interface
						EventGenericContent egc = {0};
						egc.type = TYPE_PREFERRED_STORAGE_FORMAT;
						port.fireCheck(EVENT_GENERIC_CONTENT_GET, 0, &egc);

						//	interpret return
						if (egc.bytes && !egc.real)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no real data came back");

						/*
						storage format is complex interleaved, so imag will be NULL

						if (egc.bytes
							&& (arrayTypes2dataType((PyArrayObject*)port.borrow()) & TYPE_COMPLEX)
							&& !egc.imag)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no imaginary data came back");
						*/

						if (egc.bytes != port.bytes)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "wrong number of bytes came back, " << egc.bytes << " instead of " << port.bytes);

						if (port.bytes)
						{
							//	storage format is already native (interleaved), so just blat
							memcpy(((PyArrayObject*)port.borrow())->data, egc.real, port.bytes * TYPE_COMPLEX_MULT(port.type));

					/*		if (!egc.imag) memcpy(((PyArrayObject*)port.borrow())->data, egc.real, port.bytes);
							else interleaveComplexData(
								(BYTE*)((PyArrayObject*)port.borrow())->data,
								(BYTE*)egc.real,
								(BYTE*)egc.imag,
								PyArray_SIZE((PyArrayObject*)port.borrow()),
								((PyArrayObject*)port.borrow())->descr->elsize / 2);
								*/
						}

						//	present it on the ports array
						port.present();

						//	ok
						break;
					}

					case C_FORM_BOUNDED:
					case C_FORM_UNBOUNDED:
					{
						//	have to also set the dimensions anew
						EventGenericForm egf;
						port.fireCheck(EVENT_GENERIC_FORM_CURRENT, 0, &egf);

						//	create it...
						Dims b_dims = egf.dims;
						UINT64 count = b_dims.getNumberOfElements();
						UINT64 expectedBytes = count * (TYPE_BYTES(egf.type));

						//	can't currently do this check, since we don't properly support C_FORM_BOUNDED
						//	if ((port.form == C_FORM_BOUNDED) && (expectedBytes > port.bytes))
						//		____NOT_COMPLIANT("EVENT_GENERIC_FORM_CURRENT", "too many bytes came back for C_FORM_BOUNDED, " << expectedBytes << " was greater than " << port.bytes);

						switch (count)
						{
							case 0:
							{
								//	is empty
								npy_intp dims = 0;
								port.set(PyArray_FromDims(1, &dims, genericNumericType2arrayTypes(egf.type)));
								break;
							}

							case 1:
							{
								//	is scalar
								npy_intp dims = 1;
								port.set(PyArray_FromDims(1, &dims, genericNumericType2arrayTypes(egf.type)));
								break;
							}

							default:
							{
								//	convert dims if necessary
								vector<npy_intp> dims(b_dims.size());
								if (b_dims.size()) reverse_copy(b_dims.begin(), b_dims.end(), dims.begin());
								port.set(PyArray_FromDims(
									b_dims.size(),
									b_dims.size() ? &dims[0] : NULL,
									genericNumericType2arrayTypes(egf.type)));
							}
						}
						

						//	present it on the ports array
						port.present();

						//	get content through generic access interface
						EventGenericContent egc = {0};
						egc.type = TYPE_PREFERRED_STORAGE_FORMAT;
						port.fireCheck(EVENT_GENERIC_CONTENT_GET, 0, &egc);

						//	interpret return
						if (egc.bytes && !egc.real)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no real data came back");
						
						/*
						storage format is complex interleaved, so imag will be NULL

						if (egc.bytes
							&& (arrayTypes2dataType((PyArrayObject*)port.borrow()) & TYPE_COMPLEX)
							&& !egc.imag)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "no imaginary data came back");
							*/

						if (egc.bytes != expectedBytes)
							____NOT_COMPLIANT("EVENT_GENERIC_CONTENT_GET", "wrong number of bytes came back, " << egc.bytes << " instead of " << expectedBytes);

						//	copy in
						if (expectedBytes)
						{
							//	storage format is already native (interleaved), so just blat
							memcpy(((PyArrayObject*)port.borrow())->data, egc.real, expectedBytes * TYPE_COMPLEX_MULT(port.type));

						/*	if (!egc.imag) memcpy(((PyArrayObject*)port.borrow())->data, egc.real, expectedBytes);
							else interleaveComplexData(
								(BYTE*)((PyArrayObject*)port.borrow())->data,
								(BYTE*)egc.real,
								(BYTE*)egc.imag,
								b_dims.getNumberOfElements(),
								TYPE_BYTES(egf.type));
								*/
						}

						//	but in this case, we've no interest in keeping it
						//	it will remain in existence whilst it's in the mxPorts array (following
						//	the call to present()), but the next time we present(), or when we
						//	terminate, it'll be destroyed
						port.unset();

						//	ok
						break;
					}

					default:
					{
						berr << E_INTERNAL << "unexpected";
					}
				}

			}
		}
	}
}



////////////////////////////////////////////////////////////////
//
//	PYTHON OPERATIONS INTERFACE
//
////////////////////////////////////////////////////////////////

//	Operations from Python:
//
//		(1) import brahms once at the beginning
//		(2) call brahms.operation(persist['self'], opcode, ...)
//		(3) lather, rinse, and repeat
//
//	where persist['self'] points back to the calling instance.  Users need not worry about
//	persist['self'].  It is initialised in the constructor of COMPONENT_CLASS_CPP.


//	can't use berr because python don't have *this
#define perr(errmsg) { PyErr_SetString(PyExc_TypeError, (errmsg)); return NULL; }

//	one interface, multiple bindings
vector<COMPONENT_CLASS_CPP*> BrahmsInstances;


static PyObject* BrahmsOperations(PyObject* self, PyObject* args)
{
	if (!PyTuple_Check(args)) perr("invalid operation (not tuple)");
	Py_ssize_t Nargs = PyTuple_GET_SIZE(args);
	if (Nargs < 2) perr("invalid operation (empty)");
	PyObject* pySelf = PyTuple_GET_ITEM(args, 0);
	if (!PyInt_Check(pySelf)) perr("invalid operation (first element should be persist['self'])");
	unsigned long index = PyInt_AsUnsignedLongMask(pySelf);
	if (index >= BrahmsInstances.size()) perr("invalid operation (invalid persist['self']?)");
	COMPONENT_CLASS_CPP* that = BrahmsInstances[index];
	PyObject* pyOpCode = PyTuple_GET_ITEM(args, 1);
	if (!PyInt_Check(pyOpCode)) perr("invalid operation (second element should be an op code)");
	unsigned long opCode = PyInt_AsUnsignedLongMask(pyOpCode);

	switch(opCode)
	{
		case OPERATION_NULL:
			perr("null operation");

/*
		case OPERATION_IIF_CREATE_SET:
		{
			if (Nargs != 3) perr("invalid OPERATION_IIF_CREATE_SET (wrong args)");
			string setName;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), setName)) perr("OPERATION_IIF_CREATE_SET argument 2 should have been a string");

			that->iif.addSet(setName.c_str());
			that->newInputSetAdded(setName.c_str());

			that->bout << "created input set " << setName << D_VERB;
			break;
		}

		case OPERATION_OIF_CREATE_SET:
		{
			if (Nargs != 3) perr("invalid OPERATION_OIF_CREATE_SET (wrong args)");
			string setName;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), setName)) perr("OPERATION_OIF_CREATE_SET argument 1 should have been a string");
			
			that->oif.addSet(setName.c_str());
			that->newOutputSetAdded(setName.c_str());

			that->bout << "created output set " << setName << D_VERB;
			break;
		}
		*/

		case OPERATION_SET_CONTENT:
		{
			if (Nargs != 4) perr("OPERATION_SET_CONTENT expects 2 arguments");
			PyObject* pyPort = PyTuple_GET_ITEM(args, 2);
			PyObject* pyData = PyTuple_GET_ITEM(args, 3);

			//	extract port handle and thence data type
			if (!PyInt_Check(pyPort)) perr("invalid port handle");
			Symbol hPort = PyInt_AsUnsignedLongMask(pyPort);	//	silent truncation

			//	get port object
			UINT32 s = 0, p = 0;
			for (s=0; s<that->oifx.size(); s++)
			{
				Set& set = that->oifx[s];
				for (p=0; p<set.size(); p++)
				{
					Port& port = *set[p];
					if (port.hPort == hPort)
						break;
				}
				if (p != set.size()) break;
			}
			if (s == that->oifx.size()) berr << E_INVALID_ARG << "invalid port handle";
			Set& set = that->oifx[s];
			Port& port = *set[p];

			//	check that data is present
			if (!port.hev.event.object)
				berr << E_PORT_EMPTY;

			//	check integrity
			if (!PyArray_Check(pyData))
				perr("invalid OPERATION_SET_CONTENT (data argument should be a NumPy array)");

			//	check integrity
			if (!PyArray_ISCONTIGUOUS((PyArrayObject*)pyData))
				perr("invalid OPERATION_SET_CONTENT (data argument is a NumPy array, but is not contiguous - currently, i can't handle this)");

			//	so we can do this
			PyObject* pyContiguousData = pyData;

/*

TODO: we can probably safely handle non-contigous arrays, given these notes:

retrieved from http://modular.fas.harvard.edu/docs/debian-packages/python-numeric/html/numpy-13.html 13/12/09

Contiguous arrays

An important special case of a NumPy array is the contiguous array. This is an array whose elements occupy a
single contiguous block of memory and have the same order as a standard C array. (Internally, this is decided
by examinging the array array->strides. The value of array->strides[i] is equal to the number of bytes that
one must move to get to an element when the i'th index is increased by 1). Arrays that are created from scratch
are always contiguous; non-contiguous arrays are the result of indexing and other structural array operations.

The main advantage of contiguous arrays is easier handling in C; the pointer array->data is cast to the required
type and then used like a C array, without any reference to the stride values. This is particularly important
when interfacing to existing libraries in C or Fortran, which typically require this standard data layout. A
function that requires input arrays to be contiguous must call the conversion function

PyArray_ContiguousFromObject()

described in the section "Accepting input data from any sequence type".

			//	check integrity
			PyObject* pyContiguousData = pyData;
			bool mustDecRef = false;
			if (!PyArray_ISCONTIGUOUS((PyArrayObject*)pyData))
			{
				pyContiguousData = PyArray_ContiguousFromObject(pyData, PyArray_NOTYPE, 0, 0);
				mustDecRef = true;
			}

however, i tried the above code, and it didn't work. got the ordering all wrong.

*/


			UINT32 count = PyArray_SIZE((PyArrayObject*)pyContiguousData);
			UINT32 bytesPerElement = TYPE_BYTES(port.type); // ((PyArrayObject*)pyContiguousData)->descr->elsize

			//	prepare the call
			EventGenericContent eac = {0};
			eac.type = TYPE_PREFERRED_STORAGE_FORMAT;
			eac.bytes = count * bytesPerElement;

			//	prepare de-interleaved data
			VBYTE content;
			if (count)
			{
				/*
				if (that->arrayTypes2dataType((PyArrayObject*)pyContiguousData) & TYPE_COMPLEX)
				{
					content.resize(eac.bytes);
					eac.real = &content[0];
					eac.imag = &content[eac.bytes / 2];
					deinterleaveComplexData(
						(BYTE*)eac.real,
						(BYTE*)eac.imag,
						(BYTE*)((PyArrayObject*)pyContiguousData)->data,
						count,
						bytesPerElement / 2);
						eac.bytes /= 2;
				}
				else
				*/
					
				eac.real = ((PyArrayObject*)pyContiguousData)->data;
			}

			//	make the call
//			cout << hex << UINT64(eac.real) << ", " << UINT64(eac.imag) << ", " << ((DOUBLE*)eac.real)[1] << endl;
			port.fireCheck(EVENT_GENERIC_CONTENT_SET, 0, &eac);

			//	ok
			break;
		}

		case OPERATION_ADD_PORT:
		{
			if (Nargs < 5 || Nargs > 7) perr("OPERATION_ADD_PORT expects 3 to 5 arguments");
			string setName;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), setName)) perr("OPERATION_ADD_PORT argument 1 should have been a string");
			string className;
			if (!pyToString(PyTuple_GET_ITEM(args, 3), className)) perr("OPERATION_ADD_PORT argument 2 should have been a string");
			string dataStructure;
			if (!pyToString(PyTuple_GET_ITEM(args, 4), dataStructure)) perr("OPERATION_ADD_PORT argument 3 should have been a string");

			//	optional arg port name
			string portName = "";
			if (Nargs >= 6)
				if (!pyToString(PyTuple_GET_ITEM(args, 5), portName)) perr("OPERATION_ADD_PORT argument 4 should have been a string");

			//	optional arg sample rate
			SampleRate sr = that->time->sampleRate;
			if (Nargs >= 7)
			{
				PyObject* pyArg = PyTuple_GET_ITEM(args, 6);
				if (PyInt_Check(pyArg))
				{
					unsigned long sru = PyInt_AsUnsignedLongMask(pyArg);
					if (sru <= 0)
						perr("non-positive sample rates are illegal");
					sr.num = sru;
					sr.den = 1;
				}
				else if (PyFloat_Check(pyArg))
				{
					double srd = PyFloat_AsDouble(pyArg);
					if (srd != floor(srd))
						perr("specify non-integral sample rates as a fraction (e.g. '513/10') rather than as a floating point value");
					if (srd <= 0.0)
						perr("non-positive sample rates are illegal");
					sr.num = srd;
					sr.den = 1;
				}
				else if (PyString_Check(pyArg))
				{
					char* srs = PyString_AsString(pyArg);
					if (!srs) perr("OPERATION_ADD_PORT argument 5 was an invalid string");
					string err = stringToSampleRate(srs, sr);
					if (err.length()) perr(err.c_str());
				}
				else perr("OPERATION_ADD_PORT argument 5 should be a scalar numeric or a string");
			}

			//	get set handle
			Symbol hSet = that->oif.getSet(setName.c_str());
			UINT32 s = 0;
			for (s=0; s<that->oifx.size(); s++)
			{
				if (that->oifx[s].hSet == hSet) break;
			}
			if (s == that->oifx.size()) berr << E_INTERNAL;
			Set& set = that->oifx[s];
			UINT32 portIndex = set.size();

			//	create new port
			set.push_back(new Port);
			Port& port = *set.back();

			//	create port
			port.portIndex = portIndex;
			port.hPort = that->oif.addPortHEV(hSet, className.c_str(), 0, &port.hev);
			if (portName.length()) that->oif.setPortName(port.hPort, portName.c_str());
			if (sr.num) that->oif.setPortSampleRate(port.hPort, sr);
			port.mxPorts = set.mxPorts;

			//	set structure
			EventGenericStructure eas = {0};
			eas.structure = dataStructure.c_str();
			eas.type = TYPE_PREFERRED_STORAGE_FORMAT;
			Symbol result = port.fireNoCheck(EVENT_GENERIC_STRUCTURE_SET, 0, &eas);

			if (S_ERROR(result))
			{
				std::ostringstream serr;
				serr << result << "[code|msg]failed set data structure";
				staticErrorBuffer = serr.str();
				perr(staticErrorBuffer.c_str());
			}

			//	complete generic type is returned from the call
			port.type = eas.type;

			//	ok
			return PyInt_FromLong(port.hPort);
		}

		case OPERATION_BOUT:
		{
			if (Nargs != 4) perr("OPERATION_BOUT expects 2 arguments");
			string out;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), out)) perr("OPERATION_BOUT argument 1 should have been a string");
			PyObject* pyLevel = PyTuple_GET_ITEM(args, 3);
			if (!PyInt_Check(pyLevel)) perr("OPERATION_BOUT argument 2 should be a detail level constant");
			EnumDetailLevel level = (EnumDetailLevel)PyInt_AsUnsignedLongMask(pyLevel);	//	silent truncation
			that->bout << out << level;
			break;
		}

		case OPERATION_GET_UTILITY_OBJECT:
		{
			//	new resource...
			Utility utility;
			utility.handle = that->utilities.size();

			//	wrapped script is asking for a utility object - let's fetch one, soldier!!!
			if (Nargs != 4) perr("OPERATION_GET_UTILITY_OBJECT expects 2 arguments");
			string cls;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), cls)) perr("OPERATION_GET_UTILITY_OBJECT argument 1 should have been a string");
			UINT16 release;
			if (!pyToUINT16(PyTuple_GET_ITEM(args, 3), release)) perr("OPERATION_GET_UTILITY_OBJECT argument 2 should have been a real scalar UINT16");

			//	ask framework to create utility
			EventCreateUtility data;
			data.flags = 0;
			data.hUtility = S_NULL;
			data.spec.cls = cls.c_str();
			data.spec.release = release;
			data.name = NULL;
			data.handledEvent = &utility.hev;

			EngineEvent event;
			event.hCaller = that->hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_CREATE_UTILITY;
			event.data = (void*) &data;

			utility.hUtility = brahms::brahms_engineEvent(&event);
			if (S_ERROR(utility.hUtility))
			{			  
			  std::ostringstream serr;
			  serr << utility.hUtility << "[code|msg]failed create utility";
			  staticErrorBuffer = serr.str();
			  perr(staticErrorBuffer.c_str());
			}

			//	and place the new object in the objects array
			that->utilities.push_back(utility);

			//	ok
			return PyInt_FromLong(utility.handle);
		}

		case OPERATION_GET_UTILITY_FUNCTION:
		{
			//	new resource...
			UtilityFunction function;
			function.clientHandle = that->utilityFunctions.size();

			//	wrapped script is asking for a utility function - let's find it one, marine!!!
			if (Nargs != 4) perr("OPERATION_GET_UTILITY_FUNCTION expects 2 arguments");
			UINT32 utilityHandle;
			if (!pyToUINT32(PyTuple_GET_ITEM(args, 2), utilityHandle)) perr("OPERATION_GET_UTILITY_FUNCTION argument 1 should have been a real scalar UINT16");
			if (!pyToString(PyTuple_GET_ITEM(args, 3), function.identifier)) perr("OPERATION_GET_UTILITY_FUNCTION argument 2 should have been a string");

			//	access object resource
			if (utilityHandle >= that->utilities.size()) perr("OPERATION_GET_UTILITY_FUNCTION, invalid utility handle");
			Utility* utility = &that->utilities[utilityHandle];
			function.utility = utility;

			//	ask utility object for function
			EventFunctionGet egf = {0};
			egf.name = function.identifier.c_str();
			try
			{
				utility->fireCheck(EVENT_FUNCTION_GET, 0, &egf);
			}
			catch(Symbol e)
			{
				stringstream ss;
				ss << "\"" << function.identifier << "\"";
				string s = ss.str();
				EventErrorMessage em;
				em.error = e;
				em.msg = s.c_str();
				em.flags = 0;

				EngineEvent event;
				event.hCaller = that->hComponent;
				event.flags = 0;
				event.type = ENGINE_EVENT_ERROR_MESSAGE;
				event.data = &em;
				Symbol result = brahms_engineEvent(&event);

			  std::ostringstream serr;
			  serr << result << "[code|msg]failed get utility function";
			  staticErrorBuffer = serr.str();
			  perr(staticErrorBuffer.c_str());

			}
			function.moduleHandle = egf.handle;
			UINT32 count = egf.argumentModifyCount;
			if (!function.moduleHandle) perr("unexpected response from utility object (zero event type)");

			//	generate the python objects we need...
			function.mxFunction = PyList_New(count);

			//	and place the new function in the functions array
			that->utilityFunctions.push_back(function);

			//	ok
			return PyInt_FromLong(function.clientHandle);
		}

		case OPERATION_CALL_UTILITY_FUNCTION:
		{
			if (Nargs != 4) perr("OPERATION_CALL_UTILITY_FUNCTION expects 2 arguments");
			UINT32 functionHandle;
			if (!pyToUINT32(PyTuple_GET_ITEM(args, 2), functionHandle)) perr("OPERATION_CALL_UTILITY_FUNCTION argument 1 should have been a real scalar UINT16");
			PyObject* pyArgs = PyTuple_GET_ITEM(args, 3);
			if (!PyTuple_Check(pyArgs)) perr("second argument should be a tuple of function arguments");

			//	access function resource
			if (functionHandle >= that->utilityFunctions.size()) perr("OPERATION_CALL_UTILITY_FUNCTION argument 1, invalid function handle");
			UtilityFunction* function = &that->utilityFunctions[functionHandle];

			//	prepare the argument list
			UINT32 nArgs = PyTuple_Size(pyArgs);

			vector<Argument> argsx;
			vector<Dims> argdimsx;
			vector<string> argstring;
			vector<VBYTE> contents(nArgs);

			for (UINT32 a=0; a<nArgs; a++)
			{
				//	extract ath argument
				PyObject* pyArg = PyTuple_GET_ITEM(pyArgs, a);

				//	string args
				if (PyString_Check(pyArg))
				{
					UINT32 N = PyString_Size(pyArg);

					Argument arg = {0};

					//	set type
					arg.type = TYPE_CHAR8 | TYPE_REAL | TYPE_CPXFMT_INTERLEAVED | TYPE_ORDER_COLUMN_MAJOR;

					//	set dims
					Dims dimsx;
					dimsx.push_back(N);
					argdimsx.push_back(dimsx);
					arg.dims.count = 1;

					//	store string
					string s;
					if (N) pyToString(pyArg, s);
					argstring.push_back(s);

					//	ok
					argsx.push_back(arg);
					continue;
				}

				//	push on empty string, for convenience lower down
				argstring.push_back("");

				//	numeric args
				if (!PyArray_Check(pyArg) || !PyArray_ISCONTIGUOUS((PyArrayObject*)pyArg))
					perr("all arguments to function should be contiguous arrays");

				//	start creating new argument object
				Argument arg = {0};

				//	extract dimensions of python array
				UINT32 nDims = ((PyArrayObject *)pyArg)->nd;
				npy_intp* pdims = ((PyArrayObject *)pyArg)->dimensions;

				//	reverse dimensions
				Dims argdims;
				for (UINT32 i=0; i<nDims; i++)
					argdims.push_back(pdims[nDims - i - 1]);
				argdimsx.push_back(argdims);

				//	set the numeric type of the argument
				arg.type = that->arrayTypes2dataType((PyArrayObject*)pyArg) | TYPE_CPXFMT_INTERLEAVED | TYPE_ORDER_COLUMN_MAJOR;

				//	handle complex and real cases the same, since complex data is interleaved
				arg.real = ((PyArrayObject*)pyArg)->data;
				arg.imag = NULL;

				//	push arg
				argsx.push_back(arg);
			}

			//	fix up pointers to dims
			for (UINT32 a=0; a<nArgs; a++)
			{
				INT32 nDims = argdimsx[a].size();

				//	python can represent a scalar with an empty dims array, we convert for BRAHMS, where this is not valid...
				if (!nDims)
				{
					argdimsx[a] = Dims(1);
					nDims = 1;
				}

				//	set pointers *after* making all vector extensions
				if (nDims) argsx[a].dims.dims = &argdimsx[a][0];
				else argsx[a].dims.dims = 0;
				argsx[a].dims.count = nDims;

				//	fix up pointers to strings
				UINT32 sLength = argstring[a].length();
				if (sLength) argsx[a].real = (void*) argstring[a].c_str();
			}

			//	make the call
			EventFunctionCall ecf = {0};
			ecf.handle = function->moduleHandle;
			ecf.argumentCount = nArgs;
			std::vector<Argument*> tempargs;
			for (UINT32 a=0; a<nArgs; a++)
				tempargs.push_back(&argsx[a]);
			ecf.arguments = &tempargs[0];
			Symbol err = function->utility->fireNoCheck(EVENT_FUNCTION_CALL, 0, &ecf);

			if (S_ERROR(err))
			{
				std::ostringstream serr;

				switch(err)
				{
					case E_BAD_ARG_COUNT:
						serr << E_BAD_ARG_COUNT << "[code|msg]expected " << ecf.argumentCount << " arguments";
						break;

					case E_BAD_ARG_SIZE:
						serr << E_BAD_ARG_SIZE << "[code|msg]argument " << (ecf.argumentCount + 1);
						break;

					case E_BAD_ARG_TYPE:
						serr << E_BAD_ARG_TYPE << "[code|msg]argument " << (ecf.argumentCount + 1);
						break;

					default:
						serr << err << "[code|msg]";
				}

				staticErrorBuffer = serr.str();
				perr(staticErrorBuffer.c_str());
			}

			if (err == S_NULL)
			{
				std::ostringstream serr;
				serr << "<code>" << E_NOT_SERVICED << "</code>the utility did not service EVENT_FUNCTION_CALL";
				staticErrorBuffer = serr.str();
				perr(staticErrorBuffer.c_str());
			}

			//	place the outputs (list of all modified args: additional
			//	args returned will be flagged as modified, too)

			//	first, count them
			UINT32 count = 0;
			for (UINT32 a=0; a<argsx.size(); a++)
				if (argsx[a].flags & F_MODIFIED) count++;

			//	next, make sure we've got the right number of output cells
			if (
				!function->mxFunction
				||
				!PyList_Check(function->mxFunction)
				||
				PyList_GET_SIZE(function->mxFunction) != ((INT32)count)	
				)
				perr("internal error (probably bad response from utility object)");

			//	for each array, check the size, resize if necessary, and fill it
			count = 0;
			for (UINT32 a=0; a<argsx.size(); a++)
			{
				if (argsx[a].flags & F_MODIFIED)
				{
					//	get cell
					PyObject* pyItem = PyList_GET_ITEM(function->mxFunction, count);

					//	extract some data about the argument
					//	bool complex = argsx[a].type & TYPE_COMPLEX;
					NPY_TYPES pyTypes = that->dataType2arrayTypes(argsx[a].type);

					//	break loop to recreate
					bool recreate = true;
					while (pyItem)
					{
						//	type checks
						if (!PyArray_Check(pyItem)
							|| pyTypes != (NPY_TYPES)((PyArrayObject*)pyItem)->descr->type_num)
						break;

						//	check dims
						UINT32 ndims = ((PyArrayObject*)pyItem)->nd;
						if (ndims != argsx[a].dims.count) break;
						npy_intp* pdims = ((PyArrayObject*)pyItem)->dimensions;
						bool mismatch = false;
						for (UINT32 d=0; d<ndims; d++)
						{
							if (pdims[d] != argsx[a].dims.dims[d])
							{
								mismatch = true;
								break;
							}
						}
						if (mismatch) break;

						//	ok, happy to keep the one we've got
						recreate = false;
						break;
					}

					//	recreate?
					if (recreate)
					{
						//	pyItem is deallocated by PyList_SetItem below (i.e. explicit free is not needed)
						INT32 asize = argsx[a].dims.count;
						vector<npy_intp> dims(asize);
						for (INT32 d=0; d<asize; d++)
							dims[d] = argsx[a].dims.dims[asize - d - 1];
						pyItem = PyArray_FromDims(asize, dims.size() ? &dims[0] : NULL, pyTypes);
						if (!pyItem) perr("failed to allocate pyItem");
						PyList_SetItem(function->mxFunction, count, pyItem);
					}

					//	copy stuff across
					Dims tempdims(argsx[a].dims);
					UINT64 nEls = tempdims.getNumberOfElements();
					UINT32 sEl = TYPE_BYTES(argsx[a].type);
					UINT32 bytes = nEls * sEl;

					if (bytes)
					{
						//	handle real and complex the same way, since we use interleaved storage
						memcpy(((PyArrayObject*)pyItem)->data, argsx[a].real, bytes * TYPE_COMPLEX_MULT(argsx[a].type));

						/*
						if (!complex) memcpy(((PyArrayObject*)pyItem)->data, argsx[a].real, bytes);
						else
						{
							interleaveComplexData(
								(BYTE*)((PyArrayObject*)pyItem)->data,
								(BYTE*)argsx[a].real,
								(BYTE*)argsx[a].imag,
								nEls,
								sEl
								);
						}
						*/
					}

					//	increment
					count++;
				}
			}

			//	ok
			Py_INCREF(function->mxFunction);
			return function->mxFunction;
		}

/*
		case OPERATION_GET_PROCESS_STATE:
		{
			//	wrapped script wants to access a peer's StateML (flags arg is optional)
			if (Nargs != 4 && Nargs != 5) perr("invalid OPERATION_GET_PROCESS_STATE (wrong arg count)");
			string relativeIdentifier;
			if (!pyToString(PyTuple_GET_ITEM(args, 2), relativeIdentifier)) perr("OPERATION_GET_PROCESS_STATE argument 1 should have been a string");
			string expectedClassName;
			if (!pyToString(PyTuple_GET_ITEM(args, 3), expectedClassName)) perr("OPERATION_GET_PROCESS_STATE argument 2 should have been a string");
			UINT16 flags = 0;
			if (Nargs == 5)
				if (!pyToUINT16(PyTuple_GET_ITEM(args, 4), flags)) perr("OPERATION_GET_PROCESS_STATE argument 3 should have been a scalar UINT16");

			//	make the call...
			GetProcessState gps = {0};
			gps.hCaller = that->hComponent;
			gps.identifier = relativeIdentifier.c_str();
			gps.spec.cls = expectedClassName.c_str();
			gps.spec.release = 0;
			gps.flags = flags;
			Symbol processState = getProcessState(&gps);

			//	field name is relative identifier, grepped...
			grep(relativeIdentifier, "/", "_");

			//	get state
			XMLNode xmlnode(processState);
			DataMLNode mnode(&xmlnode);
			PyObject* pyState = that->dataMLToPy(mnode);

			//	ok
			return pyState;
		}
		*/

		case OPERATION_GET_SYMBOL_STRING:
		{
			if (Nargs != 3) perr("OPERATION_GET_SYMBOL_STRING expects 1 argument");
			Symbol symbol;
#if ARCH_BITS == 32
			if (!pyToUINT32(PyTuple_GET_ITEM(args, 2), symbol))
				perr("OPERATION_GET_SYMBOL_STRING argument 1 should have been a real scalar UINT32");
#endif
#if ARCH_BITS == 64
			if (!pyToUINT64(PyTuple_GET_ITEM(args, 2), symbol))
				perr("OPERATION_GET_SYMBOL_STRING argument 1 should have been a real scalar UINT64");
#endif
			const char* symbolString = getSymbolString(symbol);
			if (!symbolString)
				perr("invalid symbol passed to OPERATION_GET_SYMBOL_STRING");
			return PyString_FromString(symbolString);
		}

		case OPERATION_GET_RANDOM_SEED:
		{
			VUINT32 seed;

			if (Nargs == 2)
			{
				//	send back as many as are available
				seed = that->getRandomSeed();
			}
			else if (Nargs == 3)
			{
				//	send back as many as were requested
				UINT32 count;
				if (!pyToUINT32(PyTuple_GET_ITEM(args, 2), count))
					perr("OPERATION_GET_RANDOM_SEED argument 1 should have been a real scalar UINT32");
				seed = that->getRandomSeed(count);
			}
			else perr("OPERATION_GET_RANDOM_SEED expects 0 or 1 arguments");

			//	return this VUINT32 to the caller
			npy_intp dims = seed.size();
			PyObject* pyRet = PyArray_FromDims(1, &dims, NPY_UINT);
			if (!pyRet) perr("internal error");
			memcpy(((PyArrayObject*)pyRet)->data, &seed[0], dims * sizeof(UINT32));
			return pyRet;
		}

		default:
			perr("unrecognised operation");
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyMethodDef BrahmsMethods[] = {
	{ "operation", BrahmsOperations, METH_VARARGS, "BRAHMS operations." },
	{ NULL, NULL, 0, NULL }
};



////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////

#ifdef __OSX__

static int brahms_____import_array(void)
{
	PyObject *numpy = PyImport_ImportModule("numpy.core.multiarray");
	PyObject *c_api = NULL;
	if (numpy == NULL) return -1;
	c_api = PyObject_GetAttrString(numpy, "_ARRAY_API");
	if (c_api == NULL) {Py_DECREF(numpy); return -1;}
	if (PyCObject_Check(c_api)) {
		PyArray_API = (void **)PyCObject_AsVoidPtr(c_api);
	}
	Py_DECREF(c_api);
	Py_DECREF(numpy);
	if (PyArray_API == NULL) return -1;
	/* Perform runtime check of C API version */
	if (NPY_VERSION != PyArray_GetNDArrayCVersion()) {
		PyErr_Format(PyExc_RuntimeError, "module compiled against "\
					 "version %x of C-API but this version of numpy is %x", \
					 (int) NPY_VERSION, (int) PyArray_GetNDArrayCVersion());
		return -1;
	}
	
	/* 
	 * Perform runtime check of endianness and check it matches the one set by
	 * the headers (npy_endian.h) as a safeguard 
	 */
	
	/* DONT DO THIS BIT it segfaults
	 
	int st = PyArray_GetEndianness();
	if (st == NPY_CPU_UNKNOWN_ENDIAN) {
		PyErr_Format(PyExc_RuntimeError, "FATAL: module compiled as unknown endian");
		return -1;
	}
#ifdef NPY_BIG_ENDIAN
	if (st != NPY_CPU_BIG) {
		PyErr_Format(PyExc_RuntimeError, "FATAL: module compiled as "\
					 "big endian, but detected different endianness at runtime");
		return -1;
	}
#elif defined(NPY_LITTLE_ENDIAN)
	if (st != NPY_CPU_LITTLE) {
		PyErr_Format(PyExc_RuntimeError, "FATAL: module compiled as"\
					 "little endian, but detected different endianness at runtime");
		return -1;
	}
#endif
	 */
	
	return 0;
}

#define brahms____import_array() {if (brahms_____import_array() < 0) {PyErr_Print(); PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import"); return; } }

#endif

COMPONENT_CLASS_CPP::COMPONENT_CLASS_CPP(EventModuleCreateBindings* emc)
{
	ComponentVersion v = {-1, -1};
	versionComponent = v;

	//	lay in path information
	releaseFolder = emc->wrapped.namespaceRootPath + string("/") + emc->wrapped.componentClass + string("/") + emc->wrapped.releasePath;
	moduleFilename = emc->wrapped.moduleFilename;
	grep(releaseFolder, "\\", "/");

	//	lay in class information
	className = emc->wrapped.componentClass;

	//	initialise "engine"
	int engine = Py_IsInitialized();
	if (!engine)
	{
		/*
		 
		 WHAT GOES WRONG
		 
		 on stuart's laptop (python 2.6), import_array() calls PyArray_GetEndianness(),
		 which is PyArray_API[210]. for reasons unknown, 210 is NULL (as is 209); only
		 208 and below are non-NULL. thus, we segfault.
		 
		 see implementation in __multiarray_api.h in the numpy core
		 
		 CORRECT SOLUTION
		 
		 find out why PyArray_GetEndianness()
		 does not get a non-NULL in the call to get the API
		 
		 WORKAROUND
		 
		 avoid the call to PyArray_GetEndianness() by recoding the import_array call
		 (we do this just above this function)
		 
		 */
		
		Py_Initialize();
		
#ifdef __OSX__
		brahms____import_array();
		
		//	avoid compiler warning/error
		if (6 == 7)
			import_array();
#else
		import_array();
#endif
	    Py_InitModule("brahms", BrahmsMethods);
	}

	//	create input object
	pyInput = PyDict_New();
	if (!pyInput) berr << E_MEMORY;

	//	create output object
	Py_INCREF(Py_None);
	pyOutput = Py_None;

	//	create persist object
	pyPersist =  PyDict_New();
	if (!pyPersist) berr << E_MEMORY;

	//	run classFile once to get function definition
	string fullPath = releaseFolder + string("/") + moduleFilename + ".py";

/*

	(TAK'S CODE) falls over, perhaps because...

	http://effbot.org/pyfaq/pyrun-simplefile-crashes-on-windows-but-not-on-unix-why.htm

	FILE* file = fopen(fullPath.c_str(), "r");
	if (!file) berr << "unable to open python script file";
	if (PyRun_SimpleFile(file, classFile.c_str()))
	{
		berr << "unable to run file";
	}
	fclose(file);

	so mitch replaced it with the following, as suggested on that web page:

*/

	//	add binary folder to python path, in case the user wants to import a module or two
	string cmd;
	cmd = "sys.path.append(\"" + releaseFolder + "\")";
	grep(cmd, "\\", "/");
	if (PyRun_SimpleString("import sys"))
		berr << E_PYTHON << "error whilst sys.path.appending class path";
	if (PyRun_SimpleString(cmd.c_str()))
		berr << E_PYTHON << "error whilst sys.path.appending class path";

	//	run python script file
	grep(fullPath, "\\", "/");
	cmd = "execfile('" + fullPath + "')";
	if (PyRun_SimpleString(cmd.c_str()))
		berr << E_PYTHON << "unable to run python script file \"" << fullPath << "\"";;

/*

	END REPLACED BY MITCH

*/

	//	get main module and its namespace
	PyObject* pyMain = PyImport_AddModule("__main__");
	if (!pyMain) berr << E_PYTHON << "unable to add main module";
	PyObject* pyDict = PyModule_GetDict(pyMain);

	//	steal the brahms process from main
	if (!(pyBrahmsProcess = PyDict_GetItemString(pyDict, "brahms_process")) || !PyCallable_Check(pyBrahmsProcess))
		berr << "\"brahms_process\" not defined in file (or defined but not callable)";
	Py_INCREF(pyBrahmsProcess);
	PyDict_DelItemString(pyDict, "brahms_process");

	//	add contants (on first call)
	if (!engine)
	{
		PyModule_AddIntConstant(pyMain, "EVENT_MODULE_INIT", EVENT_MODULE_INIT);
		PyModule_AddIntConstant(pyMain, "EVENT_STATE_GET", EVENT_STATE_GET);
		PyModule_AddIntConstant(pyMain, "EVENT_STATE_SET", EVENT_STATE_SET);
		PyModule_AddIntConstant(pyMain, "EVENT_INIT_PRECONNECT", EVENT_INIT_PRECONNECT);
		PyModule_AddIntConstant(pyMain, "EVENT_INIT_CONNECT", EVENT_INIT_CONNECT);
		PyModule_AddIntConstant(pyMain, "EVENT_INIT_POSTCONNECT", EVENT_INIT_POSTCONNECT);
		PyModule_AddIntConstant(pyMain, "EVENT_LOG_INIT", EVENT_LOG_INIT);
		PyModule_AddIntConstant(pyMain, "EVENT_BEGIN_RUNPHASE", EVENT_BEGIN_RUNPHASE);
		PyModule_AddIntConstant(pyMain, "EVENT_BEGIN_TERMPHASE", EVENT_BEGIN_TERMPHASE);
		PyModule_AddIntConstant(pyMain, "EVENT_RUN_PLAY", EVENT_RUN_PLAY);
		PyModule_AddIntConstant(pyMain, "EVENT_RUN_RESUME", EVENT_RUN_RESUME);
		PyModule_AddIntConstant(pyMain, "EVENT_RUN_SERVICE", EVENT_RUN_SERVICE);
		PyModule_AddIntConstant(pyMain, "EVENT_RUN_PAUSE", EVENT_RUN_PAUSE);
		PyModule_AddIntConstant(pyMain, "EVENT_RUN_STOP", EVENT_RUN_STOP);
		PyModule_AddIntConstant(pyMain, "EVENT_LOG_TERM", EVENT_LOG_TERM);
		PyModule_AddIntConstant(pyMain, "F_FIRST_CALL", F_FIRST_CALL);
		PyModule_AddIntConstant(pyMain, "F_LAST_CALL", F_LAST_CALL);
		PyModule_AddIntConstant(pyMain, "F_GLOBAL_ERROR", F_GLOBAL_ERROR);
		PyModule_AddIntConstant(pyMain, "F_LOCAL_ERROR", F_LOCAL_ERROR);
		PyModule_AddIntConstant(pyMain, "S_NULL", S_NULL);
		PyModule_AddIntConstant(pyMain, "C_OK", C_OK);
		PyModule_AddIntConstant(pyMain, "C_STOP_USER", C_STOP_USER);
		PyModule_AddIntConstant(pyMain, "C_STOP_EXTERNAL", C_STOP_EXTERNAL);
		PyModule_AddIntConstant(pyMain, "C_STOP_CONDITION", C_STOP_CONDITION);
		PyModule_AddIntConstant(pyMain, "C_STOP_THEREFOREIAM", C_STOP_THEREFOREIAM);
		PyModule_AddIntConstant(pyMain, "OPERATION_NULL", OPERATION_NULL);
		/*
		PyModule_AddIntConstant(pyMain, "OPERATION_IIF_CREATE_SET", OPERATION_IIF_CREATE_SET);
		PyModule_AddIntConstant(pyMain, "OPERATION_OIF_CREATE_SET", OPERATION_OIF_CREATE_SET);
		*/
		PyModule_AddIntConstant(pyMain, "OPERATION_ADD_PORT", OPERATION_ADD_PORT);
		PyModule_AddIntConstant(pyMain, "OPERATION_SET_CONTENT", OPERATION_SET_CONTENT);
		PyModule_AddIntConstant(pyMain, "OPERATION_BOUT", OPERATION_BOUT);
		PyModule_AddIntConstant(pyMain, "OPERATION_GET_UTILITY_OBJECT", OPERATION_GET_UTILITY_OBJECT);
		PyModule_AddIntConstant(pyMain, "OPERATION_GET_UTILITY_FUNCTION", OPERATION_GET_UTILITY_FUNCTION);
		PyModule_AddIntConstant(pyMain, "OPERATION_CALL_UTILITY_FUNCTION", OPERATION_CALL_UTILITY_FUNCTION);
		//PyModule_AddIntConstant(pyMain, "OPERATION_GET_PROCESS_STATE", OPERATION_GET_PROCESS_STATE);
		PyModule_AddIntConstant(pyMain, "OPERATION_GET_SYMBOL_STRING", OPERATION_GET_SYMBOL_STRING);
		PyModule_AddIntConstant(pyMain, "OPERATION_GET_RANDOM_SEED", OPERATION_GET_RANDOM_SEED);
		PyModule_AddIntConstant(pyMain, "D_NONE", D_NONE);
		PyModule_AddIntConstant(pyMain, "D_WARN", D_WARN);
		PyModule_AddIntConstant(pyMain, "D_INFO", D_INFO);
		PyModule_AddIntConstant(pyMain, "D_VERB", D_VERB);

		PyModule_AddIntConstant(pyMain, "F_UNDEFINED", F_UNDEFINED);
		PyModule_AddIntConstant(pyMain, "F_ZERO", F_ZERO);
		PyModule_AddIntConstant(pyMain, "F_NEEDS_ALL_INPUTS", F_NEEDS_ALL_INPUTS);
		PyModule_AddIntConstant(pyMain, "F_INPUTS_SAME_RATE", F_INPUTS_SAME_RATE);
		PyModule_AddIntConstant(pyMain, "F_OUTPUTS_SAME_RATE", F_OUTPUTS_SAME_RATE);
		PyModule_AddIntConstant(pyMain, "F_NOT_RATE_CHANGER", F_NOT_RATE_CHANGER);
		//PyModule_AddIntConstant(pyMain, "F_FRESH", F_FRESH);
	}

	//	template output
	PyObject* pyOutput = addItem(pyPersist, "output", PyDict_New());
	if (!pyOutput) berr << E_MEMORY;
	addItem(pyOutput, "info", PyDict_New());
	PyObject* pyOutputEvent = addItem(pyOutput, "event", PyDict_New());
	if (!pyOutputEvent) berr << E_MEMORY;
	addItem(pyOutputEvent, "response", PyInt_FromLong(S_NULL));

	//	not needed in python (hangover from matlab)
//	addItem(pyOutput, "operations", PyList_New(0));

	//	event
	pyEvent = addItem(pyInput, "event", PyDict_New());
	if (!pyEvent) berr << E_MEMORY;
	addItem(pyEvent, "type", PyInt_FromLong(0));
	addItem(pyEvent, "flags", PyInt_FromLong(0));

	//	get self id and save instance
	UINT32 self = BrahmsInstances.size();			//	there's only one manager thread (so there's no race)
	BrahmsInstances.push_back(this);
	addItem(pyPersist, "self", PyInt_FromLong(self));
}





////////////////////////////////////////////////////////////////
//
//	DESTRUCTOR
//
////////////////////////////////////////////////////////////////

COMPONENT_CLASS_CPP::~COMPONENT_CLASS_CPP()
{
	Py_DECREF(pyBrahmsProcess);
	Py_DECREF(pyInput);
	Py_DECREF(pyOutput);
	Py_DECREF(pyPersist);
}






////////////////////////////////////////////////////////////////
//
//	FIRE WRAPPED FUNCTION
//
////////////////////////////////////////////////////////////////

Symbol attachError(Symbol error, const char* msg, UINT32 flags = 0)
{
	EventErrorMessage data;
	data.error = error;
	data.msg = msg;
	data.flags = flags;

	EngineEvent event;
	event.hCaller = 0;
	event.flags = 0;
	event.type = ENGINE_EVENT_ERROR_MESSAGE;
	event.data = &data;

	return brahms::brahms_engineEvent(&event);
}

void COMPONENT_CLASS_CPP::fireWrappedFunction()
{
	PyObject* pyResult = PyObject_CallFunctionObjArgs(pyBrahmsProcess, pyPersist, pyInput, NULL);

	if (!pyResult)
	{
		if (PyErr_Occurred())
		{
			//	fetch error
			PyObject* type;
			PyObject* value;
			PyObject* traceback;
			PyErr_Fetch(&type, &value, &traceback);
			PyErr_NormalizeException(&type, &value, &traceback);

			//	empty
			Symbol err;
			EventErrorMessage data;
			data.flags = 0;
			data.error = E_USER;
			data.msg = "an error occurred in the python script, but no message was retrieved";

			//	type is actually part of the error message
			string msg = "";
			if (type)
			{
				if (PyExceptionClass_Check(type))
				{
					stringstream ss;
					ss << PyExceptionClass_Name(type);
					string s = ss.str();
					if (s.substr(0,11) == "exceptions.") s = s.substr(11);
					msg += s + ": ";
				}
				else msg += "user code raised non-exception type: ";

				Py_XDECREF(type);
			}

			//	send str(value) to err
			if (value)
			{
				PyObject* temp = PyObject_Str(value);
				const char* ctemp = PyString_AsString(temp);
				if (ctemp)
				{
					//	if from operation, may have [code|msg] token in string, which we handle differently
					string t = ctemp;
					size_t pos = t.find("[code|msg]");
					if (pos == string::npos)
					{
						msg += t;
					}
					else
					{
						string n = t.substr(0, pos);
						string m = t.substr(pos + 10);
						msg = m;

						//	try to interpret code
						stringstream ss;
						ss << n;
						Symbol sy;
						ss >> sy;
						if (sy) data.error = sy;
					}
				}

				Py_XDECREF(value);
			}

			//	store
			if (msg.length()) data.msg = msg.c_str();
			
			EngineEvent event;
			event.hCaller = hComponent;
			event.flags = 0;
			event.type = ENGINE_EVENT_ERROR_MESSAGE;
			event.data = &data;
			err = brahms_engineEvent(&event);

			//	print traceback to sys.stderr (undocumented!)
			if (traceback)
			{
				if (PyTraceBack_Check(traceback))
				{
					PyTracebackObject* tb = (PyTracebackObject*) traceback;

					vector<PyTracebackObject*> tbs;
					while (tb != NULL)
					{
						tbs.push_back(tb);
						tb = tb->tb_next;
					}

					for (int i=tbs.size(); i!=0; i--)
					{
						PyTracebackObject* tb = tbs[i-1];

						//	extract trace
						int l = tb->tb_lineno;
						const char* filename = PyString_AsString(tb->tb_frame->f_code->co_filename);
						const char* modulename = PyString_AsString(tb->tb_frame->f_code->co_name);
						
						//	construct trace
						stringstream ss;
						ss << "at line " << l << " of " << modulename << " [[ " << filename << " :: " << l << " ]]";
						string s = ss.str();

						//	add trace to error
						data.error = err;
						data.flags = F_TRACE;
						data.msg = s.c_str();
						
						EngineEvent event;
						event.hCaller = hComponent;
						event.flags = 0;
						event.type = ENGINE_EVENT_ERROR_MESSAGE;
						event.data = &data;
						err = brahms_engineEvent(&event);
					}
				}

				Py_XDECREF(traceback);
			}

			//	throw
			throw err;
		}

		berr << E_PYTHON << "\"brahms_process\" failed, reason unknown";
	}

	//	replace (persist, output) with returned values
	if (!PyTuple_Check(pyResult) || PyTuple_GET_SIZE(pyResult) != 2)
		berr << E_NOT_COMPLIANT << "function should return (persist, output)";
	Py_DECREF(pyPersist);		//	disown the old values
	Py_DECREF(pyOutput);
	pyPersist = PyTuple_GET_ITEM(pyResult, 0);
	pyOutput = PyTuple_GET_ITEM(pyResult, 1);
	Py_INCREF(pyPersist);		//	own the new values
	Py_INCREF(pyOutput);
	Py_DECREF(pyResult);

	//	read "event"
	if (!PyDict_Check(pyOutput))
		berr << E_NOT_COMPLIANT << "output should be a structure";
	PyObject* pyEvent = PyDict_GetItemString(pyOutput, "event");
	if (!pyEvent || !PyDict_Check(pyEvent))
		berr << E_NOT_COMPLIANT << "output['event'] should be scalar structure";
	PyObject* temp = PyDict_GetItemString(pyEvent, "response");
	if (!temp || !PyInt_Check(temp))
		berr << E_NOT_COMPLIANT << "output['event']['response'] should be integer";
	response = PyInt_AsUnsignedLongMask(temp);

	//	read "error"
	temp = PyDict_GetItemString(pyOutput, "error");
	if (temp)
	{
		if (!PyString_Check(temp))
			berr << E_NOT_COMPLIANT << "output['error'] should be string";
		berr << PyString_AsString(temp);
	}

}




////////////////////////////////////////////////////////////////
//
//	DATAML TO PY
//
////////////////////////////////////////////////////////////////

PyObject* COMPONENT_CLASS_CPP::dataMLToPy(const DataMLNode& node)
{
	//	switch on type of DataML node
	TYPE type = node.getElementType();
	switch (type)
	{
		case TYPE_STRUCT:
		{
			PyObject* pyRet = PyDict_New();
			VSTRING children = node.getFieldNames();
			for (UINT32 c=0; c<children.size(); c++)
			{
				DataMLNode child = node.getField(children[c].c_str());
				PyObject* temp = dataMLToPy(child);
				if (!temp || !addItem(pyRet, children[c].c_str(), temp))
					berr << E_INTERNAL << "failed to set item in dataMLToPy for " << child.name;
			}
			return pyRet;
		}

		case TYPE_CELL:
		{
			PyArrayObject* pyRet;

			//	access array metadata
			Dims b_dims = node.getDims();
			UINT32 count = node.getNumberOfElementsReal();

			//	create return array
			switch (count)
			{
				case 0:
				{
					//	is empty
					npy_intp dims = 0;
					pyRet = (PyArrayObject*)PyArray_FromDims(1, &dims, NPY_OBJECT);
					break;
				}

				case 1:
				{
					//	is scalar
//					pyRet = (PyArrayObject*)PyArray_FromDims(0, NULL, NPY_OBJECT);

					//	CHANGED: R2330 ish, because i think this is wrong (though i've not tested it)
					npy_intp dims = 1;
					pyRet = (PyArrayObject*)PyArray_FromDims(1, &dims, NPY_OBJECT);
					break;
				}

				default:
				{
					//	convert dims if necessary
					vector<npy_intp> dims(b_dims.size());
					if (b_dims.size()) reverse_copy(b_dims.begin(), b_dims.end(), dims.begin());
					pyRet = (PyArrayObject*)PyArray_FromDims(b_dims.size(), b_dims.size() ? &dims[0] : NULL, NPY_OBJECT);
				}
			}
			if (!pyRet) berr << E_PYTHON << "failed to create array";

			//	fill children
			for (UINT32 c=0; c<count; c++)
			{
				DataMLNode child = node.getCell(c);
				((PyObject**)pyRet->data)[c] = dataMLToPy(child);
			}
			return (PyObject*)pyRet;
		}

		case TYPE_CHAR16:
		{
			Dims b_dims = node.getDims();
			if (b_dims.size() != 2) berr << "malformed DataML string";
			if (b_dims[0] > 1) berr << "multi-dimensional strings not supported in python";
			if (!b_dims[1]) return PyString_FromString("");

			//	UCS-2 to ASCIIZ (by truncation)
			UINT32 count = node.getNumberOfElementsReal();
			VBYTE content(count * 2);
			node.getRaw(&content[0], NULL);
			for (UINT32 c=1; c<count; c++)
				content[c] = content[c * 2];
			content[count] = 0;
			return PyString_FromString((char*)&content[0]);
		}

		default:
		{
			//	don't worry: dataType2arrayTypes() will throw if type is not a python numeric
			PyArrayObject* pyRet;

			//	access array metadata
			Dims b_dims = node.getDims();
			bool complex = node.getComplexity();
			UINT32 count = node.getNumberOfElementsReal();
			UINT32 bytes = node.getNumberOfBytesReal();
			UINT32 bytesPerElement = node.getBytesPerElement();

			//	create return array
			vector<npy_intp> dims(b_dims.size());
			if (b_dims.size()) reverse_copy(b_dims.begin(), b_dims.end(), dims.begin());
			pyRet = (PyArrayObject*)PyArray_FromDims(b_dims.size(), b_dims.size() ? &dims[0] : NULL,
				dataType2arrayTypes(complex ? type | TYPE_COMPLEX : type | TYPE_REAL));
			if (!pyRet) berr << E_PYTHON << "failed to create array";

			//	do the blat
			if (bytes)
			{
				if (!complex) node.getRaw((BYTE*)pyRet->data, NULL);
				else
				{
					VBYTE content(bytes * 2);
					node.getRaw(&content[0], &content[bytes]);
					interleaveComplexData(
						(BYTE*)pyRet->data,
						&content[0],
						&content[bytes],
						count,
						bytesPerElement);
				}
			}

			//	ok
			return PyArray_Return(pyRet);
		}
	}

	//	unhandled type
	berr << "unhandled data type in dataMLToPy()";
}



////////////////////////////////////////////////////////////////
//
//	EVENT
//
////////////////////////////////////////////////////////////////

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	//	set in event type and flags to pyEvent
	addItem(pyEvent, "type", PyInt_FromLong(event->type));
	addItem(pyEvent, "flags", PyInt_FromLong(event->flags));

	//	switch on event type
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
			//	empty iif & oif
			pyIif = addItem(pyInput, "iif", PyDict_New());
			if (!pyIif) berr << E_MEMORY;

			//	create default set on iifx
			newInputSetAdded("");
			newOutputSetAdded("");

			//	extract event-specific data structure
			EventStateSet* data = (EventStateSet*) event->data;

			//	get parameters from that
			XMLNode xmlNode(data->state);

			//	smother them in a DataML interface
			DataMLNode nodeState(&xmlNode);

			//	create persistent variable

			//	process
			PyObject* pyProcess = addItem(pyPersist, "process", PyDict_New());
			if (!pyProcess) berr << E_MEMORY;
			addItem(pyProcess, "name", PyString_FromString(componentData->name));
			//addItem(pyProcess, "uid", PyInt_FromLong(uid));

			//	state
			if (!addItem(pyPersist, "state", dataMLToPy(nodeState)))
				berr << E_PYTHON << "failed to set state";

			//	time
			pyTime = addItem(pyInput, "time", PyDict_New());
			pyBsr = addItem(pyTime, "baseSampleRate", PyDict_New());
			if (!pyTime || !pyBsr) berr << E_MEMORY;
			addItem(pyBsr, "num", PyInt_FromLong(time->baseSampleRate.num));
			addItem(pyBsr, "den", PyInt_FromLong(time->baseSampleRate.den));
			addItem(pyBsr, "rate", PyFloat_FromDouble(sampleRateToRate(time->baseSampleRate)));
			addItem(pyBsr, "period", PyFloat_FromDouble(sampleRateToPeriod(time->baseSampleRate)));
			addItem(pyTime, "executionStop", PyInt_FromLong(time->executionStop));
			PyObject* temp = addItem(pyTime, "sampleRate", PyDict_New());
			if (!temp) berr << E_MEMORY;
			addItem(temp, "num", PyInt_FromLong(time->sampleRate.num));
			addItem(temp, "den", PyInt_FromLong(time->sampleRate.den));
			addItem(temp, "rate", PyFloat_FromDouble(sampleRateToRate(time->sampleRate)));
			addItem(temp, "period", PyFloat_FromDouble(sampleRateToPeriod(time->sampleRate)));
			addItem(pyTime, "samplePeriod", PyInt_FromLong(time->samplePeriod));
			addItem(pyTime, "now", PyInt_FromLong(time->now));

			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}


		case EVENT_INIT_PRECONNECT:
		{
			//	generate iifx ports
			for (UINT32 s=0; s<iifx.size(); s++)
			{
				//	get set
				Set& set = iifx[s];

				//	resize set
				set.resize(iif.getNumberOfPorts(set.hSet));

				//	create child fields
				set.mxIndex = addItem(set.mxSet, "index", PyDict_New());
				set.mxPorts = addItem(set.mxSet, "ports", PyList_New(set.size()));
				if (!set.mxIndex || !set.mxPorts) berr << E_MEMORY;

				//	for each port in set, add a port entry to set mxArray
				for (UINT32 p=0; p<set.size(); p++)
				{
					//	create port
					set[p] = new Port;
					Port& port = *set[p];
					port.portIndex = p;
					port.hPort = iif.getPortHEV(set.hSet, p, &port.hev);
					port.name = iif.getPortName(port.hPort);
					port.mxPorts = set.mxPorts;

					//	 name, class, structure, data
					PyObject* pyFields = PyDict_New();
					if (!pyFields) berr << E_MEMORY;

					//	stick it into python iif array
					PyObject* pyName = PyString_FromString(port.name.c_str());
					if (pyName) addItem(pyFields, "name", pyName);
					PyDict_SetItemString(pyFields, "class", Py_None);
					PyDict_SetItemString(pyFields, "structure", Py_None);
					PyDict_SetItemString(pyFields, "data", Py_None);
					PyList_SetItem(port.mxPorts, p, pyFields);

					//	make entry in index:
					//	if two ports in a set have the same name, they will overwrite, and we want
					//	to (by convention) index the first not the last, so we only
					//	do this if the index doesn't already have such an entry...
					if (!PyDict_GetItemString(set.mxIndex, port.name.c_str()))
						addItem(set.mxIndex, port.name.c_str(), PyInt_FromLong(p)); // zero-based
				}
			}

			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_INIT_CONNECT:
		{
			//	make all inputs available
			makeAllInputsAvailable();

			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_INIT_POSTCONNECT:
		{
			//	update time data (base sample rate is now settled)
			addItem(pyBsr, "num", PyInt_FromLong(time->baseSampleRate.num));
			addItem(pyBsr, "den", PyInt_FromLong(time->baseSampleRate.den));
			addItem(pyBsr, "rate", PyFloat_FromDouble(sampleRateToRate(time->baseSampleRate)));
			addItem(pyBsr, "period", PyFloat_FromDouble(sampleRateToPeriod(time->baseSampleRate)));
			addItem(pyTime, "executionStop", PyInt_FromLong(time->executionStop));
			addItem(pyTime, "samplePeriod", PyInt_FromLong(time->samplePeriod));

			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_LOG_INIT:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_LOG_TERM:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_BEGIN_RUNPHASE:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}


		case EVENT_STATE_GET:
		{
			//	need to offer current state to process, and have it return
			//	its updated state. but this requires a translation to/from
			//	DataML, and i haven't coded it yet

			//	"not serviced", rather than "no update to make"
			return S_NULL;
		}

		case EVENT_BEGIN_TERMPHASE:
		{
			//	call python
			fireWrappedFunction();

			//	we don't currently do anything in response to this

			//	ok
			return response;
		}

		case EVENT_RUN_SERVICE:
		{

			//	set step bounds
			addItem(pyTime, "now", PyInt_FromLong(time->now));

			//	make all inputs available
			makeAllInputsAvailable();

			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_RUN_PLAY:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_RUN_PAUSE:
		case EVENT_RUN_RESUME:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		case EVENT_RUN_STOP:
		{
			//	call python
			fireWrappedFunction();

			//	ok
			return response;
		}

		default:
		{
			//	make sure we're handling all possible events
			berr << E_INTERNAL << "event not handled " << getSymbolString(event->type);
			return E_ERROR;
		}

	}

}

BRAHMS_DLL_EXPORT Symbol EventHandler(Event* event)
{
	try
	{

		switch (event->type)
		{
			case EVENT_MODULE_QUERY:
			{
				brahms::EventModuleQuery* query = (brahms::EventModuleQuery*) event->data;
				query->interfaceID = N_BRAHMS_INTERFACE;
				return C_OK;
			}

			case EVENT_MODULE_INIT:
			{
				EventModuleInit* init = (EventModuleInit*) event->data;
				executionInfo = init->executionInfo;
				init->moduleInfo = &MODULE_MODULE_INFO;
				return C_OK;
			}

			case EVENT_MODULE_TERM:
			{
				//EventModuleTerm* term = (EventModuleTerm*) event->data;
				return C_OK;
			}

			case EVENT_MODULE_CREATE:
			{
				EventModuleCreateBindings* emc = (EventModuleCreateBindings*) event->data;
				//emc->info = &COMPONENT_INFO;

				COMPONENT_CLASS_CPP* newObject = new COMPONENT_CLASS_CPP(emc);
				event->object = newObject;
                           
				newObject->initialize(emc->hComponent, emc->data);
				emc->info = newObject->getThisComponentInfo();

				return C_OK;
			}

			case EVENT_MODULE_DESTROY:
			{
				delete (COMPONENT_CLASS_CPP*) event->object;
				return C_OK;
			}

			case EVENT_MODULE_DUPLICATE:
			{
				return attachError(E_NOT_IMPLEMENTED, "cannot duplicate python process");
			}

			default:
			{
				COMPONENT_CLASS_CPP* object = (COMPONENT_CLASS_CPP*) event->object;
				if (!object) return E_NO_INSTANCE;

				return object->event(event);
			}
		}

	}

	catch(Symbol e)
	{
		return e;
	}

	catch(const char* e)
	{
		return attachError(E_USER, e);
	}

	catch(...)
	{
		return E_UNRECOGNISED_EXCEPTION;
	}

}

