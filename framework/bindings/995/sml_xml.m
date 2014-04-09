%
% WARNING: THIS IS NOT A USER FUNCTION - ITS INTERFACE OR
%   OPERATION MAY CHANGE IN FUTURE RELEASES. YOU SHOULD NOT
%   CALL THIS FUNCTION DIRECTLY.
%
% out = sml_xml(op, ...)
%
% perform any of the operations listed below. "state", where
% required, is the state of the reader or writer, and may
% have been changed when it is returned.
%
% "file2text" <filename>
%   read file into XML text snippet, trimming declaration
%   and root-level whitespace, and converting all linefeeds
%   to UNIX style.
%
% "text2file" <text> <filename>
%   write XML text snippet into file, adding declaration,
%   but without linefeed translation (file will be written
%   with UNIX style linefeeds).
%
% "text2xml" <text>
%   parse XML text snippet into a matlab XML structure.
%
% "xml2text" <xml>
%   create XML text snippet from a matlab XML structure.
%
% "dataml2data" <dataml> <reader>
%   parse a matlab XML (DataML) structure snippet into
%   matlab data. errors will occur if XML is not DataML.
%
% "data2dataml" <data> <writer>
%   create a matlab XML (DataML) structure from some matlab
%   data. <writer> is returned as a second output argument,
%   and its state may have changed. <writer> may have any of
%   the following fields.
%
% <reader>
%   "supplementaryFilePath" is the path where supplementary
%   files will be found if they have relative path names in
%   the XML.
%
% <writer>
%   "rootTagName" the name to give to any root-level tags.
%   "precision" the precision at which to write textual
%   representations of numbers (significant figures). empty
%   matrix means maximum precision.
%   "supplementaryFilePath" is the path where supplementary
%   files should be written.
%   "supplementaryFileName" is the stem to use for any
%   supplementary files that need to be written. if empty,
%   no supplementary files are written (i.e. the storage is
%   encapsulated).
%   "supplementaryFileIndex" is the next index to use for
%   supplementary file filenames. if unsupplied, 1 is
%   assumed. this index is updated when the writer object is
%   returned if any supplementary files have been written.
%
%   NOTE: supplementaryFilePath is not stored in the
%   generated XML, so if you use this setting, paths to
%   supplementary files are stored in relative form. this is
%   a Good Thing, since they will still be linked to the XML
%   if the files are all moved together. if you need to
%   absolute paths in the XML, let supplementaryFilePath be
%   empty, and prepend the path to supplementaryFileName
%   instead.
%
%
%
% some shortcuts are also provided, that perform one or more
% of the above steps:
%
% "file2xml" <filename>
% "xml2file" <xml> <filename>
%
%
% __________________________________________________________
% This function is part of the "SystemML Toolbox".

% xml.name = '<tag-name>'
% xml.attr = struct('key', val, ...)
% xml.value = '<tag content>'
% xml.children = <array of similar structures>
%
% either "value" or "children" must be empty

% DataML format: a DataML tag has the following attributes:
%
% "c"
%   this is the basic data type. can be "y" (cell) or "z"
%   (struct) or one of "fdvutsponmlc" (numeric - see below
%   for details of what each means). numeric types can be
%   suffixed with "x" (complex).
%
% "b"
%   dimensions, e.g. b="3 2". if the data is scalar, this
%   attribute can be absent.
%
% "a"
%   fieldnames, as a semicolon-separated list, e.g.
%   a="abc;def;". only present if c="z".
%
% "s"
%   numeric storage format, either absent (stored directly
%   in the tag) or s="b" (stored in a binary file, and the
%   tag is the filename of that file).
%
% cell and struct tags then have child tags, all with the
% tag name "m", which are each an element of the cell or
% struct. they are always all present, and are in element
% order. for struct tags, all fields of the first element
% are listed, then all fields of the second, etc.
%
% numeric tags have the numbers stored directly in the
% content string of the tag (space-separated, no trailing
% space) or, for binary file storage, have a filename that
% points at the binary file. the binary file format is
% simply the raw data block, except that if the data is
% complex, the binary file is interleaved by the last
% dimension, so that the data is stored in the file in
% chonological order, assuming the last dimension of the
% data is time. note that for non-binary storage, complex
% data are not interleaved.

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
% $Id:: sml_xml.m 2397 2009-11-18 21:20:07Z benjmitch                    $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


