%
% [logs, eml] = sml_reportml(filename, data)
%
%   Read the specified file into an execution report
%   object. Generic ReportML fields are parsed,
%   unrecognised fields are returned as raw XML structures.
%
%   If the optional "data" is supplied, it should have
%   fields "namespaceRoots" (in case the function needs to
%   seek codecs for translation of logs into matlab format)
%   and "tagged" (boolean) to indicate if logs fields should
%   be tagged as such (this aids merging results from
%   concerto runs).
%
% There is no write functionality for ReportML.
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
% $Id:: sml_reportml.m 2398 2009-11-18 21:53:08Z benjmitch               $
% $Rev:: 2398                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:53:08 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


function [logs, eml] = sml_reportml(reader)

if ~isfield(reader, 'namespaceRoots')
	reader.namespaceRoots = '';
end

if ~isfield(reader, 'callbackProgress')
	reader.callbackProgress = [];
end

if ~isfield(reader, 'supplementaryFilePath')
	reader.supplementaryFilePath = fileparts(reader.filename);
end

if ~exist(reader.filename, 'file')
	error('file not found');
end

if ~isempty(reader.callbackProgress)
	reader.callbackProgress('Importing Report File');
end

xml = sml_xml('file2xml', reader.filename);

logs = to_sml_log(xml.children(3).children, reader);

eml = [];
eml.Warning = 'This is not a public interface and may change - do not access this structure directly';
eml.Header = xml2struct(getxmlchild(xml, 'Header'));

% extract timing
timing = getxmlchild(getxmlchild(getxmlchild(getxmlchild(xml, 'Environment'), 'Client'), 'Performance'), 'Timing');
eml.Timing = parseTiming(timing);
eml.Timing.OS = getxmlfield(getxmlchild(getxmlchild(xml, 'Environment'), 'OS'), 'Type');



% eml.Environment = xml2struct(getxmlchild(xml, 'Environment'));



%% TO SML LOG

function logs = to_sml_log(xml, reader)

% convert to hierarchy
hrc = struct();
for i = 1:length(xml)
	switch xml(i).name
		case 'Data'
			name = getxmlfield(xml(i), 'Name');
		case 'Process'
			name = [getxmlfield(xml(i), 'Name') '/process__'];
		otherwise
			error(['unrecognised log field "' xml(i).name '"']);
	end
	hrc = setfieldspecial(hrc, name, xml(i));
end

logs = sml_log(hrc, reader);



%% SET FIELD SPECIAL

function s = setfieldspecial(s, name, data)

% field name can be in the form a/b/c>>d>e, and we expand
% it to a.b.c.d.e

% special case is three chevrons together, which means the set name was
% empty, i.e. the default set. we represent this in matlab as just
% producing the data name directly
% name = strrep(name, '>>>', '>>default>');
name = strrep(name, '>>>', '/');
name = strrep(name, '>>', '/');
name = strrep(name, '>', '/');

f = find(name == '/');
if isempty(f)
	s.(name) = {data};
else
	f = f(1);
	g = name(1:f-1);
	h = name(f+1:end);
	if isfield(s,g)
		t = s.(g);
	else
		t = struct();
	end
	s.(g) = setfieldspecial(t, h, data);
end


%% GET XML FIELD

function f = getxmlfield(pml, fieldName)

for n = 1:length(pml.children)

	if strcmp(pml.children(n).name, fieldName)
		f = pml.children(n).value;
		return;
	end

end

f = '';


%% GET XML CHILD

function f = getxmlchild(pml, fieldName)

for n = 1:length(pml.children)

	if strcmp(pml.children(n).name, fieldName)
		f = pml.children(n);
		return;
	end

end

error(['field "' fieldName '" not found']);



%% XML TO STRUCT

function s = xml2struct(x)

if length(x.children) == 0
	s = x.value;
	if ischar(s) && (any(s == 13) || any(s == 10))
		s = struct();
	end
	return
end

s = struct();

for c = 1:length(x.children)
	key = x.children(c).name;
	if isfield(s, key)
% 		error(['xml2struct() failed (multiple tags with same name "' key '")']);
		s.(key)(end+1) = xml2struct(x.children(c));
	else
		s.(key) = xml2struct(x.children(c));
	end
end









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function out = parseTiming(data)

% empty structure must have proper structure for easy use in
% brahms_perf
out = struct;
out.threads = struct;
out.threads.index = [];
out.threads.irtcpu = [];
out.threads.processes = [];
out.threads = out.threads([]);

for m = 1:length(data.children)
	kid = data.children(m);
	switch(kid.name)
		case 'Thread'

			thread = [];

			for p = 1:length(kid.children)
				k = kid.children(p);
				switch(k.name)
					case 'Index'
						thread.index = sscanf(k.value, '%f');
					case 'IRTCPU'
						thread.irtcpu = reshape(sscanf(k.value, '%f'), [2 3]);
					case 'Process'
						process = [];
						process.name = getxmlfield(k, 'Name');
						process.irt = sscanf(getxmlfield(k, 'IRT'), '%f');

						if isfield(thread,'processes')
							thread.processes(end+1) = process;
						else
							thread.processes = process;
						end
					otherwise
						error(['unrecognised field "' k.name '"']);
				end
			end

			out.threads(end+1) = thread;

		case 'TicksPerSec'
			out.ticksPerSec = sscanf(kid.value, '%f');
		case 'TimeRunPhase'
			out.TimeRunPhase = sscanf(kid.value, '%f');
		case 'RDTSC'
			out.rdtsc = sscanf(kid.value, '%f');
		case 'Caller'

			sup = [];
			for p = 1:length(kid.children)
				k = kid.children(p);
				switch(k.name)
					case 'IRTCPU'
						sup.irtcpu = reshape(sscanf(k.value, '%f'), [2 3]);
					case 'IRT'
						sup.irt = sscanf(k.value, '%f');
					otherwise
						error(['unrecognised field "' k.name '"']);
				end
			end

			out.caller = sup;
		otherwise
			error(['unrecognised field "' kid.name '"']);
	end
end


