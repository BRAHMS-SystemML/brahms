%
% sml_log
%   SystemML log object constructor.


%%%% NOTES
%
% a) tab-auto-complete throws up the wrong list (i.e.
% methods are included as well as data fields). this is not
% fixable with matlab's old (pre 2008) class system
%
% b) calling out.X.Y(:,end) will load out.X.Y to determine
% its size, so is not a fast call. unfortunately,
% size(out.X.Y) suffers the same problem. therefore, the
% only way to fast-access these data is as follows.
%
%   sz = fieldsize(out.X, 'Y');
%   N = sz(end);
%   y = out.X.Y(:, N);
%
% we could move to the new matlab class system as of 2008 or
% so, which might allow us to solve both of these more
% elegantly, i think. but if we do that we are no longer
% supporting earlier versions of matlab (which are in great
% use) or possibly octave either, though i'm not sure about
% that.



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
% $Id:: sml_log.m 2428 2009-12-11 15:10:37Z benjmitch                    $
% $Rev:: 2428                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-12-11 15:10:37 +0000 (Fri, 11 Dec 2009)                   $
%__________________________________________________________________________
%



function log = sml_log(xml, reader)

if nargin == 1 && isa(xml, 'sml_log')
	% copy ctor
	log = xml;
	return
end

if nargin ~= 2
	error(['sml_log takes two arguments (xml, reader)'])
end

log = [];
log.fields = [];
log = class(log, 'sml_log');

% walk hierarchy
f = fieldnames(xml);
for i = 1:length(f)
	key = f{i};
	log.fields.(key) = translate(xml.(key), reader);
end

% log.fields = translate(xml, reader);



%% GET XML FIELD

function f = getfield(pml, fieldName)

for n = 1:length(pml.children)

	if strcmp(pml.children(n).name, fieldName)
		f = pml.children(n).value;
		return;
	end

end

f = '';



%% TRANSLATE

function x = translate(x, reader)

if iscell(x)
	
	% is Log
	x = x{1};
	
	if ~(strcmp(x.name, 'Data') || strcmp(x.name, 'Process'))
		error(['invalid Report File "' reader.filename '"']);
	end
	name = x.children(1).value;

	% get format
	if isfield(x.children(7).attr, 'Format')
		fmt = x.children(7).attr.Format;
	else
		fmt = '<unspecified>';
	end

	% only handle DataML, currently
	if ~strcmp(fmt, 'DataML')
		error(['invalid Report File (non-DataML data format) "' reader.filename '"']);
	end
	
	% translate
	x = dataml2data(x.children(7), reader.supplementaryFilePath);

else

	x = sml_log(x, reader);
	
end



%% DATAML TO DATA

function value = dataml2data(tag, suppath)

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
			value(el).(fn{n}) = dataml2data(tag.children(e+1));
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
			switch tag.attr.c(2)
				case 'x'
					% trailing dimension is real/imag
					interleaved = false;
				case 'y'
					% leading dimension is real/imag
					interleaved = true;
				% case 'z'
					% penultimate dimension is real/imag
					% case does not arise?
				otherwise
					error('malformed DataML (expected "x" or "y" as complex suffix)');
			end
			cls = tag.attr.c(1);
			cpx = true;
		else
			interleaved = false;
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
			value = [];
			value.sml_log____interface_to_file = true;
			value.filename = tag.value;
			value.format = 'DataML';
			value.size = sz;

			% convert type attr to numeric class
			f = find('fdvutsponmlc' == cls);
			if isempty(f)
				error(['unrecognised numeric type "' cls '"'])
			end
			bytesperel = [4 8 8 4 2 1 8 4 2 1 1 1];
			matcls = {'single', 'double', 'uint64', 'uint32', 'uint16', 'uint8', 'int64', 'int32', 'int16', 'int8', 'logical', 'char'};
			value.matcls = matcls{f};
			value.complex = cpx;
			value.interleaved = interleaved;
			value.bytesperelement = bytesperel(f);

			% calculate expected file size
			value.bytesinfile = value.bytesperelement * numels;
			if cpx
				value.bytesinfile = value.bytesinfile * 2;
			end

			% add path if necessary
			if isempty(fileparts(value.filename))
				value.filename = [suppath '/' value.filename];
			end

			% check exists
			if ~exist(value.filename, 'file')
				error(['file not found "' value.filename '"']);
			end

			% check file size
			d = dir(value.filename);
			if value.bytesinfile ~= d.bytes
				error(['file incorrect size "' value.filename '" (' int2str(d.bytes) ', expecting ' int2str(value.bytesinfile) ')']);
			end
			
		else

			% read data from tag content (not interleaved)
			value = sscanf(tag.value, '%f');
			if isempty(value)
				value = [];
			end
			if cpx
				if interleaved
					value = complex(value(1:2:end), value(2:2:end));
				else
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

end



