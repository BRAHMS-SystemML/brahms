
%__________________________________________________________________________
%
% This file is part of BRAHMS
% Copyright (C) 2007 Ben Mitchinson
% URL: http://brahms.sourceforge.net
%
% This program is free software; you can redistribute it and/or modify it
% under the terms of the GNU General Public License as published by the
% Free Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% General Public License for more details.
%
% You should have received a copy of the GNU General Public License along
% with this program; if not, write to the Free Software Foundation, Inc.,
% 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
%__________________________________________________________________________
%
% $Id:: developing_systems.m 2329 2009-11-09 00:13:25Z benjmitch         $
% $Rev:: 2329                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-09 00:13:25 +0000 (Mon, 09 Nov 2009)                   $
%__________________________________________________________________________
%




%% EMPTY SYSTEM

% section header
disp([10 '=== Empty System ===' 10])

% construct an empty system
sys = sml_system;

% construct a default execution
exe = brahms_execution;

% view the empty system and
% the default execution
sys
exe

% execute the system
out = brahms(sys, exe);

% view the system outputs
out



%% ADD PROCESS

% section header
disp([10 '=== Add Process ===' 10])

% choose the class of a new process
cls = 'std/2009/source/numeric';

% design the parameters of the new process
state = [];
state.data = 1:10;
state.repeat = false;

% choose its sample rate (Hz)
fS = 10;

% add process to the system using these parameters
sys = sys.addprocess('src', cls, fS, state);

% view the non-empty system
sys

% execute the system
out = brahms(sys, exe);

% view the system outputs
out




%% TURN ON LOGGING

% section header
disp([10 '=== Turn On Logging ===' 10])

% turn on logging for all output ports
exe.all = true;

% set execution stop time
exe.stop = 1;

% view the non-default execution
exe

% execute the system
out = brahms(sys, exe);

% view the system outputs
out
out.src




%% ADD LINK

% section header
disp([10 '=== Add Link ===' 10])

% choose the class of a new process
cls = 'std/2009/resample/numeric';

% design the parameters of the new process
state = [];
state.order = 1;

% choose its sample rate (Hz)
fS = 7;

% add process to the system using these parameters
sys = sys.addprocess('dst', cls, fS, state);

% view the two process system
sys

% add link from src to dst
sys = sys.link('src>out', 'dst');

% view the linked system
sys

% execute the system
out = brahms(sys, exe);

% view the system outputs
out
out.src
out.dst




%% ADD SUB-SYSTEM

% section header
disp([10 '=== Add Sub-system ===' 10])

% construct a parent system (and set its title)
psys = sml_system;

% add the original system to it as a sub-system
psys = psys.addsubsystem('sys', sys);

% add a new process to the parent system
cls = 'std/2009/resample/numeric';
fS = 10;
state = [];
state.order = 1;
psys = psys.addprocess('resamp', cls, fS, state);

% link the output of sys/dst to the input of resamp
psys = psys.link('sys/dst>out', 'resamp');

% view the parent system
psys

% execute the system
out = brahms(psys, exe);

% view the system outputs
out
out.sys
out.sys.src
out.sys.dst
out.resamp




%% EXPOSE SUB-SYSTEM

% section header
disp([10 '=== Expose Sub-system ===' 10])

% unlink
psys = psys.unlink('sys/dst>out', 'resamp');

% remove the sub-system from the parent system
psys = psys.removesubsystem('sys');

% expose the output port of the sub-system
sys = sys.expose('dst>out', 'out');

% add the modified system to the parent as a sub-system
psys = psys.addsubsystem('sys', sys);

% link to this exposed port instead
psys = psys.link('sys>out', 'resamp');

% view the parent system
psys

% execute the system
out = brahms(psys, exe);

% view the system outputs
out




%% USING CONCERTO

% section header
disp([10 '=== Using Concerto (1) ===' 10])

% instruct BRAHMS to use three voices, all local
exe.addresses = {'sockets' 3};

% view the execution
exe

% execute the system
out = brahms(psys, exe);

% view the system outputs
out

% section header
disp([10 '=== Using Concerto (2) ===' 10])

% instruct BRAHMS to use three voices, distributed
exe.addresses = {'192.168.101.1' '192.168.101.2' '192.168.101.3'};

% review the default launch line
exe.launch

% instruct BRAHMS how to launch each voice
exe.launch = [ ...
	'each ssh -XC ((ADDR)) "brahms ((EXECFILE))' ...
	' --logfmt-xml --voice-((VOICE)) --logfile-((LOGFILE))' ...
	' --exitfile-((EXITFILE)) ((ARGS))"' ...
	];

% view the execution
exe

% DISABLED
% often this won't work without some tweaking
% 
% % execute the system
% out = brahms(psys, exe);
% 
% % view the system outputs
% out
% 
% DISABLED




%% MORE ON LOGGING

% section header
disp([10 '=== More On Logging ===' 10])

% back to Solo mode
exe.addresses = {};
exe.launch = '';

% stop logging all outputs
exe.all = false;

% log some particular outputs
exe = exe.log('resamp>out');

% execute the system
out = brahms(psys, exe);

% view the system outputs
out
out.resamp




%% REVIEW PERFORMANCE

% section header
disp([10 '=== Review Performance ===' 10])

% execute the system
[out, rep] = brahms(psys, exe);

% review performance
brahms_perf(rep);

% turn on run-phase timing
exe.execPars.TimeRunPhase = 1;

% turn off multi-threading
exe.execPars.MaxThreadCount = 1;

% execute the system
[out, rep] = brahms(psys, exe);

% review performance
brahms_perf(rep);
