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

	$Id:: gui.cpp 2419 2009-11-30 18:33:48Z benjmitch          $
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

*/

#include "gui.h"

//////////////// HELPER FUNCTIONS

string n2s(UINT64 val) // any value
{
    stringstream ss;
    ss << val;
    return ss.str();
}

string f2s(DOUBLE val) // any value
{
    stringstream ss;
    ss << val;
    return ss.str();
}

string t2s(UINT32 val) // hh, mm, ss
{
    stringstream ss;
    if (val < 10) ss << "0";
    ss << val;
    return ss.str();
}

string m2s(UINT32 val) // milliseconds
{
    stringstream ss;
    if (val < 100) ss << "0";
    if (val < 10) ss << "0";
    ss << val;
    return ss.str();
}

string prettyPrintTime(DOUBLE t, bool includeMilliseconds)
{
    DOUBLE floorSecs = floor(t);
    UINT32 millisecs = (UINT32)floor((t - floorSecs) * 1000.0);

    UINT32 hours = 0;
    UINT32 mins = 0;
    UINT32 secs = (UINT32)floorSecs;

    while(secs >= 3600)
    {
        hours++;
        secs-=3600;
    }
    while(secs >= 60)
    {
        mins++;
        secs-=60;
    }

    string ret;

    // only include hours if non-zero or skipping milliseconds
    if (hours || !includeMilliseconds) ret += t2s(hours) + ":";

    // only include milliseconds if hours is zero
    if (includeMilliseconds && !hours) return ret + t2s(mins) + ":" + t2s(secs) + "." + m2s(millisecs);
    else return ret + t2s(mins) + ":" + t2s(secs);
}

string gethostname()
{
    char buffer[1024];

#ifdef __WIN__
    DWORD L = 1023;
    if (GetComputerName(buffer, &L))
        return buffer;
#endif

#ifdef __NIX__
    if (!gethostname(buffer, 1023))
        return buffer;
#endif

    return "(unable to get host name)";
}

string titleString(INT32 voiceIndex, UINT32 voiceCount)
{
    string ret;
    ret = "BRAHMS Voice " + n2s(voiceIndex + 1) + " of " + n2s(voiceCount);
    ret += " on " + gethostname();
    return ret;
}

inline DOUBLE sampleRateToRate(SampleRate& sr)
{
    return ((DOUBLE)sr.num) / ((DOUBLE)sr.den);
}

inline DOUBLE sampleRateToPeriod(SampleRate& sr)
{
    return ((DOUBLE)sr.den) / ((DOUBLE)sr.num);
}



// METRICS

// metrics (change to suit taste)
#define xawWidgetLine "384,8,384,-8"

UINT32 metric_WindowWidth = 400;
UINT32 metric_WindowHeight = 184;

const UINT32 metric_HorizontalSpacing = 8;
const UINT32 metric_VerticalSpacing = 8;
const UINT32 metric_ExternalBorder = 12;
const UINT32 metric_ProgressBarHeight = 16;
const UINT32 metric_LineHeight = 16;
const INT32 metric_ButtonWidth = 96;
const INT32 metric_ButtonHeight = 24;



// SUPPORT FOR XAWGAUGE ON LINUX

#ifdef __NIX__

// We use Edward A. Falk's (1997) XawGauge for the progress bar.  Could have used
// XmScale (Motif2.0) or QProgressDialog, but these guys are dependency headaches.

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include "Gauge.h"

XtAppContext* app;

// callback function
static void CancelCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    *(bool*)client_data = true;
}

#endif // __NIX__

extern ExecuteGUI* executeGUI;

// GUI CLASS IMPLEMENTATION

#ifdef __WIN__
// callback
int MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // store this pointer passed from CreateWindowEx
        CREATESTRUCT* cs = (CREATESTRUCT*) lParam;
        ExecuteGUI** progress = (ExecuteGUI**) cs->lpCreateParams;
        SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR) *progress);
        break;
    }

    case WM_COMMAND:
    {
        if (wParam == BTN_CANCEL)
        {
            ExecuteGUI* progress = (ExecuteGUI*) GetWindowLongPtr(hWnd, GWL_USERDATA);
            progress->cancel();
        }
        break;
    }

    default:
        return (DefWindowProc(hWnd, uMsg, wParam, lParam));
    }

    return 0;
}
#endif // __WIN__

