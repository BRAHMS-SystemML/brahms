%
% brahms_execution::subsasgn
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
% $Id:: subsasgn.m 2447 2010-01-13 01:10:52Z benjmitch                   $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%

function exe = subsasgn(exe, ref, val)

switch ref(1).type
	
	case '.'
		
		logs = false;
		
		fieldName = ref(1).subs;
		switch(fieldName)
			case {'stop'}
				fieldName = 'executionStop';
			case {'execPars'}
				fieldName = 'executionParameters';
			case {'systemFile'}
				warning('use "systemFileIn" rather than "systemFile", usage will be deprecated in future');
				fieldName = 'systemFileIn';
			case {'ips'}
				warning('use "addresses" rather than "ips", usage will be deprecated in future');
				fieldName = 'addresses';
		end
		sz = size(val);
		
		switch fieldName
			
			case {'name' 'title' 'writeFilesTo' 'workingDirectory' 'systemFileIn' 'systemFileOut' 'executionFile' ...
					'reportFile' 'logFile'}
				if ~isempty(val)
					if ~ischar(val) || sz(1) ~= 1 || length(sz) ~= 2
						error('invalid value (value must be simple string)');
					end
				end
				
			case {'launch'}
				% launch can be:
				% - text (command line)
				% - text beginning "each" (command line)
				% - cell array of text entries (command lines, per voice)
				% - function handle
				if ~isempty(val)
					if isa(val, 'function_handle')
						if ~isscalar(val)
							error('invalid value');
						end
					elseif iscell(val)
						if length(sz) ~= 2 || all(sz ~= 1)
							error('invalid value');
						end
					else
						if ~ischar(val) || sz(1) ~= 1 || length(sz) ~= 2
							error('invalid value');
						end
					end
				end
				
			case {'all', 'encapsulated', 'recurse'}
				logs = true;
				if ~islogical(val) || ~isscalar(val) || ~isreal(val)
					error('invalid value (must be scalar boolean)');
				end
				
			case 'precision'
				logs = true;
				if ~isempty(val)
					if ~isnumeric(val) || ~isscalar(val) || ~isreal(val) || val < 0 || val ~= floor(val) || val > 32
						error('invalid value (precision must be between 0 and 32)');
					end
					val = uint16(val);
				end
				
			case 'seed'
				if ~isempty(val)
					if ~isnumeric(val) || ~isscalar(val) || ~isreal(val) || val < 1 || val ~= floor(val) || val >= 2^31
						error('invalid value (seed must be scalar between 1 and 0x7FFFFFFF)');
					end
					val = uint32(val);
				end
				
			case 'window'
				logs = true;
				if ~isempty(val)
					if ~isa(val, 'char') || sz(1) ~= 1
						error('invalid value (window must be a row-vector of non-negative times in seconds)');
					end
				end
				
			case 'addresses'
				if isempty(val)
					% ok, nothing specified
					val = {};
				elseif iscell(val) && length(sz)==2 && sz(1)==1 && sz(2)==2 && ischar(val{1}) && isnumeric(val{2})
					% ok, auto-address
					protocol = val{1};
					voices = val{2};
					if ~(strcmp(protocol, 'mpi') || strcmp(protocol, 'sockets'))
						error(['unrecognised protocol "' protocol '"']);
					end
					if voices < 1 || voices ~= floor(voices)
						error(['invalid voice count "' num2str(voices) '"']);
					end
					if strcmp(protocol, 'sockets')
						protocol = '127.0.0.1';
					end
					val = cell(1,voices);
					for n = 1:voices
						val{n} = protocol;
					end
				elseif iscell(val) && length(sz)==2 && (sz(1)==1 && sz(2)>=1) || (sz(1)>=1 && sz(2)==1)
					% ok, IP addresses
					for v = 1:length(val)
						if ~ischar(val{v}) || size(val{v},1)~=1 || length(find(val{v} == '.')) ~= 3
							error(['invalid IP address "' val{v} '"']);
						end
					end
				else
					error('invalid value');
				end
				if length(val) >= 1000
					error(['It takes a thousand voices to tell a single story. But not in BRAHMS.'])
				end
				
			case 'affinity'
				if isempty(val)
					% ok, nothing specified
					val = {};
				elseif iscell(val) && length(sz)==2 && (sz(1)==1 && sz(2)>=1) || (sz(1)>=1 && sz(2)==1)
					% ok
					for v = 1:length(val)
						c = val{v};
						sz = size(c);
						if ~iscell(c) || length(sz)~=2
							disp(c);
							error('invalid affinity group (above)');
						end
					end
				else
					error('invalid value');
				end
				
			case 'executionStop'
				if ~isa(val, 'double') || ~isscalar(val) || ~isreal(val) || val < 0
					error('invalid value (stop time must be non-negative scalar time in seconds)');
				end
				val = double(val);
				
			case 'executionParameters'
				if length(ref) ~= 2 || ~strcmp(ref(2).type, '.')
					error('invalid reference');
				end
				exe = setexecpar(exe, ref(2).subs, val);
				fieldName = '';
				
			otherwise
				error(['field not found "' ref(1).subs '"']);
				
		end
		
		if ~isempty(fieldName)
			if logs
				exe.logs.(fieldName) = val;
			else
				exe.(fieldName) = val;
			end
		end
		return

	otherwise
		error('invalid reference')
		
end
