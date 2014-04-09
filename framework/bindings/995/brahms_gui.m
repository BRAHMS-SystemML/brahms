% result = brahms_gui(operation, data)
%
% This function provides the BRAHMS Invocation Bindings
% (995) GUI. It is primarily intended for use by the
% launcher (brahms.m), but you can use it also to monitor
% progress in your own code. See "brahms_test gui" for an
% example.

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
% $Id:: brahms_gui.m 2397 2009-11-18 21:20:07Z benjmitch                 $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% MAIN FUNCTION

function ret = brahms_gui(operation, data)

% no return data
ret = [];

% no action on octave
if brahms_utils('IsOctave')
	return
end

% only show one gui
global brahms_gui_data

% operations
switch operation

	case 'destroy'
		
		% destroy GUI
		if ~isempty(brahms_gui_data) && ishandle(brahms_gui_data.h.dialog)
			delete(brahms_gui_data.h.dialog);
			brahms_gui_data = [];
			return
		end



	case 'abort'

		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end

		% if aborted, break operation
		if brahms_gui_data.abort
			if isempty(brahms_gui_data.abort_message)
				msg = 'execution aborted';
			else
				msg = brahms_gui_data.abort_message;
			end
			brahms_gui('destroy');
			error('brahms:abort_execution', msg);
		end		

		
		
	case 'enable_abort'
		
		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end

		if data
			set(brahms_gui_data.h.abort, 'Enable', 'on');
		else
			set(brahms_gui_data.h.abort, 'Enable', 'off');
		end
		


	case 'configure_abort'
		
		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end

		if exist('data', 'var')
			set(brahms_gui_data.h.abort, 'String', data{1});
			brahms_gui_data.abort_message = data{2};
		else
			set(brahms_gui_data.h.abort, 'String', 'Abort');
			brahms_gui_data.abort_message = '';
		end
		


	case 'show'

		% check still exists
		if ~isempty(brahms_gui_data) && ~ishandle(brahms_gui_data.h.dialog)
			% doesn't, so abandon that old one...
			brahms_gui_data = [];
		end

		% create if not existing
		if isempty(brahms_gui_data)
			
			% matlab can't currently give you client desktop
			% window size, so we are stuck with using the full
			% screen. this won't be perfect, but shouldn't look
			% wrong on most configurations
			screensz = get(0,'ScreenSize');
			w = 400;
			h = 128;
			x = screensz(3);
			y = screensz(4);
			x = round(x * 0.75 - 0.5 * w);
			y = round(y * 0.75 - 0.5 * h);
			pos = [x y w h];

			% create dialog
			brahms_gui_data.h.dialog = locdialog( ...
				'WindowStyle', 'normal', ...
				'Color', [236 233 216]/255, ...
				'Name', 'BRAHMS Bindings (995)', ...
				'Position', pos, ...
				'Visible', 'off', ...
				'ResizeFcn', @callback_resize, ...
				'Units', 'pixels', ...
 				'KeyPressFcn', @callback_abort_key, ... % not sure if this is good...
				'CloseRequestFcn', @callback_close, ...
				'ButtonDownFcn', '' ...
				);

			% create button
			brahms_gui_data.h.abort = uicontrol(brahms_gui_data.h.dialog, ...
				'Units', 'pixels', ...
				'Callback', @callback_abort, ...
				'string', 'Abort' ...
				);
			style(brahms_gui_data.h.abort);

			% create progress bar
			brahms_gui_data.h.progbar_axis = axes( ...
				'parent', brahms_gui_data.h.dialog, ...
				'Units', 'pixels', ...
				'xtick', [], ...
				'ytick', [] ...
				);
			brahms_gui_data.h.progbar1 = rectangle( ...
				'parent', brahms_gui_data.h.progbar_axis, ...
				'position', [0 0 1e-8 1], ...
				'facecolor', [1 0 0], ...
				'edgecolor', 'none' ...
				);
			brahms_gui_data.h.progbar2 = rectangle( ...
				'parent', brahms_gui_data.h.progbar_axis, ...
				'position', [0 0 1e-8 1], ...
				'facecolor', [0 0.5 0], ...
				'edgecolor', 'none' ...
				);
			brahms_gui_data.h.progbar_border = rectangle( ...
				'parent', brahms_gui_data.h.progbar_axis, ...
				'position', [0 0 1 1], ...
				'edgecolor', [0 0 0] ...
				);
			axis(brahms_gui_data.h.progbar_axis, [0 1 0 1]);

			% create message text
			brahms_gui_data.h.msg1 = uicontrol(brahms_gui_data.h.dialog, ...
				'Style', 'text', ...
				'Units', 'pixels', ...
				'FontWeight', 'bold', ...
				'Horiz', 'left', ...
				'string', '' ...
				);
			style(brahms_gui_data.h.msg1);

			% create message text
			brahms_gui_data.h.msg2 = uicontrol(brahms_gui_data.h.dialog, ...
				'Style', 'text', ...
				'Units', 'pixels', ...
				'Horiz', 'left', ...
				'string', '' ...
				);
			style(brahms_gui_data.h.msg2);

			% create message text
			brahms_gui_data.h.time = uicontrol(brahms_gui_data.h.dialog, ...
				'Style', 'text', ...
				'Units', 'pixels', ...
				'Horiz', 'right', ...
				'string', '' ...
				);
			style(brahms_gui_data.h.time);

