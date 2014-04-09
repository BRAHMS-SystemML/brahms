%
% sys = sys.addsubsystem(name, subsys)
%
%   Add the sub-system "subsys" (another sml_system) to the
%   system, giving it the specified name.
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
% $Id:: addsubsystem.m 2397 2009-11-18 21:20:07Z benjmitch               $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function sys = addsubsystem(sys, name, sub)


if ~isa(sub, 'sml_system')
	error(['argument must be a SystemML system']);
end

validatename(sys, name);

sub.name = name;
N = length(sys.sub);
sys.sub{N+1} = sub;
sys.namesinuse.(name) = 'sub-system';