function [out, out2] = sml_xml(op, in, extra)




switch op
	
	case 'file2text'
		out = file2text(in);
		
	case 'text2file'
		text2file(in, extra);
		
	case 'text2xml'
		out = text2xml(in);
		
	case 'xml2text'
		out = xml2text(in);

	case 'dataml2data'
		if nargin < 3
			reader = [];
		else
			reader = extra;
		end
		if ~isfield(reader, 'supplementaryFilePath')
			reader.supplementaryFilePath = '';
		end
		out = dataml2data(in, reader);

	case 'data2dataml'
		if nargin < 3
			writer = [];
		else
			writer = extra;
		end
		if ~isfield(writer, 'rootTagName')
			writer.rootTagName = 'DataML';
		end
		if ~isfield(writer, 'precision')
			writer.precision = [];
		end
		if ~isfield(writer, 'supplementaryFilePath')
			writer.supplementaryFilePath = '';
		end
		if ~isfield(writer, 'supplementaryFileName')
			writer.supplementaryFileName = '';
		end
		if ~isfield(writer, 'supplementaryFileIndex')
			writer.supplementaryFileIndex = 1;
		end
		[out, out2] = data2dataml(in, writer, writer.rootTagName);
		
		% report version
		out2.DataMLVersion = 5;
		
		

%% shortcuts follow

	case 'file2xml'
		text = file2text(in);
		out = text2xml(text);
		
	case 'xml2file'
		text = xml2text(in);
		text2file(text, extra);
		
	otherwise
		error('unrecognised operation');
		
end









%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         FILE ==> TEXT
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function text = file2text(filename)

% read file
fid = fopen(filename);
if fid == -1
	error(['not found "' filename '"']);
end
text = fread(fid, '*char')';
fclose(fid);

% THIS IS SLOW AND I THINK UNNECESSARY - CRLF IS
% HANDLED CORRECTLY BY FOLLOWING STAGES
%
% CRLF ==> LF
% text = text([text(1:end-1) ~= 13 | text(2:end) ~= 10 true]);

% THIS IS SLOW AND I THINK UNNECESSARY - WHITESPACE IS
% HANDLED CORRECTLY BY FOLLOWING STAGES
%
% % trim whitespace
% notws = find(text ~= 9 & text ~= 32 & text ~= 10);
% text = text(notws(1):notws(end));

% peel off declaration
f = strfind(text, '<');
if isempty(f)
	error('systemml:no_xml_in_file', ['no XML in file "' filename '"']);
end
if strcmp(text(f(1):f(1)+4), '<?xml')
	g = strfind(text, '?>');
	if isempty(g) || g(1) < f(1)
		error('malformed XML declaration');
	end
	text = text(g(1)+2:end);
else
	error(['missing XML declaration in file "' filename '"']);
end

% peel off stylesheet
f = strfind(text, '<?xml-stylesheet');
if ~isempty(f)
	g = strfind(text, '?>');
	pre = text(1:f-1);
	pre(pre == 32 | pre == 9 | pre == 13 | pre == 10) = 0;
	if any(pre) || g < f
		error(['malformed/misplaced xml-stylesheet declaration in file "' filename '"']);
	end
	text = text(g+2:end);
end

% THIS IS SLOW AND I THINK UNNECESSARY - WHITESPACE IS
% HANDLED CORRECTLY BY FOLLOWING STAGES
%
% % trim whitespace - this second trim isn't necessary, but
% % it's tidy at least
% notws = find(text ~= 9 & text ~= 32 & text ~= 10);
% text = text(notws(1):notws(end));









%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         TEXT ==> FILE
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function text2file(text, filename)

% add XML declaration
text = ['<?xml version="1.0" encoding="ISO-8859-1"?>' 10 text];

