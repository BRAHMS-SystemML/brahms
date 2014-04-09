%
% "brahms" is the main entry point to use BRAHMS from within
% Matlab. The most common usage is the first listed below,
% which will invoke an execution and return the results, all
% in one call. If you need more control (to use a debugger
% during execution, to otherwise run an execution manually,
% to collect the results of an older execution) you can
% perform each of the three sub-tasks of the invocation
% separately (next three usage forms).
%
% [out, rep, inv, sys] = brahms(system, execution, ...)
%   This form will 'prep', 'exec' and 'collect' an
%   invocation; that is, it will run BRAHMS and return the
%   results of the execution. "system" can be an sml_system
%   object, or the filename of an existing System File.
%   "execution" should be a brahms_execution object.
%   Additional arguments are passed to the bindings, if
%   recognised, or to the executable as command-line
%   arguments, if not (pass options to BRAHMS here). Options
%   recognised by the bindings are listed below. The
%   invocation is returned as "inv". If the
%   fourth output argument, "sys", is asked for, the final
%   state of the system is returned.
%
% invocation = brahms('prep', system, execution[, invocation])
%   This form prepares an execution (writes the System and
%   Execution files into the WorkingDirectory) and returns
%   the invocation object (to pass to the next form, below).
%   Optionally, pass an invocation structure with options
%   for the bindings in the form invocation.opts.key = value.
%   If you do not pass an invocation, a default one is
%   created for you.
%
% invocation = brahms('exec', invocation, ...)
%   This form invokes a pre-prepared execution, waits for it
%   to finish, and returns the updated invocation object (to
%   pass to the next form, below). Additional arguments are
%   passed to the bindings, if recognised, or to the
%   executable as command-line arguments, if not (pass
%   options to BRAHMS here).
%
% [invocation, out, rep, sys] = brahms('collect', invocation)
%   This form collects and returns the results of a
%   completed execution, and also returns the updated
%   invocation object.
%
% [invocation, out, rep, sys] = brahms('collect', <execution filename>)
%   This form collects and returns the results of a
%   completed execution given only the filename of the
%   Execution File. The Report Files are expected to be
%   found alongside the Execution File (rather than in the
%   WorkingDirectory). This is useful for importing results
%   into Matlab that you have been sent by another user. If
%   you pass a relative filename, it will be sought in the
%   current directory first, then the WorkingDirectory.
%
% brahms
%   This form (no arguments) will report the status of your
%   BRAHMS installation.
%
% brahms(string, string, ...)
%   This form (any series of string arguments) will call the
%   BRAHMS wrapper script directly, passing all string
%   arguments to it directly.
%
% If you pass the argument "--soak" to this form, it
% will not be passed to the executable. Instead, the
% executable will be called as normal, but if an error
% occurs, it will be returned as the first output argument
% (which will, thus, be a string rather than a structure).
%
% Options recognised by the bindings:
%
% --par-ShowGUI=0
%   This is passed on to BRAHMS, but also causes the
%   Concerto Execution Monitor GUI to be hidden by the
%   bindings.
%
% --show-cmd
%   Display all system calls made.
%
% --keep-redirect
%   Do not delete redirect (stdout/stderr) files.
%
%
% __________________________________________________________
% This function is part of the "BRAHMS Matlab Bindings".

% --noeviscerate:
%   Do not shorten the log for display if it is over-long.
%

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
% $Id:: brahms.m 2417 2009-11-30 11:34:31Z benjmitch                     $
% $Rev:: 2417                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-30 11:34:31 +0000 (Mon, 30 Nov 2009)                   $
%__________________________________________________________________________
%






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function varargout = brahms(varargin)

% handle error brahms:abort_execution
try

	% "brahms" with no args becomes "brahms ''" to force the
	% correct call form to be used (direct call to executable)
	if ~nargin
		varargin = {''};
	end

	% default inv
	inv = [];
	inv.opts.keepRedirect = false;
	inv.opts.waitForFinish = false;
	inv.opts.ShowCMD = false;
	inv.opts.ShowGUI = true;
	inv.opts.allAtOnce = false; % allows us to keep the GUI open between calls, if true



	% DECIDE ON WHICH FORM WAS CALLED

	% main form
	if (isa(varargin{1}, 'sml_system') || ischar(varargin{1})) && ...
			nargin >= 2 && isa(varargin{2}, 'brahms_execution')

		exe = varargin{2};

		% split arguments into bindings and executable
		args = {};
		inv.opts.allAtOnce = true; % allows us to keep the GUI open
		for a = 3:length(varargin)
			switch(varargin{a})
