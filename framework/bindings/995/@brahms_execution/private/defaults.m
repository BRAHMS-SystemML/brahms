%
% brahms_execution::defaults
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
% $Id:: defaults.m 2440 2009-12-14 10:02:48Z benjmitch                   $
% $Rev:: 2440                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-12-14 10:02:48 +0000 (Mon, 14 Dec 2009)                   $
%__________________________________________________________________________
%


function exe = defaults(exe, full)



% writing directory may be specified explicitly
wd = exe.writeFilesTo;

% if not
if isempty(wd)
	
	% working directory may be specified explicitly
	wd = exe.workingDirectory;
	
	% if not
	if isempty(wd)
		
		% it may be specified implicitly by the path to the command file
		if ~isempty(exe.executionFile)
			[a, b, c] = fileparts(exe.executionFile);
			if ~isempty(a)
				wd = a;
			end
		end
		
		% or get the execution parameter
		if isempty(wd)
			pars = brahms_utils('GetExecutionParameters');
			if isfield(pars, 'WorkingDirectory')
				wd = pars.WorkingDirectory;
			end
		end
		
		% or refuse
		if isempty(wd)
			% use current
			wd = '.';
			
			% no actually, don't
			error(['WorkingDirectory is not set - please run brahms_manager']);
		end
		
	end
	
end


% expand leading tilde
if wd(1) == '~'
	if isunix
		wd = [getenv('HOME') wd(2:end)];
	else
		wd = [getenv('USERPROFILE') wd(2:end)];
	end
end

% store for other routines
exe.meta.wd = wd;

% name to use if no name
name = exe.name;
if isempty(name)
	name = 'unnamed';
end

if isempty(exe.systemFileIn)
	exe.systemFileIn = [name '-sys.xml'];
end

if isempty(exe.launch)
	pars = brahms_utils('GetExecutionParameters');
	if length(exe.addresses) > 1
		if ischar(exe.addresses{1}) && strcmp(exe.addresses{1}, 'mpi')
			exe.launch = pars.LaunchLineMPI;
		else
			exe.launch = pars.LaunchLineSockets;
		end
	else
		exe.launch = pars.LaunchLineSolo;
	end
	
	% on PC, we don't want to use the brahms.bat script when
	% calling from matlab, since it generates a spurious line of
	% output that messes up the XML log (and we don't need the
	% script as of R2134 - see brahms.bat for more info).
	if ispc
		exe.launch = strrep(exe.launch, 'brahms ', 'brahms-execute ');
	end
	
end

% exe.systemFileOut can be "false" (no system output file required) or
% empty (based on input file name) or specified exactly.
if islogical(exe.systemFileOut)
	exe.systemFileOut = '';
else
	if isempty(exe.systemFileOut)
		file = exe.systemFileIn;
		if length(file) > 4 && strcmp(file(end-3:end), '.xml')
			exe.systemFileOut = [file(1:end-4) '-out.xml'];
		else
			exe.systemFileOut = [name '-sys-out.xml'];
		end
	end
end

if isempty(exe.executionFile)
	exe.executionFile = [name '-exe.xml'];
end

if isempty(exe.reportFile)
	exe.reportFile = [name '-rep-((VOICE)).xml'];
end

if isempty(exe.outFile)
	exe.outFile = [name '-out-((VOICE))'];
end




% for any file that is specified, but specified relatively,
% expand it to an absolute path
if nargin > 1
	
	file = exe.executionFile;
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.executionFile = file;
	
	file = exe.reportFile;
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.reportFile = file;
	
	file = [exe.outFile];
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.outFile = file;
	
	file = [exe.outFile '.xml'];
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.logFile = file;
	
	file = [exe.outFile '.exit'];
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.exitFile = file;
	
	file = exe.systemFileIn;
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.systemFileIn = file;
	
	file = exe.systemFileOut;
	if ~isempty(file) && isempty(fileparts(file))
		file = [wd filesep file];
	end
	exe.meta.systemFileOut = file;
	
end


