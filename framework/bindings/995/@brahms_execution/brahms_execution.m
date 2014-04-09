%
% brahms_execution
%   BRAHMS execution object constructor.
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
% $Id:: brahms_execution.m 2447 2010-01-13 01:10:52Z benjmitch           $
% $Rev:: 2447                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2010-01-13 01:10:52 +0000 (Wed, 13 Jan 2010)                   $
%__________________________________________________________________________
%



function exe = brahms_execution(varargin)

if nargin == 0
	exe = emptycmd;
	exe = class(exe,'brahms_execution');
	
elseif nargin == 1 & isa(varargin{1}, 'brahms_execution')
	exe = varargin{1};
	
else
	exe = emptycmd;
	if nargin
		error('brahms_execution takes no arguments');
	end
	exe = class(exe,'brahms_execution');
	
end



function exe = emptycmd

exe = [];

% execution
exe.name = '';
exe.title = '';
exe.logs.precision = 6;
exe.logs.window = [];
exe.logs.encapsulated = [];
exe.logs.recurse = [];
exe.logs.all = false;
exe.logs.specific = {};
exe.seed = [];
exe.executionStop = 0;
exe.executionParameters = struct;

% scheduling
exe.addresses = {};
exe.affinity = {};

% localisation
exe.writeFilesTo = '';
exe.workingDirectory = '';
exe.systemFileIn = '';
exe.systemFileOut = false; % dont make an output file
exe.executionFile = '';
exe.reportFile = '';
exe.outFile = '';
exe.launch = '';

% meta is used to pass data between routines, and is not
% part of the object data itself
exe.meta = [];