ExecuteGUI::ExecuteGUI()
    : t_startRunPhase(0.0)
    , nogui(false)
    , displayed(false)
    , cancelled(false)
{
#ifdef __WIN__
    // zero gui data on windows
    memset(&gui, 0, sizeof(gui));
#endif
#ifdef __NIX__
    app = (XtAppContext*)0;
#endif
}

ExecuteGUI::ExecuteGUI(bool without_gui)
    : t_startRunPhase(0.0)
    , nogui(without_gui)
    , displayed(false)
    , cancelled(false)
{
#ifdef __WIN__
    // zero gui data on windows
    memset(&gui, 0, sizeof(gui));
#endif
#ifdef __NIX__
    app = (XtAppContext*)0;
#endif
}

ExecuteGUI::~ExecuteGUI()
{
#ifdef __WIN__
    // Destroy Brushes
    if (gui.progbar_brush_a) DeleteObject(gui.progbar_brush_a);
    if (gui.progbar_brush_b) DeleteObject(gui.progbar_brush_b);

    // Destroy Font
    if (gui.fontPhase) DeleteObject((HFONT)gui.fontPhase);
    if (gui.fontOperation) DeleteObject((HFONT)gui.fontOperation);

    // Release gui.deviceContext
    if (gui.deviceContext) ReleaseDC((HWND)gui.window,(HDC)gui.deviceContext);

    // Destroy window
    if (gui.window) DestroyWindow((HWND)gui.window);

    // Unregister class
    if (gui.windowClass && !UnregisterClass(WND_CLS_NAME, (HINSTANCE)gui.instance))
        ; // ignore
#endif // __WIN__

#ifdef __NIX__
    if (displayed && app != (XtAppContext*)0) XtDestroyApplicationContext(*app);
    if (app != (XtAppContext*)0) { delete app; }
    if (gui.display) XCloseDisplay(gui.display);
#endif
}

