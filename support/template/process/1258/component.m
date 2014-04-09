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
% $Id:: component.m 2406 2009-11-19 20:53:19Z benjmitch                  $
% $Rev:: 2406                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)                   $
%__________________________________________________________________________
%

% CUT HERE

% This is a newly created BRAHMS Process. It is a non-native
% process, and does not need to be built - BRAHMS can run it
% as it stands.




% event function
function [persist, output] = brahms_process(persist, input)



% nominal output
output = persist.output;



% switch on event type
switch input.event.type



	case persist.constants.EVENT_MODULE_INIT

		% provide component information
		output.info.component = [__TEMPLATE__RELEASE_INT__ __TEMPLATE__REVISION_INT__];
		output.info.additional = __TEMPLATE__ADDITIONAL_M__;
		output.info.flags = __TEMPLATE__FLAGS_M__;
		
		% ok
		output.event.response = persist.constants.C_OK;

	
		
	case persist.constants.EVENT_STATE_SET
		
		% ok
		output.event.response = persist.constants.C_OK;

		
	
	case persist.constants.EVENT_INIT_CONNECT
		
		% on first call
		if bitand(input.event.flags, persist.constants.F_FIRST_CALL)
			
			% do nothing
			
		end
		
		% on last call
		if bitand(input.event.flags, persist.constants.F_LAST_CALL)
			
			% do nothing
			
		end
		
		% ok
		output.event.response = persist.constants.C_OK;



	case persist.constants.EVENT_RUN_SERVICE

		% ok
		output.event.response = persist.constants.C_OK;



end

