
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
% $Id:: tutorial_3.m 1751 2009-04-01 23:36:40Z benjmitch                 $
% $Rev:: 1751                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)                   $
%__________________________________________________________________________
%


% Tutorial 3 : Resampling data
%
% By the end of this tutorial you should understand the
% following concepts:
%
%   * BRAHMS processes (and links) can have different sample 
%   rates
%   * some processes must have all their inputs match their
%   process sample rate (these are processes that set the
%   flag F_INPUTS_SAME_RATE)
%   * std/resample/numeric converts inputs at one sample
%   rate to outputs at another sample rate to help with
%   connecting together parts of a system running at
%   different rates
%   * how to use the std/resample/numeric library process
%
% Extra credit tasks are listed at the end of this code

% HOW TO WORK THROUGH THIS TUTORIAL
%
%   * First, run the tutorial as it comes. Two processes are
%   created, and they create outputs at 3Hz and 7Hz.
%
%   * Next, change "include_summer" to true, and a sum
%   process will be added that tries to add the outputs of
%   these two processes. You will see an error message
%   indicating that you cannot plug an input into the sum
%   process that does not share the sum process's sample
%   rate, since the sum process is not a rate changer.
%
%   * Next, change "include_resampler" to true, and a
%   resample process will also be included, resampling the
%   output of src2 down from 7Hz to 3Hz, before passing this
%   resampled input to the sum process. The sum process is now
%   adding two 3Hz data streams, and runs fine.

% don't include a sum process (change this first)
include_summer = false;

% don't include a resampling process (then change this)
include_resampler = false;


% construct the system: start with an empty system
sys = sml_system;



%%%% PROCESS: NUMERIC SOURCE 1 RUNS AT 3HZ

% construct a process, a source of numeric data and add it to the
% system
state = [];
state.data = 42*ones(1,2,3);  % 1x2 matrix, 3 samples
state.repeat = false;
sys = sys.addprocess('src1', 'std/2009/source/numeric', 3, state);



%%%% PROCESS: NUMERIC SOURCE 2 RUNS AT 7HZ

% construct a process, a source of random numeric data
state.dims = uint32([1 2]); % dimension of output array
state.dist = 'uniform';     % type of distribution
state.pars = [2 8];         % values will range in 2 to 8
state.complex = false;

% add the new process to the system
sys = sys.addprocess('src2', 'std/2009/random/numeric', 7, state);




%%%% PROCESS: SUM ADDS OUTPUTS OF SOURCES 1 AND 2

if include_summer

	% construct a process, a sum process
	sys = sys.addprocess('sum', 'std/2009/math/esum', 3);
    
	% now link the two sources into it as inputs
	sys = sys.link('src1>out', 'sum');
	sys = sys.link('src2>out', 'sum');

	% this connects us to src1 at 3Hz, and src2 at 7Hz  - this won't run!
	
end

%%%% PROCESS: RESAMPLE SOURCE 2 DOWN TO 3HZ BEFORE PASSING
%%%% TO SUM

if include_resampler
	
	% construct a process, a resampler. the output of the
	% resample process is just its input resampled to the sample
	% rate of the process.
	state = [];
	state.order = 0; % 1 = linear interpolation, 0 = zero-order hold
    %% this parameter sets how the resampler outputs between input samples
    %% see Wikipedia entries for both
	sys = sys.addprocess('resampler', 'std/2009/resample/numeric', 3, state);

	% now link the 7Hz source to the input of the resample
	% process
	sys = sys.link('src2>out', 'resampler');

	% change the previously created link between src2 and the
	% sum process, so that the resampled output from resampler
	% is used instead
	sys = sys.unlink('src2>out', 'sum');
	sys = sys.link('resampler>out', 'sum');
	
end




%%%% CONSTRUCT AND RUN

% construct the simulation
exe = brahms_execution;
exe.stop = 1;     % stop the simulation after one second
exe.all = true;   % log all the outputs of the entire system

% execute the system
out = brahms(sys, exe);



% Examine the output
disp([repmat('_',1,60) 10])
disp('*** all outputs listed as usual')
disp('out')
disp(out)

% Examine the different sample rates of the outputs of src1
% and src2
disp([repmat('_',1,60) 10])
disp('*** src1/2 have different output sample rates')
disp('out.src1.out')
disp(out.src1.out)
disp('out.src2.out')
disp(out.src2.out)

% If summer not present, stop there
if ~include_summer return; end

% The output of the resampler
disp([repmat('_',1,60) 10])
disp('*** Where''s that resampled output gone?')
disp('out.resampler')
disp(out.resampler)
disp('out.resampler.out')
disp(out.resampler.out)

% This resampled output can now be added to the explicit
% numeric source by the summer. Note that, because this
% source has now gone through two links with unity lag, it
% doesn't start appearing in the output of the sum block
% until the third sample.
disp([repmat('_',1,60) 10])
disp('*** Now we can sum')
disp('out.sum.out')
disp(out.sum.out)

% FOR A COMPLETE UNDERSTANDING:
%
%   * Up-sampling: first download a new copy of this
%   tutorial code (or set everything back to its initial
%   state to test your memory). Change the sample rate of
%   the sum process to 7 Hz, and set "include_summer =
%   true". This will repeat the same error as before. Now
%   change the code to up-sample numeric source 1 to 7 Hz
%   (rather than down-sample numeric source 2 to 3 Hz) by:
%
%   (i) setting "include_resampler = true;"
%   (ii) changing the sample rate of the re-sampling process
%   to 7Hz
%   (iii) linking numeric source 1 to the re-sampling
%   process
%   (iv) unlinking numeric source 1 from the sum process
%
%   * Change all the lags in the system to zero, and see how
%   the propagation of data changes. linking a zero-lag system
%   into a loop would cause a deadlock at runtime, though!
%   See "help sml_system/link".