Symbol ExecuteGUI::MonitorEventHandlerFunc(const MonitorEvent* event)
{
    // switch on event type
    switch (event->type)
    {
#ifdef __WIN__
    case EVENT_MONITOR_SHOW:
    {
        // Init Common Controls
        INITCOMMONCONTROLSEX icce;
        icce.dwSize = sizeof(icce);
        icce.dwICC = ICC_PROGRESS_CLASS;
        if (!InitCommonControlsEx(&icce))
            throw string("failed to init common controls");

        // Get instance handle
        gui.instance = GetModuleHandle(NULL);

        // Register window class
        WNDCLASSEX wce =
            {
                sizeof(WNDCLASSEX), 0,
                (WNDPROC)MainWindowProc, 0L, 0L,
                gui.instance, NULL, NULL,
                HBRUSH(COLOR_BTNFACE+1),
                NULL,
                WND_CLS_NAME,
                NULL
            };

        gui.windowClass = RegisterClassEx(&wce);
        if (!gui.windowClass)
        {
            int gle=GetLastError();
            if (gle!=0x0582) // already exists
                throw string("failed to register main window class");
        }

        // Create main window
        ExecuteGUI* progress = this;
        gui.window = CreateWindowEx(
            0/* | WS_EX_TOOLWINDOW*/, WND_CLS_NAME, titleString(event->createEngine->voiceIndex, event->createEngine->voiceCount).c_str(),
            0L, 0, 0, metric_WindowWidth, metric_WindowHeight,
            GetDesktopWindow(), NULL, gui.instance, &progress /* passed as lParam to WM_CREATE */
            );
        if (!gui.window)
            throw string("failed to create window");

        // Get client rectangle
        RECT leftrect;
        GetClientRect(gui.window, &leftrect);

        // we requested a window of a given size, but our client will
        // actually be slightly smaller than that because of the window
        // borders, so we'll measure the difference, and then re-request
        // the window size so we get the desired client window size
        metric_WindowWidth += (metric_WindowWidth - leftrect.right);
        metric_WindowHeight += (metric_WindowHeight - leftrect.bottom);
        SetWindowPos((HWND)gui.window, NULL,
                     0, 0,
                     metric_WindowWidth, metric_WindowHeight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);

        // Get client rectangle
        GetClientRect(gui.window, &leftrect);

        // fix up (note right and bottom are actually width and height)
        leftrect.top = metric_ExternalBorder;
        leftrect.bottom -= 2 * metric_ExternalBorder;
        leftrect.left = metric_ExternalBorder;
        leftrect.right -= 2 * metric_ExternalBorder;

        // 10 point
        gui.deviceContext = GetDC(gui.window);
        if (!gui.deviceContext)
            throw string("failed to get device context");
        UINT32 nHeight = -MulDiv(10, GetDeviceCaps(gui.deviceContext, LOGPIXELSY), 72);

        // Add the message zone and set message stuff
        gui.fontPhase = CreateFont(nHeight,0,0,0,FW_BOLD,0,0,0,0,0,0,0,0,"Arial");
        gui.fontOperation = CreateFont(nHeight,0,0,0,0,0,0,0,0,0,0,0,0,"Arial");

        // Set gui.deviceContext mode
        SetBkMode((HDC)gui.deviceContext, TRANSPARENT);
        SelectObject((HDC)gui.deviceContext, GetStockObject(NULL_PEN));
        SelectObject((HDC)gui.deviceContext, CreateSolidBrush(GetSysColor(COLOR_BTNFACE)));

        // add cancel button
        HWND hWndCancel = CreateWindowEx(
            0L, "BUTTON", "Cancel", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
            leftrect.left+leftrect.right-metric_ButtonWidth,
            leftrect.top+leftrect.bottom-metric_ButtonHeight,
            metric_ButtonWidth,
            metric_ButtonHeight,
            gui.window, (HMENU)BTN_CANCEL, gui.instance, NULL
            );
        if (hWndCancel && gui.fontOperation)
            SendMessage(hWndCancel, WM_SETFONT, LPARAM(gui.fontOperation), MAKELPARAM(true,0));
        leftrect.bottom -= (metric_VerticalSpacing + metric_ButtonHeight);

        // add the progress bar
        gui.progbar = leftrect;
        gui.progbar.bottom = gui.progbar.top + gui.progbar.bottom;
        gui.progbar.top = gui.progbar.bottom - metric_ProgressBarHeight;
        gui.progbar.right = gui.progbar.left + gui.progbar.right;
        leftrect.bottom -= (metric_VerticalSpacing + metric_ProgressBarHeight);

        // create progress bar brushes
        gui.progbar_brush_a = CreateSolidBrush(RGB(255,0,0));
        gui.progbar_brush_b = CreateSolidBrush(RGB(0,128,0));

        // phase (convert left/right to absolute positions for use in the DC)
        gui.phase = leftrect;
        gui.phase.bottom = metric_LineHeight + metric_VerticalSpacing;
        gui.phase.right += gui.phase.left;
        gui.phase.bottom += gui.phase.top;
        leftrect.top += metric_LineHeight + metric_VerticalSpacing;
        leftrect.bottom -= (metric_LineHeight + metric_VerticalSpacing);

        // operation (convert left/right to absolute positions for use in the DC)
        gui.operation = leftrect;
        gui.operation.right += gui.operation.left;
        gui.operation.bottom += gui.operation.top;

        // Stagger GUIs of different voices across screen

        // get screen size (work area, minus task bar)
        RECT WorkArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);

        // calculate
        DOUBLE vi = event->createEngine->voiceIndex;
        DOUBLE vc = event->createEngine->voiceCount;
        DOUBLE minl = WorkArea.left;
        DOUBLE maxl = WorkArea.right - metric_WindowWidth;
        DOUBLE mint = WorkArea.top;
        DOUBLE maxt = WorkArea.bottom - metric_WindowHeight;
        DOUBLE fac = (vi + 0.5) / vc;
        DOUBLE left = fac * (maxl - minl) + minl;
        DOUBLE top = fac * (maxt - mint) + mint;

        // set
        SetWindowPos((HWND)gui.window, NULL,
                     left, top,
                     metric_WindowWidth, metric_WindowHeight,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);

        // Show and Update the Window
        ShowWindow((HWND)gui.window, SW_SHOW);
        UpdateWindow((HWND)gui.window);

        // ok
        displayed = true;
        break;
    }
#endif // __WIN__

#ifdef __NIX__
    case EVENT_MONITOR_SHOW:
    {
        if (this->nogui == true) {
            break;
        }

        // fallback resources
        const char* fallback_resources[] = {
            "brahms*font: -*-arial-medium-r-normal-*-12-*-*-*-*-*-*-*",
            "brahms.*.foreground: black",
            "brahms.*.background: #ece9d8",
            "brahms.*.progBarUpper.foreground: #008000",
            "brahms.*.progBarLower.foreground: #FF0000",
            "brahms.*.cancelButton.background: #ece9d8",
            NULL
        };

        // check X server availability
        gui.display = XOpenDisplay(NULL);
        if (!gui.display) break;

        // Instantiate XtAppContext
        app = new XtAppContext;

        // screen size
        gui.screen = DefaultScreenOfDisplay(gui.display);
        gui.sx = WidthOfScreen(gui.screen);
        gui.sy = HeightOfScreen(gui.screen);

        // calculate
        DOUBLE vi = event->createEngine->voiceIndex;
        DOUBLE vc = event->createEngine->voiceCount;
        DOUBLE minl = 0;
        DOUBLE maxl = gui.sx - metric_WindowWidth;
        DOUBLE mint = 0;
        DOUBLE maxt = gui.sy - metric_WindowHeight;
        DOUBLE fac = (vi + 0.5) / vc;
        UINT32 left = fac * (maxl - minl) + minl;
        UINT32 top = fac * (maxt - mint) + mint;

        // open application
        XtSetLanguageProc(NULL, NULL, NULL);
        int argc = 1;
        const char* argv[] = { "brahms", NULL };
        gui.application = XtVaOpenApplication(app, "brahms", NULL, 0, &argc, const_cast<char**>(argv),
                                              const_cast<char**>(fallback_resources),
                                              sessionShellWidgetClass,
                                              XtNtitle, titleString(event->createEngine->voiceIndex, event->createEngine->voiceCount).c_str(),
                                              XtNwidth, metric_WindowWidth,
                                              XtNheight, metric_WindowHeight,
                                              XtNminWidth, metric_WindowWidth,
                                              XtNminHeight, metric_WindowHeight,
                                              XtNmaxWidth, metric_WindowWidth,
                                              XtNmaxHeight, metric_WindowHeight,
                                              XtNx, left, XtNy, top,
                                              NULL);

        // create form
        gui.form = XtVaCreateManagedWidget("form", formWidgetClass, gui.application,NULL);

        // place widgets on form
        gui.phase = XtVaCreateManagedWidget("phase", labelWidgetClass, gui.form,
                                            XtNlabel, "\n",   // reserve space for N lines
                                            XtNtop, XtChainTop,
                                            XtNleft, XtChainLeft,
                                            XtNbottom, XtChainTop,
                                            XtNright, XtChainLeft,
                                            XtNhorizDistance, metric_ExternalBorder,
                                            XtNvertDistance, metric_ExternalBorder,
                                            XtNwidth, metric_WindowWidth - 2 * metric_ExternalBorder,
                                            XtNborderWidth, 0,
                                            XtNjustify, XtJustifyLeft,
                                            NULL);
        gui.operation = XtVaCreateManagedWidget("operation", labelWidgetClass, gui.form,
                                                XtNlabel, "\n\n\n\n\n",   // reserve space for N lines
                                                XtNtop, XtChainTop,
                                                XtNleft, XtChainLeft,
                                                XtNbottom, XtChainTop,
                                                XtNright, XtChainLeft,
                                                XtNhorizDistance, metric_ExternalBorder, /* "left" */
                                                XtNvertDistance, metric_ExternalBorder + metric_LineHeight + metric_VerticalSpacing, /* "top" */
                                                XtNwidth, metric_WindowWidth - 2 * metric_ExternalBorder, /* "width" */
                                                XtNborderWidth, 0,
                                                XtNjustify, XtJustifyLeft,
                                                NULL);

        gui.progBarLower = XtVaCreateManagedWidget("progBarLower", gaugeWidgetClass, gui.form,
                                                   XtNtop, XtChainTop,
                                                   XtNleft, XtChainLeft,
                                                   XtNbottom, XtChainTop,
                                                   XtNright, XtChainLeft,
                                                   XtNhorizDistance, metric_ExternalBorder,
                                                   XtNvertDistance, metric_WindowHeight - metric_ExternalBorder - metric_ButtonHeight - metric_VerticalSpacing - (metric_ProgressBarHeight/2),
                                                   XtNwidth, metric_WindowWidth - 2 * metric_ExternalBorder,
                                                   XtNheight, metric_ProgressBarHeight,
                                                   XtNborderWidth, 0,
                                                   XtNvalue, 0,
                                                   XtNminValue, 0,
                                                   XtNmaxValue, 100,
                                                   NULL);
        gui.progBarUpper = XtVaCreateManagedWidget("progBarUpper", gaugeWidgetClass, gui.form,
                                                   XtNtop, XtChainTop,
                                                   XtNleft, XtChainLeft,
                                                   XtNbottom, XtChainTop,
                                                   XtNright, XtChainLeft,
                                                   XtNhorizDistance, metric_ExternalBorder,
                                                   XtNvertDistance, metric_WindowHeight - metric_ExternalBorder - metric_ButtonHeight - metric_VerticalSpacing - 2 * (metric_ProgressBarHeight/2)-2,
                                                   XtNwidth, metric_WindowWidth - 2 * metric_ExternalBorder,
                                                   XtNheight, metric_ProgressBarHeight,
                                                   XtNborderWidth, 0,
                                                   XtNvalue, 0,
                                                   XtNminValue, 0,
                                                   XtNmaxValue, 100,
                                                   NULL);

        gui.cancelButton = XtVaCreateManagedWidget("cancelButton", commandWidgetClass, gui.form,
                                                   XtNlabel, "Cancel",
                                                   XtNtop, XtChainTop,
                                                   XtNleft, XtChainLeft,
                                                   XtNbottom, XtChainTop,
                                                   XtNright, XtChainLeft,
                                                   XtNhorizDistance, metric_WindowWidth - metric_ButtonWidth - metric_ExternalBorder,
                                                   XtNvertDistance, metric_WindowHeight - metric_ExternalBorder - metric_ButtonHeight,
                                                   XtNwidth, metric_ButtonWidth,
                                                   XtNheight, metric_ButtonHeight,
                                                   XtNshapeStyle, XawShapeRectangle,
                                                   NULL);

        // start application
        XtAddCallback(gui.cancelButton, XtNcallback, CancelCB, (XtPointer)getPointerToCancelled());
        XtRealizeWidget(gui.application);

        displayed = true;
        break;
    }
#endif // __NIX__

    case EVENT_MONITOR_CLOSE:
    {
        if (!displayed) break;
#ifdef __WIN__
        if (gui.window)
            ShowWindow((HWND)gui.window, SW_HIDE);
#endif
        break;
    }

    case EVENT_MONITOR_SERVICE:
    {
        if (!displayed) break;

        // get progress as a fraction and update progress bars
        MonitorTime* t = event->monitorTime;
        BaseSamples tmin = t->now;
        BaseSamples tmax = t->nowAdvance;
        DOUBLE fmax = ((DOUBLE)tmax) / ((DOUBLE)t->executionStop);
        DOUBLE fmin = ((DOUBLE)tmin) / ((DOUBLE)t->executionStop);

        // measure run time in seconds
        double secsElapsed = t->wallclockTime - t_startRunPhase;
        double secsPerExecution = secsElapsed / fmin;
        double secsRemaining = (1.0 - fmin) * secsPerExecution;

        // update message
        DOUBLE bsp = sampleRateToPeriod(t->baseSampleRate);
        string out;
        out += "Base Sample Rate: " + f2s(1.0 / bsp) + "Hz, T=" + f2s(bsp) + "s" + "\n";
        out += "Base Clock: " + n2s(tmin) + "(S) - " + n2s(tmax) + "(F) / " + n2s(t->executionStop) + "\n";
        out += "Wall Clock: " + prettyPrintTime(secsElapsed, false)
            + " (rem. " + prettyPrintTime(secsRemaining, false) + ")\n";
        out += "Execution Clock: " + prettyPrintTime(double(tmin) * bsp, true)
            + " (rem. " + prettyPrintTime(double(t->executionStop - tmin) * bsp, true) + ")\n";

        // add comms info
        if (event->message)
            out += event->message;

        // call os-specific sub-function
        os_update_progress(fmin, fmax);
        os_update_operation(out);

        break;
    }

    case EVENT_MONITOR_OPERATION:
    {
        if (!displayed) break;

        // call os-specific sub-function
        os_update_operation(event->message);

        break;
    }

    case EVENT_MONITOR_PHASE:
    {
        if (!displayed) break;

        // just update message
        os_update_phase(event->message);

        // record time
        if (event->phase == EP_EXECUTE)
            t_startRunPhase = event->monitorTime->wallclockTime;

        break;
    }

    case EVENT_MONITOR_PROGRESS:
    {
        if (!displayed) break;

        // just update message
        os_update_progress(event->progressMin, event->progressMax);

        break;
    }
    }

    if (cancelled) return C_CANCEL;
    return C_OK;
}