% 				case '--noeviscerate'
% 					inv.opts.eviscerate = false;
				case '--keep-redirect'
					inv.opts.keepRedirect = true;
				case '--show-cmd'
					inv.opts.ShowCMD = true;
				otherwise
					if strcmp(varargin{a}, '--par-ShowGUI-0') || strcmp(varargin{a}, '--par-ShowGUI=0')
						inv.opts.ShowGUI = false;
					end
					args{end+1} = varargin{a};
			end
		end

		% show GUI
		if strcmp(exe.execPars.ShowGUI, '0')
			inv.opts.ShowGUI = false;
		end
		brahms_gui('show', inv.opts.ShowGUI);

		% if output system was requested, and exe is not already
		% set in this regard, modify it
		if nargout > 2 && isempty(exe.systemFileOut)
			exe.systemFileOut = ''; % cause default file name to be used
		end

		% make all three calls since "allAtOnce" is true, the GUI
		% won't be destroyed/created across the call boundaries,
		% giving a slicker result
		inv = brahms_prep(inv, varargin{1}, exe);
		inv = brahms_exec(inv, args{1:end});

		% only ask for system if requested by caller
		if nargout > 3
			[varargout{3}, varargout{1}, varargout{2}, varargout{4}] = brahms_collect(inv);
		else
			[varargout{3}, varargout{1}, varargout{2}] = brahms_collect(inv);
		end

	elseif ischar(varargin{1})

		% switch
		switch varargin{1}

			case 'prep'
				if nargin > 3
					inv = varargin{4};
				else
					% use default inv, from above
				end
				exe = varargin{3};
				if strcmp(exe.execPars.ShowGUI, '0')
					inv.opts.ShowGUI = false;
				end

			case {'exec' 'collect'}
				inv = varargin{2};

			otherwise
				txt = txtargs(varargin);
				cmd = ['brahms ' txt];
				invoke(cmd);
				return

		end

		% show GUI
		if isstruct(inv)
			brahms_gui('show', inv.opts.ShowGUI);
		end

		% switch
		switch varargin{1}

			case 'prep'
				varargout{1} = brahms_prep(inv, varargin{2}, exe);

			case 'exec'
				varargout{1} = brahms_exec(inv, varargin{3:end});

			case 'collect'
				% only ask for system if requested by caller
				if nargout > 3
					[varargout{1}, varargout{2}, varargout{3}, varargout{4}] = brahms_collect(inv);
				else
					[varargout{1}, varargout{2}, varargout{3}] = brahms_collect(inv);
				end

		end

	else

		% error
		error('unrecognised call form - see "help brahms"');

	end




	% handle error brahms:abort_execution
catch

	% close GUI (if open)
	brahms_gui('destroy');

	% handle abort_execution
	err = lasterror;
	if strcmp(err.identifier, 'brahms:abort_execution')
		% neater than showing the whole stack
		error(err.message);
	end
	if strcmp(err.identifier, 'brahms:error_in_executable')
		% neater than showing the whole stack
		error('One or more errors occurred during execution');
	end
	if strcmp(err.identifier, 'brahms:error_in_launch_line')
		% neater than showing the whole stack
		error('One or more errors occurred during launch');
	end

	% ok
	error(err);

end











%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% PREPARE INVOCATION

function invocation = brahms_prep(invocation, sys, exe)

% GUI
brahms_gui('phase', 'Preparing Execution');
invocation.callbackProgress = brahms_gui('get_callback_progress');

% performance
t0 = clock;

% exec pars
invocation.execpars = brahms_utils('GetExecutionParameters');

% pre-amble
if isempty(which('sml_utils'))
	error('SystemML Toolbox not installed');
end

% write system file using SystemML Toolbox Matlab Bindings
if isa(sys, 'sml_system')

	% write execution file using BRAHMS Matlab Bindings
	brahms_gui('operation', 'Writing Execution File');
	wd = exe.serialize();

	% if path is relative, we must append
	path = exe.systemFileIn;
	d = fileparts(path);
	if isempty(d)
		path = [wd filesep path];
	end

	% serialize
	writer = exe.write;
	writer.callbackProgress = invocation.callbackProgress;
	sys.serialize(path, writer);

