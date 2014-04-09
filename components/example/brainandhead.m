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
% $Id:: brainandhead.m 2407 2009-11-19 21:49:28Z benjmitch               $
% $Rev:: 2407                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-19 21:49:28 +0000 (Thu, 19 Nov 2009)                   $
%__________________________________________________________________________
%





function brainandhead(lang)

if ~nargin || ~(strcmp(lang, '1258') || strcmp(lang, '1262'))
	error(['Usage: brainandhead 1258/1262']);
end

% This is an example of using non-native bindings to develop
% your own BRAHMS processes. Here, we will implement the
% brain and head problem (that i've just invented). If you
% are a BRAHMS follower, you will understand.
%
% In short, the BRAIN will generate a bunch of spikes, and
% the HEAD will respond by tilting. Neat, huh?! And... wait
% for it... the HEAD will signal back to the BRAIN something
% exciting about how it tilted. So we'll get feedback, and
% "nodding" will just "emerge"!!! Wow!!!
%
% Yeah, so it's the "BRain And Head Modelling System", if
% you didn't get the reference... :D
%
% You'll see a few tricks that you didn't see in Rabbits &
% Foxes, along with the use of a new data type: spikes.
% Still pretty simplistic stuff, though.



% create system
sys = sml_system;
fS = 250;
executionStop = 1;
N = fS * executionStop;

% add brain process
state = [];
state.numchannels = 40;
sys = sys.addprocess('brain', ['client/brahms/example/' lang '/brain'], fS, state);

% add head process
sys = sys.addprocess('head', ['client/brahms/example/' lang '/head'], fS);

% link them up
sys = sys.link('brain>out', 'head');
sys = sys.link('head>tilt', 'brain');

% construct execution object
exe = brahms_execution;
exe.stop = executionStop;
exe.name = 'brainandhead';
exe.all = true;

% launch executable
[out, rep] = brahms(sys, exe);

% timing
ms = rep.Timing.caller.irt(2) / (N * 2); % 2 calls per step, one to each process
ms = round(ms * 1000000) / 1000;

% report
disp([ 10 'Binding iteration time' ...
	     10 '----------------------' 10 ...
	'  This is an estimate of the maximum total processing' 10 ...
	'  overhead, including all BRAHMS framework overheads' 10 ...
	'  and the overhead due to calling the language engine.' 10 ...
	]);

disp(['Your workstation: ' num2str(ms) 'ms']);
if strcmp(lang, '1258')
	disp(['My workstation:   ~1.2ms']);
else
	disp(['My workstation:   ~0.08ms']);
end
disp(' ')


% prepare figure
figure(1)
clf

subplot(2,1,1)
ts = out.brain.out.ts';
plot(ts(:,1), ts(:,2), 'k.')
axis([out.brain.out.t out.brain.out.s])
title('brain output')

subplot(2,1,2)
plot(out.head.tilt)
title('head tilt (nodding)')


