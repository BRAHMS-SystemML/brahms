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
% $Id:: rand.m 2419 2009-11-30 18:33:48Z benjmitch                       $
% $Rev:: 2419                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%

function [persist, output] = myfunc(persist, input)


output = persist.output;
switch input.event.type


	case persist.constants.EVENT_STATE_SET

		% set initial pars
		rand('state', 1);

		% processed
		output.event.response = persist.constants.C_OK;



	case persist.constants.EVENT_INIT_CONNECT
		
		% create output on first call
		if bitand(input.event.flags, persist.constants.F_FIRST_CALL)

			% create scalar output
			output.operations{end+1} = {
				persist.constants.OPERATION_ADD_PORT,
				'',
				'std/2009/data/numeric',
				'DOUBLE/REAL/1'
				'out',
				};

		end

		% processed
		output.event.response = persist.constants.C_OK;



	case persist.constants.EVENT_RUN_SERVICE

		% update output
		output.operations{end+1} = ...
			{persist.constants.OPERATION_SET_CONTENT, input.oif.default.out, rand};

		% processed
		output.event.response = persist.constants.C_OK;



	case persist.constants.EVENT_MODULE_INIT

		% update output
		output.info.component = [0 __REV__];
		output.info.additional = ['Author=Ben Mitch' 10 'URL=http://brahms.sourceforge.net' 10];

		% processed
		output.event.response = persist.constants.C_OK;



end

