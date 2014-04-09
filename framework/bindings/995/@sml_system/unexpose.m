%
% sys = sys.unexpose(name)
% sys = sys.unexpose(what, as)
%
%   Remove an expose from the system, by name or by
%   specification (what and as).
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
% $Id:: unexpose.m 2397 2009-11-18 21:20:07Z benjmitch                   $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = removeexpose(sys, varargin)

if nargin == 2
	name = varargin{1};
	if isempty(name)
		error('must specify a name, or specify what and as to remove an unnamed expose');
	end
	sys = removeexpose1(sys, name);
elseif nargin == 3
	what = varargin{1};
	as = varargin{2};
	sys = removeexpose2(sys, what, as);
else
	error('invalid usage');
end


function sys = removeexpose1(sys, name)

for n = 1:length(sys.expose)
	if strcmp(sys.expose{n}.name, name)
		sys.expose = sys.expose([1:n-1 n+1:end]);
		sys.namesinuse = rmfield(sys.namesinuse, name);
		return;
	end
end

error(['expose "' name '" not found']);



function sys = removeexpose2(sys, what, as)

for n = 1:length(sys.expose)
	if strcmp(sys.expose{n}.what, what) & strcmp(sys.expose{n}.as, as)
		name = sys.expose{n}.name;
		sys.expose = sys.expose([1:n-1 n+1:end]);
		if ~isempty(name)
			sys.namesinuse = rmfield(sys.namesinuse, name);
		end
		return;
	end
end

error(['expose "' what '" as "' as '" not found']);

