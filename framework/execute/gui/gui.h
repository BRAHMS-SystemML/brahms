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

	$Id:: gui.h 2419 2009-11-30 18:33:48Z benjmitch            $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/

#ifndef _GUI_H_
#define _GUI_H_

#include "brahms-client.h"
using namespace brahms;

#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;


#ifdef __WIN__
// includes
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "commctrl.h"

// data types
typedef HANDLE OS_HTHREAD;
typedef FARPROC OS_FUNCTION;
typedef UINT32 OS_TIME;

// re-direction defines
#define os_sleep Sleep
#define os_now clock
#define os_difftimesecs(a, b) (((DOUBLE)b - a) / 1000.0)

const UINT32 BTN_CANCEL = 0x8001;
const char* WND_CLS_NAME = "BRAHMS_GUIMainWnd";
#endif


#ifdef __NIX__
#include <X11/Intrinsic.h>
#include <unistd.h>
#endif

// GUI CLASS DECLARATION

class ExecuteGUI
{

public:

    ExecuteGUI();
    ~ExecuteGUI();

    Symbol MonitorEventHandlerFunc(const MonitorEvent* progress);
    void os_update_progress(DOUBLE fmin, DOUBLE fmax);
    void os_update_operation(const string& msg);
    void os_update_phase(const string& msg);

    void do_events();

    void cancel() { cancelled = true; }

    bool* getPointerToCancelled() { return &cancelled; }

private:

#ifdef __WIN__
    struct
    {
        HINSTANCE   instance;
        ATOM    windowClass;
        HWND    window;
        HDC     deviceContext;

        HFONT    fontPhase;
        HFONT    fontOperation;

        RECT    phase;
        RECT    operation;

        RECT    progbar;
        HBRUSH    progbar_brush_a;
        HBRUSH    progbar_brush_b;
    } gui;
#endif

#ifdef __NIX__
    struct
    {
        Display*   display;
        Screen*    screen;
        Window    window;
        GC     context;

        int sx, sy;

        Widget    application;
        Widget    form;

        Widget    progBarUpper;
        Widget    progBarLower;
        Widget    progressBar;

        Widget    phase;     // label widget for text
        Widget    operation;    // label widget for text
        Widget    cancelButton;
    } gui;

    XtAppContext* app;
#endif

    // common
    bool displayed;
    bool cancelled;
    DOUBLE t_startRunPhase;
};

Symbol MonitorEventHandlerFunc(const MonitorEvent* event);

#endif // _GUI_H_
