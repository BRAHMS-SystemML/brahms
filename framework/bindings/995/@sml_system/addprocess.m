%
% sys = sys.addprocess(name, class, rate[, state[, seed]])
%
%   Add a new process to the system, giving it the specified
%   name, SystemML class, sample rate, initial state and
%   seed. If the process class you are adding has no state,
%   state may be omitted. If the process does not need a
%   seed, seed can be omitted.
%
%
% __________________________________________________________
% This function is part of the "SystemML Toolbox".


%__________________________________________________________________________
%
% This file is part of the SystemML Toolbox
% Copyright (C) 2007 Ben Mitch(inson)
% URL: http://sourceforge.net/projects/systemml
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
% $Id:: addprocess.m 2397 2009-11-18 21:20:07Z benjmitch                 $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = addprocess(sys, name, class, rate, state, seed)

validatename(sys, name);

if ~ischar(class)
	error(['class must be a string']);
end

if isnumeric(rate) && isscalar(rate) && (round(rate) == rate)
	% ok
elseif ischar(rate)
	% ok
else
	error('sample rate should be a scalar integer or a fraction string');
end

proc = [];
proc.name = name;
proc.class = class;
proc.time.sampleRate = rate;

% state is optional and can be any type
if exist('state', 'var')
	proc.state = state;
else
	proc.state = [];
end

% seed is optional and should be uint32 or double (converted to
% uint32)
if exist('seed', 'var')
	sz = size(seed);
	if ~isnumeric(seed) || length(sz) ~= 2 || sz(1) ~= 1
		error('invalid seed (must be row vector of UINT32 elements)');
	end
	proc.seed = uint32(seed);
	if any(seed ~= proc.seed)
		error('invalid seed (must be row vector of UINT32 elements)');
	end
	if ~isempty(proc.seed)
		if any(proc.seed == 0)
			error('all elements of specified seed must be non-zero');
		end
	end
end

N = length(sys.proc);
sys.proc{N+1} = proc;
sys.namesinuse.(name) = 'process';



