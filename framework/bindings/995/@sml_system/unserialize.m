%
% sys = sys.unserialize(filename, reader)
%
%   Unserialize the document from the specified file.
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
% $Id:: unserialize.m 2397 2009-11-18 21:20:07Z benjmitch                $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = unserialize(sys, filename, reader)

if ~exist('reader', 'var')
	reader = struct;
end

if ~isfield(reader, 'callbackProgress')
	reader.callbackProgress = [];
end

% assume unnamed (may be overwritten if file specifies a
% name)
sys.name = 'unnamed';

% convert file to xml
if ~isempty(reader.callbackProgress)
	reader.callbackProgress('Unserializing System');
end
xml = sml_xml('file2xml', filename);

% recursively convert the XML to create a system
sys = parse(xml, reader);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function sys = parse(xml, reader)

sys = sml_system;

if ~strcmp(xml.name, 'System')
	error('invalid SystemML file (expected tag "System")');
end

if ~isfield(xml.attr, 'Version')
	error('invalid SystemML file (expected attribute "Version")');
end

if ~strcmp(xml.attr.Version, '1.0')
	error('invalid SystemML file (expected version "1.0")');
end

% for each child
for n = 1:length(xml.children)
	
	% switch on type
	switch xml.children(n).name
		
		case 'Name'
			if ~isempty(sys.name)
				error(['invalid SystemML file (too many Name tags)']);
			end
			sys.name = xml.children(n).value;

		case 'Title'
			if ~isempty(sys.title)
				error(['invalid SystemML file (too many Title tags)']);
			end
			sys.title = xml.children(n).value;

		case 'Process'
			proc = translateProcess(xml.children(n).children, reader);
			
			% can send sample rate as '1', because we set it
			% explicitly in the immediate following
			if isfield(proc, 'seed')
				sys = addprocess(sys, proc.name, proc.cls, '1', proc.state, proc.seed);
			else
				sys = addprocess(sys, proc.name, proc.cls, '1', proc.state);
			end
			
			% handle time and output
			sys.proc{end}.time = proc.time;
			sys.proc{end}.output = proc.output;
			
		case 'Link'
			l = translateLink(xml.children(n).children);
			sys = link(sys, l.name, l.src, l.dst, l.lag);
			
		case 'Expose'
			e = translateExpose(xml.children(n).children);
			sys = expose(sys, e.name, e.what, e.as);
			
		case 'Time'
			sys.time = translateTime(xml.children(n).children);
			
		case 'System'
			sub = parse(xml.children(n), reader);
			sys = addsubsystem(sys, sub.name, sub);
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' xml.children(n).name '" in "System")']);
		
	end
	
end




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function time = translateTime(tags)

time = [];

for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'BaseSampleRate'
			v = tags(n).value;
			f = find(v == '/');
			if length(f) ~= 1 || f < 2 || f > (length(v)-1)
				error(['invalid SystemML file (invalid BaseSampleRate)']);
			end
			time.baseSampleRate = v;
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Time")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function link = translateLink(tags)

link = [];
link.name = ''; % this field is not required to be in the SystemML document

for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'Name'
			link.name = tags(n).value;
			
		case 'Src'
			link.src = tags(n).value;
			
		case 'Dst'
			link.dst = tags(n).value;
			
		case 'Lag'
			link.lag = str2num(tags(n).value);
		
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Link")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function expose = translateExpose(tags)

expose = [];
expose.name = ''; % this field is not required to be in the SystemML document

for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'Name'
			expose.name = tags(n).value;
			
		case 'What'
			expose.what = tags(n).value;
			
		case 'As'
			expose.as = tags(n).value;
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Expose")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function proc = translateProcess(tags, reader)

proc = [];
proc.output = {};

for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'Name'
			proc.name = tags(n).value;
			
		case 'Class'
			proc.cls = tags(n).value;
			
		case 'Seed'
			proc.seed = str2num(tags(n).value);
			sseed = int2str(proc.seed);
			if ~strcmp(sseed, tags(n).value)
				error(['invalid SystemML file (unrecognised seed "' tags(n).value '")']);
			end
			
		case 'State'
			% if it's DataML, translate it, so that it can be
			% viewed easily by our clients
			if isfield(tags(n).attr, 'Format') && strcmp(tags(n).attr.Format, 'DataML')
				% is DataML
				proc.state = sml_xml('dataml2data', tags(n), reader);
			else
				% is not DataML, so just store the raw XML
				proc.state = tags(n);
				proc.state.stateml_write = 'raw';
			end
			
		case 'Output'
			proc.output = translateOutput(tags(n).children, reader);
			
		case 'Time'
			proc.time = translateProcessTime(tags(n).children, reader);
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Process")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function output = translateOutput(tags, reader)

output = {};

for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'Data'
			output{end+1} = translateData(tags(n).children, reader);
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Output")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function data = translateData(tags, reader)

data = [];
for n = 1:length(tags)
	
	switch tags(n).name
		
		case 'Name'
			data.name = tags(n).value;
			
		case 'Lag'
			data.lag = str2num(tags(n).value);
			
		case 'State'
			% if it's DataML, translate it, so that it can be
			% viewed easily by our clients
			if isfield(tags(n).attr, 'Format') && strcmp(tags(n).attr.Format, 'DataML')
				% is DataML
				data.state = sml_xml('dataml2data', tags(n), reader);
			else
				% is not DataML, so just store the raw XML
				data.state = tags(n);
				data.state.stateml_write = 'raw';
			end
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Data")']);
			
	end
	
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function time = translateProcessTime(tags, reader)

time = [];
for n = 1:length(tags)
	
	switch tags(n).name

		case 'BaseSampleRate'
			time.baseSampleRate = tags(n).value;
			
		case 'SampleRate'
			time.sampleRate = tags(n).value;
			
		case 'SamplePeriod'
			time.samplePeriod = str2num(tags(n).value);
			
		case 'Now'
			time.now = str2num(tags(n).value);
			
		otherwise
			error(['invalid SystemML file (unrecognised tag "' tags(n).name '" in "Time")']);
			
	end
	
end




