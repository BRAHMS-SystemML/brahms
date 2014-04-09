%
% sys.serialize(filename, writer)
%
%   Serialize the document to the specified file.
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
% $Id:: serialize.m 2397 2009-11-18 21:20:07Z benjmitch                  $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function serialize(sys, filename, writer)

if ~exist('writer', 'var')
	writer = struct;
end

if ~isfield(writer, 'precision')
	writer.precision = [];
end

if ~isfield(writer, 'encapsulated')
	writer.encapsulated = true;
end

if ~isfield(writer, 'callbackProgress')
	writer.callbackProgress = [];
end

if ~isfield(writer, 'attr')
	writer.attr = struct;
end

% persistent object
writer.AuthTool = 'SystemML Toolbox';
ver = sml_utils('Version');
writer.AuthToolVersion = int2str(ver(4));
writer.VersionSystemML = '1.0';
[writer.supplementaryFilePath, f, e] = fileparts(filename);
if writer.encapsulated
	writer.supplementaryFileName = '';
else
	writer.supplementaryFileName = [f e];
end
writer.supplementaryFileIndex = 1;

% convert to xml and then to text
if ~isempty(writer.callbackProgress)
	writer.callbackProgress('Serializing System');
end
writer.item_count = SystemML_System_Count(sys, [0 0]);
writer.items_serialized = [0 0];
[writer, xml] = SystemML_System(writer, sys);
if ~isempty(writer.callbackProgress)
	writer.callbackProgress([1 1]);
end

% write to file
if ~isempty(writer.callbackProgress)
	writer.callbackProgress('Writing System File');
end
fid = fopen(filename, 'w');
if fid == -1 error(['could not open file "' filename '"']); end
fwrite(fid, ['<?xml version="1.0" encoding="ISO-8859-1"?>' 10]);
fwrite(fid, xml);
fclose(fid);






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% <System>



function count = SystemML_System_Count(sys, count)

% add children
for k = 1:length(sys.sub)
	count = SystemML_System_Count(sys.sub{k}, count);
end
count(1) = count(1) + length(sys.proc);
count(2) = count(2) + length(sys.link);
count(2) = count(2) + length(sys.expose);



function [writer, xml] = SystemML_System(writer, sys)

% construct tag
xml = ['<System ' ...
	'Version="' writer.VersionSystemML '" ' ...
	'AuthTool="' writer.AuthTool '" ' ...
	'AuthToolVersion="' writer.AuthToolVersion '">'];
if ~isempty(sys.name) xml = [xml '<Name>' sys.name '</Name>']; end
if ~isempty(sys.title) xml = [xml '<Title>' sys.title '</Title>']; end

% if isempty(sys.time)
% 	xml = [xml '<Time/>'];
% else
% 	xml = [xml '<Time>'];
% 	xml = [xml '<BaseSampleRate>' sys.time.baseSampleRate '</BaseSampleRate>'];
% 	xml = [xml '</Time>'];
% end

% add children
for k = 1:length(sys.sub)
	[writer, subxml] = SystemML_System(writer, sys.sub{k});
	xml = [xml subxml];
end
for k = 1:length(sys.proc)
	[writer, subxml] = SystemML_Process(writer, sys.proc{k});
	xml = [xml subxml];
	
	% make progress callback
	if ~isempty(writer.callbackProgress)
		writer.items_serialized(1) = writer.items_serialized(1) + 1;
			p = writer.items_serialized ./ writer.item_count;
			writer.callbackProgress(p);
	end
end

% add children
for k = 1:length(sys.link)
	name = sys.link{k}.name;
	if ~isempty(name)
		name = ['<Name>' name '</Name>'];
	end
	src = xmlsafe(sys.link{k}.src);
	dst = xmlsafe(sys.link{k}.dst);
	lag = sys.link{k}.lag;
	xml = [xml '<Link>' name '<Src>' src '</Src><Dst>' dst '</Dst><Lag>' int2str(lag) '</Lag>'];
	xml = [xml '</Link>'];

	% make progress callback
	if ~isempty(writer.callbackProgress)
		writer.items_serialized(2) = writer.items_serialized(2) + 1;
			p = writer.items_serialized ./ writer.item_count;
			writer.callbackProgress(p);
	end
