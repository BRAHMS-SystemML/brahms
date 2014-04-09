%
% brahms_gperf(report)
%   Display BRAHMS performance information in graphical
%   form: report the performance information returned by
%   BRAHMS in human-readable form. Pass the BRAHMS report
%   structure.
%
% (see also brahms_perf)
%
% CPU tick is assumed to be at 64Hz for all CPUs (that's
% what pentiums clock at i think). "RUP" is Runtime Usermode
% Percentage, and is a measure of how much time the CPU was
% doing real work, rather than context switching or the
% like. Yes, it could easily have been "RUMP".
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
%	$Id:: brahms_gperf.m 2398 2009-11-18 21:53:08Z benjmitch   $
%	$Rev:: 2398                                                $
%	$Author:: benjmitch                                        $
%	$Date:: 2009-11-18 21:53:08 +0000 (Wed, 18 Nov 2009)       $
%________________________________________________________________
%

function brahms_gperf(rep, index)

% if passed multiple, pass them one by one, and draw a
% different figure for each
if isstruct(rep) && length(rep) > 1
	for n = 1:length(rep)
		brahms_gperf(rep(n), n)
	end
	return
end
if ~exist('index', 'var')
	index = 1;
end

% report head
head = rep.Header;

% on nix, we can't get separate user/kern times, so we return the sum of
% the two in the user field - in this case, we can't report kernel
% percentage, but otherwise we can proceed as normal
combined = ~strcmp(rep.Timing.OS, 'WIN');

% extract from report structure
timing = rep.Timing;
bindings = rep.Timing.bindings;

% open figure
figure(index)
clf
% title(['Voice ' int2str(index-1) ' Performance Data'])

% extract
Nt = length(timing.threads);

% assume a tick for percentages
tick = 1/64;








%% METRICS
base_plot_share = 0.15;
base_bottom_margin_boost = 0.03;
margin = 0.05;
kern_bar_width = 0.05;

% disabled
fwrk_bar_width = 0;

if combined
	kern_bar_width = 0.0;
end



%% PREPARE TIMES

% get user/kernel times (framework)
sp = timing.caller;
fwrk_user_irt = sp.irtcpu(1, :);
fwrk_kern_irt = sp.irtcpu(2, :);

% get user/kernel times (total over framework)
fwrk_user_tot = sum(sum(fwrk_user_irt));
fwrk_kern_tot = sum(sum(fwrk_kern_irt));

% get user/kernel times (worker threads)
th = timing.threads;
work_user_irt = [];
work_kern_irt = [];
for t = 1:length(th)
	work_user_irt(t, :) = th(t).irtcpu(1, :);
	work_kern_irt(t, :) = th(t).irtcpu(2, :);
end

% get user/kernel times (total over threads)
work_user_tot = sum(sum(work_user_irt));
work_kern_tot = sum(sum(work_kern_irt));



%% BASE PLOT
%
% displays the duration of five phases:
%   serialize (bindings)
%   init
%   run
%   term
%   unserialize (bindings)
%
% each is displayed with kernel/user time represented
% vertically and in colours (red/green) - if kernel/user
% time is too brief to be reliable (and for bindings where
% it is not available) the whole patch is painted blue to
% indicate this.
subplot('position', [margin margin+base_bottom_margin_boost 1-2*margin base_plot_share-margin-base_bottom_margin_boost])
set(gca, 'ytick', [])

% timings as measured by bindings
Tb = [0];
Tb(2) = bindings.serialize + bindings.otherprep;
Tb(3) = bindings.execute;
Tb(4) = bindings.console + bindings.collect;
Tb = cumsum(Tb);
rect([Tb(1:2) 0.2 1.2], 'ib')
rect([Tb(2:3) 0.2 1.2], 'rb')
rect([Tb(3:4) 0.2 1.2], 'ib')