% write to file
fid = fopen(filename, 'wb');
if fid == -1
	error(['could not open "' filename '"']);
end
fwrite(fid, text);
fclose(fid);










%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         TEXT ==> XML
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function xml = text2xml(text)

% create empty
xml = [];
xml.name = [];
xml.attr = [];
xml.value = [];
xml.children = [];
xml = xml(1:0);

% check
if isempty(text)
	return
end

% convert to parts
parts = text2parts(text);

% convert to xml
xml = parts2xml(parts);




% convert xml text into data,tag,data,tag,etc.

function parts = text2parts(xml)


% first, remove any comments
f = strfind(xml, '<!--');
g = strfind(xml, '-->');
ncom = length(f);
if ncom ~= length(g) || min([1; diff(reshape([f; g], ncom*2, 1))]) < 1
	error('invalid XML file, or more complex than this parser can handle');
end
g = g + 2;

keep = repmat(logical(1), 1, length(xml));
for n = 1:ncom
	keep(f(n):g(n)) = false;
end
xml = xml(keep);

% now, get boundaries of genuine tags
f = strfind(xml, '<');
g = strfind(xml, '>');
ch = 1;
tag = 1;
ntags = length(f);
if length(g) ~= ntags || min([1; diff(reshape([f; g],ntags*2,1))]) < 1
	error('invalid XML file, or more complex than this parser can handle');
end

emptystruct = struct();

% types
%   0 data (text outside of any tag)
%   1 open tag (has attributes)
%   2 close tag
%   3 open-close tag (has attributes)
%   4 comment tag

part = [];
part.type = 0;
part.data = '';
part.attr = emptystruct;

parts = part;
partscount = 0;
partsreserved = 1;

while tag<=ntags

	if ch ~= f(tag)
		% add data
		part.type = 0;
		part.data = xml(ch:f(tag)-1);
		part.attr = emptystruct;
		
		% expand storage
		if partscount == partsreserved
			partsreserved = partsreserved * 2;
			parts(partsreserved,1) = part;
		end
		
		partscount = partscount + 1;
		parts(partscount,1) = part;
		ch = f(tag);
	end

	% add tag
	wholetag = xml(f(tag)+1:g(tag)-1);
	
	% CLOSE TAGS
	if wholetag(1) == '/'
		part.type = 2; % close tag
		part.data = wholetag(2:end);
		part.attr = emptystruct;
		
	% COMMENT TAGS
	elseif wholetag(1) == '!'
		part.type = 4; % comment
		part.data = wholetag(4:end-3);
		part.attr = emptystruct;
	
	% MUST BE OPEN TAGS
	else
		
		% check if it's an OPEN-CLOSE tag
		if wholetag(end) == '/'
			part.type = 3;
			wholetag = wholetag(1:end-1);
		else
			part.type = 1;
		end
		
		% check for attributes
		attr = emptystruct;
		ucase = wholetag >= 65 & wholetag <= 90;
		lcase = wholetag >= 97 & wholetag <= 122;
		numb = wholetag >= 48 & wholetag <= 57;
		endstagname = ~lcase & ~ucase & ~numb & wholetag ~= '_';
		
		if any(endstagname)
			e = find(endstagname);
			e = e(1);
			if wholetag(e) ~= 32
				error(['malformed tag (no space after tag name) "' wholetag '"']);
			end
			attrs = wholetag(e+1:end);
			wholetag = wholetag(1:e-1);
			
			% interpret attrs
			while true
				e = find(attrs == '=');
				if isempty(e)
					error('invalid XML (no =)');
				end
				e = e(1);
				if attrs(e+1) ~= '"'
					error('invalid XML (not ")');
				end
				key = attrs(1:e-1);
				attrs = attrs(e+2:end);
				e = find(attrs == '"');
				if isempty(e)
					error('invalid XML (no ")');
				end
				e = e(1);
				val = attrs(1:e-1);
				val = xmlunsafe(val);
				attr.(key) = val;
				if e == length(attrs) break; end
				attrs = attrs(e+2:end);
			end
			
		end

		part.data = wholetag;
		part.attr = attr;
		
	end

	% expand storage
	if partscount == partsreserved
		partsreserved = partsreserved * 2;
		parts(partsreserved,1) = part;
	end
	
	partscount = partscount + 1;
	parts(partscount,1) = part;
	ch = g(tag)+1;
	tag = tag + 1;

