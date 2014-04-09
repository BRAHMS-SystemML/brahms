%
% brahms_execution::log
%
% __________________________________________________________
% This function is part of the "BRAHMS Matlab Bindings".


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
% $Id:: log.m 2447 2010-01-13 01:10:52Z benjmitch                        $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%

function exe = log(exe, name, varargin)

if ~ischar(name)
	error(['name must be a string']);
end


% validate
v = name;
v = v(v < 97 | v > 122);
v = v(v < 65 | v > 90);
v = v(v < 48 | v > 57);
v = v(v ~= '>');
v = v(v ~= '/');
v = v(v ~= '_');

f = find(name == '>');
if length(v) || ~any(length(f) == [0 1 3])
	error('invalid log request');
end

log = [];
log.name = name;
log.precision = [];
log.encapsulated = [];
log.window = [];
log.recurse = [];

% interpret args
while length(varargin)

	arg = varargin{1};
	varargin = varargin(2:end);
	
	% legacy (string key and at least one more arg)
	if ischar(arg) && length(varargin)
		key = arg;
		val = varargin{1};
		switch key
			case 'precision'
				if isempty(val) || (isnumeric(val) && isscalar(val))
					warning('legacy usage of brahms_execution.log() - see documentation for new syntax');
					arg = {key val};
					varargin = varargin(2:end);
				end
			case {'toggle' 'window'}
				error('legacy usage of brahms_execution.log() - see documentation for new syntax');
			case 'encapsulated'
				if islogical(val) && isscalar(val)
					warning('legacy usage of brahms_execution.log() - see documentation for new syntax');
					arg = {key val};
					varargin = varargin(2:end);
				end
		end
	end
	
	% argument can be cell with key/value pair
	if iscell(arg)
		
		if prod(size(arg)) ~= 2
			error('cell argument must have two elements');
		end
		
		key = arg{1};
		val = arg{2};
		sz = size(val);
		
		if ~ischar(key)
			error('first element of cell argument must be char');
		end
		
		switch key
			
			case {'precision' 'prec'}
				if ~isempty(val)
					if ~isscalar(val) | ~isnumeric(val) | ~isreal(val) | val ~= floor(val) | val < 0 | val > 30
						error(['invalid value for "' key '"']);
					end
				end
				log.precision = val;
				
			case {'window'}
				if ~isa(val, 'char') | length(sz) ~= 2 | sz(1) ~= 1
					error(['invalid value for "' key '"']);
				end
				log.window = val;
				
			case {'encapsulated' 'encap'}
				if ~isscalar(val) | ~islogical(val)
					error(['invalid value for "' key '"']);
				end
				log.encapsulated = val;
				
			case {'recurse'}
				if ~isscalar(val) | ~islogical(val)
					error(['invalid value for "' key '"']);
				end
				log.recurse = val;
				
			otherwise
				error(['unrecognised argument "' key '"']);
			
		end
		
		% ok
		continue;
		
	end
		
	% argument can be flag
	if ischar(arg)
		
		sz = size(arg);
		if sz(1) ~= 1 || length(sz) ~= 2
			error('char argument must be simple string');
		end
		
		switch arg
			
			case {'encapsulated', 'encap'}
				log.encapsulated = true;
				
			case {'unencapsulated', 'unencap'}
				log.encapsulated = false;
				
			case {'recurse'}
				log.recurse = true;
				
			case {'norecurse'}
				log.recurse = false;
				
			otherwise
				error(['unrecognised argument "' arg '"']);
			
		end
		
		% ok
		continue;
		
	end
	
	% not recognised
	error(['argument unrecognised']);
	
end

N = length(exe.logs.specific);
exe.logs.specific{N+1} = log;

