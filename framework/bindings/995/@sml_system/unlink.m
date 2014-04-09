%
% sys = sys.unlink(name)
% sys = sys.unlink(src, dst)
%
%   Remove a link from the system, by name or by
%   specification (src and dst).
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
% $Id:: unlink.m 2397 2009-11-18 21:20:07Z benjmitch                     $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = unlink(sys, varargin)

if nargin == 2
	name = varargin{1};
	if isempty(name)
		error('must specify a name, or specify src and dst to remove an unnamed link');
	end
	sys = removelink1(sys, name);
elseif nargin == 3
	src = varargin{1};
	dst = varargin{2};
	sys = removelink2(sys, src, dst);
else
	error('invalid usage');
end


function sys = removelink1(sys, name)

for n = 1:length(sys.link)
	if strcmp(sys.link{n}.name, name)
		sys.link = sys.link([1:n-1 n+1:end]);
		sys.namesinuse = rmfield(sys.namesinuse, name);
		return;
	end
end

error(['link "' name '" not found']);



function sys = removelink2(sys, src, dst)

for n = 1:length(sys.link)
	if strcmp(sys.link{n}.src, src) & strcmp(sys.link{n}.dst, dst)
		name = sys.link{n}.name;
		sys.link = sys.link([1:n-1 n+1:end]);
		if ~isempty(name)
			sys.namesinuse = rmfield(sys.namesinuse, name);
		end
		return;
	end
end

error(['link "' src '" ==> "' dst '" not found']);

