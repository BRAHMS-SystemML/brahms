%
% brahms_perf(report, digits)
%   Display BRAHMS performance information: report the
%   performance information returned by BRAHMS in
%   human-readable form. Pass the BRAHMS report structure.
%   Set "digits" to specify the number of decimal places to
%   display.
%
% (see also brahms_gperf)
%
% CPU tick is assumed to be at 64Hz for all CPUs (that's what pentiums
% clock at i think). "RUP" is Runtime Usermode Percentage, and is a measure
% of how much time the CPU was doing real work, rather than context
% switching or the like. Yes, it could easily have been "RUMP".
%
% __________________________________________________________
% This function is part of the "BRAHMS Matlab Bindings".

%________________________________________________________________
%
%	This file is part of BRAHMS
%	Copyright (C) 2007 Ben Mitchinson
%	URL: http://brahms.sourceforge.net
%
%	This program is free software; you can redistribute it and/or
%	modify it under the terms of the GNU General Public License
%	as published by the Free Software Foundation; either version 2
%	of the License, or (at your option) any later version.
%
%	This program is distributed in the hope that it will be useful,
%	but WITHOUT ANY WARRANTY; without even the implied warranty of
%	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%	GNU General Public License for more details.
%
%	You should have received a copy of the GNU General Public License
%	along with this program; if not, write to the Free Software
%	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
%________________________________________________________________
%
%	Subversion Repository Information (automatically updated on commit)
%
%	$Id:: brahms_perf.m 2398 2009-11-18 21:53:08Z benjmitch    $
%	$Rev:: 2398                                                $
%	$Author:: benjmitch                                        $
%	$Date:: 2009-11-18 21:53:08 +0000 (Wed, 18 Nov 2009)       $
%________________________________________________________________
%

function brahms_perf(rep, digits)

if nargin<2 digits = 3; end


% if passed multiple, pass them one by one
if isstruct(rep) && length(rep) > 1
	for n = 1:length(rep)
		brahms_perf(rep(n), digits)
	end
	return
end

% report head
head = rep.Header;
disp([10 'Performance Monitoring from Voice ' head.PartIndex 10]);

% extract from report structure
timing = rep.Timing;

% extract
Nt = length(timing.threads);

% assume a tick for percentages
tick = 1/64;

% extract all times to start with, so we can set some stuff
process_runphase_wallclock_times = [];
for t = 1:Nt
	
	% get timing
	thread = timing.threads(t);
	procs = thread.processes;
	Np = length(procs);
	
	% for each process in it
	for p = 1:Np
		proc = procs(p);
		process_runphase_wallclock_times(end+1,:) = [t p proc.irt(2)];
	end

end




% was timing information requested?
if Nt == 0 || sum(process_runphase_wallclock_times(:,3)) == 0
	requested = false;
else
	requested = true;
end


% % indicate which processes used up 90% of the processing time
marked_fraction = 0.9;

% sort by usage
heavy_users = zeros(0,2);
if Nt
	[temp, i] = sort(-process_runphase_wallclock_times(:,3));
	if sum(temp)
		process_runphase_wallclock_times = process_runphase_wallclock_times(i,:);
		process_runphase_wallclock_times_cum = cumsum(process_runphase_wallclock_times(:,3));
		process_runphase_wallclock_times_cum = process_runphase_wallclock_times_cum / process_runphase_wallclock_times_cum(end);
		heavy_usage = 0;
		i = 1;
		while heavy_usage < marked_fraction
			heavy_usage = process_runphase_wallclock_times_cum(i);
			heavy_users = [heavy_users; process_runphase_wallclock_times(i,1:2)];
			i = i + 1;
		end
	end
end


% set format
% dp = 4;
% highest = min(max(max(tp(:,1:end-1))),1);
% while(highest < 1)
% 	dp = dp + 1;
% 	highest = highest * 10;
% end
% if nargin < 2
% 	digits = dp;
% end
% numL = digits + 6;
numL = digits+8;
fmt = ['%' int2str(numL) '.' int2str(digits) 'f'];
nameL = 20;

% header
lineL = nameL + 4 * numL;
linebreak(lineL)
disp([padstr('PROCESS',nameL) rpadstr('INIT',numL) rpadstr('RUN',numL) rpadstr('TERM',numL) rpadstr('TOTAL',numL)])
linebreak(lineL)

