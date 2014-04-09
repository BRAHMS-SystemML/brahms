%
% sml_system::fieldnames


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
% $Id:: fieldnames.m 2419 2009-11-30 18:33:48Z benjmitch                 $
% $Rev:: 2419                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%


function f = fieldnames(sys)

if prod(size(sys)) > 1
	sys = sys(1);
end

f = {};

for n = 1:length(sys.proc)
	f{end+1, 1} = sys.proc{n}.name;
end

for n = 1:length(sys.sub)
	f{end+1, 1} = sys.sub{n}.name;
end