% 			% create log window
% 			brahms_gui_data.h.log = uicontrol(brahms_gui_data.h.dialog, ...
% 				'Style', 'listbox', ...
% 				'Units', 'pixels', ...
% 				'Enable', 'inactive', ...
% 				'Max', 2, ...
% 				'Horiz', 'left', ...
% 				'BackgroundColor', 'white', ...
% 				'string', {} ...
% 				);
% 			style(brahms_gui_data.h.log);

		else

			% clear gui items, as required
			set(brahms_gui_data.h.progbar1, 'position', [0 0 1e-8 1]);
			set(brahms_gui_data.h.progbar2, 'position', [0 0 1e-8 1]);
			set(brahms_gui_data.h.msg1, 'string', {});
			set(brahms_gui_data.h.msg2, 'string', {});
% 			set(brahms_gui_data.h.log, 'string', {});

		end

		% start timer, whether newly created or not
		brahms_gui_data.t.start = clock;
		brahms_gui_data.t.last_update = -1;
		brahms_gui_data.current_progress = [0 0];
		brahms_gui_data.abort = false;
		brahms_gui_data.abort_message = '';
		
		% show if requested
		if ~exist('data') || data
			set(brahms_gui_data.h.dialog, 'Visible', 'on');
		end
		drawnow



	case 'phase'

		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end

		% fire callback
		callbackProgress({data});



	case 'operation'

		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end

		% fire callback
		callbackProgress(data);



% 	case 'log'
% 
% 		% ensure exists
% 		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
% 			brahms_gui('show');
% 		end
% 
% 		% set
% 		s = get(brahms_gui_data.h.log, 'string');
% 		s{end+1} = data;
% 		set(brahms_gui_data.h.log, 'string', s);
% 		drawnow % allow new string to come through, so we can...
% 		set(brahms_gui_data.h.log, 'listboxtop', length(s), 'value', []);
% 		drawnow
% 
% 
% 
% 	case 'log_augment'
% 
% 		% ensure exists
% 		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
% 			brahms_gui('show');
% 		end
% 
% 		% set
% 		s = get(brahms_gui_data.h.log, 'string');
% 		if length(s)
% 			s{end} = [s{end} data];
% 		end
% 		set(brahms_gui_data.h.log, 'string', s);
% 		drawnow % allow new string to come through, so we can...
% 		set(brahms_gui_data.h.log, 'listboxtop', length(s), 'value', []);
% 		drawnow



	case 'progress'

		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end
		
		% fire callback
		callbackProgress(data);
		
	
		
	case 'hide_progress'
		
		% ensure exists
		if isempty(brahms_gui_data) || ~ishandle(brahms_gui_data.h.dialog)
			brahms_gui('show');
		end
		
		% fire callback
		hideProgress;
		
		
		
	case 'get_callback_progress'
		
		% return it
		ret = @callbackProgress;



	otherwise

		% not ok
		error(['unrecognised operation "' operation '"']);

