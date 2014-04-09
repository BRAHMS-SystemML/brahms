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
% $Id:: simplegui.m 2419 2009-11-30 18:33:48Z benjmitch                  $
% $Rev:: 2419                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%



% empty system
sys = sml_system;
fS = 500;

% add source
state = [];
state.dist = 'normal';
state.dims = [10000];
state.pars = [0 1];
state.complex = false;
cls = 'std/2009/random/numeric';
sys = sys.addprocess('src', cls, fS, state);

% add process
state = [];
cls = 'client/brahms/example/1258/gui';
sys = sys.addprocess('fft', cls, fS, state);

% link
sys = sys.link('src>out', 'fft');

% execution
exe = brahms_execution;
exe.all = true;
exe.name = 'process';
exe.stop = 1;

% run brahms
[out, rep] = brahms(sys, exe);