#ifdef __NIX__

void ExecuteGUI::os_update_progress(DOUBLE fmin, DOUBLE fmax)
{
    // draw the text and update bar
    XtVaSetValues(gui.progBarUpper, XtNvalue, (int)(fmax * 100.0), NULL);
    XtVaSetValues(gui.progBarLower, XtNvalue, (int)(fmin * 100.0), NULL);

    do_events();
}

void ExecuteGUI::os_update_phase(const string& msg)
{
    XtVaSetValues(gui.phase, XtNlabel, msg.c_str(), NULL);

    // update
    XSync(XtDisplay(gui.application), False);
    do_events();
}

void ExecuteGUI::os_update_operation(const string& msg)
{
    // must have five line-feeds to be correctly positioned
    int count = 0;
    const char* c = msg.c_str();
    for (UINT32 i=0; i<msg.length(); i++)
        if (c[i] == 10) count++;
    string msg_ = msg;
    while(count < 5)
    {
        msg_ += "\n";
        count++;
    }

    XtVaSetValues(gui.operation, XtNlabel, msg_.c_str(), NULL);

    // update
    XSync(XtDisplay(gui.application), False);
    do_events();
}

void ExecuteGUI::do_events()
{
    // do events
    if (app != (XtAppContext*)0) {
        XEvent event;
        while (XtAppPending(*app))
        {
            XtAppNextEvent(*app, &event);
            XtDispatchEvent(&event);
        }
    }
}
#endif // __NIX__

