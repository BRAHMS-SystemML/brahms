% brahms_bench(benchmark, ...)
%
% Benchmark BRAHMS on your hardware, save the results in
% your WorkingDirectory, and then call the second form of
% this function to report the results. "benchmark" can be
% any one of the following, and some benchmarks accept
% additional arguments.
%
% "operation" measure performance at a number of basic
%   operations, using BRAHMS. this test does not benchmark
%   BRAHMS, really, just your machine. 
%
% "overhead" measure the overhead of using the BRAHMS
%   framework directly, per process and per
%   inter-process-link.
%
% "cache" measure your hardware's memory access performance
%   as working-set size is varied. this benchmark reveals
%   your cache architecture, and an estimate of working-set
%   size that does not overflow your most direct cache which
%   can be used to parametrize following benchmarks.
%
% "overheadi" measure the per-process overhead indirectly,
%   by running executions with iterations of different
%   complexity.
%
% "scaling" measure how performance scales with the number
%   of processes in the system.
%
%
%
% Some arguments may be used with any of the benchmarks, as
% follows.
%
% "-s"/"-m"/"-l" will perform a (s)hort, (m)edium or (l)ong
%   version of the benchmark. default is "-s"; longer runs
%   are more accurate.
%
% NOTE: "-s" aims to give a quick and dirty (but useful)
% indication of performance, and completes in a few seconds
% on the test hardware (Quad 2.5GHz 4GB). "-m" performs a
% fairly complete and valid benchmark, and aims to complete
% in around a minute. "-l" allows a thorough benchmarking to
% be performed, and may take up to an hour for some
% benchmarks (some other benchmarks do not need "-l", and
% will drop to "-m" with a warning).
%
% "-p" parallelise using multi-threading, where appropriate.
%
% "--..." any argument beginning with a double-hyphen is
%   passed directly on to the BRAHMS executable.
%
%
%
% brahms_bench(filename)
%
% If "filename" is the filename of a benchmark results file
% stored in your WorkingDirectory or in the current
% directory, this form will graph or otherwise display the
% results from that file.
%
% Quoted results are marked *ST* or *MT* for single- or
% multi-threaded. Multi-threading will improve performance
% on heavy benchmarks when multiple cores are available but
% will increase overhead in other conditions.
%

% "-q" runs the (q)uick version of a benchmark - this is not
% very useful as a benchmark but runs very quickly for
% developing this script. it is not advertised to the user.

% "-c" parallelise using multi-processing (concerto).
%
% NOT IMPLEMENTED!




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
%	$Id:: brahms_bench.m 2407 2009-11-19 21:49:28Z benjmitch   $
%	$Rev:: 2407                                                $
%	$Author:: benjmitch                                        $
%	$Date:: 2009-11-19 21:49:28 +0000 (Thu, 19 Nov 2009)       $
%________________________________________________________________
%


function brahms_bench(firstarg, varargin)


if ~nargin
	help brahms_bench
	return
end

% check we have SystemML
if isempty(which('sml_utils'))
	error('SystemML Toolbox not installed');
end


if isstruct(firstarg)
	
	% temporary figure
	brahms_bench_report(firstarg);
	
elseif ischar(firstarg)

	switch firstarg

		case {
				'operation'
				'overhead'
				'overheadi'
				'cache'
				'scaling'
				}
			brahms_bench_run(firstarg, varargin{:});

        otherwise
			brahms_bench_report_load(firstarg);

	end

end




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function brahms_bench_run(benchmark, varargin)



%% OPTIONS

opts = {};
args = {};
opt.multithread = false;
opt.concerto = false;
opt.length = 1;
for n = 1:length(varargin)
	switch varargin{n}
		case '-p'
			opt.multithread = true;
		case '-c'
			opt.concerto = true;
		case '-q'
			opt.length = 0;
		case '-s'
			opt.length = 1;
		case '-m'
			opt.length = 2;
		case '-l'
			opt.length = 3;
		otherwise
			if ischar(varargin{n}) & length(varargin{n}) > 2 & strcmp(varargin{n}(1), '--')
				opts{end+1} = varargin{n};
			else
				args{end+1} = varargin{n};
			end
	end
end





%% PARS

mark = [];
mark.benchmark = benchmark;
mark.opt = opt;
mark.opts = opts;
mark.timestamp = datestr(now, 'yymmdd-HHMMSS');
mark.time = datestr(now);
if isunix
	[s, m] = system('uname -n');
	mark.machine = stripws(m);
else
	mark.machine = getenv('COMPUTERNAME');
end
mark.filename = ['benchmark-' mark.benchmark '-' mark.machine '-' mark.timestamp '.mat'];



%% RESULTS

result = [];
t0 = clock;


%% SWITCH ON TEST

