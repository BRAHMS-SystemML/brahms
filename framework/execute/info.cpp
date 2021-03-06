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

	$Id:: info.cpp 2389 2009-11-18 11:40:24Z benjmitch         $
	$Rev:: 2389                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-18 11:40:24 +0000 (Wed, 18 Nov 2009)       $
________________________________________________________________

*/

#include "info.h"

#include "BrahmsConfig.h" // for BUILD_PLATFORM
#include "base/text.h"
using brahms::text::toString;
#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <fstream>
using std::ifstream;
#include "tfs.h"
#include "os.h"
#include "support/os.h"

////////////////	INFORMATION OPERATIONS
#define INFO_OUT cout

#define BREAKLINE { INFO_OUT << "____________________________________________________________\n\n"; }

void brahms::info::version(bool simple)
{
    //	version (command-line client is versioned as the engine, since it's part of the same project!)
    INFO_OUT << "\nBRAHMS Version " << toString(VERSION_ENGINE) << "\n";
    INFO_OUT << "Copyright (C) 2007 Ben Mitchinson\n\n";
    INFO_OUT << "BRAHMS comes with ABSOLUTELY NO WARRANTY. This is free software,\n";
    INFO_OUT << "and you are welcome to redistribute it under certain conditions.\n";
    INFO_OUT << "For details, type \"brahms --license\", and/or visit us at\n";
    INFO_OUT << "http://brahms.sourceforge.net\n\n";
    if (simple) { return; }

    INFO_OUT << "    Built at: " __TIME__ " on " __DATE__ "\n";
    INFO_OUT << "    Built on " << BUILD_PLATFORM << "\n";

    //	defines
    INFO_OUT << "\n    Defined: ";
#ifdef __WIN__
    INFO_OUT << "__WIN__ ";
#endif
#ifdef __LINUX__
    INFO_OUT << "__LINUX__ ";
#endif
#ifdef __OSX__
    INFO_OUT << "__OSX__";
#endif
    INFO_OUT << ARCH_BITS << "BIT ";
#ifdef __GNUC__
    INFO_OUT << "__GNUC__ ";
#endif
#ifdef _MSC_VER
    INFO_OUT << "_MSC_VER ";
#endif
    INFO_OUT << "\n\n";

    INFO_OUT << "    BRAHMS include dir: ";
    brahms::info::brahmsIncludePath();
    INFO_OUT << "    BRAHMS lib dir: ";
    brahms::info::brahmsLibPath();
    INFO_OUT << "    Primary BRAHMS Namespace: ";
    brahms::info::brahmsPrimaryNamespace();

    INFO_OUT << "\n    ARCH_BITS=" << ARCH_BITS << " sizeof(UINTA)=" << sizeof(UINTA) << " sizeof(UINT32)=" << sizeof(UINT32) << " sizeof(Symbol)=" << sizeof(Symbol);
    INFO_OUT << "\n\n";
}

void brahms::info::versionshort()
{
    // Just the version number
    INFO_OUT << toString(VERSION_ENGINE) << "\n";
}

void
brahms::info::usage()
{
    version(true);
    INFO_OUT <<
/*
  "    --show-pars      show Execution Parameters (with working)\n"
  "    --show-expose    show parsed and resolved exposes\n"
  "    --show-affinity  show assignment of processes to voices/threads\n"
  "    --show-logs      show which components were logged (and not logged)\n"
  "    --show-comms     show major communications operations (inter-voice)\n"
*/
        "Usage: brahms <exefile> [<option> ...]\n"
        "       brahms <operation> [<option> ...]\n"
        "\n"

        "    <exefile>        normal usage is to pass a single argument,\n"
        "                     the name of a BRAHMS Execution File.\n"
        "                     see the documentation (via sourceforge site)\n"
        "                     for details of how to construct one.\n"
        "\n"

        "  operations:\n"
        "    --license        show release license\n"
        "    --credits        show credits\n"
        "    --version        show detailed version information\n"
        "    --ver            show just the version number\n"
        "    --showinclude    show the path to the BRAHMS API headers\n"
        "    --showlib        show the path to the BRAHMS library directory\n"
        "    --shownamespace  show the path to the primary BRAHMS Namespace\n"
        "    --walk           walk the namespace for cached components...\n"
        "    --Walk           ...and load each one for more info...\n"
        "    --WALK           ...including non-native components.\n"
        "\n"

        "  general options:\n"
        "    --voice-i        set this to be the ith voice (sockets only)\n"
        "    --voice-mpi      get voice index from MPI layer (mpi only)\n"
        "    --pause          pause before exiting worker threads\n"
        "    --par-X=Y        set Execution Parameter X to value Y\n"
        "    --nogui          Avoid displaying the execution progress window\n"
        "\n"

        "  reporting options:\n"
        "    --logfile-F      send log to file F instead of console\n"
        "    --logfmt-xml     set log format to XML (default is text)\n"
        "    --loglvl-L-T-S   set log level for threads T, section S, to L\n"
        "                     (L can be n,w,i,v,l,f, 0-5; default is i)\n"
        "    --loglvl-L       set starting log level for all threads\n"
        "    --loglvl-L-T     set starting log level for threads T\n"
        "    --loglvl-L--S    set log level for one section for all threads\n"
        "    --maxerr         return maximum error information\n"
        "                     (some is hidden by default for brevity)\n"
        "\n"

        "  shortcuts:\n"
        "    --nothreads      --par-MaxThreadCount-1\n"
        "    --d              --loglvl-f --loglvl-l--Main_Loop --maxerr\n"
        "    --dd             --loglvl-f --maxerr\n"

        ;

    INFO_OUT << endl;
}

