%
% brahms_execution::setexecpar
%
% __________________________________________________________
% This function is part of the "BRAHMS Matlab Bindings".


%__________________________________________________________________________
%
% This file is part of BRAHMS
% Copyright (C) 2007 Ben Mitchinson
% URL: http://brahms.sourceforge.net
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
% $Id:: setexecpar.m 2015 2009-06-06 20:04:45Z benjmitch                 $
% $Rev:: 2015                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-06-06 21:04:45 +0100 (Sat, 06 Jun 2009)                   $
%__________________________________________________________________________
%

function exe = setexecpar(exe, key, val)

if ~ischar(key)
	error(['key must be a string']);
end

% expect string
pars = brahms_utils('GetExecutionParameters');
if ~isfield(pars, key)
	error(['Execution Parameter "' key '" not found']);
end

% set it
if isnumeric(val)
	val = num2str(val);
end
if islogical(val)
	val = num2str(val);
end
if ~ischar(val)
	error(['Execution Parameters can only be set to numeric or string values']);
end
exe.executionParameters.(key) = val;



