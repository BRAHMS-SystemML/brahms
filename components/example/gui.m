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
% $Id:: gui.m 2419 2009-11-30 18:33:48Z benjmitch                        $
% $Rev:: 2419                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%





% event function
function [persist, output] = brahms_process(persist, input)



% nominal output
output = persist.output;



% switch on event type
switch input.event.type



	case persist.constants.EVENT_MODULE_INIT

		% provide component information
		output.info.component = [0 1];
		output.info.additional = ['Author=Ben Mitch\n' 'URL=http://brahms.sourceforge.net\n'];
		output.info.flags = [persist.constants.F_NOT_RATE_CHANGER];
		
		% ok
		output.event.response = persist.constants.C_OK;

	
		
	case persist.constants.EVENT_STATE_SET
		
		% create figure
		persist.h.fig = figure;
		
		% add scope
		persist.h.plot = plot([NaN]);
		persist.h.scope = gca;
		
		% add controls
		persist.h.button = uicontrol('parent', persist.h.fig, ...
			'style', 'pushbutton', 'string', 'Stop', 'callback', @stop_execution);
		
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
		
		% read input
		data = input.iif.default.ports(1).data;
		
		% take fft
		N = length(data);
		f = abs(fft(data));
		
		% update plot
		set(persist.h.plot, 'Xdata', (1:N)-1, 'Ydata', f);
		axis(persist.h.scope, [0 N 0 500])
		
		% might need this in some implementations
%   	drawnow
			
		% stop
		if ~isempty(get(persist.h.button, 'Tag'))
			output.event.response = persist.constants.C_STOP_USER;
			return
		end
		
		% ok
		output.event.response = persist.constants.C_OK;



end



function stop_execution(h, dat)

set(h, 'Tag', 'stop');


