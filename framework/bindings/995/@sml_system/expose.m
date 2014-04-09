%
% sys = sys.expose(what, as)
% sys = sys.expose(name, what, as)
%
%   Expose "what" as "as", where what is a SystemML
%   identifier and as is a partial identifier. Optionally,
%   provide a name for the expose itself as a reference
%   (often, naming an expose will serve no purpose).
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
% $Id:: expose.m 2397 2009-11-18 21:20:07Z benjmitch                     $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = addexpose(sys, varargin)

if nargin == 3
	% empty name has special meaning: unnamed
	name = '';
	what = varargin{1};
	as = varargin{2};
elseif nargin == 4
	name = varargin{1};
	% empty name has special meaning: unnamed
	what = varargin{2};
	as = varargin{3};
else
	error('invalid usage');
end

validatename(sys, name, true);

expose = [];
expose.name = name;
expose.what = what;
expose.as = as;
N = length(sys.expose);
sys.expose{N+1} = expose;

% empty string does not place a constraint - can be used as
% name for multiple expose/link objects
if ~isempty(name)
	sys.namesinuse.(name) = 'expose';
end


