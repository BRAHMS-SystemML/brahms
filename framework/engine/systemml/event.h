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

	$Id:: event.h 2241 2009-10-29 23:11:54Z benjmitch          $
	$Rev:: 2241                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-29 23:11:54 +0000 (Thu, 29 Oct 2009)       $
________________________________________________________________

*/

#ifndef INCLUDED_ENGINE_EVENT
#define INCLUDED_ENGINE_EVENT

namespace brahms
{
	namespace systemml
	{
		class Component;
	}
}

////////////////////////////////////////////////////////////////
//
//	START NAMESPACE
//
namespace brahms {
//
////////////////////////////////////////////////////////////////





	/*

		This is an extension to the C Event defined in the component interface
		that includes the handler function

	*/

	//	extended event structure
	class EventEx : public ::Event
	{

	public:

		EventEx(
		);

		EventEx(
			Symbol p_type,
			UINT32 p_flags,
			brahms::systemml::Component* p_component,
			void* p_data
		);

		EventEx(
			Symbol p_type,
			UINT32 p_flags,
			brahms::systemml::Component* p_component,
			void* p_data,
			bool p_requireResponse,
			brahms::output::Source* tout
		);

		//	derived class data
		brahms::systemml::Component* component;
		bool requireResponse;
		brahms::output::Source* tout;

		//	fire function
		Symbol fire();
	};






////////////////////////////////////////////////////////////////
//
//	END NAMESPACE
//
}
//
////////////////////////////////////////////////////////////////

#endif