% for each thread
tps = []; % time processes (plural)
tus = []; % time user mode (plural)
tks = []; % time kernel mode (plural)
for t = 1:Nt
	
	% get timing
	thread = timing.threads(t);
	tu = thread.irtcpu(1,:); % user mode time
	tk = thread.irtcpu(2,:); % kernel mode time
	tus = [tus; tu];
	tks = [tks; tk];
	procs = thread.processes;
	Np = length(procs);
	
	% for each process in it
	ps = [];
	process_names = {};
	for p = 1:Np
		proc = procs(p);
		process_names{end+1} = proc.name;
		ps(end+1,:) = proc.irt;
	end
	tps = [tps; ps];
	
	% sort processes by run phase wallclock time
	[temp, display_order] = sort(-ps(:,2));
	
	% and display
	for p = display_order'
		if any(heavy_users(:,1) == t & heavy_users(:,2) == p)
			marker = '**';
		else
			marker = '  ';
		end
		dispLine([marker process_names{p}], nameL, ps(p,:), fmt, ~requested);
	end
	
	% for the total over all processes
	if length(ps) > 1
		dispLine(rpadstr('(sum)',nameL), nameL, sum(ps,1), fmt, ~requested);
	end
	
	% and for the thread itself
	softlinebreak(lineL)
	
	% the figure in brackets after "USER" is the run-phase
	% user mode percentage for this thread, with errors
 	perc = percWithErr(tu(2),tu(2)+tk(2),tick,2*tick);
 	dispLine(['  USER (RUP ' perc ')'], nameL, tu, fmt);
 	dispLine('  KERN', nameL, tk, fmt);
	
	% sum of USER and KERN modes
	dispLine(rpadstr('(sum)',nameL), nameL, tu + tk, fmt);
	
	linebreak(lineL)
	
end



% do caller thread
tp = timing.caller.irt;
tu = timing.caller.irtcpu(1,:); % user mode time
tk = timing.caller.irtcpu(2,:); % kernel mode time
tus = [tus; tu];
tks = [tks; tk];
% we don't add caller thread tp time to tps, because it's the bracket timers, not
% the process time itself!
disp('  (caller thread)')

% the figure in brackets after "USER" is the run-phase
% user mode percentage for this thread, with errors
softlinebreak(lineL)
perc = percWithErr(tu(2),tu(2)+tk(2),tick,2*tick);
dispLine(['  USER (RUP ' perc ')'], nameL, tu, fmt);
dispLine('  KERN', nameL, tk, fmt);



linebreak(lineL)

% do totals
disp('  (across threads)')
dispLine('  sum over processes', nameL, sum(tps,1), fmt, ~requested);
dispLine('  bracket timers', nameL, tp, fmt);
softlinebreak(lineL)

% this percentage is the overall percentage of run phase spent
% in user mode as a fraction of the overall time spent on the CPU
perc = percWithErr(sum(tus(:,2)),sum(tus(:,2))+sum(tks(:,2)),tick,2*tick);
dispLine(['  USER (RUP ' perc ')'], nameL, sum(tus,1), fmt);
dispLine('  KERN', nameL, sum(tks,1), fmt);

linebreak(lineL)



% loading is the percentage of CPU time we had in a given
% wallclock period, and represents thus the amount of CPU we were
% scheduled overall. a figure much off 100% indicates that we are
% competing for CPU resources - could try raising our priority!

tot_wallclock = tp(2);
tot_cpu = sum(tus(:,2)) + sum(tk(:,2));
perc = percWithErr(tot_cpu,tot_wallclock,6*tick*Nt,(Nt+1)*tick);
disp([padstr('SUMMARY:',nameL) rpadstr(['CPU loading ' perc],4*numL)])

% run phase wallclock overhead is the difference between the
% run phase bracket timer and the sum of all run phase process
% times (wallclock), as a fraction of the run phrase bracket time
% this indicates the wallclock time that was spent in run phase
% but not in a process step()
%
% this is convolved with CPU loading, so represents the percentage
% of available wallclock time that we were not inside step(). this
% is not a true measure of overhead unless the CPU loading is 100%,
% but in that case it will be very accurate.

% the wallclock timing is so accurate (CPU clock speed) that we
% assume zero error...

if requested
	sum_wallclock = sum(tps(:,2));
	bracket_wallclock = tp(2);
	perc = percWithErr(bracket_wallclock - sum_wallclock, bracket_wallclock, 0, 0);
	disp(rpadstr(['run phase overhead (measured by wallclock) ' perc],nameL+4*numL))
