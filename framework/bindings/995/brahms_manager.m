%
% brahms_manager
%   The BRAHMS Manager facilitates management of your
%   local SystemML Namespace, including creating new nodes
%   based on component templates, as well as managing your
%   configuration files. Run the manager with no
%   arguments to get started.
%
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
% $Id:: brahms.m 1197 2008-05-25 18:57:54Z benjmitch
% $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function brahms_manager(varargin)

if brahms_utils('IsOctave')
	error('The BRAHMS Manager is not available under Octave - try "help brahms_utils" instead');
end

% options
rebuild = false;
select_execpar = '';
for n = 1:length(varargin)
	arg = varargin{n};
	if length(arg)>8 && strcmp(arg(1:8), 'ExecPar=')
		select_execpar = arg(9:end);
		continue;
	end
	switch arg
		case 'rebuild'
			rebuild = true;

		otherwise
			error(['unrecognised argument "' arg '"']);
	end
end


%% IF GUI EXISTS ALREADY...
set(0, 'ShowHiddenHandles', 'on');
c = get(0, 'children');
set(0, 'ShowHiddenHandles', 'off');
for n = 1:length(c)
	if strcmp(get(c(n), 'Tag'), 'BRAHMS Manager')

		if rebuild
			% delete the existing one so we can rebuild
			delete(c(n))
			break
		else
			% just show the existing one and exit
			figure(c(n));
			return
		end

	end
end



%% GLOBALS
glob = [];
glob.metrics.absentcol = [1 0.8 0.8];
glob.metrics.fontsize = 10;
glob.metrics.fixedfontname = 'Courier New';
glob.metrics.propfontname = 'Tahoma';
glob.metrics.labelheight = 18;
glob.metrics.selheight = 22;
glob.sip = brahms_utils('GetSystemMLInstallPath');


%% CHECK
if ~exist(glob.sip, 'dir')
	error(['SYSTEMML_INSTALL_PATH points at a non-existent directory']);
end




%% ENVIRONMENT
glob.writeable.machine = brahms_utils('IsConfigurationWriteable','machine');
glob.writeable.user = brahms_utils('IsConfigurationWriteable','user');
glob.execpars.install = brahms_utils('GetExecutionParameters', 'install');
glob.execpars.machine = brahms_utils('GetExecutionParameters', 'machine');
glob.execpars.user = brahms_utils('GetExecutionParameters', 'user');
glob.execpars.all = brahms_utils('GetExecutionParameters');
glob.execpars.select = select_execpar;

% CHECK
if isempty(fieldnames(glob.execpars.install))
	error(['failed to read install-level parameters - check SYSTEMML_INSTALL_PATH is set correctly']);
end

% manage NamespaceRoots in the left pane
glob.roots = explode(glob.execpars.all.NamespaceRoots);

% all other Exec Pars are managed
e = fieldnames(glob.execpars.install);
for i = 1:length(e)
	if strcmp(e{i}, 'NamespaceRoots')
		e = e([1:i-1 i+1:end]);
		break
	end
end
glob.pars.managed = e;

% alpha sort them
[temp, i] = sort(glob.pars.managed);
glob.pars.managed = glob.pars.managed(i);


%% BUILD GUI

% figure
glob.col = [149 202 255] / 255;
glob.gui = figure('Visible', 'off', 'Position', [0, 0, 800, 480]);
set(glob.gui, 'Resize', 'off');
set(glob.gui, 'Name', 'BRAHMS Manager');
set(glob.gui, 'MenuBar', 'none');
set(glob.gui, 'NumberTitle', 'off');
set(glob.gui, 'Color', glob.col);
movegui(glob.gui, 'center');
set(glob.gui, 'Tag', 'BRAHMS Manager');

% add panel
glob.panel_nspc = uipanel('Title', 'Namespace Manager');
set(glob.panel_nspc, 'BackgroundColor', glob.col);

% add namespace selector label
glob.nspc_label = label('Configured namespace roots');

% add namespace selector
r = glob.roots;
if isempty(r)
	r = {''};
end

glob.nspc = selector(r, @callback_nspc);
glob.nspc_add = button('Add', @callback_nspc_add);
glob.nspc_del = button('Remove', @callback_nspc_del);

