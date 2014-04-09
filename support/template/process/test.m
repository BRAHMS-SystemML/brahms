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
% $Id:: test.m 2419 2009-11-30 18:33:48Z benjmitch                       $
% $Rev:: 2419                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%

% CUT HERE

% this is a simple test script for your new process

function test

% empty system
sys = sml_system;

% add process
state = [];
fS = 10;
cls = '';
sys = sys.addprocess('process', __TEMPLATE_CLASS_STRING_M__, fS, state);

% execution
exe = brahms_execution;
exe.all = true;
exe.name = 'process';
exe.stop = 1;

% run brahms
[out, rep] = brahms(sys, exe);