end

if ch <= length(xml)
	
	part = [];
	part.type = 0;
	part.data = xml(ch:end);
	part.attr = struct();
	partscount = partscount + 1;
	parts(partscount,1) = part;
	
end

parts = parts(1:partscount);




function [xml, nextpart] = parts2xml(parts, nextpart, parent)

if nargin<2 nextpart = 1; end
if nargin<3 parent = []; end

% only simple (text or children, but not both) XML
% tags are supported in this format, so we store them
% separately

xml = [];
xml.name = '';
xml.attr = struct();
xml.value = '';
xml.children = [];


while nextpart <= length(parts)
	part = parts(nextpart);
	nextpart = nextpart + 1;
	
	switch part.type
		case 0
			if ~isempty(xml.children)
				% ignore whitespace if children are present, but use it otherwise
				f = find(part.data ~= 9 & part.data ~= 32 & part.data ~= 10 & part.data ~= 13);
				if isempty(f) continue; end
				% presence of non-whitespace data when we already have children is a
				% mixed XML tag
				error('mixed type (text and children) XML tags unsupported in this translation');
			end

			% just return string content
			xml.value = xmlunsafe(part.data);
			
		% 1 = opening tag
		case 1
			% presence of children when we already have non-whitespace data is a
			% mixed XML tag
			if ~isempty(xml.value)
				f = find(xml.value ~= 9 & xml.value ~= 32 & xml.value ~= 10 & xml.value ~= 13);
				if ~isempty(f) error('mixed type (text and children) XML tags unsupported in this translation'); end
				% if data is whitespace, we'll just discard it
				xml.value = [];
			end
			
			[xml_, nextpart] = parts2xml(parts, nextpart, part);
			xml_.name = part.data;
			xml_.attr = part.attr;

			% add to children
			if isempty(xml.children)
				xml.children = xml_;
			else
				xml.children(end+1) = xml_;
			end
			
		case 2
			if ~strcmp(part.data,parent.data)
				error(['invalid nesting - "' part.data '" ends "' parent.data '"']);
			end
			return;
			
		% 3 = open-close tag
		case 3
			% presence of children when we already have non-whitespace data is a
			% mixed XML tag
			if ~isempty(xml.value)
				f = find(xml.value ~= 9 & xml.value ~= 32 & xml.value ~= 10 & xml.value ~= 13);
				if ~isempty(f) error('mixed type (text and children) XML tags unsupported in this translation'); end
				% if data is whitespace, we'll just discard it
				xml.value = [];
			end
			
			xml_ = [];
			xml_.name = part.data;
			xml_.attr = part.attr;
			xml_.value = '';
			xml_.children = [];

			% add to children
			if isempty(xml.children)
				xml.children = xml_;
			else
				xml.children(end+1) = xml_;
			end
			
		otherwise
			
			error('unrecognised part code');
			
	end
end


if isempty(parent)
	% this is the root tag, and we should reach here
	% but we fix up the representation at this point
	xml = xml.children;
		
else
	error('did not find closing tag');
	
end










%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         XML ==> TEXT
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function text = xml2text(xml)

% open start tag
text = ['<' xml.name];

% attributes
if ~isempty(xml.attr)
	f = fieldnames(xml.attr);
	for a = 1:length(f)
		key = f{a};
		val = xml.attr.(f{a});
		text = [text ' ' key '="' xmlsafe(val) '"'];
	end
end

% close start tag
text = [text '>'];

% add value
if ~isempty(xml.value)
	
	if length(xml.children)
		error('mixed content not allowed');
	end
	
	text = [text xmlsafe(xml.value)];

end

% add children
for c = 1:length(xml.children)
	text = [text xml2text(xml.children(c))];
