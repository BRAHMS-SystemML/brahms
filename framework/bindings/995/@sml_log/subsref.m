%
% sml_log::subsref


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
% $Id:: subsref.m 2427 2009-12-11 04:57:31Z benjmitch                    $
% $Rev:: 2427                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-12-11 04:57:31 +0000 (Fri, 11 Dec 2009)                   $
%__________________________________________________________________________
%


function out = subsref(log, ref)



switch ref(1).type
	
	case '.'

		out = log.fields.(ref(1).subs);
		
		if isstruct(out) && isfield(out, 'sml_log____interface_to_file')
			out = subsrefx(out, ref(2:end));
			return
		end
		
		ref = ref(2:end);
		if ~isempty(ref)
			out = subsref(out, ref);
		end

	otherwise
		
		error('invalid reference')
		
end





function out = subsrefx(iface, refs)

ndims = length(iface.size);
read_all = false;
transpose_output_at_end = false;

if isempty(refs)
	
	% simple case - read in whole file at once
	read_range = 'all';
	
else
	
	% complex case - read in just some parts of file
	if ~strcmp(refs.type, '()')
		error(['invalid reference (only "()" indexing is supported)']);
	end
	nrefs = length(refs.subs);
	if nrefs > ndims
		error(['invalid reference (too many indexes)']);
	end
	if nrefs < ndims
		% treat as different size
		iface.size = [iface.size(1:nrefs-1) prod(iface.size(nrefs:end))];
		ndims = length(iface.size);
		% special case
		if ndims == 1 && isnumeric(refs.subs{1}) && size(refs.subs{1}, 1) == 1
			% we'll end up with a column vector (first dimension)
			% but matlab returns a row vector in this case
			transpose_output_at_end = true;
		end
	end
	stride = [1];
	read_range = 'all';
	for i = 1:ndims
		ind = refs.subs{i};
		if ischar(ind)
			if ~strcmp(ind, ':')
				error('internal');
			end
		else
			read_range = [];
		end
		stride(i+1) = stride(end) * iface.size(i);
	end
	stride = stride(1:end-1);
	
	% if not decided to read all after all
	if ~ischar(read_range)
		t_index = refs.subs{ndims};
		if ischar(t_index)
			t_index = 1:iface.size(ndims);
		end
		read_range = [min(t_index) max(t_index)];
	end
	
end

% check exists
d = dir(iface.filename);
if isempty(d)
	error(['file not found "' iface.filename '"']);
end
if d.bytes ~= iface.bytesinfile
	error(['file wrong size "' iface.filename '"']);
end

% open file
fid = fopen(iface.filename, 'rb');
if fid == -1
	error(['failed open "' iface.filename '"']);
end

% read file
if ischar(read_range) % 'all'
	
	sz = iface.size;

	% read whole file
	if length(sz) == 2 && ~iface.complex
		out = fread(fid, sz, ['*' iface.matcls]);
	else
		if iface.complex
			if iface.interleaved
				sz = [2 sz(1:end-1) sz(end)];
			else
				sz = [sz(1:end-1) 2 sz(end)];
			end
		end
		if length(sz) < 2
			sz = [sz 1];
		end
		out = reshape(fread(fid, prod(sz), ['*' iface.matcls]), sz);
		if iface.complex
			adims = repmat({':'}, 1, ndims-1);
			sz = iface.size;
			if iface.interleaved
				out = complex(reshape(out(1,adims{:},:), sz), reshape(out(2,adims{:},:), sz));
			else
				out = complex(reshape(out(adims{:},1,:), sz), reshape(out(adims{:},2,:), sz));
			end
		end
	end
	
else
	
	% read specified time range of file
	%
	% note that complex data is interleaved in an unencap file
	% (so that it's efficient to write to the file), so we
	% have to read a contiguous block of double the real size,
	% and then de-interleave that block by the second-to-last
	% dimension (last dimension is always time, whether
	% complex or not)
	
	% range
	t_count = (diff(read_range) + 1);
	element_start = stride(end) * (read_range(1) - 1);
	complex_mult = 1;
	if iface.complex
		complex_mult = 2;
	end
	
	% seek first byte of contiguous block
	byte_start = element_start * iface.bytesperelement * complex_mult;
	status = fseek(fid, byte_start, 'bof');
	if status == -1
		fclose(fid);
		error(['failed seek in "' iface.filename '"']);
	end
	
	% get dimensions of block to be read (source dimensions,
	% but last is time-to-be-read rather than time-available)
	sz = [iface.size(1:end-1) t_count];
	
	% stick in a two as the second-to-last, if complex, so the
	% automatic reading reads into an appropriately shaped
	% array (it's leading if interleaved, trailing if not)
	if iface.complex
		if iface.interleaved
			sz = [2 sz(1:end-1) sz(end)];
		else
			sz = [sz(1:end-1) 2 sz(end)];
		end
	end
	
	% do the read
	if length(sz) == 2
		out = fread(fid, sz, ['*' iface.matcls]);
	else
		if length(sz) < 2
			sz = [sz 1];
		end
		out = reshape(fread(fid, prod(sz), ['*' iface.matcls]), sz);
	end
	
	% if complex, de-interleave
	if iface.complex
			adims = repmat({':'}, 1, ndims-1);
			sz = [iface.size(1:end-1) t_count];
		if iface.interleaved
			out = complex(reshape(out(1,adims{:},:), sz), reshape(out(2,adims{:},:), sz));
		else
			out = complex(reshape(out(adims{:},1,:), sz), reshape(out(adims{:},2,:), sz));
		end
	end
	
	% trim matrix by all indices (including time, which may
	% have been sparse)
	if ~ischar(refs.subs{end})
		refs.subs{end} = refs.subs{end} - read_range(1) + 1;
	end
	out = subsref(out, refs);
	
	% make sure complex, if required
	if iface.complex && isreal(out)
		out = complex(out);
	end
	
end

% close file
fclose(fid);

% end
if transpose_output_at_end
	out = out';
end






