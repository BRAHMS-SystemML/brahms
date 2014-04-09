%
% brahms_execution::serialize
%
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
% $Id:: serialize.m 2447 2010-01-13 01:10:52Z benjmitch                  $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%



% returns working directory calculated from execution object data

function wd = serialize(exe)



% persistent object
writer = [];
writer.VersionBRAHMS = brahms_utils('VersionStringBRAHMS');
writer.VersionBrahmsExecutionML = '1.0';

% defaults
exe = defaults(exe, true);
wd = exe.meta.wd;

% build XML
exe = struct(exe);

% convert to xml and then to text
xml = BrahmsExecutionML(writer, exe);

% check working directory
if ~exist(wd, 'dir')
	error(['working directory "' wd '" not found - run brahms_manager']);
end

% write to file
Cfile = exe.executionFile;
if isempty(fileparts(Cfile))
	Cfile = [wd filesep Cfile];
end

fid = fopen(Cfile, 'w');
if fid == -1 error(['could not open file "' Cfile '"']); end
fwrite(fid, ['<?xml version="1.0" encoding="ISO-8859-1"?>' 10]);
fwrite(fid, xml);
fclose(fid);




function xml = BrahmsExecutionML(writer, exe)


% construct tag
xml = ['<Execution ' ...
	'Version="' writer.VersionBrahmsExecutionML '" ' ...
	'AuthTool="BRAHMS Matlab Bindings" ' ...
	'AuthToolVersion="' writer.VersionBRAHMS '">' ...
	10];

% add simple tags
xml = [xml 9 '<Title>' exe.title '</Title>' 10];
xml = [xml 9 '<SystemFileIn>' exe.systemFileIn '</SystemFileIn>' 10];
xml = [xml 9 '<SystemFileOut>' exe.systemFileOut '</SystemFileOut>' 10];
xml = [xml 9 '<ReportFile>' exe.reportFile '</ReportFile>' 10];
xml = [xml 9 '<WorkingDirectory>' exe.workingDirectory '</WorkingDirectory>' 10];
xml = [xml 9 '<ExecutionStop>' num2str(exe.executionStop) '</ExecutionStop>' 10];
xml = [xml 9 '<Seed>' num2str(double(exe.seed)) '</Seed>' 10];

% add logs tag
xml = [xml 9 '<Logs'];
if ~isempty(exe.logs.precision) xml = [xml ' Precision="' int2str(exe.logs.precision) '"']; end
if ~isempty(exe.logs.window) xml = [xml ' Window="' exe.logs.window '"']; end
if ~isempty(exe.logs.encapsulated) xml = [xml ' Encapsulated="' int2str(exe.logs.encapsulated) '"']; end
if ~isempty(exe.logs.recurse) xml = [xml ' Recurse="' int2str(exe.logs.recurse) '"']; end
xml = [xml ' All="' int2str(exe.logs.all) '"'];
xml = [xml '>' 10];
for n = 1:length(exe.logs.specific)
	out = exe.logs.specific{n};
	xml = [xml 9 9 '<Log'];
	if ~isempty(out.precision) xml = [xml ' Precision="' int2str(out.precision) '"']; end
	if ~isempty(out.window) xml = [xml ' Window="' out.window '"']; end
	if ~isempty(out.encapsulated) xml = [xml ' Encapsulated="' int2str(out.encapsulated) '"']; end
	if ~isempty(out.recurse) xml = [xml ' Recurse="' int2str(out.recurse) '"']; end

	% replace ">" with ">>>"
	f = find(out.name == '>');
	if length(f) == 1
		out.name = [out.name(1:f-1) '>>>' out.name(f+1:end)];
	end

	xml = [xml '>' out.name '</Log>' 10];
end
xml = [xml 9 '</Logs>' 10];

% add voices tag
usedmpi = 0;
xml = [xml 9 '<Voices>' 10];
for c = 1:max(1, length(exe.addresses))
	xml = [xml 9 9 '<Voice>' 10];
	if length(exe.addresses)
		addr = exe.addresses{c};
		if ischar(addr) && strcmp(addr, 'mpi')
			xml = [xml 9 9 9 '<Address protocol="mpi">' int2str(usedmpi) '</Address>' 10];
			usedmpi = usedmpi + 1;
		elseif ischar(addr) && length(find(addr=='.'))==3
			xml = [xml 9 9 9 '<Address protocol="sockets">' addr '</Address>' 10];
		else
			error('error in addresses field');
		end
	end
	xml = [xml 9 9 '</Voice>' 10];
end
xml = [xml 9 '</Voices>' 10];

% add affinity tag
xml = [xml 9 '<Affinity>' 10];
if ~isempty(exe.affinity)
	for c = 1:length(exe.affinity)
		C = exe.affinity{c};
		items = '';
		voice = [];
		for p = 1:length(C)
			P = C{p};
			if ischar(P)
				items = [items 9 9 9 '<Identifier>' P '</Identifier>' 10];
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
			xml = [xml 9 9 '<Group>' 10];
		else
			xml = [xml 9 9 '<Group Voice="' int2str(voice) '">' 10];
		end
		xml = [xml items];
		xml = [xml 9 9 '</Group>' 10];
	end
end
xml = [xml 9 '</Affinity>' 10];

% add exec pars tag
xml = [xml 9 '<ExecutionParameters>' 10];
f = fieldnames(exe.executionParameters);
for n=1:length(f)
	key = f{n};
	val = exe.executionParameters.(key);
	if isnumeric(val) val = num2str(val); end
	if islogical(val) val = num2str(val); end
	xml = [xml 9 9 '<' key '>' val '</' key '>' 10];
end
xml = [xml 9 '</ExecutionParameters>' 10];

% end tag
xml = [xml '</Execution>' 10];





function str = forwardslash(str)

str(str == '\') = '/';


