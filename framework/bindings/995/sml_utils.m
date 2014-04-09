%
% sml_utils
%
%   Without arguments, reports the status of the SystemML
%   Toolbox installation.
%
% ver = sml_utils('Version')
%
%   Returns the version of the installed SystemML Toolbox,
%   as a vector.
%
% ver = sml_utils('VersionString')
%
%   Returns the version of the installed SystemML Toolbox,
%   as a string.
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
% $Id:: sml_utils.m 2397 2009-11-18 21:20:07Z benjmitch                  $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


function out = sml_utils(request, arg)

if nargin

	switch request
	
		case 'VersionString'
			
			out = '__VERSIONSTRING__';
		
		case 'Version'

			out = [0 0 0 0];
			
		otherwise
			
			error('unrecognised argument')
			
	end
	
else
	
	disp(['SystemML Toolbox Version ' sml_utils('VersionString') ' is installed'])
	
end



