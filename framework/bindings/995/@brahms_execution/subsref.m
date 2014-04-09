%
% brahms_execution::subsref
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
% $Id:: subsref.m 2447 2010-01-13 01:10:52Z benjmitch                    $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%


function out = subsref(exe, ref)




switch ref(1).type
	
	case '.'

		fieldName = ref(1).subs;
		switch(fieldName)
			case {'stop'}
				fieldName = 'executionStop';
			case {'execPars'}
				fieldName = 'executionParameters';
		end
		
		switch fieldName
			
			case 'serialize'
				if length(ref) < 2 | ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = serialize(exe, ref(2).subs{:});
				return;

			case 'log'
				if length(ref) < 2 | ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = log(exe, ref(2).subs{:});
				return;
				
			case {'logs'}
				if length(ref) > 1
					out = subsref(exe.logs, ref(2:end));
				else
					out = exe.logs;
				end
				return

			case {'stop'}
				exe = defaults(exe, true);
				out = exe.executionStop;
				return
				
			case {'addresses' 'affinity' 'executionStop' ...
					'precision' 'window' 'seed' ...
					'writeFilesTo' ...
					'workingDirectory' ...
					'all' 'encapsulated' 'launch'}
				exe = defaults(exe);
				if length(ref) > 1
					out = subsref(exe.(fieldName), ref(2:end));
				else
					out = exe.(fieldName);
				end
				return
				
			case {'executionParameters'}
				exe = defaults(exe);
				pars = brahms_utils('GetExecutionParameters');
				f = fieldnames(exe.executionParameters);
				for n = 1:length(f)
					key = f{n};
					val = exe.executionParameters.(key);
					pars.(key) = val;
				end
				if length(ref) > 1
					out = subsref(pars, ref(2:end));
				else
					out = pars;
				end
				return
				
			case 'voices'
				exe = defaults(exe);
				out = length(exe.addresses);
				if ~out
					% solo
					out = 1;
				end
				return

			case 'isconcerto'
				out = isempty(exe.addresses);
				return

			case {'systemFileIn' 'systemFileOut' 'executionFile' ...
					'reportFile' 'outFile' 'logFile' 'exitFile'}
				exe = defaults(exe, true);
				if length(ref) > 1
					out = subsref(exe.meta.(fieldName), ref(2:end));
				else
					out = exe.meta.(fieldName);
				end
				return

			case 'write'
				% return the write structure used by the SystemML
				% Matlab Bindings serialize() function
				out = [];
				out.precision = exe.logs.precision;
				out.encapsulated = exe.logs.encapsulated;
				return;
				
		end
		
		error(['field not found "' fieldName '"']);

	otherwise
		error('invalid reference')
		
end
