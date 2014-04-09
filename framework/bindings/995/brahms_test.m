% [out, rep, sys, exe] = brahms_test(test, ...)
%
% Test your BRAHMS installation, or test the operation
% of a BRAHMS implementation. "test" can be any one of
% the following, and most tests can be suffixed ".s"
% or ".m" to multiprocess using the sockets or MPI
% layer, respectively.
%
% Test Suites (BRAHMS GUI and output are hidden)
%   solo - run all tests (solo only)
%   sockets - run all sockets-concerto tests
%   most - run solo and sockets
%   mpi - run all mpi-concerto tests
%     (note that this suite requires MPICH2)
%   all - run solo, sockets and mpi
%     (note that this suite requires MPICH2)
%
% Test Systems (single process, solo only)
%   source - framework correctly installed
%   filemode - source/numeric in File Mode
%   rng - test random number generator
%   precision - control output precision
%   io - pass big data in and out efficiently
%   window - window the output logs
%
% Test Systems (can multiprocess)
%   std - standard library correctly installed
%   pair - feed-forward, A ==> B
%   loop - feed-back, A ==> B ==> A
%   nested - hierarchical (nested) system
%   resamp - resample numeric data
%   seed - control random seeding
%   sequence - communications logic
%   nlag - n-sample delay logic
%   comms - scalable inter-process comms (speed test)
%   loneranger - lone ranger process
%   affinity - voice affinity
%   seeds - automatic random seeding
%   sets - exercise SystemML sets
%   pause - pause & continue execution
%   channels - debugging comms
%   buffers - illustrate long buffers
%
% Language Development Demos
%   language - processes in each language
%
% Systems that generate errors (not included in suites)
%   hang - generate a thread hang
%   except - throw and catch exceptions
%   abort - cause an abort
%   deadlock - a system that suffers from deadlock
%   matflict - conflict between matlab processes
%
% Tests of things under development
%   image - image/numeric
%   video - image/numeric
%     'file.avi': avi file to display
%   cat - cat/numeric
%   index - index/numeric
%   hugelogs - handle huge logs using sml_log
%   listened - check performance gain of listened
%   eval - generalised eval block
%   complex - adjacent/interleaved complex handling
%
% Additional development tools
%   examples - run all examples to confirm operation
%   gui - exercise the executable GUI
%   busy - be busy for a bit
%     'sleep': sleep instead of using CPU
%     T: be busy for about T seconds (default 60)
%
% Currently, most of these tests take no further arguments,
% but any string arguments beginning '--' are passed
% directly to BRAHMS as options. You can, for instance, set
% the debug level through this mechanism. In addition, this
% function understands the following arguments.
%
% background
%   run as quietly as possible (no output)
%
% prep
%   prepare the execution, but don't run it
%
% novalidate
%   do not validate results, if would normally
%
% nologs
%   set exe.all = false (no effect unless true by default)
%
% saveexpected
%   update the expected results for this run (developer
%   only)
%
% comp=N
%   use compression level N
%
% t=N
%   run for N times as long as the default
%
% v=N
%   use N voices (when multiprocessing only)



%   soak - like "all", but runs past errors, placing
%     error messages in files "soak-<N>.txt" in the
%     current directory. execution will continue until
%     you CTRL+C to break.

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
%	$Id:: brahms_test.m 2449 2010-01-25 16:02:52Z benjmitch    $
%	$Rev:: 2449                                                $
%	$Author:: benjmitch                                        $
%	$Date:: 2010-01-25 16:02:52 +0000 (Mon, 25 Jan 2010)       $
%________________________________________________________________
%


function [out_, rep_, sys, exe, err] = brahms_test(test, varargin)


if ~nargin
	help brahms_test
	return
end

% check we have SystemML
if isempty(which('sml_utils'))
	error('SystemML Toolbox not installed');
end





%% HANDLE SCRIPT OPTIONS

opt = [];
opt.test = test;
opt.validate = true;
opt.multiprocess = false;
opt.usempi = false;
opt.voicecount = 2;
opt.voicecountoverride = 0;
opt.tmult = 1;
opt.suppressoutput = false;
opt.compression = 0;
opt.saveexpected = false;
opt.nologs = false;
% opt.soakinstance = false;
opt.dontrun = false;

switch test(end-1:end)
	case '.s'
		test = test(1:end-2);
		opt.multiprocess = true;
	case '.m'
		test = test(1:end-2);
		opt.multiprocess = true;
		opt.usempi = true;
end




%% HANDLE BRAHMS OPTIONS

opts_ = {};
args_ = {};

for n = 1:length(varargin)

	arg = varargin{n};

	maybe = ischar(arg) & length(arg) >= 2;
	definitely = false;
	if maybe
		definitely = strcmp(arg(1:2), '--');
	end

	if definitely % to replace this: ischar(arg) & length(arg) >= 2 & strcmp(arg(1:2), '--')

		% pass to BRAHMS
		opts_{end+1} = arg;

	else

		switch arg

			case 'background'
				opt.suppressoutput = true;
				opts_{end+1} = '--par-ShowGUI-0';

			case 'prep'
				opt.dontrun = true;

			case 'novalidate'
				opt.validate = false;
				
			case 'nologs'
				opt.nologs = true;

			case 'saveexpected'
				opt.saveexpected = true;

			otherwise

				if length(arg)>5 && strcmp(arg(1:5), 'comp=')
					opt.compression = str2num(arg(6:end));

				elseif length(arg)>2 && strcmp(arg(1:2), 't=')
					opt.tmult = str2num(arg(3:end));

				elseif length(arg)>2 && strcmp(arg(1:2), 'v=')
					opt.voicecountoverride = str2num(arg(3:end));

				else
					disp(['"' arg '" assumed to be argument to specific test (may be ignored)'])
					args_{end+1} = arg;

				end

		end

	end

end

opt.opts = opts_;
opt.args = args_;





%% TEST SUITES

tests = {
	'source'
	'filemode'
	'rng'
	'precision'
	'io'
	'window'
	'std'
	'pair'
	'loop'
	'nested'
	'resamp'
	'seed'
	'sequence'
	'nlag'
	'comms'
	'loneranger'
	'affinity'
	'seeds'
	'sets'
	'channels'
	'buffers'
	'language'
	};
suite.solo = tests;
suite.sockets = addchar(tests, 's');
suite.mpi = addchar(tests, 'm');



%% SWITCH ON TEST