% add namespace list label
glob.list_label_str = 'Components under selected root';
glob.list_label = label(glob.list_label_str);

% add namespace list box
glob.list = uicontrol('Style', 'list');
set(glob.list, 'FontSize', glob.metrics.fontsize);
set(glob.list, 'FontName', glob.metrics.fixedfontname);

% add list buttons
glob.list_refresh = button('Refresh', @callback_list_refresh);
glob.list_new = button('New', @callback_list_new);
glob.list_open = button('Open', @callback_list_open);
glob.list_build = button('Build', @callback_list_build);
glob.list_test = button('Test', @callback_list_test);
glob.list_cancel = button('Cancel', @callback_list_cancel);
set(glob.list_cancel, 'Enable', 'off');

% add panel
glob.panel_pars = uipanel('Title', 'Parameters Manager');
set(glob.panel_pars, 'BackgroundColor', glob.col);

% add parameter management

% key selector
glob.pars.key.label = label('Parameter name');
glob.pars.key.selector = selector(glob.pars.managed, @callback_par_changekey);

% install-level
glob.pars.install.label = label('Install-level value (can''t change with this tool)');
glob.pars.install.value = inactivetext('');
glob.pars.install.copy = button('Copy', @callback_par_copy_install);

% machine-level
glob.pars.machine.label = label('Machine-level value (pink means "not set")');
glob.pars.machine.value = activetext('');
glob.pars.machine.save = button('Save', @callback_par_save_machine);
glob.pars.machine.delete = button('Unset', @callback_par_del_machine);
glob.pars.machine.disabled = label(glob.writeable.machine);
set(glob.pars.machine.disabled, 'Visible', 'off');
if ~isempty(glob.writeable.machine)
	set(glob.pars.machine.label, 'String', 'Machine-level value (config file not writeable):');
	set(glob.pars.machine.label, 'ForegroundColor', 0.6*[1 1 1]);
	set(glob.pars.machine.disabled, 'ForegroundColor', 0.6*[1 1 1]);
	set(glob.pars.machine.disabled, 'Visible', 'on');
	set(glob.pars.machine.value, 'Visible', 'off');
	set(glob.pars.machine.save, 'Visible', 'off');
	set(glob.pars.machine.delete, 'Visible', 'off');
end

% user-level
glob.pars.user.label = label('User-level value (pink means "not set")');
glob.pars.user.value = activetext('');
glob.pars.user.save = button('Save', @callback_par_save_user);
glob.pars.user.delete = button('Unset', @callback_par_del_user);
glob.pars.user.disabled = label(glob.writeable.user);
set(glob.pars.user.disabled, 'Visible', 'off');
if ~isempty(glob.writeable.user)
	set(glob.pars.user.label, 'String', 'Machine-level value (config file not writeable):');
	set(glob.pars.user.label, 'ForegroundColor', 0.6*[1 1 1]);
	set(glob.pars.user.disabled, 'ForegroundColor', 0.6*[1 1 1]);
	set(glob.pars.user.disabled, 'Visible', 'on');
	set(glob.pars.user.value, 'Visible', 'off');
	set(glob.pars.user.save, 'Visible', 'off');
	set(glob.pars.user.delete, 'Visible', 'off');
end

% update par display
callback_par_changekey(0, 0)

% add review button
glob.pars.review_label = label('Values at each level override earlier levels');
glob.pars.review = button('View effective parameters', @callback_par_review);

% % add XML access buttons
% glob.xml_label = label('Edit config files manually');
% glob.xml_machine = button('Machine-level', @callback_xml_machine);
% glob.xml_user = button('User-level', @callback_xml_user);

% add BRAHMS Manager help button
glob.man_help = button('Launch Documentation', @callback_man_help);

% get button color
glob.buttoncol = get(glob.man_help, 'BackgroundColor');

% call resize to do layout
drawnow
resize

% refresh items
refresh_list

% make the GUI visible.
set(glob.gui,'Visible','on');
set(glob.gui, 'HandleVisibility', 'off')
drawnow




%% AUTO LAUNCH