else

	% attach system file
	brahms_gui('operation', 'Attaching to System File');
	wd = fileparts(sys);
	if isempty(wd)
		wd = cd;
	end
	exe.systemFileIn = sys;
	if ~exist(exe.systemFileIn, 'file')
		error(['file not found "' exe.systemFileIn '"']);
	end

	% write execution file
	brahms_gui('operation', 'Writing Execution File');
	exe.serialize();

end

% performance
t1 = clock;
invocation.perf.serialize = etime(t1, t0);

% gui
brahms_gui('operation', 'Finalizing Execution');

% prepare files with absolute paths
invocation.file.report = exe.reportFile;
if isempty(fileparts(invocation.file.report))
	invocation.file.report = [wd filesep invocation.file.report];
end
invocation.file.out = exe.outFile;
if isempty(fileparts(invocation.file.out))
	invocation.file.out = [wd filesep invocation.file.out];
end
invocation.file.log = [invocation.file.out '.xml'];
invocation.file.exit = [invocation.file.out '.exit'];
invocation.file.redirect = [invocation.file.log '.redirect'];

% augment invocation
invocation.exe = exe;

% prepare launch line
launch = exe.launch;
invocation.launchfunc = [];
if isa(launch, 'function_handle')
	invocation.launchfunc = launch;
	exe.launch = '';
	info = invocation.launchfunc('info', invocation);
	invocation.mode = info.mode;
	invocation.launch = exe.launch; % provide default launch line, for the function's use
	invocation.launch = prepLaunchLine(invocation.launch, invocation);
elseif iscell(launch)
	invocation.mode = 'asynchronous';
	for c = 1:length(launch)
		invocation.launch{c} = prepLaunchLine(launch{c}, invocation);
	end
else
	invocation.mode = 'synchronous';
	if length(launch) >= 5 && strcmp(launch(1:5), 'each ')
		invocation.mode = 'asynchronous';
		launch = launch(6:end);
	end
	invocation.launch = prepLaunchLine(launch, invocation);
end

% clear files, so we don't misinterpret them as having
% been created
for n = 1:exe.voices
	safedel(strrep(invocation.file.report, '((VOICE))', int2str(n)));
	safedel(strrep(invocation.file.log, '((VOICE))', int2str(n)));
	safedel(strrep(invocation.file.exit, '((VOICE))', int2str(n)));
	safedel(strrep(invocation.file.redirect, '((VOICE))', int2str(n)));
end

% performance
t2 = clock;
invocation.perf.otherprep = etime(t2, t1);

% GUI
brahms_gui('phase', '');
brahms_gui('operation', '');
if ~invocation.opts.allAtOnce
	brahms_gui('destroy');
else
	brahms_gui('phase', 'Execution Ready.');
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function launch = prepLaunchLine(launch, invocation)

q = quote;
launch = strrep(launch, '((VOICES))', num2str(invocation.exe.voices));
launch = strrep(launch, '((EXECFILE))', [q invocation.exe.executionFile q]);
launch = strrep(launch, '((OUTFILE))', [q invocation.file.out q]);
launch = strrep(launch, '((LOGFILE))', [q invocation.file.log q]);
launch = strrep(launch, '((EXITFILE))', [q invocation.file.exit q]);

r = [' 1> ' q invocation.file.redirect q ' 2>&1 '];
launch = strrep(launch, '((REDIRECT))', r);







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% EXECUTE INVOCATION

function invocation = brahms_exec(invocation, varargin)



%%%% PREAMBLE

% gui
brahms_gui('phase', 'Running Execution');

% performance
t0 = clock;

% collate arguments for executable
[invocation.args, invocation.soak] = txtargs(varargin);

% gui - disable abort, because if we run in synchronous mode, an
% abort won't have an effect until the end of the execution,
% and it would look silly if that was the behaviour.
if strcmp(invocation.mode, 'synchronous')
	brahms_gui('enable_abort', false);
else
	brahms_gui('configure_abort', {'Abandon' 'execution abandoned'});
end



%%%% UPDATE GUI

if strcmp(invocation.mode, 'asynchronous')

	% gui
	brahms_gui('operation', 'Launching (asynchronous mode)...');
	drawnow

else

	% gui
	brahms_gui('operation', 'Running (synchronous mode)...');
	brahms_gui('hide_progress');
	drawnow

end



%%%% LAUNCH ALL VOICES (WAIT, OR NOT WAIT)

RESULT_ERROR = 66; % "B"
% RESULT_ERROR_INIITAL_PARSE = RESULT_ERROR + 1;

if isa(invocation.launchfunc, 'function_handle')
	
	% launch using matlab function
	launch = invocation.launchfunc('launch', invocation);
	