end






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% NESTED HELPER FUNCTIONS FOLLOW



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	function s = prettytime(t)

		t = floor(t);
		secs = mod(t, 60);
		s = [':' sprintf('%02d', secs)];
		t = (t - secs) / 60;
		mins = mod(t, 60);
		hrs = (t - mins) / 60;
		s = [sprintf('%02d', mins) s];
		if hrs
			s = [sprintf('%d', hrs) ':' s];
		end

	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	function style(h)

		fontname = 'Arial';

		set(h, 'BackgroundColor', [236 233 216]/255);
		set(h, 'fontname', fontname);
		set(h, 'fontsize', 10);
		set(h, 'fontunits', 'points');

	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	function callback_resize(src, evt)

		% get rect rectangle
		sz = get(src, 'position');
		rect = [0 0 sz(3:4)];

		% metrics
		external_border = 12;
		border = 8;
		button_width = 96;
		button_height = 24;
		progbar_height = 15; % actually renders 16 pixels high
		time_width = 64;
		item_height = 16;

		% add main border
		rect = rect + external_border * [1 1 -2 -2];

		% abort button
		pos = [rect(1)+rect(3)-button_width rect(2) button_width button_height];
		set(brahms_gui_data.h.abort, 'position', pos);
		d = button_height + border;
		rect = rect + [0 d 0 -d];

		% progress bar
		pos = [rect(1) rect(2) rect(3) progbar_height];
		set(brahms_gui_data.h.progbar_axis, 'position', pos);
		d = progbar_height + border;
		rect = rect + [0 d 0 -d];

		% time text
		pos = [rect(1) rect(2)+rect(4)-item_height rect(3) item_height];
		pos_ = [pos(1)+pos(3)-time_width pos(2) time_width pos(4)];
		set(brahms_gui_data.h.time, 'position', pos_);
		pos_ = [pos(1) pos(2) pos(3)-time_width-border pos(4)];
		set(brahms_gui_data.h.msg1, 'position', pos_);
		d = item_height + border;
		rect = rect + [0 0 0 -d];

		% msg2
 		set(brahms_gui_data.h.msg2, 'position', rect);


	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	function callback_close(src, evt)

		% user wants GUI to disappear, and not reappear, so
		% let's hide it, instead of closing it (if we close it,
		% the next call to update it will reopen it)
%  		set(brahms_gui_data.h.dialog, 'Visible', 'off');

		% or... do nothing, ignore user's requests
		
		% or, actually close it - it will be reopened on the
		% next update call
		delete(src);

	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	
	function callback_abort(src, evt)
		
		brahms_gui_data.abort = true;
		
	end

	function callback_abort_key(src, evt)
		
		if strcmp(evt.Key, 'escape')
	 		brahms_gui_data.abort = true;
		end
		
	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	function hideProgress

		set(brahms_gui_data.h.time, 'string', '');
		
	end

	function callbackProgress(w)
		
		% do nothing if not present
		if isempty(brahms_gui_data)
			abort = false;
			return
		end
		if ~ishandle(brahms_gui_data.h.dialog)
			abort = false;
			return
		end
		dodrawnow = false;

		% progress bar
		if isnumeric(w) && ~isempty(w)
			
			w(isnan(w)) = 1; % 0/0 means 'complete', but may be passed as 'NaN'
			
			w(w>1) = 1;
			N = 100;
			w = floor(w * N) / N;
			w(w<1e-8) = 1e-8;
			
			if any(brahms_gui_data.current_progress ~= w)
				
				brahms_gui_data.current_progress = w;
			
				% we show the most progress with the bar at the
				% back, and the least with the bar at the front
				if w(1) < w(2)
					w = [w(2) w(1)];
				end
				pos = [0 0 w(1) 1];
				set(brahms_gui_data.h.progbar1, 'position', pos);
				pos = [0 0 w(2) 1];
				set(brahms_gui_data.h.progbar2, 'position', pos);
				
				% refresh
				dodrawnow = true;
				
			end
			
		end
		
		% msg1
		if iscell(w)
			set(brahms_gui_data.h.msg1, 'string', w{1});

			% refresh
			dodrawnow = true;
		end
		
		% msg2
		if ischar(w)
			set(brahms_gui_data.h.msg2, 'string', w);

			% refresh
			dodrawnow = true;
		end
		
		% get time
		t = floor(etime(clock, brahms_gui_data.t.start));
		
		% time - only do these reasonably often
		if t ~= brahms_gui_data.t.last_update
			brahms_gui_data.t.last_update = t;
			set(brahms_gui_data.h.time, 'string', prettytime(t));

			% refresh
			dodrawnow = true;
		end
		
		% ok
		if dodrawnow
			drawnow
		end
		
		if brahms_gui_data.abort
			brahms_gui('abort');
		end
		
	end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% END MAIN FUNCTION

end










function h = locdialog(varargin)

if brahms_utils('IsOctave')
	bg = [236 233 216] / 255;
	
	h = figure( ...
		'ButtonDownFcn', 'if isempty(allchild(gcbf)), close(gcbf), end', ...
		'Colormap', [], ...
		'Color', bg, ...
		'DockControls', 'off', ...
		'HandleVisibility', 'callback', ...
		'IntegerHandle', 'off', ...
		'InvertHardcopy', 'off', ...
		'MenuBar', 'none', ...
		'NumberTitle', 'off', ...
		'PaperPositionMode', 'auto', ...
		'Resize', 'off', ...
		'Visible', 'on', ...
		'WindowStyle', 'modal' ...
		);
else
	bg = get(0, 'DefaultUicontrolBackgroundColor');
	
	h = dialog(varargin{:});
end

end