if isempty(glob.execpars.all.WorkingDirectory)
	select_par('WorkingDirectory');
	advise([glob.pars.user.save], 'orange', 'Your Working Directory is not set - please set it now.');
elseif ~exist(glob.execpars.all.WorkingDirectory, 'dir')
	select_par('WorkingDirectory');
	advise([glob.pars.user.save], 'orange', 'Your Working Directory is set to a non-existent directory - please change it now.');
elseif ~isempty(select_execpar)
	select_par(select_execpar);
	advise([glob.pars.user.save], 'orange', 'The Execution Parameter you should modify is selected.');
end

if ~length(glob.roots)
	% 	advise(glob.nspc_add, 'red', 'No User Namespace Roots are configured - please add one now.');
else
	for n = 1:length(glob.roots)
		root = glob.roots{n};
		if ~exist(root, 'dir')
			advise(glob.nspc_del, 'red', 'At least one of your User Namespace Roots is set to a non-existent directory - please correct these now.');
		end
	end
end







%% DIALOGS

	function info(msg)

		warn(msg)

	end

	function warn(msg)

		% create and get children
		w = warndlg(msg, 'Warning', 'modal');

		% DON't because it causes an unsightly motion artefact
		% 		% position it centrally to manager
		% 		set(w, 'units', 'pixels');
		% 		mp = get(glob.gui, 'position');
		% 		wp = get(w, 'position');
		% 		xy =  mp(1:2);
		% 		dxy = mp(3:4) - wp(3:4);
		% 		xy = xy + 0.5 * dxy;
		% 		set(w, 'position', [xy wp(3:4)]);
		% 		drawnow

		% wait for it to close
		uiwait(w);

	end

	function recolor(h)

		set(h, 'BackgroundColor', glob.buttoncol);

	end

%% ADVISE

	function advise(h, col, msg)

		% color item
		switch col
			case 'orange'
				set(h, 'BackgroundColor', [255 100 0]/255);
			case 'red'
				set(h, 'BackgroundColor', col);
		end

		% give advice
		msg = [msg 10 10 'The button to perform this operation has been coloured ' col '.' 10 10 'If you need help with this, please click on the "Launch Documentation" button in the bottom right of the Manager window.'];
		info(msg);

	end