% timings as measured by BRAHMS
Te = [0 timing.caller.irt'];
Te = cumsum(Te);
Te = Te + Tb(2); % offset correctly to bindings timings
rect([Te(1:2) 0.8 1.8], 'ie')
rect([Te(2:3) 0.8 1.8], 're')
rect([Te(3:4) 0.8 1.8], 'ie')

% ok
axis([0 Tb(end) 0 2]);
xlabel('execution timeline: prepare, execute(init, run, term), collect (wallclock seconds)')



%% PER-THREAD PLOT
%
% displays data for each thread, showing kernel time and
% user time, and how that user time was split between the
% processes being executed
threadcount = length(th);
width = 1 - 2 * margin;
width_avail = width - (threadcount-1) * margin;
width_plot = width_avail / threadcount;
step_plot = width_plot + margin;
maxT = 0;
totT = [];
sph = [];
avail_height = (1-base_plot_share)-2*margin;

for threadindex = 1:threadcount
	
	x = margin+step_plot*(threadindex-1);
	sph(threadindex) = subplot('position', [x base_plot_share+margin width_plot avail_height]);
	set(gca, 'xtick', [])
	xlabel(['thread ' int2str(threadindex-1) ' CPU'])
	thread_user_irt = work_user_irt(threadindex, :);
	thread_kern_irt = work_kern_irt(threadindex, :);
	T = cumsum([0 thread_user_irt]);
	
	if any(T)
		
		% plot a patch for each of init, run, term phases -
		% these are actual CPU seconds in height, corresponding
		% to the y axis labels (so for 4 cpus, we'll get way
		% more CPU seconds than the wallclock timeline at the
		% bottom gives us wallclock seconds...)
		rect([0 1 T(1:2)], 'ie')
		rect([0 1 T(2:3)], 're')
		rect([0 1 T(3:4)], 'ie')
		axis([0 1 0 T(end)])
		
		% red kernel bar displays the *fraction* of total CPU
		% time that was spent in the kernel - it does not,
		% therefore, match up with the time on the Y axis (which
		% is actual CPU seconds), but is simply some fraction of
		% the height of the graph representing the fraction of
		% kernel usage. that is, good performance has this bar
		% small!
		if ~combined
			kern_frac = sum(thread_kern_irt)/(sum(thread_kern_irt)+T(end));
			kern_frac_for_display = kern_frac * T(end); % as fraction of total graph height, not to scale with the y axis, but just a proportion
			rect([1-kern_bar_width 1 0 kern_frac_for_display], 'kbl')
			rect([1-kern_bar_width 1 kern_frac_for_display T(end)], 'kbu')
		end
		totT(threadindex) = T(end);
		maxT = max(T(end), maxT);
		
		% per-process times - these are wallclock, so we can't
		% display them on the same scale as these CPU times,
		% they won't make sense. instead, we use the wallclock
		% times to estimate the proportion of the CPU time that
		% was spent on each process, and scale the CPU time
		% using these proportions.
		
		% first, sort by runphase time
		ps = th(threadindex).processes;
		t = [];
		for p=1:length(ps)
			t(p) = ps(p).irt(2);
			names{p} = ps(p).name;
		end
		[temp, i] = sort(-t);
		Np = length(i);
		
		% only show first 8
		if length(i) > 8 i = i(1:8); end
		names = names(i);
		
		% for each phase
		for phs = 1:3
			t = [];
			for p=1:length(ps)
				t(p) = ps(p).irt(phs);
			end
			t = t(i);
			W = (1-kern_bar_width)/Np;
			Wi = W * 0.8;
			Wm = W * 0.1;
			base = T(phs);
			height = T(phs+1) - T(phs);
% 			if phs == 2 && ~TimeRunPhase
% 				x = (1-kern_bar_width)/2;
% 				y = base+height/2;
% 				h = text(x,y,'n/a','hori','center');
% 				continue;
% 			end
			if max(t) > 0
				scale = height / max(t) * 0.98;
				for p = 1:length(t)
					h = scale * t(p);
					l = Wm + W * (p-1);
					if phs == 2
						rect([l l+Wi base base+h], 'rp')
					else
						rect([l l+Wi base base+h], 'ip')
					end
				end
			end
		end
		for p = 1:length(names)
			l = Wm + W * (p-1);
			x = l+0.5*Wi;
			y = 0;
			h = text(x,y,['  ' names{p}]);
			set(h, 'rotation', 90);
			set(h, 'interpreter', 'none');
		end
		
	else
	 	set(gca, 'ytick', [])
		t = text(0.5, 0.5, ['too brief' 10 'no info']);
		set(t, 'hori', 'center');
		axis([0 1 0 1])
		totT(threadindex) = 0;
	end
end

% % framework thread
% x = margin;
% user = sum(fwrk_user_irt);
% kern = sum(fwrk_kern_irt);
% tot = user + kern;
% kern_frac = kern/tot;
% kern_frac_for_display = kern_frac * tot; % as fraction of total graph height, not to scale with the y axis, but just a proportion
% sph(threadindex+1) = subplot('position', [x base_plot_share+margin fwrk_bar_width avail_height]);
% rect([0 1 0 kern_frac_for_display], 'kbl')
% rect([0 1 kern_frac_for_display tot], 'kbu')
% totT(threadindex+1) = tot;

% leave this one out, so we get the best we can see of the
% other graphs
% maxT = max(maxT, tot);

% set(gca, 'xtick', [])
% axis([0 1 0 tot])

% all on same scale
for threadindex = 1:threadcount
	if totT(threadindex)
		p = get(sph(threadindex), 'position');
		p(4) = avail_height * totT(threadindex) / maxT;
		set(sph(threadindex), 'position', p);
	end
end

% brief text report
cpu_tot = work_user_tot + work_kern_tot + fwrk_user_tot + fwrk_kern_tot;
total_wallclock_time = Te(4)-Te(1);

disp([repmat('_', [1 60]) 10])
disp(['BRAHMS Gperf (try brahms_perf for more detailed breakdown)'])
disp(['Figures here are "total" (init, run and term phases collated)'])
disp([repmat('_', [1 60]) 10])
disp(['Wallclock time: ' num2str(total_wallclock_time) ' secs']);
disp(['Total CPU time: ' num2str(cpu_tot) ' secs']);
if combined
	disp(['Process CPU time: ' num2str(work_user_tot) ' secs']);
	disp(['Framework CPU time: ' num2str(fwrk_user_tot) ' secs']);
	disp(['Overhead (Framework): ' int2str(100-work_user_tot/cpu_tot*100) '%']);
	disp(['Kernel/User breakdown not available on this system']);
else
	disp(['Process User CPU time: ' num2str(work_user_tot) ' secs']);
	disp(['Framework User CPU time: ' num2str(fwrk_user_tot) ' secs']);
	disp(['Kernel CPU time: ' num2str(work_kern_tot+fwrk_kern_tot) ' secs']);
	disp(['Overhead (Framework/Kernel): ' int2str(100-work_user_tot/cpu_tot*100) '% (NB: init, run & term)']);
end
disp([repmat('_', [1 60]) 10])

if ~timing.TimeRunPhase
	disp('process times are unavailable (BRAHMS runs faster like this)');
	disp('to get them, set Execution Parameter "TimeRunPhase" to true');
	disp([repmat('_', [1 60]) 10])
end

if isunix
	warning('thread times are meaningless on linux currently - ignore them (process times should be good though)')
end






function rect(p, col)


% comes in as [x1 x2 y1 y2]
% out as [x y w h]
P = [p(1) p(3) p(2)-p(1) p(4)-p(3)];

if P(3) && P(4)
	r = rectangle('position', P);
	set(r, 'facecolor', gcol(col));
	set(r, 'edgecolor', [1 1 1]);
end


function col = gcol(col)

switch col
	case 'ib'
		col = [30 213 89];
	case 'rb'
		col = [18 126 53];
	case 'ie'
		col = [243 188 73];
	case 're'
		col = [243 96 7];
	case 'ip'
		col = [45 150 255];
	case 'rp'
		col = [23 105 240];
	case 'kbl'
		col = [255 0 0];
	case 'kbu'
		col = [0 255 0];
	otherwise
		error(col)
end

col = col / 255;



