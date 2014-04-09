%
% sys = sys.link(src, dst[, lag])
% sys = sys.link(name, src, dst[, lag])
%
%   Add a link to the system from src to dst, where src and
%   dst are SystemML identifiers. Optionally, provide a name
%   for the link itself as a reference (often, naming a link
%   will serve no purpose). The optional extra parameter lag
%   specifies the lag on the link, which defaults to unity.
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
% $Id:: link.m 2397 2009-11-18 21:20:07Z benjmitch                       $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = link(sys, varargin)

% link(sys, src, dst[, lag])
% link(sys, name, src, dst[, lag])

% number of text arguments coming before a numeric or end of args
% indicates which form we're using
if nargin < 3
	error('not enough arguments');
end
if nargin == 3
	% empty name has special meaning: unnamed
	name = '';
	src = varargin{1};
	dst = varargin{2};
	lag = 1;
elseif nargin == 4
	if isnumeric(varargin{3})
		name = '';
		src = varargin{1};
		dst = varargin{2};
		lag = varargin{3};
	else
		name = varargin{1};
		src = varargin{2};
		dst = varargin{3};
		lag = 1;
	end
elseif nargin == 5
		name = varargin{1};
		% empty name has special meaning: unnamed
		src = varargin{2};
		dst = varargin{3};
		lag = varargin{4};
else
	error('invalid usage');
end

if ~ischar(name) | ~ischar(src) | ~ischar(dst) | ~isnumeric(lag)
	error('invalid usage');
end

validatename(sys, name, true);

link = [];
link.name = name;
link.src = src;
link.dst = dst;
link.lag = lag;
N = length(sys.link);
sys.link{N+1} = link;


% empty string does not place a constraint - can be used as
% name for multiple expose/link objects
if ~isempty(name)
	sys.namesinuse.(name) = 'link';
end