void ExecuteGUI::hide_gui()
{
    this->nogui = true;
}

#ifdef __WIN__

void ExecuteGUI::os_update_progress(DOUBLE fmin, DOUBLE fmax)
{
    cout << "os_update_progress" << endl;
    // draw progress bar background
    HGDIOBJ oldb, oldp;
    oldb = SelectObject(gui.deviceContext, GetStockObject(WHITE_BRUSH));
    oldp = SelectObject(gui.deviceContext, GetStockObject(BLACK_PEN));
    Rectangle(gui.deviceContext, gui.progbar.left, gui.progbar.top, gui.progbar.right, gui.progbar.bottom);
    SelectObject(gui.deviceContext, oldb);
    SelectObject(gui.deviceContext, oldp);

    // draw progress bars
    oldp = SelectObject(gui.deviceContext, GetStockObject(NULL_PEN));
    oldb = SelectObject(gui.deviceContext, gui.progbar_brush_a);
    UINT32 right = gui.progbar.left + 1 + fmax * ((DOUBLE)(gui.progbar.right - gui.progbar.left - 1));
    Rectangle(gui.deviceContext, gui.progbar.left+1, gui.progbar.top+1, right, gui.progbar.bottom);
    SelectObject(gui.deviceContext, gui.progbar_brush_b);
    right = gui.progbar.left + 1 + fmin * ((DOUBLE)(gui.progbar.right - gui.progbar.left - 1));
    Rectangle(gui.deviceContext, gui.progbar.left+1, gui.progbar.top+1, right, gui.progbar.bottom);
    SelectObject(gui.deviceContext, oldb);
    SelectObject(gui.deviceContext, oldp);
}