void
brahms::info::license()
{
    //	header
    version(true);
    INFO_OUT <<

        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation; either version 2\n"
        "of the License, or (at your option) any later version.\n"
        "\n"

        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"

        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA\n"
        "02110-1301, USA.\n"
        "\n"
        "(a copy of the license follows...)\n"
        "\n"

        ;


    //	just pass through license file
#if STANDALONE_INSTALL
    string systemml_install_path = os::getenv("SYSTEMML_INSTALL_PATH", false);
    if (systemml_install_path.empty()) {
        systemml_install_path = INSTALL_PREFIX;
        systemml_install_path += brahms::os::PATH_SEPARATOR;
        systemml_install_path += "SystemML";
    }
    string filename = systemml_install_path;
    filename += brahms::os::PATH_SEPARATOR;
    filename += "BRAHMS";
    filename += brahms::os::PATH_SEPARATOR;
    filename += "LICENSE";
#else
    string filename(INSTALL_PREFIX);
    filename += brahms::os::PATH_SEPARATOR;
    filename += "share";
    filename += brahms::os::PATH_SEPARATOR;
    filename += "brahms";
    filename += brahms::os::PATH_SEPARATOR;
    filename += "LICENSE";
#endif
    ifstream file(filename.c_str());
    if (file) {
        string s;
        while (getline(file, s)) {
            INFO_OUT << s << "\n";
        }
        file.close();
    } else {
        INFO_OUT <<
            "License file \"LICENSE\" not found - you should be able to find a\n"
            "copy on the web (GNU General Public License version 2), probably at:\n"
            "\n"
            "http://www.gnu.org/licenses/gpl.txt\n"
            "\n"
            "or, alternatively, from the author of this software.\n"
            "\n";
    }
}

void
brahms::info::credits()
{
    //	header
    version(true);
    INFO_OUT <<
        "BRAHMS is primarily the work of Ben Mitch [1], with substantive\n"
        "contributions from Tak-Shing Chan [1], and discursive\n"
        "contributions from others at the ABRG [1] and from Martin\n"
        "Pearson at BRL [2]. Seb James re-engineered the build system (using cmake).\n"
        "Specific inclusions of third-party code are\n"
        "Marsaglia's & Tsang's Ziggurat RNG [3] and Edward Falk's Gauge\n"
        "widget for X Windows [4]. BRAHMS also uses zlib [5] as an\n"
        "external library.\n\n"
        "[1] http://brahms.sourceforge.net\n"
        "[2] http://www.brl.ac.uk\n"
        "[3] http://www.jstatsoft.org/v05/i08\n"
        "[4] http://www.efalk.org/Widgets\n"
        "[5] http://www.zlib.net\n"
        "\n";
}

void
brahms::info::audit()
{
    //	this hidden option just dumps data to console, and is used by bindings
    INFO_OUT << "VERSION_ENGINE=" << toString(VERSION_ENGINE) << endl;
    INFO_OUT << "ARCH_BITS=" << ARCH_BITS << endl;
}

void
brahms::info::brahmsIncludePath()
{
    //	Show the include directory that we're going to install the
    //	brahms-1199.h file (and friends) into.
    INFO_OUT << BRAHMS_INCLUDE_PATH << endl;
}

void
brahms::info::brahmsLibPath()
{
    //	Show the lib directory that we're going to install the
    //	brahms libs into.
    INFO_OUT << BRAHMS_LIB_PATH << endl;
}

void
brahms::info::brahmsPrimaryNamespace()
{
    //	Show the primary, set-at-compile-time BRAHMS Namespace.
    INFO_OUT << PRIMARY_NAMESPACE << endl;
}
