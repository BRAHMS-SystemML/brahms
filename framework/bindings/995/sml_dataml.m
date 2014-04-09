function [writer, text] = sml_dataml(writer, state)

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
% $Id:: sml_dataml.m 2397 2009-11-18 21:20:07Z benjmitch                 $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


% add appropriate tags
writer.rootTagName = 'State';

% ask sml_xml to do the work
[xml, writer] = sml_xml('data2dataml', state, writer);

% add root tags
xml.attr.Format = 'DataML';
xml.attr.Version = num2str(writer.DataMLVersion);
xml.attr.AuthTool = 'SystemML Toolbox';
ver = sml_utils('Version');
xml.attr.AuthToolVersion = int2str(ver(4));

% ask sml_xml to do the work
text = sml_xml('xml2text', xml);
