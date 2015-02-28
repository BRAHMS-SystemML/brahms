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

	$Id:: register.h 2241 2009-10-29 23:11:54Z benjmitch       $
	$Rev:: 2241                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-29 23:11:54 +0000 (Thu, 29 Oct 2009)       $
________________________________________________________________

*/



#ifndef INCLUDED_ENGINE_REGISTER
#define INCLUDED_ENGINE_REGISTER

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "support/os.h"
#include "base/os.h" // brahms::os::Mutex

////////////////	NAMESPACE

namespace brahms
{



	////////////////	CLASSES WE CAN RESOLVE IN OBJECT REGISTER

	namespace systemml
	{
		class Component;
		class Process;
		class Data;
		class Utility;
		class Port;
		class InputPortLocal;
		class OutputPort;
		class Set;
		class InputSet;
		class OutputSet;
	}

	namespace xml
	{
		class XMLNode;
	}



	////////////////	GLOBAL (ACROSS ENGINES) OBJECT REGISTER

		const Symbol S_OBJECT_HANDLE_MIN = S_TYPE_RESERVED_MIN;
		const Symbol S_OBJECT_HANDLE_MAX = S_TYPE_RESERVED_MIN + 0x0FFFFFFF;

		//	ComponentType is extended to ObjectType
		typedef UINT32 ObjectType;

		const ObjectType CT_COMPONENT		= CT_DATA | CT_PROCESS | CT_UTILITY;
		const ObjectType CT_XMLNODE			= 0x0008;
		const ObjectType CT_INPUT_SET		= 0x0010;
		const ObjectType CT_OUTPUT_SET		= 0x0020;
		const ObjectType CT_SET				= CT_INPUT_SET | CT_OUTPUT_SET;
		const ObjectType CT_INPUT_PORT		= 0x0040;
		const ObjectType CT_OUTPUT_PORT		= 0x0080;
		const ObjectType CT_PORT			= CT_INPUT_PORT | CT_OUTPUT_PORT;
		const ObjectType CT_ENGINE			= 0x0100;

		class RegisteredObject;

		class ObjectRegister
		{

		public:

			ObjectRegister();
			~ObjectRegister();

			//	register object
			Symbol add(RegisteredObject* object);

			//	resolve handle back to registered object
			RegisteredObject* resolve(Symbol objectHandle, ObjectType expectedType);

			//	resolve, validate type, and cast to type
			brahms::systemml::Component* resolveComponent(Symbol objectHandle)
			{
				return (brahms::systemml::Component*) resolve(objectHandle, CT_COMPONENT);
			}

			brahms::systemml::Process* resolveProcess(Symbol objectHandle)
			{
				return (brahms::systemml::Process*) resolve(objectHandle, CT_PROCESS);
			}

			brahms::systemml::Data* resolveData(Symbol objectHandle)
			{
				return (brahms::systemml::Data*) resolve(objectHandle, CT_DATA);
			}

			brahms::systemml::Utility* resolveUtility(Symbol objectHandle)
			{
				return (brahms::systemml::Utility*) resolve(objectHandle, CT_UTILITY);
			}

			brahms::systemml::Set* resolveSet(Symbol objectHandle)
			{
				return (brahms::systemml::Set*) resolve(objectHandle, CT_SET);
			}

			brahms::systemml::OutputSet* resolveOutputSet(Symbol objectHandle)
			{
				return (brahms::systemml::OutputSet*) resolve(objectHandle, CT_OUTPUT_SET);
			}

			brahms::systemml::Port* resolvePort(Symbol objectHandle)
			{
				return (brahms::systemml::Port*) resolve(objectHandle, CT_PORT);
			}

			brahms::systemml::OutputPort* resolveOutputPort(Symbol objectHandle)
			{
				return (brahms::systemml::OutputPort*) resolve(objectHandle, CT_OUTPUT_PORT);
			}

			brahms::xml::XMLNode* resolveXMLNode(Symbol objectHandle)
			{
				return (brahms::xml::XMLNode*) resolve(objectHandle, CT_XMLNODE);
			}

			//	free some space
			void remove(Symbol objectHandle, RegisteredObject* object);

		private:

			//	data
			brahms::os::Mutex mutex;
			vector<RegisteredObject*> objects;

			//	debug information (not used if DEBUG is not defined)
			UINT32 numberAdded;
			UINT32 numberRemoved;
			UINT32 peakNumberRegistered;
		};

		extern ObjectRegister objectRegister;

		class RegisteredObject
		{

			friend class ObjectRegister;

		public:

			RegisteredObject(ObjectType p_objectType, string p_objectName)
			{
				objectType = p_objectType;
				objectName = p_objectName;
				objectHandle = objectRegister.add(this);
			}

			RegisteredObject(const RegisteredObject& copy)
			{
				objectType = copy.objectType;
				objectName = copy.objectName;
				objectHandle = objectRegister.add(this);
			}

			virtual ~RegisteredObject()
			{
				//	unregister object on destruction
				objectRegister.remove(objectHandle, this);
			}

			ObjectType getObjectType() const
			{
				return objectType;
			}

			Symbol getObjectHandle() const
			{
				return objectHandle;
			}

			const string& getObjectName() const
			{
				return objectName;
			}

			void setObjectName(const string& p_objectName)
			{
				//	object name *can* be changed - some objects use it as their practical name,
				//	for efficiency of not having to store a second, probably identical, string
				objectName = p_objectName;
			}

		private:

			ObjectType objectType;
			Symbol objectHandle;
			string objectName;

		};



////////////////	NAMESPACE

}



////////////////	INCLUSION GUARD

#endif