switch(benchmark)



%% OPERATION

	case 'operation'

		% process count
		cnt = 8;
		fS = 1000;
		fscale = 1500;
		iscale = 15000;
		mscale = 500;
		nscale = 100;

		switch opt.length
			case 0
				rng = 4:6:10;
				fac = 1/10;
				f = rng * fscale * fac;
				i = rng * iscale * fac;
				m = rng * mscale * fac;
				n = rng * nscale * fac;
			case 1
				rng = 1:3:10;
				fac = 1/4;
				f = rng * fscale * fac;
				i = rng * iscale * fac;
				m = rng * mscale * fac;
				n = rng * nscale * fac;
			case 2
				rng = 0:2:10;
				fac = 2;
				f = rng * fscale * fac;
				i = rng * iscale * fac;
				m = rng * mscale * fac;
				n = rng * nscale * fac;
			case 3
				rng = 0:1:10;
				fac = 4;
				f = rng * fscale * fac;
				i = rng * iscale * fac;
				m = rng * mscale * fac;
				n = rng * nscale * fac;
		end

		% exe
		exe = brahms_execution;
		exe.name = 'brahms_bench';
		exe.stop = 1;
		exe.execPars.ShowGUI = 0;
		exe.execPars.Priority = 1;
		
		% raise pri
% 		exe.execPars.Priority = 1;

		if ~opt.multithread
			exe.execPars.MaxThreadCount = 1;
		end

		ci = 0;
		cf = length(f) + length(i) + length(m) + length(n);
		hw = brahms_waitbar(0, 'Benchmarking...');
		
		% f
		fv = f * cnt * fS / 1e6; % total MFLOPS
		ft = NaN(size(f));
		for fi = 1:length(f)
			ft(fi) = operation(f(fi), 0, 0, 0, cnt, fS, exe, opts);
			ci = ci + 1;
			brahms_waitbar(ci/cf, hw);
			drawnow
		end

		% i
		iv = i * cnt * fS / 1e6; % total MIPS
		it = NaN(size(i));
		for ii = 1:length(i)
			it(ii) = operation(0, i(ii), 0, 0, cnt, fS, exe, opts);
			ci = ci + 1;
			brahms_waitbar(ci/cf, hw);
			drawnow
		end

	
		% m (cache write - 4096 bytes)
		mv = m * cnt * 8 * fS / 1e6; % total MB written
		mt = NaN(size(m));
		for mi = 1:length(m)
			mt(mi) = operation(0, 0, m(mi), 4096, cnt, fS, exe, opts);
			ci = ci + 1;
			brahms_waitbar(ci/cf, hw);
			drawnow
		end

		% n (bus write - 8MB x 8 = 64MB working set)
		nv = n * cnt * 8 * fS / 1e6; % total MB written
		nt = NaN(size(n));
		for ni = 1:length(n)
			nt(ni) = operation(0, 0, n(ni), uint32(1024*1024*8), cnt, fS, exe, opts);
			ci = ci + 1;
			brahms_waitbar(ci/cf, hw);
			drawnow
		end
		
		safeclose(hw);
		
		
		% store
		result.fv = fv;
		result.ft = ft;
		result.iv = iv;
		result.it = it;
		result.mv = mv;
		result.mt = mt;
		result.nv = nv;
		result.nt = nt;
		mark.result = result;



		
%% OVERHEAD

	case 'overhead'

		% measure how much cost is associated with each sort of
		% BRAHMS object on this system, and how much speed-up is
		% achieved by multi-threading

		hiperf = false;
		
		switch opt.length
			case 0
				reps = 1;
				fS = 250;
				P = 10:40:50;
				L = 0:2:2;
			case 1
				reps = 1;
				fS = 5000;
				P = 10:20:50;
				L = 0:2:4;
			case 2
				reps = 3;
				fS = 10000;
				P = 10:10:50;
				L = 0:4;
			case 3
				hiperf = true;
				reps = 10;
				fS = 10000;
				P = 10:10:100;
				L = 0:5;
		end

		T = NaN(length(P), length(L), reps);

		% execution is one second, no logging
		exe = brahms_execution;
		exe.name = 'brahms_bench';
		exe.stop = 1;
		exe.execPars.ShowGUI = 0;
		exe.execPars.Priority = 1;
		if ~opt.multithread
			exe.execPars.MaxThreadCount = 1;
		end

		% result
		result.fS = fS;
		result.P = P;
		result.L = L;
		result.reps = reps;

		% run each possible system
		ci = 0;
		cf = length(L) * length(P) * reps;
		hw = brahms_waitbar(0, 'Benchmarking...');
		for ir = 1:reps
			for il = 1:length(L)
				for ip = 1:length(P)

					% num process/link
					p = P(ip);
					l = L(il);

					% empty system
					sys = sml_system;

					% add np processes
					for n = 1:p
						sys = sys.addprocess(['p' int2str(n)], ...
							'client/brahms/bench/overhead', fS, []);
					end

					% add nd links
					src = 1:p;
					for dx = 1:l
						dst = mod(src+dx-1,p)+1;
						for n = 1:p
							s = src(n);
							d = dst(n);
							sys = sys.link(['p' int2str(s) '>out'], ['p' int2str(d)], 1);
						end
					end

					% run it
					[out, rep] = brahms(sys, exe, opts{:});
					T(ip, il, ir) = rep.Timing.caller.irt(2);
					
					% plot it
					result.T = T;
					mark.result = result;
					
					if ~hiperf
						brahms_bench_report(mark, true);
					end

					% workbar
					ci = ci + 1;
					brahms_waitbar(ci/cf, hw);
					drawnow

				end
			end
		end
		safeclose(hw)