else

	if strcmp(invocation.mode, 'asynchronous')

		% gui
		brahms_gui('operation', 'Launching...');
		
		% asynchronous mode
		lch.cmd = '';
		lch.result = [];
		lch.log = '';
		launch = lch(1:0);

		% for each voice
		for voice = 1:invocation.exe.voices

			% get specific cmd for this voice
			if iscell(invocation.launch)
				cmd = invocation.launch{min(voice, length(invocation.launch))};
			else
				cmd = invocation.launch;
			end

			% swap tokens in cmd
			cmd = strrep(cmd, '((ARGS))', invocation.args);
			cmd = strrep(cmd, '((VOICE))', num2str(voice));
			cmd = strrep(cmd, '((ADDR))', invocation.exe.addresses{voice});

			% add "no wait" to cmd
			if isunix
				cmd = [cmd ' &'];
			else
				cmd = ['start /B ' cmd];
			end

			% invoke
% 			cmd = [cmd ' 2> d:\temp\WorkingDirectory\stdout_stderr-' int2str(voice) '.log'];
			lch.cmd = cmd;
			if invocation.opts.ShowCMD
				disp(lch.cmd);
			end
			
			% switch on environment
			isoct = brahms_utils('IsOctave');
			if isoct
				% on octave, asking for logs causes the system to wait
				% for the command to complete - not good!
				lch.result = system(lch.cmd);
				% dummy (empty) launch output
				lch.log = '';
			else
				[lch.result, lch.log] = system(lch.cmd);
			end
			
			% store
			launch(voice) = lch;

		end

	else

		% synchronous mode
		cmd = invocation.launch;
        
		% swap tokens in cmd
		cmd = strrep(cmd, '((ARGS))', invocation.args);
		
		% if running solo, can replace voice
		if isempty(invocation.exe.addresses)
			cmd = strrep(cmd, '((VOICE))', '1');
		end
        
		% invoke
		launch = [];
		launch.cmd = cmd;
		if invocation.opts.ShowCMD
			disp(cmd);
		end
		[launch.result, launch.log] = system(launch.cmd);

	end

end



%%%% INFO ABOUT HOW WE LAUNCHED

Nl = length(launch);
launch_by_voice = (Nl == invocation.exe.voices);
launch_at_once = ~launch_by_voice && Nl == 1;



%%%% CHECK FOR LAUNCH ERRORS

some_errors = false;

for l = 1:Nl
	if launch(l).result && launch(l).result < RESULT_ERROR
		% error in launch line
		disp(['________________________________________________________________' 10])
		if launch_by_voice
			disp(['Voice ' int2str(l) ': error during launch'])
		else
			disp(['Launch ' int2str(l) ': error during launch'])
		end
		disp(['Command: ' launch(l).cmd 10]);
		log = trimws(launch(l).log);
		if isempty(log)
			disp(['(the above command returned non-zero, but generated no' 10 ...
				'further error messages - try it in a shell prompt?)']);
		else
			disp(log);
		end
		some_errors = true;
	end
end

if some_errors
	disp(['________________________________________________________________' 10])
	error('brahms:error_in_launch_line', '')
end



%%%% IF REQUESTED, WAIT HERE

if invocation.opts.waitForFinish
	uiwait(warndlg('You requested "waitForFinish". Click OK when the execution has finished, and data collection will proceed.', 'Wait for finish'));
end



%%%% IN ASYNCHRONOUS MODE, WAIT FOR EXIT FILES TO APPEAR

pause_length = 0.01;
some_errors = false;
some_output = false;