switch(test)





	%% SOLO, ALL

	case {'all' 'most' 'solo' 'sockets' 'mpi' 'soak'}

		execpars = brahms_utils('GetExecutionParameters');
		if isempty(execpars.WorkingDirectory)
			disp(['WorkingDirectory is not set - this will make a lot of' 10 ...
				'files in the current directory, so I won''t start. Run' 10 ...
				'"brahms_manager" first, and set WorkingDirectory.']);
			return
		end

		% if 'soak', we loop indefinitely
		% 		soak = false;
		% 		soakerrcount = 0;
		while 1

			switch test
				case 'all'
					tests = {suite.solo{:} suite.sockets{:} suite.mpi{:}}';
				case 'most'
					tests = {suite.solo{:} suite.sockets{:}}';
				case 'solo'
					tests = suite.solo;
				case 'sockets'
					tests = suite.sockets;
				case 'mpi'
					tests = suite.mpi;
					% 				case 'soak'
					% 					tests = {suite.solo{:} suite.sockets{:}}';
					% 					soak = true;
			end

			for n = 1:length(tests)
				disp(['________________________________________________' 10]);
				disp(['brahms_test "' tests{n} '"']);
				% 				if soak
				% 					err = brahms_test(tests{n}, 'background', '--soak', opts{:});
				% 					if ischar(err)
				% 						soakerrcount = soakerrcount + 1;
				% 						filename = ['soak-' int2str(soakerrcount) '.txt'];
				% 						fid = fopen(filename, 'wt');
				% 						if fid ~= -1
				% 							fwrite(fid, err);
				% 							fclose(fid);
				% 						end
				% 						disp(['WROTE ' filename]);
				% 					end
				% 				else
				brahms_test(tests{n}, 'background', opt.opts{:});
				% 				end
			end
			disp(['________________________________________________' 10]);

			% 			% break while 1 unless soak
			break
			% 			if ~soak break; end

		end


		% return, so we don't do execute at the bottom
		return




		%% SOURCE

	case 'source'

		% just test source, very basic test
		sys = sml_system;

		% process
		state = [];
		state.data = magic(3);
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', 10, state);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 3;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		% plot
		if ~opt.suppressoutput
			figure(1)
			clf
			hold on
			plot(expected.src.out', 'cs')
			plot(out.src.out', 'r')
		end



		%% FILEMODE

	case 'filemode'

		% test source reading from binary file
		sys = sml_system;
		
		% get filename
		sip = brahms_utils('GetSystemMLInstallPath');
		
		% set this flag to make a mistake
		wrong = false;
		if length(opt.args)
			arg = opt.args{1};
			% whatever the arg is, just get it wrong
			wrong = true;
		end

		% process
		state = [];
		state.data = [sip '/BRAHMS/media/interleaved-128.dat'];
		state.sourceIsAdjacent = wrong;;
		state.start = 0;
		state.type = 'UINT8';
		state.dims = [128 3 128];
		state.repeat = true;
		sys = sys.addprocess('src1', 'std/2009/source/numeric', 128, state);
		
		% process
		state = [];
		state.data = [sip '/BRAHMS/media/adjacent-128.dat'];
		state.sourceIsAdjacent = ~wrong;;
		state.type = 'UINT8';
		state.dims = [128 3 128];
		sys = sys.addprocess('src2', 'std/2009/source/numeric', 128, state);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		if wrong opt.validate = false; end
		opt.validate = true;
		[out, rep, expected] = execsys(sys, exe, opt);
		
		% plot
		if ~opt.suppressoutput
			figure(1)
			clf
			subplot(2,2,1)
			im = permute(real(out.src1.out), [1 3 2]);
			image(im)
			title('interleaved (real)');
			subplot(2,2,2)
			im = permute(imag(out.src1.out), [1 3 2]);
			image(im)
			title('interleaved (imag)');
			subplot(2,2,3)
			im = permute(real(out.src2.out), [1 3 2]);
			image(im)
			title('adjacent (real)');
			subplot(2,2,4)
			im = permute(imag(out.src2.out), [1 3 2]);
			image(im)
			title('adjacent (imag)');
		end

		

		%% RNG

	case 'rng'

		% just test source, very basic test
		sys = sml_system;
		state = [];
		state.dims = 1;
		state.dist = 'normal';
		state.pars = [1 0.1];
		state.complex = false;
		sys = sys.addprocess('rng', 'std/2009/random/numeric', 1000, state);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;
		exe.seed = 1;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% STD

	case 'std'

		% construct SystemML system
		sys = sml_system;
		fS = 100;

		% construct SystemML sub-system
		sub = sml_system;

		% add a process
		state = [];
		randn('state', 42);
		state.data = randn(3,fS) + i * randn(3,fS);
		state.repeat = true;
		sub = sub.addprocess('source_numeric', 'std/2009/source/numeric', fS, state);

		% add a process (alternative grammar 1)
		seed = 42;
		sub = sub.addprocess('random_numeric', 'std/2009/random/numeric', fS, [], seed);
		state = [];
		state.dist = 'normal';
		state.pars = [2 0.2];
		state.dims = uint32([1]);
		state.complex = false;
		sub.random_numeric.state = state;

		% add a process (alternative grammar 2)
		seed = 42;
		sub = sub.addprocess('source_spikes', 'std/2009/random/spikes', fS, [], seed);
		sub.source_spikes.state.dist = 'exponential';
		sub.source_spikes.state.streams = 25;
		sub.source_spikes.state.pars = [0.02 0.03];

		% expose sub/source/numeric>out as sub>out
		sub = sub.expose('source_numeric>out', 'out');

		% add sub-system to main system
		sys = sys.addsubsystem('sub', sub);

		% add some processes that don't need state
		sys = sys.addprocess('eproduct', 'std/2009/math/eproduct', fS);
		sys = sys.addprocess('esum', 'std/2009/math/esum', fS);
		state = [];
		state.order = 1;
		sys = sys.addprocess('resample_numeric', 'std/2009/resample/numeric', fS * 2, state);
		sys = sys.addprocess('resample_spikes', 'std/2009/resample/spikes', fS * 2);

		% add a link or two
		sys = sys.link('sub>out', 'eproduct');
		sys = sys.link('sub/random_numeric>out', 'eproduct');
		sys = sys.link('eproduct>out', 'esum');
		sys = sys.link('eproduct>out', 'esum');
		sys = sys.link('sub/random_numeric>out', 'resample_numeric');
		sys = sys.link('sub/source_spikes>out', 'resample_spikes');

		% construct exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 10;
		exe.encapsulated = true;
		exe.precision = [];

		% execute
		if brahms_utils('IsOctave')
			% can't validate, since randn()s are different
			opt.validate = false;
		end
		[out, rep, expected] = execsys(sys, exe, opt);



		%% PAIR

	case 'pair'

		% just a pair of processes, communicating
		sys = sml_system;
		fS = 100;

		% process 1 (source)
		state = [];
		state.data = magic(3);
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', fS, state);

		% process 2 (sum)
		state = [];
		sys = sys.addprocess('sum', 'std/2009/math/esum', fS, state);

		% link
		sys = sys.link('src>out', 'sum');

		% command
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% LOOP

	case 'loop'

		% just a pair of processes, communicating
		sys = sml_system;
		fS = 100;

		% process 1 (sum)
		state = [];
		state.complex = false;
		state.dims = [2 3];
		state.type = 'DOUBLE';
		sys = sys.addprocess('sum1', 'std/2009/math/esum', fS, state);

		% process 2 (sum)
		state = [];
		state.complex = false;
		state.dims = [2 3];
		state.type = 'DOUBLE';
		sys = sys.addprocess('sum2', 'std/2009/math/esum', fS, state);

		% link
		sys = sys.link('sum1>out', 'sum2');
		sys = sys.link('sum2>out', 'sum1');

		% command
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% NESTED

	case 'nested'

		% create a system that has sub-systems, and test it all
		% computes correctly in BRAHMS
		%
		% ROOT
		%   GENERATOR
		%     RAND
		%     CONSTANT
		%     MULT
		%     TESTSUB
		% 			RAND
		% 			CONSTANT
		% 			MULT
		%   INTEGRATOR
		%     SUM (RECURSIVE CONNECTION)
		%   SUM

		% empty sub-system
		generator = sml_system;

		% process
		state = [];
		state.dims = uint32([2 2]);
		state.dist = 'normal';
		state.pars = [0 3];
		state.complex = false;
		generator = generator.addprocess('rnd', 'std/2009/random/numeric', 100, state);

		% process
		state = [];
		state.data = 42*ones(1,101);
		state.repeat = false;
		generator = generator.addprocess('cst', 'std/2009/source/numeric', 100, state);

		% process
		generator = generator.addprocess('prd', 'std/2009/math/eproduct', 100);

		% links
		generator = generator.link('rnd>out', 'prd');
		generator = generator.link('cst>out', 'prd');

		% empty sub-sub-system
		testsub = sml_system;

		% process
		state = [];
		state.dims = uint32([2 2]);
		state.dist = 'normal';
		state.pars = [1 2];
		state.complex = false;
		testsub = testsub.addprocess('rnd', 'std/2009/random/numeric', 100, state);

		% process
		state = [];
		state.data = 42*ones(1,101);
		state.repeat = false;
		testsub = testsub.addprocess('cst', 'std/2009/source/numeric', 100, state);

		% process
		testsub = testsub.addprocess('prd', 'std/2009/math/eproduct', 100, state);

		% links
		testsub = testsub.link('rnd>out', 'prd');
		testsub = testsub.link('cst>out', 'prd');

		% expose some of testsub's internals
		testsub = testsub.expose('prd>out', 'out');

		% add testsub to generator
		generator = generator.addsubsystem('testsub', testsub);

		% expose some of generator's internals
		generator = generator.expose('prd>out', 'out1');
		generator = generator.expose('testsub>out', 'out2'); % this is what testsub exposed, now we're exposing it up another level

		% a second sub-system
		integrator = sml_system;

		% process
		state = [];
		state.data = 42*ones(2,2,101);
		state.repeat = false;
		integrator = integrator.addprocess('cst', 'std/2009/source/numeric', 100, state);

		% process
		state = [];
		state.complex = false;
		integrator = integrator.addprocess('sum', 'std/2009/math/esum', 100, state);

		% links
		integrator = integrator.link('cst>out', 'sum');
		integrator = integrator.link('sum>out', 'sum'); % recursive, reflexive, whatever...

		% expose some of integrator's internals
		integrator = integrator.expose('sum<<<in', 'in'); % 'sum<<<in' is entirely equivalent to 'sum<in' (default set in both cases, explicitly or implicitly)

		% build the top-level system, at last
		sys = sml_system;

		% add the sub-systems
		sys = sys.addsubsystem('generator', generator);
		sys = sys.addsubsystem('integrator', integrator);

		% and add an extra process
		state = [];
		sys = sys.addprocess('sum', 'std/2009/math/esum', 100);

		% and link it all together
		sys = sys.link('generator>out1', 'sum');
		sys = sys.link('generator>out2', 'integrator<in');

		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;
		exe.seed = 1;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% RESAMP

	case 'resamp'


		% RESAMPLE a numeric of arbitrary type
		f1 = 13;
		f2 = '513/10';
		T1 = 1 / f1;
		T2 = 1 / str2num(f2);
		r1 = 0:T1:1-1e-8;
		r2 = 0:T2:1-1e-8;
		srcdata = single(sin(r1*pi*2));

		% empty system
		sys = sml_system;

		% add process
		state = [];
		state.data = srcdata;
		state.repeat = false;
		sys = sys.addprocess('src', 'std/2009/source/numeric', f1, state);

		% add process
		state = [];
		state.order = 1;
		sys = sys.addprocess('resamp', 'std/2009/resample/numeric', f2, state);

		% links
		sys = sys.link('src>out', 'resamp');

		% exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;
		exe.precision = [];

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		% plot results
		if ~opt.suppressoutput
			clf
			hold on
			g = 0.8 * [1 1 1];
			plot(r1, expected.src.out, 'co-')
			plot(r2, expected.resamp.out, 'co-')
			plot(r1, out.src.out, 'k.-')
			plot(r2, out.resamp.out, 'r.-')
			hold off
		end



		%% SEED

	case 'seed'

		% random seeding
		sys = sml_system;

		state = [];
		state.dims = 1;
		state.pars = [0 1];
		state.dist = 'normal';
		state.complex = false;
		sys = sys.addprocess('srcfixedseed', 'std/2009/random/numeric', 100, state, 1);

		state = [];
		state.dims = 1;
		state.pars = [0 3];
		state.dist = 'normal';
		state.complex = false;
		sys = sys.addprocess('srcclockseed', 'std/2009/random/numeric', 100, state);

		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;
		exe.precision = [];
		% exe.seed = 1; % global fixed seed will fix outputs of all processes

		% can't check random results!
		opt.nocheck = {'srcclockseed'};

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		if ~opt.suppressoutput
			clf
			plot(out.srcclockseed.out,'r');
			hold on
			plot(expected.srcfixedseed.out,'cx');
			plot(out.srcfixedseed.out,'k');
			hold off
			axis([0 100 -10 10])
		end



		%% PRECISION

	case 'precision'

		sys = sml_system;
		fS = 1;
		N = 4;

		state = [];
		state.data = [0.123 0.123456 0.123456789 0.12345678912345];
		state.repeat = false;
		sys = sys.addprocess('src', 'std/2009/source/numeric', fS, state);

		% construct exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = N;
		exe.precision = []; % default "max"
		exe.encapsulated = true; % in non-encapsulated mode, data is returned at full precision, regardless of the precision setting

		format long g
		format compact

		for precision = {3, 6, 9, []}

			exe2 = exe.log('src>out', {'precision', precision{1}});
			[out, rep] = brahms(sys, exe2, opt.opts{:});

			if ~isempty(precision{1})
				p = ['SF' int2str(precision{1}) '  '];
			else
				p = ['MAX' '  '];
			end

			s = sprintf('%.16f  ', out.src.out);
			s(s == '0') = ' ';

			if ~opt.suppressoutput
				disp([p s])
			end

		end

		format



		%% IO

	case 'io'

		% large numeric source
		N = 500;
		NN = N * N;
		randn('seed', 1);

		% generate data (not timed)
		data = randn(N);
		data(1,1) = Inf; % test propagation
		data(2,1) = NaN; % test propagation
		data(3,1) = -0.17e-34;
		data(4,1) = 91e38;

		% construct system passing data through the SystemML
		sys = sml_system;
		state = [];
		state.data = data;
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', N, state);

		% timing
		t0 = clock;

		% construct exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = 'io-encap';
		exe.stop = 1;
		exe.precision = []; % default "max"
		exe.encapsulated = true;

		% execute
		t1 = etime(clock, t0);
		[out, rep, inv] = brahms(sys, exe, opt.opts{:});
		t2 = etime(clock, t0);

		% check integrity (nan/inf can't be equal)
		% matching may not be absolutely perfect, because
		% the representation generated when DataML is written
		% is at the mercy of matlab's sprintf().
		err = abs(out.src.out(3:NN) - data(3:NN));
		relerr = err ./ data(3:NN);
		abserr = abs(relerr);
		if any(any(abserr > 1e-14))
			warning('mismatch, max abserr is:');
			max(max(abserr))
		end

		% timing
		t3 = etime(clock, t0);

		% timing
		if ~opt.suppressoutput
			irt = rep.Timing.caller.irt';
			disp(' ')
			disp(['pass data through SystemML:'])
			disp(['-------------------------------'])
			disp(['    prepare:       ' sprintf('%8.3f', t1)])
			disp(['    run:           ' sprintf('%8.3f', t2-t1)])
			disp(['      serialize:   ' sprintf('%8.3f', inv.perf.serialize) '    (write ExecutionML/SystemML in matlab)'])
			disp(['      execute:     ' sprintf('%8.3f', inv.perf.execute)])
			disp(['        init:      ' sprintf('%8.3f', irt(1)) '    (read SystemML in BRAHMS)'])
			disp(['        run:       ' sprintf('%8.3f', irt(2))])
			disp(['        term:      ' sprintf('%8.3f', irt(3)) '    (write ExecutionML in BRAHMS)'])
			disp(['      unserialize: ' sprintf('%8.3f', inv.perf.collect) '    (read ExecutionML in matlab)'])
			disp(['    integrity:     ' sprintf('%8.3f', t3-t2)])
			disp(['-------------------------------'])
			disp(['    total:         ' sprintf('%8.3f', t3)])
			disp(' ')
		end

		% timing
		t0 = clock;

		% construct exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = 'io-unencap';
		exe.stop = 1;
		exe.precision = [];

		% execute
		t1 = etime(clock, t0);
		[out, rep, inv] = brahms(sys, exe, opt.opts{:});
		t2 = etime(clock, t0);

		% check integrity
		err = abs(out.src.out(3:NN) - data(3:NN));
		relerr = err ./ data(3:NN);
		abserr = abs(relerr);
		if any(any(abserr > 1e-14))
			warning('mismatch, max abserr is:');
			max(max(abserr))
		end

		% timing
		t3 = etime(clock, t0);

		% timing
		if ~opt.suppressoutput
			irt = rep.Timing.caller.irt';
			disp(' ')
			disp(['pass data through binary file:'])
			disp(['-------------------------------'])
			disp(['    prepare:       ' sprintf('%8.3f', t1)])
			disp(['    run:           ' sprintf('%8.3f', t2-t1)])
			disp(['      serialize:   ' sprintf('%8.3f', inv.perf.serialize) '    (write ExecutionML/SystemML & Binary File in matlab)'])
			disp(['      execute:     ' sprintf('%8.3f', inv.perf.execute)])
			disp(['        init:      ' sprintf('%8.3f', irt(1)) '    (read SystemML & Binary File in BRAHMS)'])
			disp(['        run:       ' sprintf('%8.3f', irt(2)) '    (write Binary File in BRAHMS)'])
			disp(['        term:      ' sprintf('%8.3f', irt(3)) '    (write ExecutionML in BRAHMS)'])
			disp(['      unserialize: ' sprintf('%8.3f', inv.perf.collect) '    (read ExecutionML & Binary File in matlab)'])
			disp(['    integrity:     ' sprintf('%8.3f', t3-t2)])
			disp(['-------------------------------'])
			disp(['    total:         ' sprintf('%8.3f', t3)])
			disp(' ')
		end

		% info
		if ~opt.suppressoutput
			disp(['For large datasets (here ' int2str(NN) ' values) it is faster' 10 ...
				'to pass data through a binary file than through a StateML' 10 ...
				'tag. This is due to the time taken to parse numeric data to' 10 ...
				'and from the tag. The great bulk of this time is used by matlab' 10 ...
				'since the BRAHMS numeric parsers are fairly fast, but that still' 10 ...
				'means you have to wait for it, unless you use some other system' 10 ...
				'to read and write your files. Passing your data in and out as' 10 ...
				'raw binary has the disadvantage that your System/Output are no' 10 ...
				'longer self-contained, but this does not matter during' 10 ...
				'development.' 10 10 ...
				'To use binary files, turn off encapsulation. When you are' 10 ...
				'ready to publish, turn encapsulated input and output' 10 ...
				'back on to perform an execution with self-contained io files.' 10 10 ...
				'NOTE: we may well work on the speed of the matlab parts of the' 10 ...
				'i/o in future, perhaps by mexifying the operations. You will get' 10 ...
				'transparent advantage from this by installing a new release of' 10 ...
				'BRAHMS when this improvement becomes available.' ...
				]);
		end



		%% WINDOW

	case 'window'

		% just test source, very basic test
		sys = sml_system;
		state = [];
		state.data = 0:0.01:0.99;
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', 100, state);

		exe = brahms_execution;
		exe.name = test;
		exe.stop = 1;

		window = '0.03,0.07:0.27; @0.33:0.1:0.63,0.02:0.04,0.06,0.08; @0.7:0.04:0.9,0.01';
		exe = exe.log('src>out', {'window', window});
% 		exe.window = window;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		if ~opt.suppressoutput
			figure(1)
			plot(out.src.out)
		end



		%% SEQUENCE

	case 'sequence'

		% Benchmark system for validating comms logic
		%
		% Voice0: 0c->0a, 1c->0b, 2c->0c
		% Voice1: 0a->1a, 2a->1b, 1b->1c
		% Voice2: 1a->2a, 0b->2b, 2b->2c

		% prepare exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 10;

		% prepare system
		sub = sml_system;
		state = [];
		state.tmax = 10;
		cls = 'client/brahms/test/sequence';
		rates = [8 1 6 3 5 7 4 9 2];
		inds = '000111222';
		chrs = 'abcabcabc';
		srcs = {'0c', '1c', '2c', '0a', '2a', '1b', '1a', '0b', '2b'};
		seed = 42;

		% processes
		for p = 1:9
			state.state = p * 13;
			name = ['sequence' inds(p) chrs(p)];
			src = ['sequence' srcs{p} '>out'];
			sub = sub.addprocess(name, cls, rates(p), state, seed);
			sub = sub.link(src, name);
		end

		sys = sml_system;
		sys = sys.addsubsystem('sub', sub);

		% options
		opt.voicecount = 3;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% NLAG

	case 'nlag'

		% 		rand('state', 1);
		% 		randn('state', 1);
		% 		warning('fixing lags');

		sys = sml_system;
		fS = 1;
		N = 20;
		t = 0:N-1;

		% construct signal generators (sine, square, sawtooth, triangle)
		generators = [sin(2*pi*t/N); square(2*pi*t/N, 50); sawtooth(2*pi*t/N, 1); sawtooth(2*pi*t/N,.5)];

		% add sine generator
		state = [];
		state.data = generators(1,:);
		state.repeat = true;
		sys = sys.addprocess('sin', 'std/2009/source/numeric', fS, state);

		% add square generator
		state.data = generators(2,:);
		sys = sys.addprocess('square', 'std/2009/source/numeric', fS, state);

		% add sawtooth generator
		state.data = generators(3,:);
		sys = sys.addprocess('sawtooth', 'std/2009/source/numeric', fS, state);

		% add triangle generator
		state.data = generators(4,:);
		sys = sys.addprocess('triangle', 'std/2009/source/numeric', fS, state);

		% add four sum blocks
		state = [];
		sys = sys.addprocess('sum1', 'std/2009/math/esum', fS, state);
		sys = sys.addprocess('sum2', 'std/2009/math/esum', fS, state);
		sys = sys.addprocess('sum3', 'std/2009/math/esum', fS, state);
		sys = sys.addprocess('sum4', 'std/2009/math/esum', fS, state);

		% generate random lags
		lags = floor(rand(4) * N);

		% connect with lags (except the diagonal)
		sys = sys.link('square>out', 'sum1', lags(1, 2));
		sys = sys.link('sawtooth>out', 'sum1', lags(1, 3));
		sys = sys.link('triangle>out', 'sum1', lags(1, 4));

		sys = sys.link('sin>out', 'sum2', lags(2, 1));
		sys = sys.link('sawtooth>out', 'sum2', lags(2, 3));
		sys = sys.link('triangle>out', 'sum2', lags(2, 4));

		sys = sys.link('sin>out', 'sum3', lags(3, 1));
		sys = sys.link('square>out', 'sum3', lags(3, 2));
		sys = sys.link('triangle>out', 'sum3', lags(3, 4));

		sys = sys.link('sin>out', 'sum4', lags(4, 1));
		sys = sys.link('square>out', 'sum4', lags(4, 2));
		sys = sys.link('sawtooth>out', 'sum4', lags(4, 3));

		% precompute answers (the filter is mitch's, that's why it's so elegant)
		% oi, don't be facetious :)
		nlag = zeros(4, N);
		for n = 1:4
			for m = 1:4
				if n ~= m
					nlag(n,:) = nlag(n,:) + filter([zeros(1, lags(n, m)) 1], 1, generators(m,:));
				end
			end
		end

		% compare with brahms
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = N;
		exe.precision = [];

		% multiprocess?
		if opt.multiprocess
			if opt.usempi
				exe.addresses = {'mpi', 4};
			else
				exe.addresses = {'sockets', 4};
			end
		end

		% execute
		[out, rep] = brahms(sys, exe, opt.opts{:});

		% handle soak
		if ~ischar(out)

			% collate
			collated = [out.sum1.out; out.sum2.out; out.sum3.out; out.sum4.out];

			% plot with cd-r style (c expected, r actual, following brahms convention)
			if ~opt.suppressoutput
				clf;
				for n = 1:4
					subplot(4,1,n);
					hold on;
					plot(t, nlag(n,:), 'cd');
					plot(t, collated(n,:), 'r');
					hold off;
				end
			end

			% check results
			if ~similar(nlag, collated)
				error('computation incorrect');
			end

		end



		%% COMMS

	case 'comms'

		% construct SystemML system
		sys = sml_system;
		fS = 1;

		% arg is amount of data to pass
		if length(opt.args) < 1
			bytes = 8;
		else
			bytes = opt.args{1};
			if ischar(bytes)
				bytes = str2num(bytes);
			end
			disp(['bytes=' int2str(bytes)])
		end

		% number of reps
		if opt.multiprocess
			reps = 100;
		else
			reps = 10000;
		end

		% state
		state = [];
		state.complex = false;
		state.dims = [ceil(bytes/8)];
		state.type = 'DOUBLE';

		% processes
		sys = sys.addprocess('sum0', 'std/2009/math/esum', fS, state);
		sys = sys.addprocess('sum1', 'std/2009/math/esum', fS, state);

		% add a link or two
		sys = sys.link('sum0>out', 'sum1');
		sys = sys.link('sum1>out', 'sum0');

		% construct exe
		exe = brahms_execution;
		% 		exe.all = true; % don't log, we're interested in comms complexity only
		exe.name = test;
		exe.stop = reps;
		if strcmp(test, 'commsx')
			exe.ips = {'143.167.74.18', '143.167.72.2'};
			exe.executionFile = '\\kroncha.shef.ac.uk\scratch\shared-exe.xml';
		end

		% make sure threading is off
		opt.opts{end+1} = '--nothreads';

		% nothing stored, so nothing to validate
		opt.validate = false;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		% report performance
		if opt.multiprocess
			t = rep(1).Timing.caller.irt(2);
		else
			t = rep.Timing.caller.irt(2);
		end

		np = length(sys.processes);
		nl = length(sys.links);
		disp([int2str(np) ' processes, ' int2str(nl) ' links; ' num2str(t) ' seconds (' num2str(round(t/exe.stop*1e7)/10) 'uS/iteration)']);



		%% LONERANGER

	case 'loneranger'

		%% LONE-RANGER PROCESS DEMO

		% a lone-ranger occurs when a process of low computational
		% requirements is not constrained to stay behind with the
		% other processes in the system. it may then run far ahead,
		% causing a communications back-log as well as possibly
		% causing timeout problems when it reaches stop.
		%
		% the constraints that can be placed upon a process
		% preventing it from going lone-ranger are:
		%
		%   i) it receives input from another process, and must thus
		%   wait for this input.
		%
		%   ii) it generates output to another process and passes it
		%   into a buffer which is of finite size.
		%
		%   iii) it is in the same processing thread as a process
		%   that has one of the above constraints.
		%
		% note that in solo, buffers are of finite size, so no
		% process can go lone-ranger unless it is entirely
		% unconnected (and if it *is* entirely unconnected, it
		% doesn't do any harm if it goes lone-ranger anyway). in
		% concerto, buffers may be of infinite size (since comms are
		% simply squirted into the ether, without waiting for an
		% ACK) so it is generally in concerto that lone-ranger
		% processes are a problem.

		fS = 1;

		% system
		sys = sml_system;

		% light (src)
		state = [];
		state.data = [1 2 3];
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', fS, state);

		% light (via)
		state = [];
		state.msec = 1;
		state.sleep = false;
		sys = sys.addprocess('via', 'client/brahms/test/busy', fS, state);

		% heavy (dst)
		state = [];
		state.msec = 25;
		state.sleep = false;
		sys = sys.addprocess('dst', 'client/brahms/test/busy', fS, state);

		% in multiprocessing, we can go loneranger despite that
		% we've got a fully linked system
		if opt.multiprocess

			% one voice per process
			opt.voicecount = 3;

			% link
			sys = sys.link('src>out', 'via', 1);
			sys = sys.link('via>out', 'dst', 1);

		else

			% in solo, we need a system that has entirely
			% independent parts, if we're to see a loneranger, so
			% we add no links

		end

		% command
		exe = brahms_execution;
		exe.name = test;
		exe.stop = 250;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% AFFINITY

	case 'affinity'

		% construct SystemML system
		sys = sml_system;
		fS = 100;

		% construct SystemML sub-system
		sub = sml_system;

		% add a process
		state = [];
		randn('state', 42);
		state.data = randn(3,fS) + i * randn(3,fS);
		state.repeat = true;
		sub = sub.addprocess('source_numeric', 'std/2009/source/numeric', fS, state);

		% add a process (alternative grammar 1)
		seed = 42;
		sub = sub.addprocess('random_numeric', 'std/2009/random/numeric', fS, [], seed);
		state = [];
		state.dist = 'normal';
		state.pars = [2 0.2];
		state.dims = uint32([1]);
		state.complex = false;
		sub.random_numeric.state = state;

		% add a process (alternative grammar 2)
		seed = 42;
		sub = sub.addprocess('source_spikes', 'std/2009/random/spikes', fS, [], seed);
		sub.source_spikes.state.dist = 'exponential';
		sub.source_spikes.state.streams = 25;
		sub.source_spikes.state.pars = [0.02 0.03];

		% expose sub/source/numeric>out as sub>out
		sub = sub.expose('source_numeric>out', 'out');

		% add sub-system to main system
		sys = sys.addsubsystem('sub', sub);

		% add some processes that don't need state
		sys = sys.addprocess('eproduct', 'std/2009/math/eproduct', fS);
		sys = sys.addprocess('esum', 'std/2009/math/esum', fS);
		state = [];
		state.order = 1;
		sys = sys.addprocess('resample_numeric', 'std/2009/resample/numeric', fS * 2, state);
		sys = sys.addprocess('resample_spikes', 'std/2009/resample/spikes', fS * 2);

		% add a link or two
		sys = sys.link('sub>out', 'eproduct');
		sys = sys.link('sub/random_numeric>out', 'eproduct');
		sys = sys.link('eproduct>out', 'esum');
		sys = sys.link('eproduct>out', 'esum');
		sys = sys.link('sub/random_numeric>out', 'resample_numeric');
		sys = sys.link('sub/source_spikes>out', 'resample_spikes');

		% construct exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;
		exe.encapsulated = true;
		exe.precision = [];

		% options
		opt.voicecount = 4;
		if opt.multiprocess
			exe.affinity = {{'sub'}, {'eproduct' 'esum'}};
		end

		% execute
		if brahms_utils('IsOctave')
			% can't validate, since randn()s are different
			opt.validate = false;
		end
		[out, rep, expected] = execsys(sys, exe, opt);



		%% SEEDS

	case 'seeds'

		% random seeding
		sys = sml_system;

		% RNG state
		state = [];
		state.dims = 1;
		state.pars = [0 1];
		state.dist = 'normal';
		state.complex = false;

		% RNG processes (one with explicit seed)
		sys = sys.addprocess('src1', 'std/2009/random/numeric', 1, state);
		sys = sys.addprocess('src2', 'std/2009/random/numeric', 1, state, 12345);
		sys = sys.addprocess('src3', 'std/2009/random/numeric', 1, state);
		sys = sys.addprocess('src4', 'std/2009/random/numeric', 1, state);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 100;
		exe.precision = [];
		exe.seed = 42; % global fixed seed will fix outputs of all processes

		% options
		opt.voicecount = 4;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% SETS

	case 'sets'

		% exercise sets
		sys = sml_system;

		% process
		state = [];
		sys = sys.addprocess('sets', 'client/brahms/test/sets', 1, state);

		% source processes
		state = [];
		state.data = 0.3:0.3:3;
		state.repeat = true;
		first = [5 8];
		for s = 1:9
			nm = ['src' int2str(s)];
			sys = sys.addprocess(nm, 'std/2009/source/numeric', 1, state);
			if s < first(1)
				sys = sys.link([nm '>out'], 'sets<<double', 0);
			elseif s < first(2)
				sys = sys.link([nm '>out'], 'sets<<round', 0);
			else
				sys = sys.link([nm '>out'], 'sets<<invert', 0);
			end
		end

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 100;
		exe.precision = [];

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);

		% plot
		if ~opt.suppressoutput
			figure(1)
			clf
			hold on
			N = length(state.data);
			t = 1:N;
			plot(t, state.data, 'k:')
			plot(t, out.sets.double.a0(1:N), 'r')
			plot(t, out.sets.round.a5(1:N), 'g')
			plot(t, out.sets.invert.a8(1:N), 'b')
		end



		%% PAUSE

	case 'pause'

		sys = sml_system;
		sub = sml_system;

		% pars
		fS = 13;

		% here, we illustrate "pause & continue" processing,
		% by completing a 2s execution in two 1s chunks. first,
		% we compute the whole 2s as one lump, so we know what
		% we expect. then we compute the 2s pieces, and check
		% that the results are the same.

		% source process
		state = [];
		state.data = 1:7;
		state.repeat = true;
		cls = 'std/2009/source/numeric';
		sub = sub.addprocess('src1', cls, fS, state);

		% add subsystem
		sys = sys.addsubsystem('sub', sub);

		% source process
		state = [];
		state.data = (1:3)+7;
		state.repeat = true;
		cls = 'std/2009/source/numeric';
		sys = sys.addprocess('src2', cls, fS, state);

		% source process
		state = [];
		state.data = (1:11)+10;
		state.repeat = true;
		cls = 'std/2009/source/numeric';
		sys = sys.addprocess('src3', cls, fS, state);

		% sum process
		state = [];
		cls = 'std/2009/math/esum';
		sys = sys.addprocess('sum', cls, fS, state);

		% link
		sys = sys.link('sub/src1>out', 'sum', 3);
		sys = sys.link('src2>out', 'sum', 7);
		sys = sys.link('src3>out', 'sum', 0);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;

		% execute two seconds, to see what it *should* do
		exe.stop = 2;
		t = (((1:2*fS)-1)/fS);
		out = execsys(sys, exe, opt);

		% execute one second, asking for an output file
		exe.stop = 1;
		exe.name = [test '1'];
		opt.test = [test '1'];
		t1 = (((1:fS)-1)/fS);
		[out1, rep1, exp1, sys1] = execsys(sys, exe, opt);

		% execute a second time
		exe.stop = 2;
		exe.name = [test '2'];
		opt.test = [test '2'];
		t2 = (((1:fS)-1)/fS)+1;
		[out2, rep2, inv2, sys2] = execsys(sys1, exe, opt);

		% display
		if ~opt.suppressoutput
			figure(1)
			clf
			hold on
			plot(t, [out.sub.src1.out; out.src2.out; out.src3.out]', 'k:')
			plot(t, [out.sum.out]', 'k-')
			plot(t1, [out1.sub.src1.out; out1.src2.out; out1.src3.out]', 'b:', 'linewidth', 2)
			plot(t1, [out1.sum.out]', 'b-', 'linewidth', 2)
			plot(t2, [out2.sub.src1.out; out2.src2.out; out2.src3.out]', 'r:', 'linewidth', 2)
			plot(t2, [out2.sum.out]', 'r-', 'linewidth', 2)
		end



		%% CHANNELS

	case 'channels'

		sys = sml_system;
		randn('state', 1);

		% pars
		fS = 1000;
		t_stop = 10;

		% different size channels
		elements = [1 10 100 1000];

		% add components
		for c = 1:length(elements)

			e = elements(c);

			state = [];
			state.data = randn(e, fS);
			state.repeat = true;
			src = ['src' int2str(e)];
			sys = sys.addprocess(src, 'std/2009/source/numeric', fS, state);

			% 			% src faster than dst...
			% 			state = [];
			% 			state.sleep = true;
			% 			state.msec = 10;
			% 			dst = ['dst' int2str(e)];
			% 			sys = sys.addprocess(dst, 'client/brahms/test/busy', fS, state);

			% dst faster than src...
			state = [];
			dst = ['dst' int2str(e)];
			sys = sys.addprocess(dst, 'std/2009/math/esum', fS, state);

			sys = sys.link([src '>out'], dst, 1);

		end

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;
		exe = exe.log('dst10'); % just log one (small) channel for validation

		% make sure threading is off
		opt.opts{end+1} = '--nothreads';

		% execute
		if brahms_utils('IsOctave')
			% can't validate, since randn()s are different
			opt.validate = false;
		end
		[out, rep, expected] = execsys(sys, exe, opt);



		%% BUFFERS

	case 'buffers'

		sys = sml_system;
		randn('state', 1);

		% pars
		fS = 200;
		lag = 50;
		t_stop = 1;

		% add process 1b (to slow down process 1)
		state = [];
		state.sleep = 1;
		state.msec = 10;
		sys = sys.addprocess('proc1', 'client/brahms/test/busy', fS, state);

		% add process 2
		state = [];
		state.sleep = 1;
		state.msec = 1;
		sys = sys.addprocess('proc2', 'client/brahms/test/busy', fS, state);

		% add process 3
		state = [];
		state.sleep = 1;
		state.msec = 1;
		sys = sys.addprocess('proc3', 'client/brahms/test/busy', fS, state);

		% links
		sys = sys.link('proc1>out', 'proc2', lag);
		sys = sys.link('proc2>out', 'proc3', lag);

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;

		% opts
		opt.voicecount = 3;
		opt.validate = 0;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% HANG

	case 'hang'

		% test BRAHMS can throw/catch an exception correctly
		sys = sml_system;

		state = [];
		state.data = magic(3);
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', 10, state);

		state = [];
		state.event = 'EVENT_RUN_RESUME';
		state.signals = false;
		sys = sys.addprocess('hang', 'client/brahms/test/hang', 1, state);

		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 100;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% EXCEPT

	case 'except'

		% test BRAHMS can throw/catch an exception correctly
		sys = sml_system;

		state = [];
		state.data = magic(3);
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', 10, state);

		state = [];
		state.event = 'EVENT_RUN_SERVICE';
		state.time = 0.4;
		sys = sys.addprocess('except2', 'client/brahms/test/except', 10, state);

		% other instances don't throw
		state.event = '';
		sys = sys.addprocess('except1', 'client/brahms/test/except', 10, state);
		sys = sys.addprocess('except3', 'client/brahms/test/except', 10, state);
		sys = sys.addprocess('except4', 'client/brahms/test/except', 10, state);
		sys = sys.addprocess('except5', 'client/brahms/test/except', 10, state);
		sys = sys.addprocess('except6', 'client/brahms/test/except', 10, state);
		sys = sys.addprocess('except7', 'client/brahms/test/except', 10, state);

		sys = sys.link('src>out', 'except1');
		sys = sys.link('src>out', 'except2');
		sys = sys.link('src>out', 'except3');
		sys = sys.link('src>out', 'except4');
		sys = sys.link('src>out', 'except5');
		sys = sys.link('src>out', 'except6');
		sys = sys.link('src>out', 'except7');

		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 3;
		opt.voicecount = 3;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% ABORT

	case 'abort'

		% test BRAHMS bindings handle an abort correctly
		sys = sml_system;

		state = [];
		state.data = magic(3);
		state.ndims = 1;
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', 10, state);

		state = [];
		state.event = 'EVENT_RUN_SERVICE';
		state.time = 0.4;
		sys = sys.addprocess('abort2', 'client/brahms/test/abort', 10, state);

		% other instances don't throw
		state.event = '';
		sys = sys.addprocess('abort1', 'client/brahms/test/abort', 10, state);
		sys = sys.addprocess('abort3', 'client/brahms/test/abort', 10, state);
		sys = sys.addprocess('abort4', 'client/brahms/test/abort', 10, state);
		sys = sys.addprocess('abort5', 'client/brahms/test/abort', 10, state);
		sys = sys.addprocess('abort6', 'client/brahms/test/abort', 10, state);
		sys = sys.addprocess('abort7', 'client/brahms/test/abort', 10, state);

		sys = sys.link('src>out', 'abort1');
		sys = sys.link('src>out', 'abort2');
		sys = sys.link('src>out', 'abort3');
		sys = sys.link('src>out', 'abort4');
		sys = sys.link('src>out', 'abort5');
		sys = sys.link('src>out', 'abort6');
		sys = sys.link('src>out', 'abort7');

		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 3;
		opt.voicecount = 3;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% DEADLOCK

	case 'deadlock'

		sys = sml_system;

		% add some spurious processes we won't use, to show how
		% their outputs are listed on E_DEADLOCK
		state = [];
		state.data = zeros(1,200);
		sys = sys.addprocess('src1', 'std/2009/source/numeric', 10, state);
		sys = sys.addprocess('src2', 'std/2009/source/numeric', 10, state);

		% if the state is specified explicitly, then deadlock is
		% avoided...
		state = [];

		% but, comment any single line to generate deadlock...
		state.type = 'INT32';
		% 		state.dims = [1 2 3];
		state.complex = false;
		% but, comment any single line to generate deadlock...

		sys = sys.addprocess('sum', 'std/2009/math/esum', 10, state);
		sys = sys.link('sum>out', 'sum');
		exe = brahms_execution;
		exe.name = test;
		exe.all = true;
		exe.stop = 1;

		if ~opt.suppressoutput
			disp(['Output from test script...'])
			sys
			disp([10 'Output from executable...'])
		end

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% MATFLICT
		%
		% this process illustrates how two matlab processes will
		% conflict if run in the same engine. in this case, they
		% both use the same RNG (rand()), and take alternate
		% values out of it.

	case 'matflict'
		
		disp(['matflict: expected random numbers (left) get doled out to processes (middle and right) in order, since processes are forced to run in the same thread']);
		disp(['matflict.s: expected random numbers get doled out to processes in unpredictable order, since processes are running asynchronously']);

		sys = sml_system;

		% process
		state = [];
		sys = sys.addprocess('mat1', 'client/brahms/test/rand', 10, state);

		% process
		state = [];
		sys = sys.addprocess('mat2', 'client/brahms/test/rand', 10, state);

		% exe
		exe = brahms_execution;
		exe.name = test;
		exe.all = true;
		exe.stop = 1;

		% execute
		opt.validate = false;
		[out, rep, expected] = execsys(sys, exe, opt);

		if ~opt.suppressoutput
			rand('state', 1);
			expected = rand(1, 10);
			[expected' out.mat1.out' out.mat2.out']
		end




		%% LANGUAGE
		%
		% can be called with one argument, 'm', 'c', '+', 'p' to run
		% just one of the languages, or with no arguments to run
		% them all. each language computes the same operation, and
		% uses the util/rng to generate identical noise.

	case 'language'

		% check args
		if length(opt.args)
			arg = opt.args{1};
		else
			arg = 'pmc+';
		end

		% empty system
		sys = sml_system;
		fS = 25;

		% add source data process
		state = [];
		state.data = [10 0 0 0 0];
		state.repeat = true;
		sys = sys.addprocess('src', 'std/2009/source/numeric', fS, state);

		% common state for all three processes
		state = [];
		state.count = 6;
		state.noise = 2;
		state.suppress_output = uint32(opt.suppressoutput);
		seed = [42];

		% add process
		if any(arg == '+')
			sys = sys.addprocess('processcpp', 'client/brahms/language/1199', fS, state, seed);
			sys = sys.link('src>out', 'processcpp');
		end

		% add process
		if any(arg == 'c')
			sys = sys.addprocess('processc', 'client/brahms/language/1266', fS, state, seed);
			sys = sys.link('src>out', 'processc');
		end

		% add process
		if any(arg == 'm')
			sys = sys.addprocess('processm', 'client/brahms/language/1258', fS, state, seed);
			sys = sys.link('src>out', 'processm');
		end

		% add process
		if any(arg == 'p')
			sys = sys.addprocess('processpy', 'client/brahms/language/1262', fS, state, seed);
			sys = sys.link('src>out', 'processpy');
		end

		% exe
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		opt.validate = false; % may not have selected all langs
		if strcmp(arg, 'pmc+') opt.validate = true; end
		[out, rep, expected] = execsys(sys, exe, opt);

		% plot results
		if ~opt.suppressoutput
			clf
			hold on
			if isfield(out, 'processcpp')
				plot(out.processcpp.out'+0, 'b-')
			end
			if isfield(out, 'processc')
				plot(out.processc.out'+10, 'g-')
			end
			if isfield(out, 'processm')
				plot(out.processm.out'+20, 'r-')
			end
			if isfield(out, 'processpy')
				plot(out.processpy.out'+30, 'm-')
			end
			hold off
		end



		%% IMAGE

	case 'image'

		sys = sml_system;

		% pars
		fS = 1000;
		interval = 1/1000;

		% src data
		N = 50;
		S = 167;
		[x, y] = meshgrid(-N:N);
		d = sqrt((x-N/2).^2 + y.^2) / N;
		e = exp(-d.^2);
		X = zeros(2*N+1, 2*N+1, S);
		for s = 1:S
			t = s / S * pi;
			a = sin(t);
			X(:, :, s) = a * e;
		end

		% src 1 (colormap, range -1 to +1)
		state = [];
		state.data = cat(3, X, -X);
		state.repeat = true;
		cls = 'std/2009/source/numeric';
		sys = sys.addprocess('src1', cls, fS, state);

		% src 2 (rgb)
		z = 0 * X;
		X = uint8(X * 255);
		z = uint8(z);
		r = permute(cat(3, X, z, z), [1 2 4 3]);
		g = permute(cat(3, z, X, z), [1 2 4 3]);
		b = permute(cat(3, z, z, X), [1 2 4 3]);
		rgb = cat(3, r, g, b);
		state = [];
		state.data = rgb;
		state.repeat = true;
		sys = sys.addprocess('src2', cls, fS, state);

		% add probes
		state = [];
		state.colormap = 'hot';
		state.range = [-1 1];
		state.interval = interval;
		cls = 'dev/std/image/numeric';

		% image
		state.location = [4 4 2 2];
		sys = sys.addprocess('image1', cls, fS, state);
		sys = sys.link('src1>out', 'image1');

		% image (polar map, default colors)
		state.location = [4 4 2 3];
		state.colormap = 'bilin';
		sys = sys.addprocess('image2', cls, fS, state);
		sys = sys.link('src1>out', 'image2');

		% image (lin map from black to white)
		state.colormap = 'lin';
		state.colors = {[0 0 0] [1 1 1]};
		state.location = [4 4 3 2];
		state.show.time = false;
		sys = sys.addprocess('image3', cls, fS, state);
		sys = sys.link('src1>out', 'image3');

		% image
		state.location = [4 4 3 3];
		sys = sys.addprocess('image4', cls, fS, state);
		sys = sys.link('src2>out', 'image4');

		% execution
		exe = brahms_execution;
		exe.all = false;
		exe.name = test;
		exe.stop = 1;
		exe.execPars.MaxThreadCount = 1;
		exe.execPars.ShowGUI = false;

		% not logging, so no validate
		opt.validate = false;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% VIDEO

	case 'video'

		% filename
		if ~isempty(opt.args)
			filename = opt.args{1};
		else
			filename = [getenv('SYSTEMML_INSTALL_PATH') '/BRAHMS/media/scratchbot.avi'];
		end

		if ~exist(filename, 'file')
			error(['usage: brahms_test video <filename.avi>']);
		end

		% src data
		info = aviinfo(filename);
		fS = round(info.FramesPerSecond);
		T = 1 / fS;
		avi = aviread(filename);
		data = avi(1).cdata;
		for n = length(avi):-1:2
			data(:,:,:,n) = avi(n).cdata;
		end

		sys = sml_system;

		% src
		state = [];
		state.data = data;
		state.repeat = false;
		cls = 'std/2009/source/numeric';
		sys = sys.addprocess('src', cls, fS, state);

		% image
		state = [];
		cls = 'dev/std/image/numeric';
		state.location = [3 3 2 2];
		sys = sys.addprocess('image', cls, fS, state);

		% busy
		state = [];
		state.msec = uint32(T*1000);
		state.sleep = true;
		sys = sys.addprocess('busy', 'client/brahms/test/busy', fS, state);

		% link
		sys = sys.link('src>out', 'image');
		sys = sys.link('src>out', 'busy');

		% execution
		exe = brahms_execution;
		exe.all = false;
		exe.name = test;
		exe.stop = info.NumFrames / fS;
		% 		exe.execPars.ShowGUI = false;

		% not logging, so no validate
		opt.validate = false;

		% execute
		[out, rep, expected] = execsys(sys, exe, opt);



		%% CAT

	case 'cat'

		sys = sml_system;

		% pars
		fS = 1000;

		% cat block
		state = [];
		state.dim = 2;
		cls = 'dev/std/cat/numeric';
		sys = sys.addprocess('cat', cls, fS, state);

		% src data
		sz = [2 3 4];
		for n = 1:3
			state = [];
			state.data = n^2 * ones(4, sz(n), 2, 100);
			state.repeat = true;

			name = ['src' int2str(n)];
			cls = 'std/2009/source/numeric';
			sys = sys.addprocess(name, cls, fS, state);
			sys = sys.link([name '>out'], 'cat', 0);
		end

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		opt.validate = false;
		[out, rep, expected] = execsys(sys, exe, opt);

		% display
		if ~opt.suppressoutput
			disp([10 'Output dimensions are:'])
			disp(size(out.cat.out))
			out.cat.out(:,:,:,1)
		end



		%% INDEX

	case 'index'

		sys = sml_system;

		% pars
		fS = 3;

		% index block (note we squeeze dim 2)
		state = [];
		state.index = {[2 5], {[3]}, [2 3 4]};
		cls = 'dev/std/index/numeric';
		sys = sys.addprocess('index', cls, fS, state);

		% src data
		sz = [5 6 7];
		ne = prod(sz);
		data = 1:ne;
		data = reshape(data, sz);

		% mark data we're interested in
		data = data - 0.1;
		data([2 5], [3], [2 3 4]) = data([2 5], [3], [2 3 4]) + 0.2;

		% add src process
		state = [];
		state.data = single(data);
		state.repeat = true;
		state.ndims = length(sz);
		cls = 'std/2009/source/numeric';
		sys = sys.addprocess('src', cls, fS, state);
		sys = sys.link('src>out', 'index', 0);

		% execution
		exe = brahms_execution;
		exe.all = true;
		exe.name = test;
		exe.stop = 1;

		% execute
		opt.validate = false;
		[out, rep, expected] = execsys(sys, exe, opt);

		% display
		if ~opt.suppressoutput
			disp([10 'Output dimensions are:'])
			size(out.index.out)
			disp([10 'Output is:'])
			out.index.out
		end



		%% HUGELOGS

	case 'hugelogs'

		sys = sml_system;
		randn('state', 1);

		% pars
		fS = 10000;
		t_stop = 1;
		N1 = 1300;
		N2 = 2000;
		[x, y, z] = meshgrid(1:10);
		xyz = max(max(x, y), z);
		A = 1:100;
		B = reshape(A, 10, 10);
		C = complex(B, B');

		% add component
		state = [];
		state.data = randn(10,5,7);
		state.repeat = true;
		sys = sys.addprocess('src1', 'std/2009/source/numeric', fS, state);

		% add component
		state = [];
		state.data = B;
		state.repeat = true;
		sys = sys.addprocess('src2', 'std/2009/source/numeric', fS, state);

		% add component
		state = [];
		state.data = C;
		state.repeat = true;
		sys = sys.addprocess('src3', 'std/2009/source/numeric', fS, state);

		% subsys
		sub = sml_system;

		% add component
		state = [];
		state.data = repmat(1:42, N2, 1);
		state.repeat = true;
		sub = sub.addprocess('src1', 'std/2009/source/numeric', fS, state);
		sub = sub.addprocess('src2', 'std/2009/source/numeric', fS, state);

		% add
		sys = sys.addsubsystem('sub', sub);

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;
		exe.all = true;

		% execute
		[out, rep] = brahms(sys, exe);



		%% LISTENED

	case 'listened'

		sys = sml_system;

		% pars
		N = 10;
		fS = 10000;
		t_stop = 1;

		% add component
		state = [];
		state.publishAnyway = false;
		state.N = N;
		sys = sys.addprocess('listened', 'client/brahms/test/listened', fS, state);

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;
		exe.execPars.BufferingPolicy = 'OnlyMemory';
		% 	exe.all = true;

		% listen to both, takes twice as long (twice as much
		% computation is done)
		exe = exe.log('listened>A');
		% 	exe = exe.log('listened>B');

		% execute
		[out, rep] = brahms(sys, exe);



		%% EVAL

	case 'eval'

		sys = sml_system;

		% pars
		fS = 1000;
		t_stop = 2;
		t = (1:(fS*t_stop))/fS;
		randn('state', 1);

		% add component
		state = [];
		state.data = t;
		state.repeat = true;
		sys = sys.addprocess('t', 'std/2009/source/numeric', fS, state);

		% add component
		state = [];
		state.data = exp(t*i*2*pi*10);
		state.repeat = true;
		sys = sys.addprocess('s', 'std/2009/source/numeric', fS, state);

		% add component
		state = [];
		state.function = 'real($s)+3*exp(-$t)';
		sys = sys.addprocess('eval', 'dev/std/eval/python', fS, state);

		% links
		sys = sys.link('t>out', 'eval<t', 0);
		sys = sys.link('s>out', 'eval<s', 0);

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;
		exe.all = true;

		% execute
		[out, rep] = brahms(sys, exe);
		
		% plot
		if ~opt.suppressoutput
			figure(1)
			clf
			plot(out.t.out, out.eval.out);
		end



		%% COMPLEX

	case 'complex'

		sys = sml_system;

		% pars
		fS = 10;
		t_stop = 1;

		% add component
		state = [];
		sys = sys.addprocess('complex', 'client/brahms/test/complex', fS, state);
		
		% add sinks
		state = [];
		state.interleaved = true;
		sys = sys.addprocess('snkai', 'client/brahms/test/complexr', fS, state);
		sys = sys.link('complex>adjacent', 'snkai', 0);

		% add sinks
		state = [];
		state.interleaved = false;
		sys = sys.addprocess('snkaa', 'client/brahms/test/complexr', fS, state);
		sys = sys.link('complex>adjacent', 'snkaa', 0);

		% add sinks
		state = [];
		state.interleaved = true;
		sys = sys.addprocess('snkii', 'client/brahms/test/complexr', fS, state);
		sys = sys.link('complex>interleaved', 'snkii', 0);

		% add sinks
		state = [];
		state.interleaved = false;
		sys = sys.addprocess('snkia', 'client/brahms/test/complexr', fS, state);
		sys = sys.link('complex>interleaved', 'snkia', 0);
		
		% add python (which uses interleaved)
		state = [];
		state.data = complex([1 2 3], [4 5 6]);
		sys = sys.addprocess('py1', 'client/brahms/test/complexp', fS, state);
		sys = sys.link('complex>adjacent', 'py1', 0);

		% add python (which uses interleaved)
		state = [];
		state.data = complex([1 2 3], [4 5 6]);
		sys = sys.addprocess('py2', 'client/brahms/test/complexp', fS, state);
		sys = sys.link('py1>out', 'py2', 0);

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t_stop;
		exe.all = true;

		% execute
		opt.validate = false;
		[out, rep] = execsys(sys, exe, opt);
		
		% check
		a = out.complex.adjacent(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.complex.interleaved(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.snkaa.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.snkia.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.snkai.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.snkii.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.py1.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a = out.py2.out(:,:,end);
		disp([real(a(:)); imag(a(:))]')
		a
		

		%% EXAMPLES

	case 'examples'

		no_exec = true;

		path = [getenv('SYSTEMML_INSTALL_PATH') '/BRAHMS/support/example/995'];
		c = pwd;

		try
			cd(path)
			rabbitsandfoxes('1258')
			pause(1)
			rabbitsandfoxes('1262')
			pause(1)
			brainandhead('1258')
			pause(1)
			brainandhead('1262')
			pause(1)
		catch
			cd(c);
			err = lasterror;
			rethrow(err)
		end
		cd(c);



		%% GUI

	case 'gui'

		if brahms_utils('IsOctave')
			error('Invocation Bindings GUI not currently supported under Octave');
		end


		% here, we exercise the various aspects of the GUI
		sys = sml_system;

		% arg can be "q", in which case we don't log
		if length(opt.args) && strcmp(opt.args{1}, 'q')
			quick = true;
		else
			quick = false;
		end

		% parameters;
		if opt.multiprocess
			Ns = 1e3;
			opt.voicecount = 4;
		else
			Ns = 1e4;
		end
		Np = 256;
		Nt = 1;
		if quick
			Np = 64;
		end

		% process
		state = [];
		sys = sys.addprocess('sum', 'std/2009/math/esum', 1, state);

		% processes
		state = [];
		state.data = zeros(1, 2);
		state.ndims = 1;
		state.repeat = true;
		brahms_gui('phase', 'Building System...');
		callback = brahms_gui('get_callback_progress');
		for n = 1:Np
			nm = ['src' int2str(n)];
			sys = sys.addprocess(nm, 'std/2009/source/numeric', 1, state);
			sys = sys.link([nm '>out'], 'sum');
			callback([n/Np 0]);
		end

		% execution
		exe = brahms_execution;
		exe.all = ~quick;
		exe.name = test;
		exe.stop = Ns;
		exe.execPars.MaxThreadCount = Nt;
		exe.execPars.BufferingPolicy = 'OnlyMemory';

		% execute
		opt.validate = false;
		[out, rep] = execsys(sys, exe, opt);



		%% BUSY

	case 'busy'

		sys = sml_system;

		sleep = false;
		t = 60;

		for a = 1:length(opt.args)
			arg = opt.args{a};
			switch arg
				case 'sleep'
					sleep = true;
				otherwise
					error(['did not understand "' arg '"']);
			end
		end

		% processes
		state = [];
		state.msec = 1000;
		state.sleep = sleep;
		for p = 1:8
			sys = sys.addprocess(['busy' int2str(p)], 'client/brahms/test/busy', 1, state);
		end

		% execution
		exe = brahms_execution;
		exe.name = test;
		exe.stop = t;

		% execute
		opt.validate = false;
		[out, rep] = execsys(sys, exe, opt);



		%% SWITCH ON TEST

	otherwise

		error('incorrect usage - see help "brahms_test" ');

end






if nargout >= 1
	out_ = out;
end

if nargout >= 2
	rep_ = rep;
end











%% EXECUTE SYSTEM

function [out, rep, expected, sys] = execsys(sys, exe, opt)

% multiprocessing
if opt.multiprocess
	if opt.usempi
		protocol = 'mpi';
	else
		protocol = 'sockets';
	end
	v = opt.voicecount;
	if opt.voicecountoverride
		v = opt.voicecountoverride;
	end
	exe.addresses = {protocol v};
end

% tmult
exe.stop = opt.tmult * exe.stop;

% compression
exe.execPars.IntervoiceCompression = opt.compression;

% nologs
if opt.nologs
	exe.all = false;
end

% execute
if opt.dontrun
	out = brahms('prep', sys, exe, opt.opts{:});
	return
else
	if nargout > 3
		[out, rep, inv, sys] = brahms(sys, exe, opt.opts{:});
	else
		[out, rep] = brahms(sys, exe, opt.opts{:});
	end
end

% % soak check
% if opt.soakinstance && ischar(out)
% 	out_ = out;
% 	return
% end

% validate
if opt.validate

	% normalise
	out_s = struct(out);

	% uncomment this to save the new expected data
	if opt.saveexpected
		if strcmp('Yes', questdlg(['Are you sure you want to overwrite the results for test "' opt.test '"?'], 'Confirm', 'Yes', 'No', 'No'))
			save_expected(opt.test, out_s);
		end
	end

	% expected results
	if isfield(opt, 'expected')
		expected = opt.expected;
	else
		expected = load_expected(opt.test);
	end

	% no check
	if isfield(opt, 'nocheck')

		expected_ = expected;

		for f = 1:length(opt.nocheck)
			out_s = rmfield(out_s, opt.nocheck{f});
			expected_ = rmfield(expected_, opt.nocheck{f});
		end

		c = checksame(out_s, expected_);

	else

		c = checksame(out_s, expected);

	end

	% check (or save) results
	disp([int2str(c) ' result fields validated OK']);

else

	expected = [];

end





















function s = similar(a, b)

% return true if arrays are similar (within computational
% error)
s = false;

sa = size(a);
sb = size(b);
if length(sa) ~= length(sb)
	disp(['error in dimension'])
	return
end
if any(sa ~= sb)
	disp(['error in dimension'])
	return
end
if ~strcmp(class(a), class(b))
	disp(['error in class (expected "' class(b) '", got "' class(a) '")'])
	return
end
if isreal(a) ~= isreal(b)
	disp(['error in complexity'])
	return
end

% handle complex case
if ~isreal(a)
	if ~similar(real(a), real(b)) return; end
	if ~similar(imag(a), imag(b)) return; end
end


d = mmax(abs(double(a) - double(b)));
m = max([mmax(a)-mmin(a), mmax(b)-mmin(b), 1]);
e = 1e-6;
rd = d / m;
if rd > e
	disp(['relative error was ' num2str(rd)])
	return
end
s = true;




function c = checksame(a, b, pt)

c = 0;

if nargin < 3
	pt = '';
end

% check each field in a is similar in b
fn = fieldnames(a);
for n = 1:length(fn)
	fa = a.(fn{n});
	fb = b.(fn{n});
	if isstruct(fa)
		c = c + checksame(fa, fb, [fn{n} '.']);
	else
		if ~similar(fa, fb)
			error(['computation incorrect at "' pt fn{n} '"']);
		end
		c = c + 1;
	end
end

% check each field in b exists in a
fn = fieldnames(b);
for n = 1:length(fn)
	if ~isfield(a, fn{n})
		error(['field "' fn{n} '" missing in output']);
	end
end





function expected = load_expected(test)

path = [fileparts(which(mfilename)) filesep mfilename '.expected'];
expected = [];

% different load syntax
if brahms_utils('IsOctave')
	load(path)
else
	load(path, '-mat')
end

if length(test) > 2 && strcmp(test(end-1:end), '.s')
	test = test(1:end-2);
end
if length(test) > 2 && strcmp(test(end-1:end), '.m')
	test = test(1:end-2);
end
expected = expected.(test);

% octave can't recognise data saved as "single" in .mat
% files
if strcmp(test, 'resamp')
	if brahms_utils('IsOctave')
		expected.src.out = single(expected.src.out);
		expected.resamp.out = single(expected.resamp.out);
	end
end



function save_expected(test, out)

disp(['saving expected results for test "' test '"...']);

path = [fileparts(which(mfilename)) filesep mfilename '.expected'];
expected = [];
load(path, '-mat')
if length(test) > 2 && strcmp(test(end-1:end), '.s')
	error('save using solo - not concerto!')
end
expected.(test) = out;
save(path, 'expected')





function m = mmax(m)

while ~isscalar(m)
	if isreal(m)
		m = max(m);
	else
		m = max(max(real(m)), max(imag(m)));
	end
end

m = double(m);


function m = mmin(m)

while ~isscalar(m)
	if isreal(m)
		m = min(m);
	else
		m = min(min(real(m)), min(imag(m)));
	end
end

m = double(m);





function y = sawtooth(t,width)

rt = rem(t,2*pi)*(1/2/pi);
i1 = find( ((rt<=width)&(rt>=0)) | ((rt<width-1)&(rt<0)) );
i2 = 1:length(t(:));
i2(i1) = [];      % complement set
y = zeros(size(t));

y(i1) = ( ((t(i1)<0)&(rt(i1)~=0)) + rt(i1) - .5*width)*2;
if (width ~= 0),
	y(i1) = y(i1)*(1/width);
end
y(i2) = ( -(t(i2)<0) - rt(i2) + 1 - .5*(1-width))*2;
if (width ~= 1),
	y(i2) = y(i2)*(1/(1-width));
end





function s = square(t,duty)

tmp = mod(t,2*pi);
w0 = 2*pi*duty/100;
nodd = (tmp < w0);
s = 2*nodd-1;




function l = addchar(l, c)

for i = 1:length(l)
	l{i} = [l{i} '.' c];
end