end

% add end tag
text = [text '</' xml.name '>'];











%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         DATAML ==> DATA
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function value = dataml2data(tag, reader)

% get dims
if isfield(tag.attr, 'b')
	sz = sscanf(tag.attr.b, '%f')';
else
	% no sz attr means scalar
	sz = 1;
end
numels = prod(sz);

% check for simple string storage
if ~isfield(tag.attr, 'c')
	
	% it's a simple string
	value = tag.value;
	return
	
end

% otherwise switch on class
switch tag.attr.c
	
	case 'z'

		% create scalar structure
		fn = {};
		f = [0 find(tag.attr.a == ';')];
		for n = 1:length(f)-1
			fn{n} = tag.attr.a(f(n)+1:f(n+1)-1);
		end
		nfields = length(fn);
		value = struct();
		for n = 1:length(fn)
			value.(fn{n}) = [];
		end

		% expand it to required sz if not scalar
		if numels > 1
			cmd = ['value = repmat(value, [' sz ']);'];
			eval(cmd);
		end

		% for each <struct> element
		for e = 0:length(tag.children)-1
			el = floor(e / nfields) + 1;
			n = mod(e, nfields) + 1;
			value(el).(fn{n}) = dataml2data(tag.children(e+1), reader);
		end

	case 'y'

		% create cell array
		value = cell(sz);
		
		% for each <cell> element
		for n = 1:length(tag.children)
			value{n} = dataml2data(tag.children(n), reader);
		end

	otherwise

		% get numeric class (and complexity)
		if length(tag.attr.c) == 2
			if tag.attr.c(2) ~= 'x'
				error('malformed DataML');
			end
			cls = tag.attr.c(1);
			cpx = true;
		else
			cls = tag.attr.c;
			cpx = false;
		end

		% check for binary storage format
		if isfield(tag.attr, 's')

			% validate
			storage_protocol = tag.attr.s;
			if ~ischar(storage_protocol) || ~isscalar(storage_protocol) || storage_protocol ~= 'b'
				error('cannot recognise storage protocol');
			end

			% tag content is filename
			filename = tag.value;

			% convert type attr to numeric class
			f = find('fdvutsponmlc' == cls);
			if isempty(f)
				error(['unrecognised numeric type "' cls '"'])
			end
			bytesperel = [4 8 8 4 2 1 8 4 2 1 1 1];
			matcls = {'single', 'double', 'uint64', 'uint32', 'uint16', 'uint8', 'int64', 'int32', 'int16', 'int8', 'logical', 'char'};
			bytesperel = bytesperel(f);
			matcls = matcls{f};
			
			% calculate expected file size
			expbytes = bytesperel * numels;
			if cpx
				expbytes = expbytes * 2;
			end
			
			% add path if necessary
			if isempty(fileparts(filename))
				filename = [reader.supplementaryFilePath '/' filename];
			end
			
			% check file size
			d = dir(filename);
			if isempty(d)
				error(['file not found "' filename '"']);
			end
			if expbytes ~= d.bytes
				error('binary file incorrect size');
			end
			
			% open file
			fid = fopen(filename, 'rb');
			if fid == -1
				error(['failed open "' filename '"']);
			end

			% read and close file
			try
				[value, count] = fread(fid, Inf, ['*' matcls]);
			catch
				fclose(fid);
				rethrow(lasterror);
			end
			fclose(fid);
			
			% complex?
			if cpx
				
				% interleaved?
				if length(sz) > 1

					% check sizes
					szf = sz(end);
					szi = prod(sz(1:end-1));
					if count/2 ~= numels
						error('something went wrong reading the binary file');
					end

					% prepare real and imag for separate collation
					real_vals = value(1:numels);
					imag_vals = real_vals;

					% pre-calculate indices into arrays
					ind_one_step = 1:szi;

					% loop through last dimension
					for t = 1:szf

						% prepare offsets
						real_offset = (t-1) * szi;
						comp_offset = ((t-1)*2) * szi;

						% collate real and imag
						real_vals(real_offset + ind_one_step) = value(comp_offset + ind_one_step);
						imag_vals(real_offset + ind_one_step) = value(comp_offset + szi + ind_one_step);

					end

					% complexify
					value = complex(real_vals, imag_vals);

				else
					
					% no, not interleaved
					value = complex(value(1:end/2), value(end/2+1:end));
					
				end
				
			end % if cpx
			
		else
			
			% read data from tag content (not interleaved)
			value = sscanf(tag.value, '%f');
			if cpx
				value = complex(value(1:end/2), value(end/2+1:end));
			end
			
		end

		% reshape data appropriately
		if length(sz) < 2
			sz = [sz 1];
		end
		if length(sz) > 1
			value = reshape(value, sz);
		end

		% and re-class it too
		switch cls
			case 'f', value = single(value);
			case 'd', value = double(value);
			case 'v', value = uint64(value);
			case 'u', value = uint32(value);
			case 't', value = uint16(value);
			case 's',  value = uint8(value);
			case 'p', value = int64(value);
			case 'o', value = int32(value);
			case 'n', value = int16(value);
			case 'm',  value = int8(value);
			case 'l',  value = logical(value);
			case 'c',  value = char(value);
			otherwise
				error(['uncoded DataML class "' cls '"']);
		end
		
		% force complex if data was real
		if cpx && isreal(value)
			value = complex(value);
		end

