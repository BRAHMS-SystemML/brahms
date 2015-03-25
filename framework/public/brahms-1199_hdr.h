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

	$Id:: brahms-1199.h 2413 2009-11-20 17:27:39Z benjmitch    $
	$Rev:: 2413                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 17:27:39 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/

#ifndef _BRAHMS_1199_HDR_H_
#define _BRAHMS_1199_HDR_H_

namespace brahms
{
    // COMPONENT CLASS
    struct Component
    {
        Component() {
            time = NULL;
            componentData = NULL;
            hComponent = S_NULL;

            // can perform 32/64 bit asserts here
            if (sizeof(UINT64) != 8) berr << "failed assert: sizeof(UINT64) == 8";
            if (sizeof(INT64) != 8) berr << "failed assert: sizeof(INT64) == 8";
            if (sizeof(UINT32) != 4) berr << "failed assert: sizeof(UINT32) == 4";
            if (sizeof(INT32) != 4) berr << "failed assert: sizeof(INT32) == 4";
            if (sizeof(UINT16) != 2) berr << "failed assert: sizeof(UINT16) == 2";
            if (sizeof(INT16) != 2) berr << "failed assert: sizeof(INT16) == 2";
            if (sizeof(INT16) != 2) berr << "failed assert: sizeof(INT16) == 2";
            if (sizeof(DOUBLE) != 8) berr << "failed assert: sizeof(DOUBLE) == 8";
        }

        Component(const Component& src) {
            time = NULL;
            hComponent = S_NULL;
        }

        Component& operator=(const Component& src) {
            berr << E_INTERNAL << "Component() assignment";
            return *this;
        }

        virtual ~Component() {}

        VUINT32 getRandomSeed(UINT32 count = 0) {
            VUINT32 seed;
            EventGetRandomSeed rseed;
            rseed.flags = 0;
            rseed.count = count;

            if (count)
            {
                seed.resize(count);
                rseed.seed = &seed[0];

                EngineEvent event;
                event.hCaller = hComponent;
                event.flags = 0;
                event.type = ENGINE_EVENT_GET_RANDOM_SEED;
                event.data = (void*) &rseed;

                ____SUCCESS(brahms_engineEvent(&event));
            }
            else
            {
                rseed.seed = NULL;

                EngineEvent event;
                event.hCaller = hComponent;
                event.flags = 0;
                event.type = ENGINE_EVENT_GET_RANDOM_SEED;
                event.data = (void*) &rseed;

                ____SUCCESS(brahms_engineEvent(&event));

                // if zero, can have whatever we want - we choose 1
                if (!rseed.count) rseed.count = 1;
                seed.resize(rseed.count);
                rseed.seed = &seed[0];

                ____SUCCESS(brahms_engineEvent(&event));
            }

            return seed;
        }

        ComponentData getComponentData(Symbol hSubject) {
            // can still return our own
            if (hSubject == hComponent)
                return *componentData;

            // otherwise that's an error
            berr << E_INTERNAL;
	    
	    // This line never called, but avoids warning.
	    return *componentData;
        }

        // TODO: should be removed in a future binding, since we now use the interface in utility.h to create a utility
        Symbol createUtility(const char* cls, UINT16 release, const char* name = NULL) {
            EventCreateUtility data;
            data.flags = 0;
            data.hUtility = S_NULL;
            data.spec.cls = cls;
            data.spec.release = release;
            data.name = name;
            data.handledEvent = NULL;

            EngineEvent event;
            event.hCaller = hComponent;
            event.flags = 0;
            event.type = ENGINE_EVENT_CREATE_UTILITY;
            event.data = (void*) &data;

            ____SUCCESS(brahms_engineEvent(&event));

            return data.hUtility;
        }

        void stillActive() {
            EngineEvent event;
            event.hCaller = hComponent;
            event.flags = 0;
            event.type = ENGINE_EVENT_STILL_ACTIVE;
            event.data = 0;

            ____SUCCESS(brahms_engineEvent(&event));
        }

        virtual Symbol event(Event* event) = 0;
        virtual void initialize(Symbol hComponent, const ComponentData* data) = 0;

        const ComponentTime* time;
        const ComponentData* componentData;
        Symbol hComponent;
        ComponentOut bout;

#ifndef BRAHMS_NO_LEGACY_SUPPORT
        void assertClass(Symbol hData, const char* cls, UINT16 release = 0) {
            // data
            ComponentSpec spec;
            spec.cls = cls;
            spec.release = release;

            // make legacy call
            brahms::EngineEvent event;
            event.hCaller = hData;
            event.flags = 0;
            event.type = ENGINE_EVENT_ASSERT_COMPONENT_SPEC;
            event.data = (void*) &spec;

            ____SUCCESS(brahms::brahms_engineEvent(&event));
        }

        Symbol sendSignal(Symbol signal) {
            if (signal == ENGINE_EVENT_STILL_ACTIVE)
            {
                EngineEvent event;
                event.hCaller = hComponent;
                event.flags = 0;
                event.type = signal;
                event.data = 0;

                ____SUCCESS(brahms_engineEvent(&event));
            }

            else
            {
                berr << E_INTERNAL << "signal not recognised";
            }

            return C_OK;
        }
#endif
    };

    // COMPONENT SUBCLASSES

#ifdef COMPONENT_PROCESS
    struct Process : public Component
    {
        Process() {}

        void initialize(Symbol hComponent, const ComponentData* data) {
            this->hComponent = hComponent;
            this->time = data->time;
            this->componentData = data;
            bout.initialize(hComponent);
            iif.initialize(hComponent, F_IIF);
            oif.initialize(hComponent, F_OIF);
        }

        SystemMLInterface iif;
        SystemMLInterface oif;
    };
#endif

#ifdef COMPONENT_DATA
    struct Data : public Component
    {
        Data() {}

        void initialize(Symbol hComponent, const ComponentData* data) {
            this->hComponent = hComponent;
            this->time = data->time;
            this->componentData = data;
            bout.initialize(hComponent);
        }
    };
#endif

#ifdef COMPONENT_UTILITY
    struct Utility : public Component
    {
        Utility() {}

        void initialize(Symbol hComponent, const ComponentData* data) {
            this->hComponent = hComponent;
            this->time = data->time;
            this->componentData = data;
            bout.initialize(hComponent);
        }
    };
#endif
}

#endif // _BRAHMS_1199_HDR_H_
