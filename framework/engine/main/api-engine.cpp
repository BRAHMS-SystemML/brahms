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

	$Id:: api-engine.cpp 2376 2009-11-15 23:34:44Z benjmitch   $
	$Rev:: 2376                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-15 23:34:44 +0000 (Sun, 15 Nov 2009)       $
________________________________________________________________

*/



#undef CATCH_API 
#undef CATCH_API_S 
#undef CATCH_API_O 
#undef CATCH_API_X

//	we handle errors in this API by pushing them onto the error stack, and returning E_ERROR
#define CATCH_API_S catch(const exception& se) { brahms::error::Error e(E_STD, se.what()); ____AT(e); errorStack.push(e); return E_ERROR; }
#define CATCH_API_O catch(brahms::error::Error& e) { ____AT(e); errorStack.push(e); return E_ERROR; }
#define CATCH_API_X catch(...) { brahms::error::Error e(E_UNRECOGNISED_EXCEPTION); ____AT(e); errorStack.push(e); return E_ERROR; }
#define CATCH_API CATCH_API_O CATCH_API_S CATCH_API_X



////////////////	ENGINE API HELPER

struct EngineAPIHelper
{
	EngineAPIHelper()
	{
		engine = NULL;
	}

	string create(CreateEngine* create)
	{
		try
		{
			engine = new Engine(create);
		}

		catch(brahms::error::Error& e)
		{
			return e.format(FMT_TEXT, true);
		}

		catch(...)
		{
			return "unrecognised exception whilst creating Engine";
		}

		return "";
	}

	bool resolve(Symbol hEngine)
	{
		try
		{
			engine = (Engine*) objectRegister.resolve(hEngine, CT_ENGINE);
		}

		catch(...)
		{
			return false;
		}

		return true;
	}

	Symbol call(const char* op, void* arg1 = 0, void* arg2 = 0)
	{
		try
		{
			if (!engine) return E_INVALID_HANDLE;

			if (!strcmp(op, "engine->getObjectHandle()"))
			{
				return engine->getObjectHandle();
			}

			if (!strcmp(op, "engine->up()"))
			{
				engine->up();
				return C_OK;
			}

			if (!strcmp(op, "engine->down()"))
			{
				if (!engine->down((UINT32*)arg1)) return E_ERROR;
				return C_OK;
			}

			if (!strcmp(op, "delete engine"))
			{
				delete engine;
				engine = NULL;
				return C_OK;
			}

			if (!strcmp(op, "engine->walk()"))
			{
				WalkLevel level = *((WalkLevel*) arg1);
				engine->walk(level);
				return C_OK;
			}

			if (!strcmp(op, "engine->open()"))
			{
				engine->open();
				return C_OK;
			}

			if (!strcmp(op, "engine->execute()"))
			{
				engine->execute();
				return C_OK;
			}

			if (!strcmp(op, "engine->close()"))
			{
				engine->close();
				return C_OK;
			}

			//	erm...
			ferr << E_INTERNAL << op;
			return E_ERROR; // avoid compiler warning
		}

		catch(brahms::error::Error& e)
		{
			if (engine)
			{
				____AT(e);
				e.trace("whilst processing \"" + string(op) + "\"");
				engine->engineData.core.caller.storeError(e, engine->engineData.core.caller.tout);
			}
			return E_ERROR;
		}

		catch(const exception& se)
		{
			if (engine)
			{
				brahms::error::Error e(E_STD, se.what());
				____AT(e);
				e.trace("whilst processing \"" + string(op) + "\"");
				engine->engineData.core.caller.storeError(e, engine->engineData.core.caller.tout);
			}
			return E_ERROR;
		}

		catch(...)
		{
			if (engine)
			{
				brahms::error::Error e(E_UNRECOGNISED_EXCEPTION);
				____AT(e);
				e.trace("whilst processing \"" + string(op) + "\"");
				engine->engineData.core.caller.storeError(e, engine->engineData.core.caller.tout);
			}
			return E_ERROR;
		}
	}

	Engine* engine;
};


////////////////	ENGINE API

#ifdef __WIN__
//	CRT security enhancements
#define strncpy(dst, src, l) strncpy_s((dst), (l), (src), (l))
#endif

	BRAHMS_ENGINE_VIS Symbol engine_create(CreateEngine* createEngine)
	{
		EngineAPIHelper engine;
		string err = engine.create(createEngine);
		if (err.length())
		{
			strncpy(createEngine->errorMessage, err.c_str(), CREATEENGINE_MSG_LENGTH-1);
			createEngine->errorMessage[CREATEENGINE_MSG_LENGTH-1] = 0;
			return E_ERROR;
		}
		return engine.call("engine->getObjectHandle()");
	}

	BRAHMS_ENGINE_VIS Symbol engine_up(Symbol hEngine)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->up()");
	}

	BRAHMS_ENGINE_VIS Symbol engine_down(Symbol hEngine, UINT32* messageCount)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->down()", messageCount);
	}

	BRAHMS_ENGINE_VIS Symbol engine_destroy(Symbol hEngine)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("delete engine");
	}

	BRAHMS_ENGINE_VIS Symbol engine_walk(Symbol hEngine, WalkLevel walkLevel)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->walk()", (void*) &walkLevel);
	}

	BRAHMS_ENGINE_VIS Symbol engine_open(Symbol hEngine)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->open()");
	}

	BRAHMS_ENGINE_VIS Symbol engine_execute(Symbol hEngine)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->execute()");
	}

	BRAHMS_ENGINE_VIS Symbol engine_close(Symbol hEngine)
	{
		EngineAPIHelper engine;
		if (!engine.resolve(hEngine)) return E_INVALID_HANDLE;
		return engine.call("engine->close()");
	}