%% RESIZE GUI

	function resize

		% get client rectangle size
		sz = get(glob.gui, 'Position');
		sz = sz(3:4);

		% do margins (client is [x y w glob])
		margin = 8;
		wdt = glob.metrics.selheight * 3;
		client = [margin margin sz-2*margin];

		% brahms man help button
		[client, rect] = bottom(client, glob.metrics.selheight, margin);
		wdt_ = wdt * 3;
		[rect, brect] = right(rect, wdt_, margin);
		set(glob.man_help, 'Position', brect);

		% split left/right for namespace/other
		[client, rclient] = right(client, 0.5*client(3), margin);

		% do panel
		set(glob.panel_nspc, 'Units', 'pixels')
		set(glob.panel_nspc, 'Position', client);
		client = client + margin*[1 1 -2 -4];

		% reposition nspc label
		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.nspc_label, 'Position', rect);

		% reposition nspc selector
		[client, subclient] = top(client, glob.metrics.selheight, margin);
		[subclient, rect] = right(subclient, wdt, margin);
		set(glob.nspc_del, 'Position', rect);
		[subclient, rect] = right(subclient, wdt, margin);
		set(glob.nspc_add, 'Position', rect);
		set(glob.nspc, 'Position', subclient);

		% reposition list label
		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.list_label, 'Position', rect);

		% reposition list buttons
		hgt = glob.metrics.selheight;
		[client, subclient] = right(client, wdt, margin);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_refresh, 'Position', rect);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_cancel, 'Position', rect);
		[subclient, rect] = top(subclient, hgt, margin);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_new, 'Position', rect);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_open, 'Position', rect);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_build, 'Position', rect);
		[subclient, rect] = top(subclient, hgt, margin);
		set(glob.list_test, 'Position', rect);

		% reposition list
		set(glob.list, 'Position', client);

		% do right hand side
		client = rclient;

		% do panel
		set(glob.panel_pars, 'Units', 'pixels')
		set(glob.panel_pars, 'Position', client);
		client = client + margin*[1 1 -2 -4];

		% execution parameters
		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.pars.key.label, 'Position', rect);
		[client, rect] = top(client, glob.metrics.selheight, margin);
		set(glob.pars.key.selector, 'Position', rect);

		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.pars.install.label, 'Position', rect);
		[client, rect] = top(client, glob.metrics.selheight, margin);
		[lrect, rrect] = right(rect, wdt, margin);
		set(glob.pars.install.value, 'Position', lrect);
		set(glob.pars.install.copy, 'Position', rrect);

		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.pars.machine.label, 'Position', rect);
		[client, rect] = top(client, glob.metrics.selheight, margin);
		set(glob.pars.machine.disabled, 'Position', rect);
		[lrect, rrect] = right(rect, wdt, margin);
		set(glob.pars.machine.save, 'Position', rrect);
		[lrect, rrect] = right(lrect, wdt, margin);
		set(glob.pars.machine.delete, 'Position', rrect);
		set(glob.pars.machine.value, 'Position', lrect);

		[client, rect] = top(client, glob.metrics.labelheight, margin);
		set(glob.pars.user.label, 'Position', rect);
		[client, rect] = top(client, glob.metrics.selheight, margin);
		set(glob.pars.user.disabled, 'Position', rect);
		[lrect, rrect] = right(rect, wdt, margin);
		set(glob.pars.user.save, 'Position', rrect);
		[lrect, rrect] = right(lrect, wdt, margin);
		set(glob.pars.user.delete, 'Position', rrect);
		set(glob.pars.user.value, 'Position', lrect);

		[client, rect] = bottom(client, glob.metrics.selheight, margin);
		set(glob.pars.review, 'Position', rect);
		[client, rect] = bottom(client, glob.metrics.labelheight, margin);
		set(glob.pars.review_label, 'Position', rect);

		% 		% reposition config files
		% 		[client, rect] = top(client, glob.metrics.labelheight, margin);
		% 		set(glob.xml_label, 'Position', rect);
		% 		[client, subclient] = top(client, glob.metrics.selheight, margin);
		% 		wdt = (subclient(3) - 1 * margin) / 2;
		% 		[subclient, rect] = left(subclient, wdt, margin);
		% 		set(glob.xml_user, 'Position', rect);
		% 		[subclient, rect] = left(subclient, wdt, margin);
		% 		set(glob.xml_machine, 'Position', rect);
		% 		[subclient, rect] = right(subclient, wdt, margin);
		% 		set(glob.xml_install, 'Position', rect);

	end

	function [c, r] = top(c, u, m)

		% use u pixels of rectangle c (from top), plus m pixels
		% for margin, then return the remaining c and the used r
		r = [c(1) c(2)+c(4)-u c(3) u];
		c(4) = c(4) - u - m;

	end

	function [c, r] = bottom(c, u, m)

		% use u pixels of rectangle c (from bottom), plus m pixels
		% for margin, then return the remaining c and the used r
		r = [c(1) c(2) c(3) u];
		c(2) = c(2) + u + m;
		c(4) = c(4) - u - m;

	end

	function [c, r] = left(c, u, m)

		% use u pixels of rectangle c (from left), plus m pixels
		% for margin, then return the remaining c and the used r
		r = [c(1) c(2) u c(4)];
		c(1) = c(1) + u + m;
		c(3) = c(3) - u - m;

	end

	function [c, r] = right(c, u, m)

		% use u pixels of rectangle c (from right), plus m pixels
		% for margin, then return the remaining c and the used r
		r = [c(1)+c(3)-u c(2) u c(4)];
		c(3) = c(3) - u - m;

	end