%% CACHE

	case 'cache'

		% P process count
		% E elements per process
		% N iterations per execution
		% O operation count per element per iteration

		%% BENCHMARK 1
		%
		% vary E to change the WSS

		switch opt.length
			case 0
				pars.reps = 1;
				pars.P = 2;
				pars.N = 10;
				E = 2.^[5:4:17]; % keep off the bus!
 				pars.O = 1;
			case 1
				pars.reps = 1;
				pars.P = 4;
				pars.N = 10;
				E = 2.^[15:2:23];
 				pars.O = 1;
			case 2
				pars.reps = 1;
				pars.P = 16;
				pars.N = 10;
				E = 2.^[11:1:21];
 				pars.O = 1;
			case 3
				pars.reps = 10;
				pars.P = 16;
				pars.N = 100;
				E = 2.^[9:1:21];
 				pars.O = 1;
		end
		
		[E, i] = unique(round(E));
		
		% storage
		T = [];
		T.br = [];
		T.bi = [];
		T.tr = [];
		T.ti = [];
		T.mr = [];
		T.mi = [];
		T.pars = pars;
		T.pars.E = E;

		% workbar
		Ne = length(E);
		ci = 0;
		cf = Ne * pars.reps;
		hw = brahms_waitbar(0, 'Benchmarking...');

		% execute
		for rep = 1:pars.reps
			for n = 1:Ne

				% pars
				pars.E = E(n);

				% run
				pars.threads = false;
				tb = elements_exec('sb', pars);
				pars.threads = true;
				tt = elements_exec('sb', pars);
				tm = elements_exec('sm', pars);

				% store
				T.br(n, rep) = tb.run;
				T.bi(n, rep) = tb.init;
				T.tr(n, rep) = tt.run;
				T.ti(n, rep) = tt.init;
				T.mr(n, rep) = tm.run;
				T.mi(n, rep) = tm.init;

				% workbar
				ci = ci + 1;
				brahms_waitbar(ci/cf, hw);
				drawnow

			end
		end
		safeclose(hw);

		% store
		mark.result = T;

		
		

%% OVERHEADI

	case 'overheadi'
		
		%% BM 2
		
		% having established the working set size that makes
		% computation the bottleneck, we can run a test where we
		% keep the wss fixed, and vary either the process count or
		% the computational complexity of each process
		
		hiperf = false;
		
		switch opt.length
			case 0
				pars.reps = 1;
				pars.P = 2;
				pars.N = 10;
				pars.E = 2^5;
				O = 2.^[0:4:12];
			case 1
				pars.reps = 3;
				pars.P = 16;
				pars.N = 100;
				pars.E = 2^7;
				O = 2.^[0:4];
			case 2
				pars.reps = 3;
				pars.P = 16;
				pars.N = 1000;
				pars.E = 2^5;
				O = 2.^[0:6];
			case 3
				hiperf = true;
				pars.reps = 100;
				pars.P = 16;
				pars.N = 1000;
				pars.E = 2^5;
				O = 2.^[0:8];
		end

		O = unique(round(O));
		
		% storage
		T = [];
		T.br = [];
		T.tr = [];
		T.mr = [];
		T.pars = pars;
		T.pars.O = O;
		
		% workbar
		N = length(O);
		ci = 0;
		cf = N * pars.reps;
		hw = brahms_waitbar(0, 'Benchmarking...');

		% execute
		for rep = 1:pars.reps
			for n = 1:N

				% pars
				pars.O = O(n);

				pars.threads = false;
				tb = elements_exec('sb', pars);
				pars.threads = true;
				tt = elements_exec('sb', pars);
				tm = elements_exec('sm', pars);

				% store
				T.br(n, rep) = tb.run;
				T.tr(n, rep) = tt.run;
				T.mr(n, rep) = tm.run;
				
				% fig
				mark.result = T;

					
				if ~hiperf
					brahms_bench_report(mark, true);
				end

				% workbar
				ci = ci + 1;
				brahms_waitbar(ci/cf, hw);
				drawnow

			end
		end
		safeclose(hw);

		