if strcmp(invocation.mode, 'asynchronous')

	% stored output is only from launch, so display it now
	for l = 1:Nl
		log = trimws(launch(l).log);
		launch(l).log = '';
		if ~isempty(log)
			disp(['________________________________________________________________' 10])
			disp(['Launch ' int2str(l) ': ' launch(l).cmd 10 10 log]);
			some_output = true;
		end
	end

	% gui
	brahms_gui('operation', 'Waiting for Execution (Asynchronous Mode)');

	% prepare list of exitfiles
	exitfiles = {};
	for voice = 1:invocation.exe.voices
		exitfiles{voice} = strrep(invocation.file.exit, '((VOICE))', int2str(voice));
	end
	exitfile_voicenotset = invocation.file.exit;

	% initial message
	wd = invocation.execpars.WorkingDirectory;
	fname = exitfiles{1};
	if length(fname) > length(wd) && strcmp(fname(1:length(wd)), wd)
		fname = ['...' fname(length(wd)+1:end)];
	end
	msg = ['waiting for "' fname '"'];

	% gui/console
	if invocation.opts.ShowGUI
		brahms_gui('operation', msg);
	else
		disp(msg);
	end

	% wait for termination
	t0 = clock;
	treport = t0;
	exitfilesseen = 0;
	callbackProgress = brahms_gui('get_callback_progress');
	while true

		% check for exitfile
		while true
			filename = exitfiles{exitfilesseen+1};
			d = dir(fileparts(filename)); % force update, in case the folder is a network folder
			if exist(filename, 'file')
				exitfilesseen = exitfilesseen + 1;
				if exitfilesseen == invocation.exe.voices
					break
				end
				fname = exitfiles{exitfilesseen+1};
				if length(fname) > length(wd) && strcmp(fname(1:length(wd)), wd)
					fname = ['...' fname(length(wd)+1:end)];
				end
				msg = ['waiting for "' fname '"'];

				% gui/console
				if invocation.opts.ShowGUI
					brahms_gui('operation', msg);
				else
					disp(msg);
				end

			else
				break
			end
		end

		% if completed, break
		if exitfilesseen == invocation.exe.voices
			break
		end
		
		% check for exitfile_voicenotset
		if exist(exitfile_voicenotset, 'file')
			break
		end

		% wait
		pause(pause_length);
		pause_length = pause_length * 1.03;
		if pause_length > 0.25
			pause_length = 0.25;
		end

		% update timer and offer abort
		if invocation.opts.ShowGUI
			callbackProgress([]);
		else
			tnow = clock;
			td = etime(tnow, treport);
			if td >= 10
				treport = tnow;
				disp(['elapsed ' num2str(etime(tnow, t0)) 'secs...']);
			end
		end

	end

	if ~invocation.opts.ShowGUI
		disp('all exit files now seen')
	end

end



%%%% READ AND DESTROY EXIT FILES

% read and then remove exit files (if present) - they will
% be present in all cases, unless the caller has changed
% invocation.args to not produce them

ex = [];
ex.result = [];
ex.msgcount = [];
ex.localerror = '';
exe = ex(1:0);

for voice = 1:invocation.exe.voices
	efile = strrep(invocation.file.exit, '((VOICE))', int2str(voice));
	if exist(efile, 'file')
		
		% file may be present but not yet have content, if
		% we've hit exactly that spot in the timing
		retries = 0;
		
		% should be a matter of milliseconds, but i guess if
		% the system were really busy we might get a long
		% latency... in any case, the length of the wait
		% does not bother the user, beucase they never see
		% this wait unless it is (a) required or (b) going
		% to end in an internal error anyway
		max_retries = 50;
		
		while true

			try
				xml = sml_xml('file2xml', efile);
				break;
			catch
				err = lasterror;
				if retries < max_retries && strcmp(err.identifier, 'systemml:no_xml_in_file')
					% empty file, retry
% 					disp(['exit file not ready, waiting...']);
					pause(1/max_retries);
					retries = retries + 1;
				else
					rethrow(lasterror);
				end
			end
			
		end
		
		try
			ex.result = str2num(xml.children(1).value);
			ex.msgcount = str2num(xml.children(2).value);
			ex.localerror = xml.children(3).value;
			exe(voice) = ex;
		catch
			error(['bad exit file format "' efile '"']);
		end
	 	safedel(efile);
	else
		ex.result = [];
		ex.msgcount = [];
		ex.localerror = '';
		exe(voice) = ex;
	end
end

% also pick up one that hasn't had the ((VOICE)) token
% replaced, in case MPI failed to go up
efile = invocation.file.exit;
if exist(efile, 'file')
	xml = sml_xml('file2xml', efile);
	serr = '';
	try
		localerror = xml.children(3).value;
	catch
		error(['bad exit file format "' efile '"']);
	end
	safedel(efile);
	if ~isempty(localerror)
		disp(['________________________________________________________________' 10])
		disp(['found exit file without ((VOICE)) set (usually indicates' 10 'that MPI had not started when the error occurred):' 10 10 trimws(localerror)]);
		disp(['________________________________________________________________' 10])
		error('brahms:error_in_executable', '')
	end
end



%%%% READ AND DELETE REDIRECT FILES

