
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
% $Id:: tutorial_4.m 1751 2009-04-01 23:36:40Z benjmitch                 $
% $Rev:: 1751                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)                   $
%__________________________________________________________________________
%


% Tutorial 4 : Recursive connectivity
%
% By the end of this tutorial you should understand the
% following concepts:
%
%   * that BRAHMS processes can connect to themselves...
%   * ...and more than once!
%   * BRAHMS Execution Report and how to use it
%
% Extra credit tasks are listed at the end of this code

% HOW TO WORK THROUGH THIS TUTORIAL
%
% In this tutorial, we will connect two inputs to a sum
% block: a numeric input, and the sum block's own output.
% The sum block will, thus, integrate, computing:
%
%   y(n) = y(n-1) + x(n-1)
%
% where the "-1" parts reflect the unity lag of the links we
% are using (you have seen how to change this lag in a
% previous tutorial).
%
% * Run the tutorial as provided to see this in action.
%
% * Next, set "connect_twice" to true, to connect the output
% of the sum block as two of the sum block's inputs. Then,
% the sum block is computing:
%
%   y(n) = y(n-1) + y(n-1) + x(n-1)
%   y(n) = 2y(n-1) + x(n-1)
%
% which, over time, tends towards
%
%   y(n) = 2y(n-1)
%
% i.e. exponential growth with time.

% parameters
fS = 5;                  % sample rate
stop = 1;                % execution stop time
connect_twice = false;   % connect twice?




% construct the system: start with an empty system
sys = sml_system;

%%%% PROCESS: NUMERIC SOURCE 1 RUNS AT 5HZ

% construct a process, a source of numeric data
state = [];
state.data = 1;             % output is always just a scalar unity
state.ndims = 1;            % since we're using a scalar, we have to set ndims explicitly
state.repeat = true;        % reuse data if it runs out (i.e. keep sending "1")
sys = sys.addprocess('src', 'std/2009/source/numeric', fS, state);




%%%% PROCESS: SUM ADDS OUTPUT OF SOURCE 1 AND ITS OWN OUTPUT

% construct a process, a sum block (don't worry about the
% state information now, we'll cover that elsewhere - or
% look at the standard library reference for the sum block)
state = [];
state.complex = false;
state.dims = uint32(1);
sys = sys.addprocess('sum', 'std/2009/math/esum', fS, state);


% link the numeric source to the input of the sum block, as
% well as the output of the sum block itself
sys = sys.link('src>out', 'sum');
sys = sys.link('sum>out', 'sum');


if connect_twice

    % construct a second link joining the output of sum to
    % the input of sum
		sys = sys.link('sum>out', 'sum');
	
end



%%%% EXECUTION

% construct the execution
exe = brahms_execution;
exe.stop = stop;     % stop the simulation after one second
exe.all = true;      % log all the outputs of the entire system

% execute the system - this time adding an extra return field
% to access the Execution Report
[out, rep] = brahms(sys, exe);




%%%% DISPLAY

% plot the output of the sum against time
t = (1:(exe.executionStop*fS)) / fS;
plot(t,out.src.out,'b.-')
hold on
plot(t,out.sum.out,'r.-')
hold off
xlabel('time (s)')
ylabel('output');
axis([0 exe.stop 0 max(out.sum.out)])
legend('src','sum',2)



%%%% PERFORMANCE MONITORING

% the Execution Report returns (or is slated to return) a
% wide assortment of metadata regarding how the execution
% went, but currently the only salient data therein is
% performance monitoring (timing) data. you can explore the
% structure "rep" yourself, but it's easier to get to grips
% with it by passing it to "brahms_perf". more info on this
% in a later tutorial which will cover performance
% monitoring and how to use it to improve performance.
brahms_perf(rep)



% FOR A COMPLETE UNDERSTANDING:
%
%   * With exponential growth disabled (i.e. with the system
%   only singly connected), the first output of the
%   integrator is zero, at t = 1/fS, and the second is equal
%   to unity. Why?
%
%   * Try messing about with the sample rate "fS", and the
%   simulation length "stop".
%
%   * Try examining how long each process took to compute
%   using the call "brahms_perf(rep)". To get interesting
%   timings, you may need to set "fS" and "stop" very high.
%   With such a simple system, you should find that the
%   "run phase overhead" is rather large (perhaps upwards
%   of 50%) since as much time is spent switching and
%   marshalling as actually processing these simple
%   operations. If you turn off multi-threading, as shown
%   below, you'll see the change reflected in the output of
%   brahms_perf.
%
% Execute without multi-threading:
%   [out, rep] = brahms(sys, exe, '--nothreads');