%% SCALING
	
	case 'scaling'
		
		% BM 3

		% having established the working set size that makes
		% computation the bottleneck, we can run a test where we
		% keep the wss fixed, and vary either the process count or
		% the computational complexity of each process
		
		hiperf = false;
		
		switch opt.length
			case 0
				pars.reps = 1;
				P = 2.^[0:3:6];
				pars.N = 10;
				pars.E = 2^5;
				pars.O = 2^5;
			case 1
				pars.reps = 3;
				P = 2.^[0:4];
				pars.N = 10;
				pars.E = 2^8;
				pars.O = 2^8;
			case 2
				pars.reps = 3;
				P = 2.^[0:6];
				pars.N = 10;
				pars.E = 2^8;
				pars.O = 2^8;
			case 3
				hiperf = true;
				pars.reps = 10;
				P = 2.^[0:10];
				pars.N = 25; % might get even closer to the bone in MT mode with N = 100
				pars.E = 2^8;
				pars.O = 2^8; % or, alternatively, by increasing this by 2 or 4 factor
		end

		P = unique(round(P));
		
		% storage
		T = [];
		T.br = [];
		T.tr = [];
		T.mr = [];
		T.pars = pars;
		T.pars.P = P;

		% workbar
		N = length(P);
		ci = 0;
		cf = N * pars.reps;
		hw = brahms_waitbar(0, 'Benchmarking...');

		% execute
		for rep = 1:pars.reps
			for n = 1:N

				% pars
				pars.P = P(n);

				% run
				pars.threads = false;
				tb = elements_exec('sb', pars);
				pars.threads = true;
				tt = elements_exec('sb', pars);
				tm = elements_exec('sm', pars);

				% store
				T.br(n, rep) = tb.run;
				T.tr(n, rep) = tt.run;
				T.mr(n, rep) = tm.run;

				% fig
				mark.result = T;
					
				if ~hiperf
					brahms_bench_report(mark, true);
				end
				
				% workbar
				ci = ci + 1;
				brahms_waitbar(ci/cf, hw);
				drawnow

			end
		end
		safeclose(hw);
		
		
		
		%% UNRECOGNISED

	otherwise

		error('incorrect usage of "brahms_bench" - see help');

end




% augment results
mark.t_elapsed = etime(clock, t0);

% save results
ep = brahms_utils('GetExecutionParameters');
wd = ep.WorkingDirectory;
filename = [wd '/' mark.filename];
save(filename, 'mark');

% call report function
brahms_bench_report_load(mark.filename);









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function brahms_bench_report_load(filename)

if length(filename) < 4 || ~strcmp(filename(end-3:end), '.mat')
    filename = [filename '.mat'];
end

if ~exist(filename, 'file')
	ep = brahms_utils('GetExecutionParameters');
	wd = ep.WorkingDirectory;
	filename = [wd '/' filename];
end
load(filename)

brahms_bench_report(mark);



function brahms_bench_report(mark, noprint)

if nargin<2
    noprint = false;
end

% known results
known = {
	};

% BEHEMOTH
% R2021 (0.7.2 RC7)
% this machine is a me-built tower with variously sourced
% components
known(end+1, :) = {
	'  2.40GHz  2x4MB  1061MHz  DDR2-6400  XP SP2 (Intel Core2 Quad Q6600)'
	[231 137]
	[216 2206 1070 674]
	};

% STUART'S MACBOOK
% R2120 (post 0.7.2)
%       Model Name: MacBook
%       Model Identifier: MacBook2,1
%       Processor Name: Intel Core 2 Duo
%       Processor Speed: 2 GHz
%       Number Of Processors: 1
%       Total Number Of Cores: 2
%       L2 Cache (per processor): 4 MB
%       Memory: 1 GB
%       Bus Speed: 667 MHz
known(end+1, :) = {
	'  2.00GHz  2x4MB   667MHz  DDR2-5300  OSX 10.4.11 (MacBook Intel Core2 Duo)'
	[316 168]
	[378 1704 668 388]
	};

