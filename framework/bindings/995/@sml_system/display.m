%
% systemml::display
%
%   This is a private function.
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
% $Id:: display.m 2397 2009-11-18 21:20:07Z benjmitch                    $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function display(sys)

subdisplay(sys, '');



function subdisplay(sys, indent)

hangp = '    ';
hangl = '    ';
hange = '<-- ';

% if a sub-system, display an extra blank line
disp([indent '--------------------------------'])

nm = sys;
if isempty(nm.name)
	nm.name = '<root system>';
end

if isempty(nm.title)
	disp([indent '  "' nm.name '"']);
else
	disp([indent '  "' nm.name '" (''' nm.title ''')']);
end

% if a sub-system, display an extra blank line
disp([indent '--------------------------------'])

for n = 1:length(sys.sub)
	sub = sys.sub{n};
	subdisplay(sub, [indent '    ']);
end

for n = 1:length(sys.proc)
	proc = sys.proc{n};
	disp([indent hangp '"' proc.name '" (' proc.class ')'])
end

for n = 1:length(sys.link)
	link = sys.link{n};
	linktxt = ['==>	' link.src ' =' int2str(link.lag) '=> ' link.dst];
	disp([indent hangl linktxt])
end

for n = 1:length(sys.expose)
	expose = sys.expose{n};
	exposetxt = [expose.what ', ' expose.as];
	disp([indent hange exposetxt])
end

% if a sub-system, display an extra blank line
disp([indent '--------------------------------'])