redirect = {};
for voice = 1:invocation.exe.voices
	rfile = strrep(invocation.file.redirect, '((VOICE))', int2str(voice));
	if ~isempty(rfile) && exist(rfile, 'file')
		fid = fopen(rfile, 'rt');
		if fid == -1
			warning(['failed read "' rfile '"']);
		else
			redirect{voice} = fread(fid, Inf, '*char')';
			fclose(fid);
		end
		
% 		% don't delete (no need, and may not yet have been
% 		% released, so it may fail with a warning)
% 	 	safedel(rfile);
	end
end

rfile = invocation.file.redirect;
if ~isempty(rfile) && exist(rfile, 'file')
	fid = fopen(rfile, 'r');
	if fid == -1
		warning(['failed read "' rfile '"']);
	else
		r = fread(fid, Inf, '*char')';
		disp(['________________________________________________________________' 10])
		disp(['found redirect file without ((VOICE)) set (usually indicates' 10 'that MPI had not started when the error occurred):' 10 10 trimws(r)]);
		disp(['________________________________________________________________' 10])
		error('brahms:error_in_executable', '')
		fclose(fid);
	end
% 	safedel(rfile);
end



%%%% IF NOT LAUNCH BY VOICE, DISPLAY LAUNCH OUTPUT NOW

if ~launch_by_voice
	for l = 1:Nl
		log = trimws(launch(l).log);
		launch(l).log = '';
		if ~isempty(log)
			disp(['________________________________________________________________' 10])
			disp(['Launch ' int2str(l) ': ' launch(l).cmd 10 10 log]);
			some_output = true;
		end
	end
end



%%%% DISPLAY LOGS AND ERRORS FOR EACH VOICE

% copy log.xslt in, if not already there
copyInXSLT(invocation.file.log);