end














%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         DATA ==> DATAML
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function [xml, writer] = data2dataml(data, writer, tagName)

% get size
sz = size(data);

% create xml object
xml = [];
xml.name = tagName;
xml.attr = struct();
xml.value = '';
xml.children = [];

% most DataML fields are standardised arrays, but we allow a
% special case for simple strings for readability and brevity,
% that is just a tag with no attributes and the string as content.
if ischar(data) && length(sz) == 2 && sz(1) == 1 && all(data >= 32) && all(data <= 127)
	xml.value = data;
	return
end

% calculate more stuff
numels = prod(sz);
isscalar = numels == 1;

% trim trailing scalar dimensions
while length(sz) > 1 && sz(end) == 1
	sz = sz(1:end-1);
end

% in DataML, scalar can have missing sz attribute
if ~isscalar
	xml.attr.b = numtostr(sz, []);
end




% given the note above and the note below, it's worth
% summarising the protocol for identifying the type of a
% dataml node. the "c" attribute can only be left out in two
% cases, a non-empty string or a structure element,
% therefore on encountering a node we can assess its type as
% follows:
%
% 1) if explicitly specified using c="type"
% 2) otherwise, if it has text, c="char" and b="1 length(text)"
% 3) otherwise, c="struct" (a struct element can never have text)

cls = class(data);