end

% add children
for k = 1:length(sys.expose)
	name = sys.expose{k}.name;
	if ~isempty(name)
		name = ['<Name>' name '</Name>'];
	end
	what = xmlsafe(sys.expose{k}.what);
	as = xmlsafe(sys.expose{k}.as);
	xml = [xml '<Expose>' name '<What>' what '</What><As>' as '</As></Expose>'];

	% make progress callback
	if ~isempty(writer.callbackProgress)
		writer.items_serialized(2) = writer.items_serialized(2) + 1;
			p = writer.items_serialized ./ writer.item_count;
			writer.callbackProgress(p);
	end
end


% end tag
xml = [xml '</System>'];





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% <Head>




function xml = SystemML_Head(writer, head)


xml = ['<Head>'];
user = '';

f = fieldnames(head);
for n = 1:length(f)
	key = f{n};
	val = head.(key);
	switch(key)
		case 'title'
			xml = [xml '<Title>' val '</Title>'];
		otherwise
			user = [user '<' key '>' val '</' key '>'];
	end
end

if ~isempty(user)
	xml = [xml '<User>' user '</User>'];
end

xml = [xml '</Head>'];









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% <Process>




function [writer, xml] = SystemML_Process(writer, obj)

% obtain valid state (absent or empty state is permitted)
if ~isfield(obj,'state')
	obj.state = struct;
end
if isempty(obj.state)
	obj.state = struct;
end

% construct tag
xml = ['<Process>' ...
	'<Name>' obj.name '</Name>' ...
	'<Class>' obj.class '</Class>'];
if isfield(obj, 'seed')
	xml = [xml '<Seed>' num2str(double(obj.seed)) '</Seed>'];
end

% time tag
xml = [xml '<Time>'];
xml = [xml '<SampleRate>' num2str(obj.time.sampleRate) '</SampleRate>'];
if isfield(obj.time, 'baseSampleRate')
	xml = [xml '<BaseSampleRate>' num2str(obj.time.baseSampleRate) '</BaseSampleRate>'];
end
if isfield(obj.time, 'samplePeriod')
	xml = [xml '<SamplePeriod>' num2str(obj.time.samplePeriod) '</SamplePeriod>'];
end
if isfield(obj.time, 'now')
	xml = [xml '<Now>' num2str(obj.time.now) '</Now>'];
end
xml = [xml '</Time>'];

% see if the state specifies a StateML writer, else use the DataML writer
if isfield(obj.state, 'stateml_write')
	stateml_write = obj.state.stateml_write;
else
	stateml_write = 'sml_dataml';
end

% handle raw XML
if strcmp(stateml_write, 'raw')
	
	% xml is raw
	stateml = sml_xml('xml2text', obj.state);
	
else

	% custom stateml, check if the StateML writer is on the path
	if isempty(which(stateml_write))
		error(['StateML writer "' stateml_write '" specified for state of process "' obj.name '" is not accessible']);
	end

	% run the writer
	[writer, stateml] = feval(stateml_write, writer, obj.state);

end

% add the stateml
xml = [xml stateml];

% write output interface
if isfield(obj, 'output')
	xml = [xml '<Output>'];
	for k = 1:length(obj.output)
		name = obj.output{k}.name;
		lag = int2str(obj.output{k}.lag);
		state = sml_xml('xml2text', obj.output{k}.state);
		xml = [xml '<Data><Name>' name '</Name><Lag>' lag '</Lag>' state '</Data>'];
	end
	xml = [xml '</Output>'];
end


% write the StateML and close State and Process tags
xml = [xml '</Process>'];









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% helpers






function data = xmlsafe(data)

data = strrep(data, '"', '&quot;');
data = strrep(data, '''', '&apos;');
data = strrep(data, '&', '&amp;');
data = strrep(data, '>', '&gt;');
data = strrep(data, '<', '&lt;');






function s = stateTag(attr)


s = '<State';
f = fieldnames(attr);
for i = 1:length(f)
	key = f{i};
	val = attr.(key);
	s = [s ' ' key '="' val '"'];
end
s = [s '>'];