void ExecuteGUI::os_update_operation(const string& msg)
{
    // update msg
    if (gui.deviceContext)
    {
        // draw the text
        SelectObject(gui.deviceContext, gui.fontOperation);
        Rectangle(gui.deviceContext,
                  gui.operation.left, gui.operation.top, gui.operation.right + 1, gui.operation.bottom + 1);
        DrawText(gui.deviceContext,
                 msg.c_str(), msg.length(), &gui.operation, DT_LEFT);

        // repaint
        PostMessage(gui.window, WM_PAINT, 0, 0);

        // do events
        do_events();
    }
}

void ExecuteGUI::os_update_phase(const string& msg)
{
    // update msg
    if (gui.deviceContext)
    {
        // draw the text
        SelectObject(gui.deviceContext, gui.fontPhase);
        Rectangle(gui.deviceContext,
                  gui.phase.left, gui.phase.top, gui.phase.right + 1, gui.phase.bottom + 1);
        DrawText(gui.deviceContext,
                 msg.c_str(), msg.length(), &gui.phase, DT_LEFT);

        // repaint
        PostMessage(gui.window, WM_PAINT, 0, 0);

        // do events
        do_events();
    }
}

void ExecuteGUI::do_events()
{
    MSG m;
    while(PeekMessage(&m, 0, 0, 0, PM_REMOVE))
        DispatchMessage(&m);
}
#endif // __WIN__

// STATIC EXPORTED FUNCTION
Symbol MonitorEventHandlerFunc(const MonitorEvent* progress)
{
    return executeGUI->MonitorEventHandlerFunc(progress);
}