% LATITUDE XT2 (tilt-and-slide touchscreen)
% R2121
% Manufacturer	Dell Inc.
% Product Name	Latitude XT2
% Physical Memory	3024 MB Total, 2224 MB Free
% Memory Load	26%
% Number of CPU(s)	One Physical Processor / 2 Cores / 2 Logical Processors / 64 bits
% Vendor	GenuineIntel
% CPU Name	Intel Mobile Core 2 Duo
% CPU Code Name	Penryn
% Platform Name	Socket P (478)
% CPU Full Name	Intel(R) Core(TM)2 Duo CPU U9400 @ 1.40GHz
% Revision	R0
% Technology	45 nm
% Instructions	MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, ET64, XD, VMX, SMX, EST
% Original Clock	1400 MHz
% Original System Clock	200 MHz
% Original Multiplier	7.0
% CPU Clock	1396 MHz
% System Clock	199.4 MHz
% FSB	797.5 MHz
% L1 Data Cache	2 x 32 KBytes
% L1 Instructions Cache	2 x 32 KBytes
% L2 Cache	3072 KBytes
known(end+1, :) = {
	'  1.40GHz  1x3MB   800MHz  DDR3-6400  XP SP2 (Dell Latitude XT2, Intel Mobile Core2 Duo)'
	[394 220]
	[133 1328 813 334]
	};



if isfield(mark, 't_elapsed')
	elap = [', ' num2str(mark.t_elapsed) 's to regenerate'];
else
	elap = '';
end


% plot
bench_selectfigure(mark.benchmark)
clf


switch mark.benchmark
	

	
%% PLOT OPERATION

	case 'operation'

		fv = mark.result.fv;
		ft = mark.result.ft;
		iv = mark.result.iv;
		it = mark.result.it;
		mv = mark.result.mv;
		mt = mark.result.mt;
		nv = mark.result.nv;
		nt = mark.result.nt;
		
		% f
		subplot(2,2,1)
		plot(fv, ft, 'k.-')
		xlabel('MFLOPS')
		ylabel('time (sec)')
		v = axis;
		v(1) = 0;
		v(3) = 0;
		axis(v);

		% i
		subplot(2,2,2)
		plot(iv, it, 'k.-')
		xlabel('MIPS')
		ylabel('time (sec)')
		v = axis;
		v(1) = 0;
		v(3) = 0;
		axis(v);

		% m (cache write - 4096 bytes)
		subplot(2,2,3)
		plot(mv, mt, 'k.-')
		xlabel('MB (cache)')
		ylabel('time (sec)')
		v = axis;
		v(1) = 0;
		v(3) = 0;
		axis(v);

		% n (bus write - 8MB x 8 = 64MB working set)
		subplot(2,2,4)
		plot(nv, nt, 'k.-')
		xlabel('MB (bus)')
		ylabel('time (sec)')
		v = axis;
		v(1) = 0;
		v(3) = 0;
		axis(v);
		
		rep(1) = fv(end) / ft(end);
		rep(2) = iv(end) / it(end);
		rep(3) = mv(end) / mt(end);
		rep(4) = nv(end) / nt(end);
		rep = round(rep);
		
		% report
		if all(~isnan(rep)) && ~noprint
			disp(['________________________________________________________________________________' 10])
			disp(['   Float Integer   BW(1)   BW(2)          CPU     L2      FSB   RAM       OS']);
			disp(['________________________________________________________________________________' 10])
			disp([dsp(rep(1),8) dsp(rep(2),8) dsp(rep(3),8) dsp(rep(4),8) '    This machine'])
			for n = 1:size(known, 1)
				t = known{n, 1};
				d = known{n, 3};
				disp([dsp(d(1),8) dsp(d(2),8) dsp(d(3),8) dsp(d(4),8) '    ' t])
			end
			disp(['________________________________________________________________________________' 10])
			disp(['Bandwidth (MB/s) with working Set 32kB (1) / 64MB (2)' elap])
			disp(['________________________________________________________________________________' 10])
		end
		
		drawnow
		

		
