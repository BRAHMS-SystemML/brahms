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

	$Id:: environment.cpp 2410 2009-11-20 10:18:18Z benjmitch  $
	$Rev:: 2410                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-20 10:18:18 +0000 (Fri, 20 Nov 2009)       $
________________________________________________________________

*/

#include "support.h"

using namespace brahms::output; // for D_INFO, etc.

namespace brahms
{

		Environment::Environment(brahms::base::Core& p_core)
			:
			core(p_core)
		{
			nodeExecPars.nodeName("ExecutionParameters");
			nodeExecPars.setControl(&controlExecPars);
		}

		Environment::~Environment()
		{
		}

		void Environment::add(string key)
		{
			//	since set() falls over if we try to add a par
			//	that doesn't exist, we need this function so
			//	we can add pars that aren't specified in a
			//	file (i.e. run-time allocated, or dynamic, pars)
                    const brahms::xml::XMLNodeList* nodes = nodeExecPars.childNodes();
                    for(UINT32 n=0; n<nodes->size(); n++)
                    {
                        if (nodes->at(n)->nodeName() == key)
                            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \""
                                 << key << "\" already exists during add()";
                    }

                    //	modify the tree
                    controlExecPars.readonly = false;
                    nodeExecPars.appendChild(new brahms::xml::XMLNode(key.c_str()));
                    core.execPars.set(key.c_str(), "");
                    controlExecPars.readonly = true;
		}

    void Environment::set(string key, string value, brahms::output::Source& fout)
    {
        const brahms::xml::XMLNodeList* nodes = nodeExecPars.childNodes();
        for(UINT32 n=0; n<nodes->size(); n++)
        {
            if (nodes->at(n)->nodeName() == key)
            {
                //	modify the tree
                controlExecPars.readonly = false;
                nodes->at(n)->nodeText(value.c_str());
                core.execPars.set(key.c_str(), value.c_str());
                controlExecPars.readonly = true;
                fout << key << " = " << value << D_VERB;
                return;
            }
        }

        ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
             << "\" not found during set()";
    }

    void Environment::set(string key, DOUBLE value, brahms::output::Source& fout)
    {
        ostringstream ss;
        ss << value;
        set(key, ss.str(), fout);
    }

    void Environment::load(string path, bool insist, brahms::output::Source& fout)
    {
        //	report
        //grep(mpath, "Program Files", "Pro...les");
        //grep(mpath, "Documents and Settings", "Doc...ngs");
        //grep(mpath, "Application Data", "App...ata");
        fout << "loading from \"" << path << "\"" << D_VERB;

        //	check exists
        if (!brahms::os::fileexists(path))
        {
            if (insist) {
                ferr << E_EXECUTION_PARAMETERS << "file not found \"" << path << "\"";
            }
            fout << "(file absent)" << D_VERB;
            return;
        }
        fout << "(file present)" << D_VERB;

        //	load the specified settings file
        ifstream file(path.c_str());
        if (!file) {
            ferr << E_EXECUTION_PARAMETERS << "error opening \"" << path << "\"";
        }
        brahms::xml::XMLNode node;
        try
        {
            node.parse(file);
        }
        CATCH_TRACE_RETHROW("parsing \"" + string(path) + "\"")

        file.close();

        //	get execution parameters out of that
        load(path.c_str(), node, fout);
    }

    void Environment::load(const char* path,
                           brahms::xml::XMLNode& nodeFile,
                           brahms::output::Source& fout)
    {
        if (nodeFile.hasChild("ExecutionParameters"))
        {
            brahms::xml::XMLNode* nodeFilePars = nodeFile.getChild("ExecutionParameters");
            const brahms::xml::XMLNodeList* nodes = nodeFilePars->childNodes();

            for (UINT32 c=0; c<nodes->size(); c++)
            {
                brahms::xml::XMLNode* node = nodes->at(c);
                if (node->hasChildNodes()) {
                    ferr << E_EXECUTION_PARAMETERS << "malformed ExecutionParameters in \""
                         << path << "\"";
                }
                string key = node->nodeName();
                string value = node->nodeText();

                //	do translation where necessary
                if (key == "WorkingDirectory")
                {
                    //	do path translations
                    value = brahms::os::expandpath(value);
                }

                //	modify the tree
                controlExecPars.readonly = false;
                if (nodeExecPars.hasChild(key.c_str())) {
                    nodeExecPars.getChild(key.c_str())->nodeText(value.c_str());
                } else {
                    nodeExecPars.appendChild(new brahms::xml::XMLNode(key.c_str(), value.c_str()));
                }
                fout << key << " = " << value << D_VERB;
                core.execPars.set(key.c_str(), value.c_str());
                controlExecPars.readonly = false;
            }
        }
    }

    string Environment::gets(const char* key) const
    {
        //	find node
        string skey = key;
        const brahms::xml::XMLNodeList* nodes = nodeExecPars.childNodes();
        for(UINT32 n=0; n<nodes->size(); n++)
        {
            if (nodes->at(n)->nodeName() == skey) {
                return nodes->at(n)->nodeText();
            }
        }

        //	error
        ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key << "\" not found";

        //	avoid debug-mode warning...
        return 0;
    }

    DOUBLE Environment::get(const char* key, DOUBLE min, DOUBLE max) const
    {
        string nodeText = gets(key);
        DOUBLE value;
        if (nodeText == "NEARLYZERO") {
            value = EFFECTIVELY_ZERO;
        } else {
            //	numeric
            value = brahms::text::s2n(nodeText.c_str());
            if (value == brahms::text::S2N_FAILED)
                ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key << "\" malformed";
        }

        //	check constraints
        if (value < min) {
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
                 << "\" should be at least " << min;
        }
        if (value > max) {
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
                 << "\" should be not more than " << max;
        }
        //	ok
        return value;
    }

