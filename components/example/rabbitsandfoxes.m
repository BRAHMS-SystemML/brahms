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
% $Id:: rabbitsandfoxes.m 2407 2009-11-19 21:49:28Z benjmitch            $
% $Rev:: 2407                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-19 21:49:28 +0000 (Thu, 19 Nov 2009)                   $
%__________________________________________________________________________
%







function rabbitsandfoxes(lang)

if ~nargin || ~(strcmp(lang, '1258') || strcmp(lang, '1262'))
	error(['Usage: rabbitsandfoxes 1258/1262']);
end


% This is an example of using a non-native binding to develop your own
% BRAHMS processes. Here, we will implement the rabbits and foxes
% problem, according to the specification given at
%
% http://www.joma.org/images/upload_library/4/vol2/
% ... liteapplets/Parameter_Plane/rabbits_and_fox.html
%
% d_rabbits = 2 * rabbits - 1.2 * rabbits * foxes
% d_foxes = -foxes + 0.9 * rabbits * foxes



% prepare figure
figure(1)
clf

% first, in matlab script
rabbits = 0.5;
foxes = 0.5;
fS = 100;
executionStop = 10;
N = fS * executionStop;
T = 1/fS;
trace = zeros(N, 2);
for n = 1:N
	d_rabbits = 2 * rabbits - 1.2 * rabbits * foxes;
	d_foxes = -foxes + 0.9 * rabbits * foxes';
	rabbits = rabbits + T * d_rabbits;
	foxes = foxes + T * d_foxes;
	trace(n, :) = [rabbits foxes];
end

% plot
subplot(2, 1, 1)
plot(trace(:,1), 'b-')
hold on
plot(trace(:,2), 'r-')
hold off
drawnow

% then, in BRAHMS, using the matlab wrapper
sys = sml_system;

% add rabbits process
sys = sys.addprocess('rabbits', ['client/brahms/example/' lang '/rabbits'], fS);

% add foxes process
sys = sys.addprocess('foxes', ['client/brahms/example/' lang '/foxes'], fS);

% link them up
sys = sys.link('rabbits>out', 'foxes');
sys = sys.link('foxes>out', 'rabbits');

% construct and launch exe
exe = brahms_execution;
exe.stop = executionStop;
exe.name = 'rabbitsandfoxes';
exe.all = true;

% execute
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
	disp(['My workstation:   ~0.06ms']);
end
disp(' ')

% check
d = [out.rabbits.out(end) out.foxes.out(end)] - [0.222537 0.624588];
maxd = max(abs(d));
if (maxd > 0.001) warning('results look duff!'); end

% plot
subplot(2, 1, 2)
plot(out.rabbits.out, 'b-')
hold on
plot(out.foxes.out, 'r-')
hold off