for voice = 1:invocation.exe.voices
	
	v = int2str(voice);
	ex = exe(voice);
	
	show_this_voice = false;

	% cmd
	if launch_by_voice
		cmd = launch(voice).cmd;
	elseif launch_at_once
		cmd = launch.cmd;
	else
		cmd = '<command used to launch this voice is not known>';
	end
	
	% exe_result
	if isempty(ex.result)
		exe_result = 0;
	else
		exe_result = ex.result;
		if exe_result
			show_this_voice = true;
			some_errors = true;
		end
	end
	
	% launch report
	if launch_by_voice
		lrep = trimws(launch(voice).log);
		if ~isempty(lrep)
			show_this_voice = true;
			lrep = [10 lrep];
		end
	else
		lrep = '';
	end
	
	% redirect
	if length(redirect) >= voice && ~isempty(redirect{voice})
		show_this_voice = true;
		red = [10 trimws(redirect{voice})];
	else
		red = '';
	end

	% logfile
	lfile = invocation.file.log;
	if iscell(lfile)
		lfile = lfile{min(voice, length(lfile))};
	end
	lfile = strrep(lfile, '((VOICE))', int2str(voice));
	errmsg = '';	
	
	% if logfile is present
	if exist(lfile, 'file')
		
		% get file info
		d = dir(lfile);
			
		% offer log
		if ~isempty(ex.msgcount)
			if ex.msgcount
				sz = [' (' int2str(ex.msgcount) ' messages)'];
				show_this_voice = true;
			else
				sz = ' (Empty)'; % empty
			end
		else
			% no exit file, so we can't know how many msgs are
			% contained in the log file
			sz = [' (' num2str(round(d.bytes/1024*10)/10) 'kB)'];
			show_this_voice = true;
		end
		
		% switch on evironment
		isoct = brahms_utils('IsOctave');
		if isoct
			log_link = ['Log File "' lfile '"'];
		else
			log_link = ['<a href="matlab: brahms_utils(''OpenURL'', ''' lfile ''')">Log File' sz '</a>'];
		end
		
		% collect errors, only if necessary
		if exe_result
			xml = sml_xml('file2xml', lfile);
			tag = xml.children(end);
			if ~strcmp(tag.name, 'errors')
				error(['malformed log file "' lfile '"']);
			end
			errmsg = '';
			for e = 1:length(tag.children)
				errmsg = [errmsg formatError(tag.children(e))];
			end
			errmsg = [10 trimws(errmsg)];
		end
		
	else
		
		% log file absent
		log_link = '(Log File absent)';
		some_errors = true;
		show_this_voice = true;
		
	end
		
	% display
	if show_this_voice
		
		% display header
		some_output = true;
		disp(['________________________________________________________________' 10])
		disp(['Voice ' v ': ' log_link]);

		% display command
		disp(['Command: ' cmd]);
		
		% display launch output
		if ~isempty(lrep)
			disp(lrep)
		end
		
		% display output
		if ~isempty(red)
			disp(red)
		end
		
		% display errors
		disp(errmsg)
		
	end

	% local errors
	if ~isempty(ex.localerror)
		disp([10 trimws(ex.localerror)])
	end
	
end



%%%% POSTAMBLE

% gui
if strcmp(invocation.mode, 'synchronous')
	brahms_gui('enable_abort', true);
else
	brahms_gui('configure_abort');
end

% time
t1 = clock;
invocation.perf.execute = etime(t1, t0);

% tail
if some_output
	disp(['________________________________________________________________' 10])
end

% raise if error
if some_errors
	error('brahms:error_in_executable', '');
end

% time
t2 = clock;
invocation.perf.console = etime(t2, t1);
invocation.cmds = {launch.cmd};

% gui
brahms_gui('phase', '');
brahms_gui('operation', '');
if ~invocation.opts.allAtOnce
	brahms_gui('destroy');
else
	brahms_gui('phase', 'Execution Complete.');
end










%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% COLLECT INVOCATION RESULTS

function [invocation, out, rep, sys] = brahms_collect(invocation)

% GUI
brahms_gui('phase', 'Collecting Results');

% performance
t0 = clock;

% if an invocation, it will be a struct
if isstruct(invocation)

	% collect results
	out = [];
	rep = struct;
	for voice = 1:invocation.exe.voices

		% msg
		brahms_gui('phase', ['Collecting Results (Voice ' int2str(voice) ')']);

		% load individual result
		efile = strrep(invocation.file.report, '((VOICE))', int2str(voice));
		if ~exist(efile, 'file')

			% report file not found - probably indicates an abort
            trace = '';
			trace = [trace 10 'report file not found: "' efile '"' 10 10];
			trace = [trace 'If you are collecting old results, this file was not where it was expected.' 10];
			trace = [trace 'If you just executed BRAHMS, it probably aborted (segmentation fault). This' 10];
			trace = [trace 'may have occurred in a user process (one you are developing?) or, if not, in' 10];
			trace = [trace 'BRAHMS itself. In the latter case, you should try and repeat this error, and' 10];
			trace = [trace 'then report the steps required to do so. Thanks!' 10];
            disp(trace)
            error(['report file "' efile '" not found...']);

		end

		reader = [];
		reader.filename = efile;
		reader.namespaceRoots = invocation.execpars.NamespaceRoots;
		reader.callbackProgress = brahms_gui('get_callback_progress');
		[logs, eml] = sml_reportml(reader);

		% collate results from concerto runs
		out = merge(logs, out);

		% collate
		if voice == 1
			rep = eml;
		else
			rep(voice) = eml;
		end

	end

	% get output system file
	if nargout > 3
		sys = sml_system;
		sys = sys.unserialize(invocation.exe.systemFileOut);
	end

	% time
	t1 = clock;
	invocation.perf.collect = etime(t1, t0);

else

	% else it will be the filename of the exec file
	filename = invocation;
	invocation = [];
	invocation.execpars = brahms_utils('GetExecutionParameters');

	% it might be a relative filename, in which case it's
	% either in the current directory or the working directory
	if isempty(fileparts(filename))
		if ~exist(filename, 'file')
			filename  = [invocation.execpars.WorkingDirectory '/' filename];
		end
	end

	%% TODO: should really unserialize() the whole execution
	%% file, here, since we may need it if the caller wants
	%% (e.g.) the output system file back

	% collect it
	eml = sml_xml('file2xml', filename);
	if ~strcmp(eml.name, 'Execution')
		error('filename should be an Execution File')
	end

	% from where we can obtain information we need
	addr = getXMLField(eml, 'Voices');
	invocation.exe.voices = length(addr.children);

	% find the report files alongside the exec file (rather
	% than in the WD)
	path = fileparts(filename);
	invocation.file.report = [path '/' getXMLFieldValue(eml, 'ReportFile')];

	% in case there are errors, pass empty data for what came
	% from the console and what cmds were used to launch
	% brahms
	invocation.console = cell(invocation.exe.voices);
	invocation.cmds = cell(invocation.exe.voices);

	% do the job
	if nargout == 4
		[invocation, out, rep, sys] = brahms_collect(invocation);
	else
		[invocation, out, rep] = brahms_collect(invocation);
	end

end

% store bindings timings in Performance/Timing of all reps,
% for ease of use by brahms_perf and similar tools
for v = 1:length(rep)
	rep(v).Timing.bindings = invocation.perf;
end

% GUI
brahms_gui('destroy');









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function out = mergeConcerto(out, eml)

f = fieldnames(eml);
for n = 1:length(f)
	if strcmp(f{n}, 'LogML_container')
		% don't merge these
	else
		% it's something real, but is it a container or an output?
		if isstruct(eml.(f{n})) && isfield(eml.(f{n}), 'LogML_container')
			% container
			if ~isfield(out, f{n})
				out.(f{n}) = struct;
			end
			out.(f{n}) = mergeConcerto(out.(f{n}), eml.(f{n}));
		else
			% data
			if isfield(out, f{n})
				error('conflict whilst merging concerto results');
			end
			out.(f{n}) = eml.(f{n});
		end
	end
end





%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [out, soak] = txtargs(in)

soak = false;
q = quote;
out = '';
for i = 1:length(in)
	if ~ischar(in{i})
		disp(in{i})
		error('non-text argument (above) unexpected here');
	end
	if strcmp(in{i}, '--soak')
		soak = true;
	else
		if ~isempty(out)
			out = [out ' '];
		end
		out = [out q in{i} q];
	end
end












%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function f = getXMLFieldValue(pml, fieldName)

for n = 1:length(pml.children)

	if strcmp(pml.children(n).name, fieldName)
		f = pml.children(n).value;
		return;
	end

end

error(['"' pml.name '" should have sub-field "' fieldName '"']);






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function f = getXMLField(pml, fieldName)

for n = 1:length(pml.children)

	if strcmp(pml.children(n).name, fieldName)
		f = pml.children(n);
		return;
	end

end

error(['"' pml.name '" should have sub-field "' fieldName '"']);






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function [status, result] = invoke(cmd)

[status, result] = system(cmd);

% check status
if status
	while ~isempty(result) && result(end) == 10
		result = result(1:end-1);
	end
% 	disp(['Command: ' cmd]);
	disp([10 result])
	disp(['________________________________________________________________' 10])
	error('brahms:error_in_executable', '')
else
	disp(result)
end




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function safedel(path)

% we don't bother recycling these files, which can take
% considerable time (presumably more if the recycle bin is
% fullish)
isoct = brahms_utils('IsOctave');
if ~isoct
	status = recycle;
	recycle('off');
end
if exist(path, 'file')
	delete(path)
end
if ~isoct
	recycle(status);
end






%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function s = formatErrorLine(s)

f = strfind(s, '[[ ');
g = strfind(s, ' :: ');
h = strfind(s, ' ]]');
if length([f g h]) == 3
	if h > g && g > f
		filename = s(f+3:g-1);
		if exist(filename, 'file')
			line = str2num(s(g+4:h-1));
			isoct = brahms_utils('IsOctave');
			if isoct
				s = [s(1:f-1) '(' filename ', line ' int2str(line) ')' s(h+3:end)];
			else
				lnk = ['matlab: opentoline(''' filename ''', ' int2str(line) ')'];
				s = [s(1:f-1) '<a href="' lnk '">(open in editor)</a>' s(h+3:end)];
			end
		end
	end
end

function s = formatError(e)

code = getXMLFieldValue(e, 'code');
msg = getXMLFieldValue(e, 'msg');
trace = '';

for f = 1:length(e.children)
	c = e.children(f);
	switch c.name
		case 'trace'
			trace = [trace 9 formatErrorLine(c.value) 10];
	end
end

isoct = brahms_utils('IsOctave');
if isoct
	s = [10 code ': ' msg 10 trace];
else
	s = [10 brahms_utils('DocLink', ['Errors#' code], code) ': ' msg 10 trace];
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function q = quote

if ispc
	q = '"';
else
	q = '''';
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function s = trimws(s)

f = find(s ~= 10 & s ~= 13 & s ~= 32 & s ~= 9);
s = s(min(f):max(f));



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function copyInXSLT(file)

if iscell(file)
    for i = 1:length(file)
        copyInXSLT(file{i});
    end
    return
end

dst = [fileparts(file) '/log.xslt'];

if ~exist(dst, 'file')
    s = 0;
    sip = getenv('SYSTEMML_INSTALL_PATH');
    if ~isempty(sip)
        src = [sip '/BRAHMS/media/log.xslt'];
        if exist(src, 'file')
            copyfile(src, dst);
            s = 1;
        end
    end
    if ~s
        warning('brahms:failed_copy_logxslt', 'failed to copy log.xslt over; logs will not display properly');
    end
end