switch cls

	case 'cell'
		
		% store type attribute
		xml.attr.c = 'y';
		
		% store individual elements as children
		for n = 1:numels
			[child, writer] = data2dataml(data{n}, writer, 'm');
			if isempty(xml.children)
				xml.children = child;
			else
				xml.children(end+1) = child;
			end
		end

	case 'struct'
		
		% store type attribute
		xml.attr.c = 'z';
		
		% store field names in attribute "a"
		fn = fieldnames(data);
		fns = '';
		for f = 1:length(fn)
			fns = [fns fn{f} ';'];
		end
		xml.attr.a = fns;

		% store individual elements as children, with all the
		% zeroth fields listed, then all the oneth fields, etc.
		for n = 1:numels
			for f = 1:length(fn)
				[child, writer] = data2dataml(data(n).(fn{f}), writer, 'm');
				if isempty(xml.children)
					xml.children = child;
				else
					xml.children(end+1) = child;
				end
			end
		end

	case {'char' 'single' 'double' 'int8' 'int16' 'int32' 'int64' 'uint8' 'uint16' 'uint32' 'uint64' 'logical'}
		
		% translate class into type attribute
		switch cls
			case 'single', cls = 'f';
			case 'double', cls = 'd';
			case 'uint64', cls = 'v';
			case 'uint32', cls = 'u';
			case 'uint16', cls = 't';
			case 'uint8',  cls = 's';
			case 'int64', cls = 'p';
			case 'int32', cls = 'o';
			case 'int16', cls = 'n';
			case 'int8',  cls = 'm';
			case 'logical',  cls = 'l';
			case 'char',  cls = 'c';
			otherwise
				error(['uncoded DataML class "' cls '"']);
		end
		
		% append 'x' if data is complex and store attribute
		if ~isreal(data)
			cls = [cls 'x'];
		end
		xml.attr.c = cls;
		
		% store data, using binary files if array is very large
		if numels <= 1000 || isempty(writer.supplementaryFileName)
			
			% if we're storing encapsulated, just because the
			% element count is small, then we have to use maximum
			% precision (like a binary file) else we'll get very
			% confusing results!
			if ~isempty(writer.supplementaryFileName)
				xml.value = numtostr(data, []);
			else	
				xml.value = numtostr(data, writer.precision);
			end
			
		else
			
			% store as filename and binary file
			filename = [writer.supplementaryFileName '.' sprintf('%04i', writer.supplementaryFileIndex)];
			writer.supplementaryFileIndex = writer.supplementaryFileIndex + 1;
			numtofile([writer.supplementaryFilePath filesep filename], data, sz);
			
			% storage class "b" means binary file
			xml.attr.s = 'b';
			
			% store filename relative, if requested
			xml.value = filename;

		end

	otherwise
		
		error(['DataML cannot represent data of class "' class(data) '", in node named "' tagName '"'])

end




function s = numtostr(n, precision)

if isempty(n)
	s = '';
	return
end

if isreal(n)
	
	if isfloat(n)
		if isempty(precision)
			s = sprintf('%.20g ', n);
		else
			s = sprintf(['%.' int2str(precision) 'g '], n);
		end
	else
		% integers are written at maximum precision
		s = sprintf('%d ', n);
	end
	
	% trim trailing space
	s = s(1:end-1);
	
else
	
	% store real then imag (i.e. not interleaved)
	s = [numtostr(real(n), precision) ' ' numtostr(imag(n), precision)];
	
end






function numtofile(filename, data, sz)

fid = fopen(filename, 'wb');
if fid == -1
	error(['could not open "' filename '"'])
end

N = sz(end);
C = prod(sz(1:end-1));

if isreal(data)
	
	% we can just output the whole lot as it comes
	fwrite(fid, data, class(data));
	
else

	% if complex, interleave the data real/imag/real/...
	r = real(data);
	i = imag(data);
	cls = class(r);

	rng = 1:C;
	offset = 0;
	
	% if elements per block is low, we get much better fwrite()
	% performance by buffering into memory between writes...
	buffer_elements = 4096;
	if C < buffer_elements

		buffer = zeros(0, 0, cls);
		buffer_used = 0;

		for n = 1:N
			
			% store into buffer
 			buffer(buffer_used+rng) = r(offset+rng);
 			buffer(buffer_used+rng+C) = i(offset+rng);
			buffer_used = buffer_used + 2 * C;
			offset = offset + C;
			
			% flush buffer if full
			if buffer_used >= buffer_elements
	 			fwrite(fid, buffer, cls);
				buffer_used = 0;
			end
			
		end

		% fwrite buffer remainder
		if buffer_used
			fwrite(fid, buffer(1:buffer_used), cls);
		end
		
	else

		for n = 1:N
			fwrite(fid, [r(offset+rng) i(offset+rng)], cls);
			offset = offset + C;
		end

	end
	
end

fclose(fid);












%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%%         HELPERS
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%

function data = xmlunsafe(data)

data = strrep(data, '&lt;', '<');
data = strrep(data, '&gt;', '>');
data = strrep(data, '&apos;', '''');
data = strrep(data, '&quot;', '"');
data = strrep(data, '&amp;', '&');

function data = xmlsafe(data)

data = strrep(data, '&', '&amp;');
data = strrep(data, '"', '&quot;');
data = strrep(data, '''', '&apos;');
data = strrep(data, '>', '&gt;');
data = strrep(data, '<', '&lt;');