%% PLOT OVERHEAD

	case 'overhead'
		
		fS = mark.result.fS;
		L = mark.result.L;
		P = mark.result.P;
		T = mark.result.T;
		
		% get iter time (us)
		T = T / fS * 1e6;
		
		% get min over reps (dimension 3) - NaNs will be
		% ignored, if benchmark is in progress
		if size(T, 3) > 1
			T = min(T, [], 3);
		end
		
		% calculate overhead O = Jn x P + Jd x D + J0
		NN = P'*ones(1,length(L));
		DD = (ones(length(P),1)*L) .* NN;
		NN = reshape(NN, prod(size(NN)), []);
		DD = reshape(DD, prod(size(DD)), []);
		TT = reshape(T, prod(size(T)), []);
		U = [NN DD ones(size(NN))];
		b = U \ TT;

		ns = b * 1e3;
		ns = floor(ns);
		
		% report
		leg = {};
		for nd = 1:length(L)
			leg{nd} = [int2str(L(nd)) ' data/proc'];
		end
		
		% plot
		plot(P, T, '.-')
		xlabel('P (number of processes)');
		ylabel('system iteration time (us)');
		v = axis;
		axis([0 max(P) 0 v(4)])
		legend(leg, 2)

		% show model
		hold on
		for nd = 1:length(L)
			O = ones(size(P'));
			U = [P' (L(nd)*O).*(P') O];
			TT = U*b;
			plot(P, TT, 'r:')
		end
		
		% report (constant offset ns(3) is too noisy to report,
		% really)
		if all(~isnan(ns)) && ~noprint
			disp(['________________________________________________________________________________' 10])
			disp([' Process    Link          CPU     L2      FSB   RAM       OS']);
			disp(['________________________________________________________________________________' 10])
			disp([dsp(ns(1),8) dsp(ns(2),8) '    This machine'])
			for n = 1:size(known, 1)
				t = known{n, 1};
				d = known{n, 2};
				disp([dsp(d(1),8) dsp(d(2),8) '    ' t])
			end
			disp(['________________________________________________________________________________' 10])

			% warning
			disp(['Process/Link figures are overhead times (in nanoseconds) per item' elap])
			disp(['Warning: if results are not fairly linear, we probably overflowed cache RAM'])
			disp(['________________________________________________________________________________' 10])
		end

		drawnow

		
		
%% PLOT CACHE

	case 'cache'

		% BM 1
		
		TT = mark.result;
		[ws, ops] = elements_wss(TT.pars);

		T = min(TT.br, [], 2)';
		tb = T ./ ops * 1e9;
		semilogx(ws, tb, 'b.-');
		hold on

		T = min(TT.tr, [], 2)';
		tt = T ./ ops * 1e9;
		semilogx(ws, tt, 'b.:');

		T = min(TT.mr, [], 2)';
		tm = T ./ ops * 1e9;
		semilogx(ws, tm, 'r.-');

		xlabel('working set size')
		ylabel('elemental operation time (nS)')
		ax = axis;

		% plot "in cache" lines
		t_single = mean([tm(1:4) tb(1:4)]);
		v = ws([1 4]);
		plot(v, t_single * [1 1], 'k:')
		t_multi = t_single / 2;
		plot(v, t_multi * [1 1], 'k:')
		t_multi = t_single / 4;
		plot(v, t_multi * [1 1], 'k:')

		% plot "out of cache" lines
		t_single = mean([tm(end-1:end) tb(end-1:end)]);
		t_multi = t_single / 2;
		v = ws([end-2 end]);
		plot(v, t_single * [1 1], 'k:')
		plot(v, t_multi * [1 1], 'k:')

		while length(ws) > 8
			ws = ws(1:2:end);
		end
		set(gca, 'xtick', ws);
		set(gca, 'xticklabel', prettybytes(ws));
		
		ax(1:2) = ws([1 end]);
		ax(3) = 0;
		axis(ax)
		legend('BRAHMS ST', 'BRAHMS MT', 'Monolithic', 2)
		title(['Cache Size Estimate' elap])
		
		
		
		
%% PLOT OVERHEADI

	case 'overheadi'
		
		% BM 2
		
		hold on
		
		TT = mark.result;
		[ws, ops] = elements_wss(TT.pars);
		
		nrng = size(TT.mr, 1);
		ops = ops(1:nrng)';
		
		% plot against process iteration time
		T = bbmin(TT.mr);
		x = T(end) / (TT.pars.P * TT.pars.N); % iter time of longest run
		x = x / ops(end) * ops; % expected (extrapolated) iter time of other runs
		x = x * 1e6; % in us
		
		% plot min
		T = bbmin(TT.br) ./ ops * 1e9;
		plot(x, T, 'b.-');
		T = bbmin(TT.tr) ./ ops * 1e9;
		plot(x, T, 'g.-');
		T = bbmin(TT.mr) ./ ops * 1e9;
		plot(x, T, 'r.-');

		% store monolithic time
		tm = T;
		
		% plot mean
		T = bbmean(TT.br) ./ ops * 1e9;
		plot(x, T, 'b.:');
		T = bbmean(TT.tr) ./ ops * 1e9;
		plot(x, T, 'g.:');
		T = bbmean(TT.mr) ./ ops * 1e9;
		plot(x, T, 'r.:');
		
		
		xlabel('process iteration time (uS)')
		ylabel('elemental operation time (nS)')
		ax = axis;

		% plot x2/x4 speed-up lines (x1 line based on monolithic
		% performance)
		s = sort(tm);
		t_single = mean(s(1:ceil(length(s)/3)));
		v = [1 1e6];
		plot(v, t_single * [1 1], 'k:')
		t_10 = t_single * 1.10;
		plot(v, t_10 * [1 1], 'k:')
		t_multi = t_single / 4;
		plot(v, t_multi * [1 1], 'k:')

		ax(1:2) = x([1 end]);
		ax(3) = 0;
		try
			axis(ax)
		end
		legend('BRAHMS ST', 'BRAHMS MT', 'Monolithic', 3)
		p = prettybytes(ws);
		p = p{1};
		title(['Iteration Overhead Estimate' elap ', WSS=' p])
		set(gca, 'xscale', 'log')

		
		
%% PLOT SCALING

	case 'scaling'
		
		% BM 3

		hold on

		TT = mark.result;
		[ws, ops] = elements_wss(TT.pars);

		nrng = size(TT.mr, 1);
		ops = ops(1:nrng)';

		% plot against process count
		x = TT.pars.P(1:nrng);
		
		% plot min
		T = bbmin(TT.br) ./ ops * 1e9;
		plot(x, T, 'b.-');
		T = bbmin(TT.tr) ./ ops * 1e9;
		plot(x, T, 'g.-');
		T = bbmin(TT.mr) ./ ops * 1e9;
		plot(x, T, 'r.-');

		% store monolithic time
		tm = T;
		
		% plot mean
		T = bbmean(TT.br) ./ ops * 1e9;
		plot(x, T, 'b.:');
		T = bbmean(TT.tr) ./ ops * 1e9;
		plot(x, T, 'g.:');
		T = bbmean(TT.mr) ./ ops * 1e9;
		plot(x, T, 'r.:');

		xlabel('process count')
		ylabel('elemental operation time (nS)')
		ax = axis;

		% plot x2/x4 speed-up lines (x1 line based on monolithic
		% performance)
		t_single = mean(tm);
		v = [x([1 end])];
		plot(v, t_single * [1 1], 'k:')
		t_multi = t_single / 2;
		plot(v, t_multi * [1 1], 'k:')
		t_multi = t_single / 4;
		plot(v, t_multi * [1 1], 'k:')

		ax(1:2) = x([1 end]);
		ax(3) = 0;
		try
			axis(ax)
		end
		legend('BRAHMS ST', 'BRAHMS MT', 'Monolithic', 3)
		p1 = prettybytes(ws(1));
		p2 = prettybytes(ws(end));
		p = [p1{1} ' to ' p2{1}];
		title(['Scaling Analysis' elap ', ' p])
		set(gca, 'xscale', 'log')



end

% complete plot commands before returning to benchmark code
drawnow










%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t = operation(f, i, m, n, cnt, fS, exe, opts)

% prepare
sys = sml_system;

% state
state.f = round(f);
state.i = round(i);
state.m = round(m);
state.n = round(n);

% add eight times so that we can test for multi-threading
% performance on up to eight cores
for n = 1:cnt
	p = mod([n n+1], 8) + 1;
	n = char(96 + p);
	sys = sys.addprocess(n(1), ...
		'client/brahms/bench/operation', fS, state);
	sys = sys.link([n(1) '>out'], n(2));
end

% run
[out, rep] = brahms(sys, exe, opts{:});
t = rep.Timing.caller.irt(2);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function s = dsp(n, L)

s = num2str(n);
s = [repmat(' ', 1, L-length(s)) s];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function s = stripws(s)


f = find(s ~= 9 & s ~= 32 & s~= 10 & s~= 13);

if isempty(f)
	s = '';
else
	s = s(min(f):max(f));
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [timing, log] = elements_exec(target, pars)

switch target
	case 'sb'
		func = @elements_brahms;
	case 'sm'
		func = @elements_monolithic;
end

% run it
c = cd;
try
	timing = func(pars);
catch
	cd(c);
	rethrow(lasterror)
end
cd(c);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [timing, log] = elements_monolithic(pars)

c = cd;
bench = true;

try
	path = [getenv('SYSTEMML_INSTALL_PATH') '/BRAHMS/support/bench/elements'];
	cd(path);

	if isunix
		cmd = './elements_monolithic';
	else
		cmd = 'elements_monolithic';
	end

	cmd = [cmd ' ' num2str(pars.P)];
	cmd = [cmd ' ' num2str(pars.E)];
	cmd = [cmd ' ' num2str(pars.O)];
	cmd = [cmd ' ' num2str(pars.N)];

	if ~bench
		cmd = [cmd ' logfile'];
	end

	[out, rep] = system(cmd);
	if out
		error(rep)
	end

	s = load('timing');
	timing = [];
	timing.init = s(1);
	timing.run = s(2);

	if ~bench
		f = fopen('logfile', 'rb');
		log = fread(f, Inf, 'double');
		log = reshape(log, [pars.E pars.P pars.N]);
		log = permute(log, [2 1 3]);
		fclose(f);
		delete('logfile')
	end

catch
	
	cd(c)
	rethrow(lasterror)
	
end

cd(c);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [timing, log] = elements_brahms(pars)

bench = true;
sys = sml_system;

pars.E = round(pars.E);
pars.O = round(pars.O);
pars.N = round(pars.N);

lnm = ['process_' int2str(pars.P)];
for n = 1:pars.P
	pars.p = n - 1;
	nm = ['process_' int2str(n)];
	sys = sys.addprocess(nm, 'client/brahms/bench/elements', 1, pars);
	
	% link
	sys = sys.link([lnm '>out'], nm, 1);
	lnm = nm;
end

exe = brahms_execution;
exe.name = 'brahms_bench';
exe.stop = pars.N;
exe.precision = [];

if pars.threads
	exe.execPars.MaxThreadCount = 'x1';
else
	exe.execPars.MaxThreadCount = '1';
end
exe.execPars.ShowGUI = false;
exe.execPars.Priority = 1;

if ~bench
	exe.all = true;
end

[out, rep] = brahms(sys, exe);

irt = rep.Timing.caller.irt;

timing = [];
timing.init = irt(1);
timing.run = irt(2);
timing.term = irt(3);

if ~bench
	log = zeros(pars.P, pars.E, pars.N);
	for n = 1:pars.P
		log(n, :, :) = out.(['process_' int2str(n)]).out;
	end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% return the (estimated) working set size for the given
% parameters. note that BRAHMS will use three times this,
% because it maintains two output buffers for each process
% on top of the process state

% also, return the number of operations in total

function [ws, ops] = elements_wss(pars)

ws = pars.P .* pars.E .* 4; % 4 bytes per element (UINT32)
ops = pars.P * pars.E .* pars.O .* pars.N;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% h = selectfigure(label)
%
% select (scf) or return (h) figure handle of figure labelled
% 'label'. if it does not exists, it is created.

function h = bench_selectfigure(label)

c = get(0,'children');
for n=1:length(c)
	sf = objGetValue(c(n),'selectfigure_id');
	if strcmp(sf, label)
		if nargout
			h = c(n);
		else
			figure(c(n));
		end
		return
	end
end

% otherwise, create
f = figure;
objSetValue(f,'selectfigure_id',label);
set(f,'Name',label);
set(f,'NumberTitle','off');
set(f,'IntegerHandle','off');
f = gcf;

if nargout
	h = f;
end

function val = objGetValue(obj, key)

% val = objGetValue(obj, key)
%
% try to get value in UserData of obj. this may fail if obj
% does not point to an object, or if that object has a
% non-standard UserData (should be a structure).

if nargin<1
	error('not enough arguments');
end

if nargin<2
	key = [];
end

if nargout
	val = [];
end

try
	dat = get(obj,'UserData');
catch
	error(['Handle "' num2str(obj) '" is invalid'])
end

if isempty(dat)
	dat = [];
elseif ~isstruct(dat)
	error('Object has non-standard UserData');
end

if isempty(key)
	% just show object's values
	disp([10 'user values for object with handle "' num2str(obj) '"' 10])
	disp(dat)
else
	% retrieve only specified value
	if isfield(dat,key)
		val = getfield(dat,key);
	end
end

function objSetValue(obj, key, val)

% objSetValue(obj, key, val)
%
% try to set value in UserData of obj. this may fail if obj
% does not point to an object, or if that object has a
% non-standard UserData (should be a structure).

if nargin<3
	error('not enough arguments');
end

try
	dat = get(obj,'UserData');
catch
	error(['Handle "' num2str(obj) '" is invalid'])
end

if isempty(dat)
	dat = [];
elseif ~isstruct(dat)
	error('Object has non-standard UserData');
end

dat = setfield(dat,key,val);
set(obj,'UserData',dat);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function ss = prettybytes(bs)

ss = cell(size(bs));

for n = 1:prod(size(bs))

	b = bs(n);
	
	if b < 1024
		s = [int2str(b) 'B'];
	elseif b < 1024 * 1024
		b = ceil(b / 1024);
		s = [int2str(b) 'kiB'];
	elseif b < 1024 * 1024 * 1024
		b = ceil(b / 1024 / 1024);
		s = [int2str(b) 'MiB'];
	else
		s = 'xxx';
	end
	
	ss{n} = s;
	
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t = bbmin(T)

% each output element is the min over all non-zero elements
% of T in that row

T(T==0) = max(max(T));
t = min(T, [], 2);

function t = bbmean(T)

% each output element is the mean over all non-zero elements
% of T in that row

for n = 1:size(T, 1)
	tr = T(n, :);
	tr = tr(tr ~= 0);
	t(n, 1) = mean(tr);
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function h = brahms_waitbar(varargin)

if brahms_utils('IsOctave')
	h = [];
	return
else
	h = waitbar(varargin{:});
end

function safeclose(h)

if ~isempty(h)
	close(h)
end




