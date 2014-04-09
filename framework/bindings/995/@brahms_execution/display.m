%
% brahms_execution::display
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
% $Id:: display.m 2447 2010-01-13 01:10:52Z benjmitch                    $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%

function display(exe)


exe = defaults(exe, 1);


disp(['--------------------------------'])
disp('  BRAHMS execution object');
disp(['--------------------------------'])
show('Name', exe.name);
show('Title', exe.title);
show('Precision', exe.logs.precision);
show('Window', exe.logs.window);
show('Encapsulated', exe.logs.encapsulated);
show('Recurse', exe.logs.recurse);
show('Log All', exe.logs.all);
show('Seed', exe.seed);
if isempty(exe.addresses)
	show('Addresses', 'Solo');
elseif isnumeric(exe.addresses)
	show('Addresses', ['MPI (' int2str(exe.addresses) ')']);
else
	show('Addresses', cell2str(exe.addresses));
end
if ~isempty(exe.affinity)
	for c = 1:length(exe.affinity)
		C = exe.affinity{c};
		items = {};
		voice = [];
		for p = 1:length(C)
			P = C{p};
			if ischar(P)
				items{end+1} = P;
				continue;
			elseif isnumeric(P) && isscalar(P)
				P = double(P);
				if P == floor(P) && P >= 0 && isempty(voice)
					voice = P;
					continue;
				end
			end				
			error(['invalid "affinity" data']);
		end
		if isempty(voice)
			disp(['  Affinity (Group ' int2str(c) '):']);
		else
			disp(['  Affinity (Group ' int2str(c) ', ==> Voice ' int2str(voice) '):']);
		end
		for p = 1:length(items)
			disp(['    ' items{p}])
		end
	end
else
	show('Affinity', '');
end
show('Execution Stop', exe.executionStop);
show('Write Files To', exe.writeFilesTo);
show('Working Directory', exe.workingDirectory);
show('Launch Line', exe.launch);
show('System File (In)', exe.systemFileIn);
show('System File (Out)', exe.systemFileOut);
show('Execution File', exe.executionFile);
show('Out File(s)', exe.meta.outFile);
if ~isempty(exe.logs)
	disp('  Logs:')
	for n = 1:length(exe.logs.specific)
		log = exe.logs.specific{n};
		line = ['    "' log.name '"'];
		if ~isempty(log.precision)
			line = [line ', precision=' int2str(log.precision)];
		end
		if ~isempty(log.window)
			line = [line ', window=''' log.window ''''];
		end
		if log.encapsulated
			line = [line ' (encapsulated)'];
		end
		if ~log.recurse
			line = [line ' (no recursion)'];
		end
		disp(line)
	end
end
disp(['--------------------------------'])



function show(name, var)

if isempty(var)
	svar = '';
elseif isnumeric(var)
	svar = num2str(var);
elseif ischar(var)
	svar = var;
elseif islogical(var)
	svar = 'true';
	if ~var
		svar = 'false';
	end
elseif isa(var, 'function_handle')
	svar = ['@' char(var)];
elseif iscell(var)
	svar = ['{cell array}'];
else
	warning(['error displaying field "' name '"']);
	return;
end

if isempty(svar)
	svar = '(not set)';
end
disp(['  ' name ': ' svar]);


function str = cell2str(cll)

if ischar(cll)
	str = cll;
else
	str = '';
	for n = 1:prod(size(cll))
		str = [str cll{n}];
		if n < prod(size(cll))
			str = [str ', '];
		end
	end
end