%% NAMESPACE CALLBACKS

	function callback_nspc(hObj, eventData)

		% refresh list
		refresh_list

	end

	function callback_nspc_add(hObj, eventData)

		% display UI
		if isunix
			path = '~';
		else
			path = getenv('USERPROFILE');
		end
		msg = 'Select new Namespace Root Path to add';
		path = uigetdir(path, msg);

		% cancel?
		if ~ischar(path)
			return
		end

		% otherwise, set it
		glob.roots{end+1} = path;

		% update GUI
		changed_roots(length(glob.roots));

		% set exec par
		glob.execpars.NamespaceRoots = implode(glob.roots);
		brahms_utils('SetExecutionParameter', 'user', 'NamespaceRoots', glob.execpars.NamespaceRoots);

		% recolor button
		recolor(glob.nspc_add);

	end

	function callback_nspc_del(hObj, eventData)

		% get namespace
		[root, index] = getSelectedNamespaceRoot();
		if isempty(root) return; end
		% confirm
		if ~confirm(['Are you sure you want to remove this Namespace Root Path?' 10 10 root])
			return
		end

		% remove from roots
		glob.roots = glob.roots([1:index-1 index+1:end]);

		% update GUI
		changed_roots(0);

		% set exec par
		glob.execpars.NamespaceRoots = implode(glob.roots);
		brahms_utils('SetExecutionParameter', 'user', 'NamespaceRoots', glob.execpars.NamespaceRoots);

		% recolor button
		recolor(glob.nspc_del);

	end



