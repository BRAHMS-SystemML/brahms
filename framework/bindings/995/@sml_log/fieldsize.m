%
% sml_log::fieldsize


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
% $Id:: fieldsize.m 2397 2009-11-18 21:20:07Z benjmitch                  $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


function sz = fieldsize(log, fieldname)

if isstruct(log.fields)
	if isfield(log.fields, fieldname)
		field = getfield(log.fields, fieldname);
		if isstruct(field) && isfield(field, 'sml_log____interface_to_file')
			sz = field.size;
		else
			sz = size(field);
		end
		return
	end
end

error(['no field named "' fieldname '" in sml_log object'])