end

% run phase CPU overhead is the difference between the sum of run
% phase user times of all worker threads and the sum of run phase
% user and kernel times over all threads. for long runs, this will
% be a better estimate of internal overhead, since it is not convolved
% with loading
%
% this is not convolved with CPU loading, so represents the percentage
% of available CPU time that we were not crunching processes, and
% is the really telling number for the BRAHMS system, provided the
% soak is long enough that the resolution is reliable (the run-phase
% kernel time should be at least several CPU ticks, the tick freq.
% on ABRG-MITCH-HOME i'm seeing as 64Hz right now, so that would
% mean a run-phase kernel time of >100ms or so would be
% needed for a half-decent result).
%
% however, this figure is convolved with the time spent in a worker
% thread, but outside any process. again, not a good estimate of
% true overhead.

realwork_runphase_cpu = sum(tus(1:end-1,2));
runphase_cpu = sum(tus(:,2)) + sum(tks(:,2));
perc = percWithErr(runphase_cpu - realwork_runphase_cpu, runphase_cpu, Nt*tick, 2*(Nt+1)*tick);
disp(rpadstr(['run phase overhead (measured by CPU) ' perc],nameL+4*numL))



% display timing used
softlinebreak(lineL);
disp(rpadstr(['(WARNING this value assumed) CPU ticker ' num2str(1/tick) 'Hz'],lineL))
if isfield(timing,'ticksPerSec')
	if timing.ticksPerSec == 0
		disp(rpadstr('ticks per second was not recorded',lineL));
	elseif timing.rdtsc
		disp(rpadstr(['wallclock ticker ' sprintf('%.3fGHz', timing.ticksPerSec/1000000000) ' (RDTSC timer)'],lineL))
	else
		disp(rpadstr(['wallclock ticker ' sprintf('%.3fMHz', timing.ticksPerSec/1000000) ' (OS timer)'],lineL))
	end
end
if requested
	disp(rpadstr([int2str(marked_fraction * 100) '% of crunching within processes marked **'],lineL))
else
	softlinebreak(lineL);
	disp(rpadstr('process times are unavailable (BRAHMS runs faster like this)',lineL));
	disp(rpadstr('to get them, set Execution Parameter "TimeRunPhase" to true',lineL));
end
linebreak(lineL)

if isunix
	warning('thread times are meaningless on linux currently - ignore them (process times should be good though)')
end




function linebreak(L)

disp(repmat('_',1,L))
disp(' ')



function softlinebreak(L)

disp(repmat('-',1,L))




function str = padstr(str, N)

if length(str) > N
	str = [str(1:(N-3)) '...'];
end

str = [str repmat(' ',1,N - length(str))];



function str = rpadstr(str, N)

if length(str) > N
	str = [str(1:(N-3)) '...'];
end

str = [repmat(' ',1,N - length(str)) str];



function dispLine(name, nameL, t, fmt, blank_second)

if nargin<5
	blank_second = false;
end

out = [padstr(name, nameL)];
for n=1:length(t)
	if blank_second && n == 2
		out = [out rpadstr('-',length(sprintf(fmt, 0)))];
	else
		out = [out sprintf(fmt, t(n))];
	end
end
out = [out sprintf(fmt, sum(t))];
disp(out)



function perc = percWithErr(A,B,Ae,Be)

pMin = max(A-Ae,0)/(B+Be);
if max(B-Be,0) == 0
	pMax = 1;
else
	pMax = (A+Ae)/max(B-Be,0);
end
if pMin < 0 pMin = 0; end
if pMax > 1 pMax = 1; end
pMin = round(pMin * 100);
pMax = round(pMax * 100);

if pMin == pMax
	perc = [int2str(pMin) '%'];
else
	perc = [int2str(pMin) '-' int2str(pMax) '%'];
end




function out = breakup(str)

L = 64;
out = '';
indent = repmat(' ',1,4);

while 1
	if length(str) <= L
		out = [out 10 indent str];
		return
	end
	
	% find convenient space
	f = find(str(1:(L+1)) == 32);
	if isempty(f)
		out = [out 10 indent str(1:40)];
		str = str(41:end);
		continue;
	end
	
	f = f(end);
	out = [out 10 indent str(1:(f-1))];
	str = str((f+1):end);
end