    UINT32 Environment::getu(const char* key, UINT32 max) const
    {
        DOUBLE dvalue = get(key, 0.0, max);
        UINT32 uvalue = (UINT32)dvalue;
        if (uvalue != dvalue) {
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
                 << "\" should be an integer";
        }
        return uvalue;
    }

    VUINT32 uexplode(const string delim, const string& data, const char* name)
    {
        //	get vector of strings
        VSTRING vs = brahms::text::explode(delim, data);
        VUINT32 ret;

        //	convert each one to a UINT32
        for (UINT32 i=0; i<vs.size(); i++)
        {
            DOUBLE val = brahms::text::s2n(vs[i]);
            UINT32 uval = val;
            if (val == brahms::text::S2N_FAILED || uval != val) {
                ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << name
                     << "\" was incorrectly formatted";
            }
            ret.push_back(uval);
        }

        //	ok
        return ret;
    }

    VUINT32 Environment::getulist(const char* key, const UINT32 count) const
    {
        string s = gets(key);
        VUINT32 ret = uexplode(",", s, key);
        if (ret.size() != count) {
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \""
                 << key << "\" should have three entries";
        }
        return ret;
    }

    INT32 Environment::geti(const char* key, INT32 min, INT32 max) const
    {
        DOUBLE dvalue = get(key, min, max);
        INT32 ivalue = (INT32)dvalue;
        if (ivalue != dvalue)
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
                 << "\" should be an integer";
        return ivalue;
    }

    bool Environment::getb(const char* key) const
    {
        DOUBLE dvalue = get(key, -1e16, 1e16);
        if (dvalue == 1.0) return true;
        if (dvalue == 0.0) return false;
        ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << key
             << "\" should be a boolean";
        //	avoid debug-mode warning...
        return 0;
    }

    void Environment::assertType(const char* name, char type) const
    {
        string value = gets(name);
        switch(type)
        {
        case 'b':
            if (value == "0" || value == "1") { return; }
            ferr << E_EXECUTION_PARAMETERS << "Execution Parameter \"" << name
                 << "\" should be boolean (\"0\" or \"1\")";
        }
    }

    void Environment::finalize(UINT32 voiceCount, UINT32 voiceIndex, brahms::output::Source& fout)
    {
        fout << "Execution Parameters (inferred):" << D_VERB;

        //	generate unique ID for this instance from time (i know, not great)
        ostringstream ss;
        time_t t = ::time(NULL);

        //	store ExecutionUID
        ss << "T" << t;
        add("ExecutionID");
        set("ExecutionID", ss.str(), fout);

        //	store VoiceUID
        if (voiceIndex == VOICE_UNDEFINED) ss << "-Vx";
        else ss << "-V" << (voiceIndex + 1);
        add("VoiceID");
        set("VoiceID", ss.str(), fout);

        //	set MultiVoice as MultiExecution || Concerto - i.e. we *may* have two voices
        //	running concurrently either if the user has specified MultiExecution (more than
        //	one *execution* may run concurrently) or if we are running Concerto (in which
        //	case there's more than one voice within *this* execution)
        /*
          add("MultiVoice");
          if (voiceCount > 1) set("MultiVoice", true, fout);
          else set("MultiVoice", getb("MultiExecution"), fout);
        */

        //	get host name
        add("HostName");
        set("HostName", brahms::os::gethostname(), fout);

        //	construct list of namespace roots
        string NamespaceRoots = brahms::os::getenv("SYSTEMML_INSTALL_PATH");
#if 0
        if (!NamespaceRoots.length()) {
            ferr << E_INSTALLATION << "env var SYSTEMML_INSTALL_PATH not set";
        }
#endif
        NamespaceRoots += brahms::os::PATH_SEPARATOR;
        NamespaceRoots += "Namespace";
        // FIXME: Set up namespace here somehow: It
        // needs to be set up to match wherever the
        // software is installed; probably something
        // compile-time brought in from BrahmsConfig.h.
        string s = gets("NamespaceRoots");
        if (s.length())
        {
            NamespaceRoots += brahms::os::ENV_SEPARATOR + s;
            if (s.substr(s.length()-1, 1) == brahms::os::ENV_SEPARATOR) {
                NamespaceRoots = NamespaceRoots.substr(s.length()-1);
            }
        }
        fout << "NamespaceRoots=" << NamespaceRoots << D_VERB;

        set("NamespaceRoots", NamespaceRoots, fout);

        //	validate
        //assertType("MultiProcessor", 'b');
        //assertType("MultiExecution", 'b');
        assertType("TimeRunPhase", 'b');
        assertType("ShowGUI", 'b');
        assertType("SocketsUseNagle", 'b');

        //	find node
        const brahms::xml::XMLNodeList* nodes = nodeExecPars.childNodes();

        fout << "Final Execution Parameters:" << D_VERB;
        for(UINT32 n=0; n<nodes->size(); n++)
        {
            const char* nodeName = nodes->at(n)->nodeName();
            const char* nodeText = nodes->at(n)->nodeText();
            fout << nodeName << " = " << nodeText << D_VERB;
        }

        //	set XML to read only, now, so components can't change them
        //	functions in this object will change them by setting this
        //	temporarily to false
        controlExecPars.readonly = true;
    }

} // namespace brahms