%% LIST CALLBACKS

	function callback_list_refresh(hObj, eventData)

		% refresh list
		refresh_list

	end

	function callback_list_new(hObj, eventData)

		% get namespace root
		root = getSelectedNamespaceRoot();
		if isempty(root) return; end

		% delegate to brahms_utils
		disp(['---- brahms_manager executes: brahms_utils CreateNewComponent ''' root '''']);
		cls = brahms_utils('CreateNewComponent', root);

		% update list
		if ~isempty(cls)
			refresh_list(cls);
		end

	end

	function callback_list_open(hObj, eventData)

		% get node
		cls = getSelectedNodeClass();
		if isempty(cls) return; end

		% delegate to brahms_utils
		disp(['---- brahms_manager executes: brahms_utils OpenClassReleasePath ''' cls '''']);
		brahms_utils('OpenClassReleasePath', cls)

	end

	function callback_list_build(hObj, eventData)

		% get node
		cls = getSelectedNodeClass;
		if isempty(cls) return; end

		% delegate to brahms_utils
		disp(['---- brahms_manager executes: brahms_utils RunBuildScript ''' cls '''']);
		brahms_utils('RunBuildScript', cls);

	end

	function callback_list_test(hObj, eventData)

		% get node
		cls = getSelectedNodeClass;
		if isempty(cls) return; end

		% delegate to brahms_utils
		disp(['---- brahms_manager executes: brahms_utils RunTestScript ''' cls '''']);
		brahms_utils('RunTestScript', cls);

	end

	function callback_list_cancel(a, b)

		set(glob.list_cancel, 'Enable', 'off');

	end



%% PARAMETER CALLBACKS

	function select_par(key)

		for i = 1:length(glob.pars.managed)
			if strcmp(glob.pars.managed{i}, key)
				set(glob.pars.key.selector, 'Value', i);
				callback_par_changekey(0, 0);
				break
			end
		end

	end

	function callback_par_changekey(hObj, eventData)

		% update value box
		v = get(glob.pars.key.selector, 'Value');
		key = glob.pars.managed{v};
		pars = glob.execpars;

		% absent or present?
		settext(glob.pars.install.value, pars.install, key, true);
		settext(glob.pars.machine.value, pars.machine, key, false);
		settext(glob.pars.user.value, pars.user, key, false);

		% for some values, save buttons become browse buttons
		if strcmp(key, 'WorkingDirectory')
			set(glob.pars.machine.save, 'String', '...');
			set(glob.pars.machine.value, 'Enable', 'inactive');
			set(glob.pars.user.save, 'String', '...');
			set(glob.pars.user.value, 'Enable', 'inactive');
		else
			set(glob.pars.machine.save, 'String', 'Save');
			set(glob.pars.machine.value, 'Enable', 'on');
			set(glob.pars.user.save, 'String', 'Save');
			set(glob.pars.user.value, 'Enable', 'on');
		end

	end

	function settext(h, s, key, indicateEmptyString)

		if isfield(s, key)
			v = s.(key);
			if isempty(v) && indicateEmptyString
				v = '<empty string>';
			end
			set(h, 'String', v);
			set(h, 'BackgroundColor', 'white');
		else
			set(h, 'String', '');
			set(h, 'BackgroundColor', glob.metrics.absentcol);
		end

	end

	function v = getfieldornull(s, k)

		% return field or empty string
		if isfield(s, k)
			v = s.(k);
		else
			v = '';
		end

	end

	function s = rmfieldorignore(s, k)

		% return field or empty string
		if isfield(s, k)
			s = rmfield(s, k);
		end

	end

	function callback_par_save_machine(a, b)

		callback_par_save('machine');

	end

	function callback_par_save_user(a, b)

		callback_par_save('user');

	end

	function callback_par_copy_install(a, b)

		s = get(glob.pars.install.value, 'String');
		clipboard('copy', s);

	end

	function callback_par_save(level)

		% get key and src
		v = get(glob.pars.key.selector, 'Value');
		key = glob.pars.managed{v};

		% for some values, save buttons become browse buttons
		if strcmp(key, 'WorkingDirectory')

			% display UI
			path = getfieldornull(glob.execpars.(level), 'WorkingDirectory');
			if isempty(path) || ~exist(path, 'dir')
				if isunix
					path = '~';
				else
					path = getenv('USERPROFILE');
				end
			end
			msg = 'Select Working Directory';
			path = uigetdir(path, msg);

			% cancel?
			if ischar(path)

				% set it
				brahms_utils('SetExecutionParameter', level, key, path);
				glob.execpars.(level).(key) = path;

				% recolor button
				recolor(glob.pars.machine.save);
				recolor(glob.pars.user.save);

			end

		else

			% get new value
			value = get(glob.pars.(level).value, 'String');

			% set (locally and on disc)
			brahms_utils('SetExecutionParameter', level, key, value);
			glob.execpars.(level).(key) = value;

		end

		% refresh
		callback_par_changekey(0, 0)

	end

	function callback_par_del_machine(a, b)

		callback_par_del('machine');

	end

	function callback_par_del_user(a, b)

		callback_par_del('user');

	end

	function callback_par_del(level)

		% get key and src
		v = get(glob.pars.key.selector, 'Value');
		key = glob.pars.managed{v};

		% set (locally and on disc)
		brahms_utils('DeleteExecutionParameter', level, key);
		glob.execpars.(level) = rmfieldorignore(glob.execpars.(level), key);

		% refresh
		callback_par_changekey(0, 0)

	end

	function callback_par_review(a, b)

		% review effective parameters
		disp(['________________________________________________________________' 10]);
		disppars('Execution Parameters', brahms_utils('GetExecutionParameters'));
		disp(['________________________________________________________________' 10]);

	end

	function disppars(label, pars)

		f = fieldnames(pars);
		disp(label)
		disp(' ')
		L = 0;
		for n = 1:length(f)
			L = max(L, length(f{n}));
		end
		L = L + 2;
		[temp, i] = sort(f);
		for n = i'
			disp([repmat(' ', [1 L-length(f{n})]) f{n} ': ' pars.(f{n})])
		end

	end

%% OTHER CALLBACKS

	function callback_man_help(hObj, eventData)

		% display docs
		brahms_utils('OpenDocumentation', 'BRAHMS Manager');

	end

% 	function callback_xml_install(hObj, eventData)
%
% 		% warn
% 		warn(['You should not edit this file (your install-level config file), unless you have a really good reason. Modify either the machine-level or user-level configuration file instead.' 10 10 'It may be useful to copy parameters out of it, however, so you can place them in your other config files and modify them.']);
%
% 		% open
% 		[path, err] = brahms_utils('GetConfigFilePath', 'install', true);
% 		if ~isempty(err)
% 			warn(['Cannot edit that config file:' 10 10 err]);
% 		else
% 			launchEditor(path);
% 		end
%
% 	end

% 	function callback_xml_machine(hObj, eventData)
%
% 		% open
% 		[path, err] = brahms_utils('GetConfigFilePath', 'machine', true);
% 		if ~isempty(err)
% 			warn(['Cannot edit that config file:' 10 10 err]);
% 		else
% 			launchEditor(path);
% 		end
%
% 	end
%
% 	function callback_xml_user(hObj, eventData)
%
% 		% open
% 		[path, err] = brahms_utils('GetConfigFilePath', 'user', true);
% 		if ~isempty(err)
% 			warn(['Cannot edit that config file:' 10 10 err]);
% 		else
% 			launchEditor(path);
% 		end
%
% 	end

% function launchEditor(filename)
%
% edit(filename)
%
% end




%% HELPERS

	function changed_roots(sel)

		% and update the GUI
		if length(glob.roots)
			set(glob.nspc, 'String', glob.roots);
		else
			set(glob.nspc, 'String', {''});
		end

		% which to select?
		if sel
			set(glob.nspc, 'Value', sel);
		else
			set(glob.nspc, 'Value', 1);
		end

		% update GUI
		refresh_list

	end

	function refresh_list(cls)

		% get namespace
		root = getSelectedNamespaceRoot(false);
		if isempty(root)

			% clear list
			set(glob.list, 'String', {});
			return
		end

		% if no selected cls specified, use current
		if ~nargin
			cls = get(glob.list, 'String');
			ind = get(glob.list, 'Value');
			if isempty(cls) || isempty(ind)
				cls = '';
			else
				cls = cls{ind};
			end
		end

		% get nodes under root
		set(glob.list_cancel, 'Enable', 'on');
		nodes = findnodes(root, '', glob.list_label, glob.list_cancel);
		set(glob.list_label, 'string', glob.list_label_str);
		set(glob.list_cancel, 'Enable', 'off');

		% lay into list
		set(glob.list, 'String', nodes);

		% select correct entry
		for n = 1:length(nodes)
			if strcmp(nodes{n}, cls)
				set(glob.list, 'Value', n);
				return
			end
		end

		% or, just select first
		if length(nodes)
			set(glob.list, 'Value', 1);
		end

	end





%% UI CTORS

	function h = selector(string, callback)

		h = uicontrol('Style', 'popupmenu', 'String', string);
		set(h, 'Callback', callback);
		set(h, 'FontSize', glob.metrics.fontsize);
		set(h, 'FontName', glob.metrics.fixedfontname);
		set(h, 'BackgroundColor', 'white');

	end

	function h = button(label, callback)

		h = uicontrol('Style', 'pushbutton', 'String', label);
		set(h, 'Callback', callback);
		set(h, 'FontSize', glob.metrics.fontsize);
		set(h, 'FontName', glob.metrics.propfontname);

	end

	function h = label(text, fixedwidth)

		h = uicontrol('Style', 'text', 'String', text);
		set(h, 'FontSize', glob.metrics.fontsize);
		if nargin>1 && fixedwidth
			set(h, 'FontName', glob.metrics.fixedfontname);
		else
			set(h, 'FontName', glob.metrics.propfontname);
		end
		set(h, 'Horiz', 'left');
		set(h, 'BackgroundColor', glob.col);

	end

	function h = inactivetext(text)

		h = uicontrol('Style', 'edit', 'String', text);
		set(h, 'FontSize', glob.metrics.fontsize);
		set(h, 'FontName', glob.metrics.fixedfontname);
		set(h, 'Horiz', 'left');
		set(h, 'Enable', 'off')
		set(h, 'BackgroundColor', 'white');

	end

	function h = activetext(text)

		h = uicontrol('Style', 'edit', 'String', text);
		set(h, 'FontSize', glob.metrics.fontsize);
		set(h, 'FontName', glob.metrics.fixedfontname);
		set(h, 'Horiz', 'left');
		set(h, 'BackgroundColor', 'white');

	end



%% UI HELPERS

	function [root, index] = getSelectedNamespaceRoot(do_warn)

		root = [];
		index = 0;
		if ~nargin
			do_warn = true;
		end

		% check we have some
		if ~length(glob.roots)
			if do_warn
				warn('You need to add a Namespace Root before you can do this.');
			end
			return
		end

		% get the currently selected Namespace root
		index = get(glob.nspc, 'Value');
		root = glob.roots{index};

	end

	function cls = getSelectedNodeClass

		cls = [];

		% check we have something
		list = get(glob.list, 'String');
		if isempty(list)
			warn('You need to add a Process to this Namespace before you can do this.');
			return
		end

		% get the currently selected node
		index = get(glob.list, 'Value');
		cls = list{index};

	end

% 	function data = getSelectedNode
%
% 		data = [];
%
% 		% get root
% 		root = getSelectedNamespaceRoot();
% 		if isempty(root) return; end
%
% 		% check we have some
% 		list = get(glob.list, 'String');
% 		if isempty(list)
% 			warn('You need to add a Process to this Namespace before you can do this.');
% 			return
% 		end
%
% 		% get the currently selected node
% 		index = get(glob.list, 'Value');
%
% 		% lay in data
% 		data.path.root = root;
% 		data.cls = list{index};
% 		data.path.node = [root '/' data.cls];
%
% 		% get release directory path
% 		data.path.release = [data.path.node '/brahms/imp'];
% 		d = dir(data.path.release);
% 		d = d(3:end);
%
% 		% get first matching implementation
% 		imps = {'1199' '1065' '1266' '1262' '1258'};
% 		imp = '';
% 		for n = 1:length(imps)
% 			for i = 1:length(d)
% 				if strcmp(d(i).name, imps{n})
% 					imp = imps{n};
% 					break
% 				end
% 			end
% 			if ~isempty(imp)
% 				break
% 			end
% 		end
% 		if isempty(imp)
% 			error('no implementations on node');
% 		end
%
% 		data.lang = imp;
% 		data.path.release = [data.path.release '/' data.lang];
% 		d = dir(data.path.release);
% 		d = d(3:end);
%
% 		% get first matching release folder
% 		release = '';
% 		for i = 1:length(d)
% 			if strcmp(d(i).name, '0')
% 				release = d(i).name;
% 				break;
% 			end
% 		end
% 		if isempty(release)
% 			error('no releases on node');
% 		end
%
% 		data.release = release;
% 		data.path.release = [data.path.release '/' data.release];
%
% 		% fix slashes
% 		if ~isunix
% 			data.path.release(data.path.release == '/') = '\';
% 		end
%
% 	end
%
%
%
% end







%% HELPERS


	function e = explode(s)

		% explode according to environment variable separator for
		% the environment
		if isunix
			sep = ':';
		else
			sep = ';';
		end

		f = find(s == sep);
		l = [1 f+1];
		r = [f-1 length(s)];
		e = {};
		for n = 1:length(l)
			ss = s(l(n):r(n));
			if ~isempty(ss)
				e{end+1} = ss;
			end
		end


	end




	function s = implode(e)

		% implode according to environment variable separator for
		% the environment
		if isunix
			sep = ':';
		else
			sep = ';';
		end

		s = '';
		for n = 1:length(e)
			if length(s)
				s = [s sep];
			end
			s = [s e{n}];
		end


	end




	function c = confirm(q)

		a = questdlg(q, 'Please confirm', 'Yes', 'No', 'No');
		c = strcmp(a, 'Yes');

	end





	function nodes = findnodes(root, subpath, h_feedback, h_cancel)

		set(h_feedback, 'string', ['...' subpath]);
		drawnow

		% if we're a leaf, just return us!
		nodexml = [root subpath  '/node.xml'];
		if exist(nodexml, 'file')

			xml = sml_xml('file2xml', nodexml);

			type = '';
			for c = 1:length(xml.children)
				if strcmp(xml.children(c).name, 'Type')
					type = xml.children(c).value;
					break
				end
			end
			if isempty(type)
				error(['invalid node file "' nodexml '"']);
			end
			switch type
				case {'Process' 'Data' 'Utility' 'Leaf'}
					nodes = {subpath(2:end)};
					return
				case 'Branch'
				otherwise
					error(['invalid node file "' nodexml '"']);
			end
		end

		% otherwise, search sub-folders and collect from them
		nodes = {};
		d = dir([root subpath]);
		for n = 1:length(d)

			if ~strcmp(get(h_cancel, 'Enable'), 'on')
				return
			end

			if d(n).name(1) == '.' continue; end
			if ~d(n).isdir continue; end
			subnodes = findnodes(root, [subpath '/' d(n).name], h_feedback, h_cancel);
			nodes = cat(1, nodes, subnodes);
		end

	end








end


