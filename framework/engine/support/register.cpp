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

	$Id:: register.cpp 2283 2009-11-02 00:22:31Z benjmitch     $
	$Rev:: 2283                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-02 00:22:31 +0000 (Mon, 02 Nov 2009)       $
________________________________________________________________

*/



#include "support.h"



//#define DEBUG_OBJECT_REGISTER




//#define USE_MUTEXES_RESOLVE
#define USE_MUTEXES_ADD_REMOVE



////////////////	NAMESPACE

namespace brahms
{



////////////////	GLOBAL (ACROSS ENGINES) OBJECT REGISTER

	//#define OTSCASE(c) case c: return #c;

	#define OTSCASE(c) if (type & c) { if (ret.length()) ret += " | "; ret += #c; };

	string objectTypeString(ObjectType type)
	{
		string ret = "";

		OTSCASE(CT_DATA);
		OTSCASE(CT_PROCESS);
		OTSCASE(CT_UTILITY);
		OTSCASE(CT_XMLNODE);
		OTSCASE(CT_INPUT_SET);
		OTSCASE(CT_OUTPUT_SET);
		OTSCASE(CT_INPUT_PORT);
		OTSCASE(CT_OUTPUT_PORT);
		OTSCASE(CT_ENGINE);

		if (!ret.length())
		{
			stringstream ss;
			ss << "<unknown-0x" << hex << type << ">";
			ret = ss.str();
		}

		return ret;
	}

	//	register object
	Symbol ObjectRegister::add(RegisteredObject* object)
	{
#ifdef USE_MUTEXES_ADD_REMOVE
		//	obtain exclusive access to objects array
		brahms::os::MutexLocker locker(mutex);
#endif

		//	generate new handle
		Symbol objectHandle = S_OBJECT_HANDLE_MIN + objects.size();

		//	validate
		if (objectHandle >= S_OBJECT_HANDLE_MAX)
			ferr << E_INTERNAL << "object register overflow";

		//	add
		objects.push_back(object);

#ifdef DEBUG_OBJECT_REGISTER
		numberAdded++;
		if (objects.size() > peakNumberRegistered)
			peakNumberRegistered = objects.size();
#endif

#ifdef DEBUG_OBJECT_REGISTER
		cerr << "\nObjectRegister::add(\n";
		cerr << "\t(handle) 0x" << hex << objectHandle << dec << "\n";
		cerr << "\t(type)   \"" << objectTypeString(object->objectType) << "\"\n";
		cerr << "\t(name)   \"" << object->objectName << "\"\n";
		cerr << "\t(this)   0x" << object << "\n";
		cerr << ")\n" << endl;
#endif

		//	ok
		return objectHandle;
	}

	//	resolve handle back to registered object
	RegisteredObject* ObjectRegister::resolve(Symbol objectHandle, ObjectType expectedType)
	{
		/*

			We have to do bounds checking on every call, because the handles
			being passed to us are usually from user code, so they could
			be any value at all.

		*/

		//	convert handle to index
		UINT32 index = objectHandle - S_OBJECT_HANDLE_MIN;

		//	obtain exclusive access to objects array
#ifdef USE_MUTEXES_RESOLVE
		//	obtain exclusive access to objects array
		brahms::os::MutexLocker locker(mutex);
#endif

		//	validate
		if (index >= objects.size())
			ferr << E_INVALID_HANDLE << "invalid object handle/index (0x" << hex << objectHandle << ", range was " << dec << objects.size() << ")";

		//	resolve
		RegisteredObject* result = objects[index];

		//	validate
		if (expectedType != CT_NULL)
		{
			if ((expectedType & objects[index]->objectType) != objects[index]->objectType)
				ferr << E_WRONG_HANDLE_TYPE << "valid handle but incorrect object type (" << objectTypeString(objects[index]->objectType) << ", when " << objectTypeString(expectedType) << " was expected)";
		}

		//	ok
		return result;
	}

	//	free some space
	void ObjectRegister::remove(Symbol objectHandle, RegisteredObject* object)
	{
		//	currently, ignore - can add this later if we think we need it
		//	it's only got mileage in it really if the global register is
		//	remaining resident whilst engines are started and finished. this
		//	may be the case in some implementations, so we should keep an
		//	eye on things.
#ifdef DEBUG_OBJECT_REGISTER
		numberRemoved++;
#endif

#ifdef USE_MUTEXES_ADD_REMOVE
		//	obtain exclusive access to objects array
		brahms::os::MutexLocker locker(mutex);
#endif

		//	zero item
		UINT32 index = objectHandle - S_OBJECT_HANDLE_MIN;
		if (objects[index] != object)
		{
			stringstream ss;
			____WARN("ObjectRegister::remove() object does not match (" << objects[index] << " != " << object << ")");
		}
		objects[index] = NULL;

		//	reduce array size if this was the last one
		if ((index + 1) == objects.size())
		{
			//	count how many we can remove
			while(index && !objects[index-1])
				index--;

			//	do resize
			objects.resize(index);
		}
	}

	//	tors
	ObjectRegister::ObjectRegister()
	{
#ifdef DEBUG_OBJECT_REGISTER
		cerr << "ObjectRegister()" << endl;
#endif

		numberAdded = 0;
		numberRemoved = 0;
		peakNumberRegistered = 0;
	}

	//	tors
	ObjectRegister::~ObjectRegister()
	{
#ifdef DEBUG_OBJECT_REGISTER
		int count = 0;
		for (int i=0; i<objects.size(); i++)
			if (objects[i])
				count++;

		cerr << "\n~ObjectRegister() on exit:\n" << dec;
		cerr << "\tobjects.size() == " << objects.size() << "\n";
		cerr << "\t(of which non-NULL) == " << count << "\n";
		cerr << "\tnumberAdded == " << numberAdded << "\n";
		cerr << "\tnumberRemoved == " << numberRemoved << "\n";
		cerr << "\tpeakNumberRegistered == " << peakNumberRegistered << "\n\n";
		cerr << endl;

		for (int i=0; i<objects.size(); i++)
		{
			if (objects[i])
				cerr << "\"" << objects[i]->objectName << "\" " << objectTypeString(objects[i]->objectType)) << endl;
		}
#endif
	}

	//	instance
	ObjectRegister objectRegister;



////////////////	NAMESPACE

}


