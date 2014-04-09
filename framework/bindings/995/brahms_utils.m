%
% [out, err] = brahms_utils(operation, ...)
%
%   This form (first argument a recognised string) will
%   perform some operation and may return a result, listed
%   below. Below the list of operations is a description of
%   the arguments they can be passed. Where a function
%   encounters an error, it is returned in err, if that
%   argument is requested, otherwise raised. Next to some of
%   the operations strings is a shortcut character string in
%   brackets that has the same effect.
%
%
%
% CONFIGURATION
%
%   "GetSystemMLInstallPath"
%     Return the SystemML install path (from the environment
%     variable "SYSTEMML_INSTALL_PATH")
%
%   "VersionBRAHMS"
%     Return the BRAHMS version as an array.
%
%   "VersionStringBRAHMS"
%     Returns the BRAHMS version as a string.
%
%   "ArchBitsBRAHMS"
%     Returns the bit-width of the installed version of
%     BRAHMS.
%
%   "GetConfigFilePath" <level> <ensureExists>
%     Return the path to the configuration file at the
%     specified level. If the file does not exist, and
%     <ensureExists> is supplied and evaluates true, it will
%     be created. If it cannot be created, an error is
%     raised.
%
%   "GetExecutionParameters" (gep) <level>
%     Return the current Execution Parameters as a
%     structure. If <level> is specified ('all', 'install',
%     'machine' or 'user'), only parameters at that level
%     are returned.
%
%   "SetExecutionParameter" (sep) <level> <key> <value>
%     Set an Execution Parameter at specified level.
%
%   "DeleteExecutionParameter" (dep) <level> <key>
%     Delete an Execution Parameter at specified level.
%
%   "IsConfigurationWriteable" <level>
%     Return whether configuration at specified level is
%     writeable by the current user. Empty string means yes,
%     non-empty string is the reason why not.
%
%   "DropCache" (drop)
%     brahms_utils caches the current Execution Parameters,
%     and will usually drop the cache automatically when
%     they are changed (through brahms_manager or
%     brahms_utils). If you change them through some other
%     mechanism, this operation will force the cache to be
%     dropped.
%
%
%
% NAMESPACE
%
%   "CreateNewComponent" <nspcroot> <lang>
%     Interactively create a new component, based on one of
%     the templates installed with BRAHMS.
%
%   "GetClassNodePath" <class>
%     Return the node path of the component with the
%     specified class, or empty if not installed.
%
%   "GetClassReleasePath" <class> <language>
%     Return the path of the latest release of the component
%     with the specified class. If <language> is supplied,
%     only releases in that language are considered. If no
%     release is found, the empty string is returned.
%
%   "OpenClassReleasePath" <class> <language>
%     As above, but try to open the folder in an explorer
%     window.
%
%   "RunBuildScript" (rbs) <class> <language>
%     Run the build script in the release folder of the
%     specified class. If only one language is installed,
%     <language> need not be supplied. If no build script is
%     present, create a default one.
%
%   "RunTestScript" (rts) <class> <language>
%     Run the script "test.m" in the release folder of the
%     specified class. If only one language is installed,
%     <language> need not be supplied.
%
%
%
% HELPERS
%
%   "OpenDocumentation" (doc) <title>
%     Open the documentation page with the specified title.
%     If title is unspecified, the start page is displayed.
%
%   "OpenURL" <url>
%     Open the given URL in an appropriate application.
%     Usually, <url> will be a web site address or the
%     filename of a (XML) log file.
%
%   "OpenFolder" <path>
%     Open the given folder in an appropriate application.
%
%   "PrepareConfigurationReport" (pcr)
%     Generate a report on your local configuration, for
%     dispatch to the developers. The report is opened in
%     the editor, for manual copy-and-paste into an email.
%
%   "IsOctave"
%     returns true if running under Octave (false if running
%     under Matlab).
%
%   "DocLink" <link> <label>
%     generate a link to the best available documentation
%     page (<link>.html) - offline if available, online if
%     not.
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
% $Id:: brahms_utils.m 2424 2009-12-03 17:05:35Z benjmitch               $
% $Rev:: 2424                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-12-03 17:05:35 +0000 (Thu, 03 Dec 2009)                   $
%__________________________________________________________________________
%




function [out, err] = brahms_utils(op, varargin)

if nargin == 0
	help brahms_utils
	return
end



err = '';

switch op

	case 'GetSystemMLInstallPath'

		out = getenv('SYSTEMML_INSTALL_PATH');
		if isempty(out)
			err = 'environment variable "SYSTEMML_INSTALL_PATH" not set';
			if nargout < 2 error(err); end
		end

	case 'CreateNewComponent'

		% return empty (nothing created)
		if nargout
			out = '';
		end
		
		% get args
		[root, lang] = args(varargin, {'', ''});
		sip = brahms_utils('GetSystemMLInstallPath');

		% get namespace roots
		roots = brahms_utils('GetExecutionParameters');
		roots = explode(pathsep, roots.NamespaceRoots);
		if length(roots) == 0
			error('cannot create a new component - add a NamespaceRoot first');
		end
		
		% choose a root
		root = dlg_choose( ...
			roots, roots, root, ...
			'Which NamespaceRoot do you want to create your new Component under?', ...
			'NamespaceRoot' ...
			);
		if isempty(root)
			return
		end
		
		% find which template languages are available
		templatespath = [sip '/BRAHMS/support/template/process'];
		d = dir(templatespath);
		langs = {};
		for n = 1:length(d)
			if d(n).name(1) ~= '.'
				langs{end+1} = d(n).name;
			end
		end

		% convert to known languages
		langdescs = {};
		for n = 1:length(langs)
			switch langs{n}
				case '1199'
					langdescs{n} = '1199 (C++, Native Process)';
				case '1266'
					langdescs{n} = '1266 (C, Native Process)';
				case '1258'
					langdescs{n} = '1258 (Matlab, Non-native Process)';
				case '1262'
					langdescs{n} = '1262 (Python, Non-native Process)';
				otherwise
					langdescs{n} = [langs{n} ' (User Template)'];
			end
		end
		
		% choose a lang
		lang = dlg_choose( ...
			langs, langdescs, lang, ...
			'Which language (binding) do you want to author your new Component against?', ...
			'Binding' ...
			);
		if isempty(lang)
			return
		end

		% srcpath
 		srcpath = [templatespath '/' lang];
		
		% choose class name
		cls = 'dev/my/new/class';
		while true
			
			% ask user
			cls = dlg_inputstring( ...
					cls, ...
					['Choose the SystemML Class Name of the new Component. This should have the form ' ...
					'"dev/my/new/class" (words separated by forward slashes). Only alphanumerics and ' ...
					'underscore may be used, and the last character must not be a slash. For more ' ...
					'information, see the documentation node "Namespace".'], ...
					'Class Name' ...
				);
			if isempty(cls)
				return
			end
			
			% validate offering
			err = validateClassName(cls);
			if ~isempty(err)
				dlg_warning(['The specified Class Name is invalid, because ' err '.']);
				continue
			end

			% check it's free
			nodepath = [root '/' cls];
			if exist(nodepath)
				dlg_warning(['The specified Class already exists - choose another.']);
				continue
			end
			
			% ok
			break
			
		end

		% add implementation folder
		release = '0';
		releasepath = [nodepath '/brahms/' release];

		% confirm
		if ~dlg_confirm( ...
			['Ready to create a new component at "' nodepath '"'] ...
			) return; end
	
		% create it
		result = mkdir(releasepath);
		if ~result
			error(['error creating "' releasepath '"']);
		end

		% copy over contents of src dir
		copyfile([srcpath '/*.*'], releasepath);
		
		% move node file
		movefile([releasepath '/node.xml'], [nodepath '/node.xml']);

		% modify process source file
		d = dir([releasepath '/component*']);
		filename = [releasepath '/' d.name];
		fid = fopen(filename, 'r');
		if fid == -1
			error(['failed opening "' filename '"']);
		end
		text = fread(fid, '*char')';
		fclose(fid);
		text = cuthere(text);
		data = [];
		data.lang = lang;
		data.cls = cls;
		data.release = release;
		if ispc
			data.username = getenv('USERNAME');
		else
			data.username = getenv('USER');
		end
		text = specificate(text, data);
		fid = fopen(filename, 'w');
		if fid == -1
			error(['failed opening "' filename '"']);
		end
		fwrite(fid, text);
		fclose(fid);

		% modify process test file
		filename = [releasepath '/test.m'];
		fid = fopen(filename, 'r');
		if fid == -1
			error(['failed opening "' filename '"']);
		end
		text = fread(fid, '*char')';
		fclose(fid);
		text = cuthere(text);
		text = specificate_test(text, data);
		fid = fopen(filename, 'w');
		if fid == -1
			error(['failed opening "' filename '"']);
		end
		fwrite(fid, text);
		fclose(fid);

		% return new class name
		if nargout
			out = cls;
		end

	case 'GetClassNodePath'

		% get roots
		roots = brahms_utils('GetExecutionParameters');
		roots = explode(pathsep, roots.NamespaceRoots);
		roots{end+1} = [brahms_utils('GetSystemMLInstallPath') '/Namespace'];

		% get args
		cls = args(varargin);

		% find node folder
		for r = 1:length(roots)
			path = [roots{r} filesep cls];
			if exist([path '/node.xml'], 'file')
				out = path;
				out(out == '\') = '/';
				return
			end
		end

		% not found
		out = '';

	case 'GetClassReleasePath'

		% get args
		out = '';
		[cls, lang] = args(varargin, {''});

		% get node path
		nodepath = brahms_utils('GetClassNodePath', cls);
		
		% get lang path
		data = getClassReleasePath(cls, nodepath, lang);
		out = data.releasepath;

	case 'OpenClassReleasePath'
		
		% get args
		[cls, lang] = args(varargin, {''});

		% get node path
		nodepath = brahms_utils('GetClassNodePath', cls);
		
		% get lang path
		data = getClassReleasePath(cls, nodepath, lang);
		path = data.releasepath;
		
		% must exist
		if isempty(path)
			disp(['release folder for class "' cls '" not found']);
			return
		end
		
		% open it
		brahms_utils('OpenFolder', path);
		
	case {'RunBuildScript' 'rbs'}

		% get args
		[cls, lang] = args(varargin, {''});

		% get node path
		nodepath = brahms_utils('GetClassNodePath', cls);
		
		% get lang path
		data = getClassReleasePath(cls, nodepath, lang);
		if isempty(data.releasepath)
			error(['release folder for class "' cls '" not found']);
		end
		
		% nothing to do
		switch data.lang
			case {'1065' '1199' '1266'}
			otherwise
				disp(['Build is not required for class "' cls '"']);
				return
		end
		
		% create it if it does not exist
		data = create_build_script(data);
	
		% run script
		c = cd;
		try
			
			% preamble
			cd(data.releasepath)
% 			disp(['Running build script for class "' data.cls '"...']);

			% run build script
			if isunix
				[result, output] = system('./build --matlab');
			else
				% run with --matlab so it forces cmd to exit with
				% code, rather than just the script (windows)
				[result, output] = system('build --matlab');
			end

			% throw error
			if result
				disp(output)
				error(['An error occurred whilst running the build script']);
			end
			
			% or just display output
			disp(output)
			disp('RunBuildScript OK.');

		catch
			
			cd(c);
			rethrow(lasterror)
			
		end
		cd(c);

	case {'RunTestScript' 'rts'}

		% get args
		[cls, lang] = args(varargin, {''});

		% get node path
		nodepath = brahms_utils('GetClassNodePath', cls);
		
		% get lang path
		data = getClassReleasePath(cls, nodepath, lang);
		if isempty(data.releasepath)
			error(['release folder for class "' cls '" not found']);
		end
		
		% run script
		c = cd;
		try
			
			% preamble
			cd(data.releasepath)
% 			disp(['Running test script for class "' data.cls '"...']);

			% run test script
			test
			disp('RunTestScript OK.');
			
		catch
			
			cd(c);
			rethrow(lasterror)
			
		end
		cd(c);

	case {'OpenDocumentation' 'doc'}

		% concatenate arguments
		title = '';
		for v = 1:length(varargin)
			arg = varargin{v};
			if v>1
				title = [title ' '];
			end
			title = [title arg];
		end
		if isempty(title)
			title = 'index';
		end

		% strip hash
		l = find(title == '#', 1);
		if isempty(l)
			file = title;
			hash = '';
		else
			file = title(1:l-1);
			hash = title(l:end);
		end

		% get filename
		filename = [file '.html'];

		% check local docs
		url = [brahms_utils('GetSystemMLInstallPath') '/BRAHMS/support/docs/' filename];
		if exist(url)
			openURL(['file:///' url hash]);
			return
		end

		% otherwise, just launch at remote
		url = ['http://brahms.sourceforge.net/docs/' filename hash];
		openURL(url);

	case {'OpenURL'}

		% concatenate arguments
		url = '';
		for v = 1:length(varargin)
			arg = varargin{v};
			if v>1
				url = [url ' '];
			end
			url = [url arg];
		end
		if isempty(url)
			error('URL not specified');
		end

		% launch
		openURL(url);

	case {'OpenFolder'}

		% concatenate arguments
		path = '';
		for v = 1:length(varargin)
			arg = varargin{v};
			if v>1
				path = [path ' '];
			end
			path = [path arg];
		end
		if isempty(path)
			error('path not specified');
		end

		% launch
		openFolder(path);

	case {'VersionBRAHMS' 'VersionStringBRAHMS' 'ArchBitsBRAHMS'}

		% get from cache
		cache = getCache();
		out = cache.(op);

	case 'GetConfigFilePath'

		% check args
		[level, ensureExists] = args(varargin, {false});

		% get path
		cfg = getConfigFile(level, ensureExists);
		if ~isempty(cfg.writeerror)
			if nargout < 2
				error(cfg.writeerror);
			end
			out = [];
			err = cfg.writeerror;
			return
		end
		out = cfg.filename;

	case {'DropCache' 'drop'}

		% invalidate cache
		invalidateCache();

	case {'GetExecutionParameters' 'gep'}

		% specific level
		[level] = args(varargin, {'all'});
		switch level

			case 'all'
				% get from cache
				cache = getCache();
				out = cache.pars;

			otherwise
				% get from file - we don't cache them separately
				out = readPars(level, 'ExecutionParameters', struct());

		end

	case {'SetExecutionParameter' 'sep'}

		% get args
		[level, key, val] = args(varargin);
		if strcmp(level, 'install')
			error('cannot set install-level Execution Parameters');
		end

		% set
		cfg = getConfigFile(level);
		pars = xml_getChild(cfg.xml, 'ExecutionParameters');
		pars = xml_setField(pars, key, val);
		cfg.xml = xml_setChild(cfg.xml, pars);
		setConfigFile(cfg);

		% invalidate cache
		invalidateCache();

	case {'DeleteExecutionParameter' 'dep'}

		% get args
		[level, key] = args(varargin);
		if strcmp(level, 'install')
			error('cannot delete install-level Execution Parameters');
		end

		% set
		cfg = getConfigFile(level);
		pars = xml_getChild(cfg.xml, 'ExecutionParameters');
		pars = xml_deleteField(pars, key);
		cfg.xml = xml_setChild(cfg.xml, pars);
		setConfigFile(cfg);

		% invalidate cache
		invalidateCache();

	case 'IsConfigurationWriteable'

		% get args
		[level] = args(varargin);
		if strcmp(level, 'install')
			out = 'Install-level configuration can not be changed with these tools (only manually)';
			return
		end

		% get filename
		cfg = getConfigFile(level);
		out = cfg.writeerror;

	case {'PrepareConfigurationReport' 'pcr'}

		% filename
		filename = ['cr_' num2str(floor(double(now*24*3600)), 16) '.txt'];
		line = ['________________________________________________________________' 10 10];

		% header
		writeto(filename, line);
		writeto(filename, ['BRAHMS Configuration Report']);
		writeto(filename, ['Timestamp: ' datestr(now)]);
		writeto(filename, line);
		writeto(filename, ['**** PRIVACY INFORMATION ****']);
		writeto(filename, [' ']);
		writeto(filename, ['Please forward an up-to-date configuration report along with any']);
		writeto(filename, ['bug report or help request you send. We do not aim to collect any']);
		writeto(filename, ['personal information, but you should review the contents of this']);
		writeto(filename, ['file and delete anything you prefer not to reveal before you send']);
		writeto(filename, ['it. In particular ***PLEASE NOTE*** the final section may contain']);
		writeto(filename, ['personal and/or identity information in some cases.']);
		writeto(filename, line);

		% BRAHMS
		writeto(filename, ['Matlab "brahms_utils VersionStringBRAHMS": ' brahms_utils('VersionStringBRAHMS')]);
		writeto(filename, ' ');
		writeoscmd(filename, 'brahms --version');
		writeto(filename, line);

		% MATLAB
		writeto(filename, ['Matlab "version": ' trimws(version)]);
		val = num2str([isunix ispc ismac]);
		writeto(filename, ['Matlab "isunix ispc ismac": ' val]);
		writeto(filename, ' ');
		vs = ver;
		for n = 1:length(vs)
			v = vs(n);
			writeto(filename, ['    ' v.Name ': ' v.Version ' ' v.Release ' ' v.Date]);
		end
		writeto(filename, line);

		if ispc
			writeoscmd(filename, 'ver');
			writeenv(filename, 'NUMBER_OF_PROCESSORS');
			writeenv(filename, 'PROCESSOR_IDENTIFIER');
			writeenv(filename, 'OS');
		else
			writeoscmd(filename, 'uname -a');
			writeoscmd(filename, 'cat /proc/version');
			writeoscmd(filename, 'echo $BASH');
			writeoscmd(filename, 'echo $BASH_VERSION');
			writeoscmd(filename, 'echo $MACHTYPE');
		end
		writeenv(filename, 'MATLAB');
		writeenv(filename, 'SYSTEMML_INSTALL_PATH');
		writeenv(filename, 'SYSTEMML_TEMP_PATH');
		writeenv(filename, 'SYSTEMML_MATLAB_PATH');
		writeenvsplit(filename, 'PATH');

		writeto(filename, line);

		% brahms_utils gep
		lvls = {'install' 'machine' 'user' 'all'};
		for l = 1:length(lvls)
			pars = brahms_utils('gep', lvls{l});
			if l > 1
				writeto(filename, ' ');
			end
			writeto(filename, ['Execution Parameters "' lvls{l} '"']);
			f = fieldnames(pars);
			for n = 1:length(f)
				s = [f{n} '=' pars.(f{n})];
				writeto(filename, s);
			end
		end
		writeto(filename, line);

		% brahms --walk
		writeoscmd(filename, 'brahms --walk');
		writeto(filename, line);

		% brahms --Walk
		writeto(filename, 'Please note that this section _may_ contain personal information.');
		writeto(filename, ' ');
		writeoscmd(filename, 'brahms --Walk');
		writeto(filename, line);

		% open in editor
		edit(filename);

	case 'IsOctave'

		out = exist('OCTAVE_VERSION');

	case 'DocLink'

		% get args
		[link, label] = args(varargin);

		% form link
		out = ['<a href="matlab: brahms_utils(''OpenDocumentation'', ''' link ''')">'];
		out = [out label '</a>'];

	otherwise

		% error
		error(['unrecognised operation "' op '"']);

end










%% DATA CACHE

% we keep a cache of commonly used data in a global
% variable, so that we are much quicker in retrieving this
% data. the cache is invalidated whenever we have reason to
% think that it is out of date, and of course when we
% restart matlab.

function brahms_utils_cache_ret = getCache

global brahms_utils_cache
if isempty(brahms_utils_cache)

	% rebuild cache
	cache = [];
	disp('brahms_utils: rebuilding cache...')

	% exec pars
	pars = readPars('install', 'ExecutionParameters', struct());
	pars = readPars('machine', 'ExecutionParameters', pars);
	cache.pars = readPars('user', 'ExecutionParameters', pars);

	% get audit data from BRAHMS
	[status, audit] = system(['brahms --audit']);
	if status
		error(audit)
	end

	% extract audit data: BRAHMS Version
	f = strfind(audit, 'VERSION_ENGINE=');
	if isempty(f)
		error('BRAHMS audit data was invalid');
	end
	g = find(audit == 10);
	g = g(g>f);
	if isempty(g)
		error('BRAHMS audit data was invalid');
	end
	cache.VersionStringBRAHMS = audit(f+15:g(1)-1);
	cache.VersionBRAHMS = sscanf(cache.VersionStringBRAHMS, '%i.')';

	% extract audit data: BRAHMS Version
	f = strfind(audit, 'ARCH_BITS=');
	if isempty(f)
		err = 'BRAHMS audit data was invalid';
		if nargout < 2 error(err); end
	end
	g = find(audit == 10);
	g = g(g>f);
	if isempty(g)
		err = 'BRAHMS audit data was invalid';
		if nargout < 2 error(err); end
	end
	cache.ArchBitsBRAHMS = audit(f+10:g(1)-1);

	% ok
	brahms_utils_cache = cache;

end
brahms_utils_cache_ret = brahms_utils_cache;

function invalidateCache

global brahms_utils_cache
brahms_utils_cache = [];
















%% HELPER FUNCTIONS

%% getConfigFilePath

function cfg = getConfigFile(level, ensureExists)

% calculate path to config file
subpath = 'BRAHMS';
switch level
	case 'install'
		path = brahms_utils('GetSystemMLInstallPath');
	case 'machine'
		if isunix
			path = ['/etc'];
		else
			path = [getenv('ALLUSERSPROFILE') '/Application Data'];
		end
	case 'user'
		if isunix
			path = ['~'];
			subpath = '.BRAHMS';
		else
			path = [getenv('USERPROFILE') '/Application Data'];
		end
	otherwise
		error(['unrecognised level "' level '"'])
end

% default return is an empty file
cfg.filename = [path '/' subpath '/brahms.xml'];
cfg.level = level;
cfg.xml = getDefaultConfigXML;
cfg.writeerror = '';

% ensure parent path, or return default
if ~exist(path, 'dir')
	cfg.writeerror = ['directory "' path '" not found'];
	return
end

% ensure sub path, or return default
if ~exist([path '/' subpath], 'dir')
	if exist([path '/' subpath], 'file')
		cfg.writeerror = ['file "' [path filesep subpath] '" is a directory'];
		return
	end
	if ~mkdir(path, subpath)
		cfg.writeerror = ['directory "' path filesep subpath '" could not be created'];
		return
	end
end

% if file present, read it, else return default
if exist(cfg.filename, 'file')

	if exist(cfg.filename, 'dir')
		cfg.writeerror = ['file "' cfg.filename '" is a directory'];
		return
	end

	% use SystemML Toolbox
	cfg.xml = sml_xml('file2xml', cfg.filename);

	% check it is valid
	if ~strcmp(cfg.xml.name, 'Brahms')
		error(['BRAHMS config file malformed "' cfg.filename '"'])
	end

	% make sure it has children
	[tag, cfg.xml] = xml_getChild(cfg.xml, 'ExecutionParameters');

	% find out if it's writeable
	fid = fopen(cfg.filename, 'a');
	if fid == -1
		cfg.writeerror = ['file "' cfg.filename '" could not be written (permission problem?)'];
	else
		fclose(fid);
	end

else

	% if ensureExists, do so
	if exist('ensureExists', 'var') && ensureExists
		setConfigFile(cfg);

	else

		% if not, just see if it's writeable
		fid = fopen(cfg.filename, 'w');
		if fid == -1
			cfg.writeerror = ['file "' cfg.filename '" could not be created (permission problem?)'];
		else
			fclose(fid);
			delete(cfg.filename);
		end

	end

end

%% setConfigFile

function setConfigFile(cfg)

% if write error already known, throw that
if ~isempty(cfg.writeerror)
	error(['cannot write config file: ' cfg.writeerror]);
end

% check we're not trying to write install-level
if strcmp(cfg.level, 'install')
	error('cannot write install-level configuration file');
end

% write file
sml_xml('xml2file', cfg.xml, cfg.filename);

%% readPars

function pars = readPars(level, type, pars)

cfg = getConfigFile(level);
xml = xml_getChild(cfg.xml, type);

tags = xml.children;
for t = 1:length(tags)
	tag = tags(t);
	key = tag.name;
	if isfield(tag, 'value')
		val = tag.value;
	else
		val = '';
	end
	pars.(key) = val;
end

%% getDefaultConfigXML

function xml = getDefaultConfigXML

xml = [];
xml.name = 'Brahms';
xml.attr = [];
xml.value = '';
xml.children = [];

pars = [];
pars.name = 'ExecutionParameters';
pars.attr = [];
pars.value = '';
pars.children = [];
if isempty(xml.children)
	xml.children = pars;
else
	xml.children(end+1) = pars;
end

%% xml_getChild

function [child, xml] = xml_getChild(xml, key)

c = xml.children;
for index = 1:length(c)
	if strcmp(c(index).name, key)
		child = c(index);
		return
	end
end

% IF MODIFIED XML ASKED FOR AS OUTPUT ARGUMENT...
if nargout == 2
	% create if absent
	child = [];
	child.name = key;
	child.attr = [];
	child.value = '';
	child.children = [];
	if isempty(xml.children)
		xml.children = child;
	else
		xml.children(end+1) = child;
	end
else
	% error if absent
	error(['child "' key '" not found']);
end

%% xml_getField

function field = xml_getField(xml, key)

c = xml.children;
for index = 1:length(c)
	if strcmp(c(index).name, key)
		child = c(index);
		field = child.value;
		return
	end
end

field = '';

%% xml_setChild

function xml = xml_setChild(xml, child)

N = length(xml.children);
for n = 1:N
	par = xml.children(n);
	if strcmp(par.name, child.name)
		xml.children(n) = child;
		return
	end
end

% create
xml.children(end+1) = child;

%% xml_setField

function xml = xml_setField(xml, key, value)

N = length(xml.children);
for n = 1:N
	par = xml.children(n);
	if strcmp(par.name, key)
		xml.children(n).value = value;
		return
	end
end

% create
par = [];
par.name = key;
par.attr = [];
par.value = value;
par.children = [];
if isempty(xml.children)
	xml.children = par;
else
	xml.children(end+1) = par;
end

%% xml_deleteField

function xml = xml_deleteField(xml, key, value)

N = length(xml.children);
for n = 1:N
	par = xml.children(n);
	if strcmp(par.name, key)
		xml.children = xml.children([1:n-1 n+1:end]);
		return
	end
end

%% openURL

function openURL(url)

% the call "web(url, '-browser')" is supposed to do this,
% but it's flaky in some matlab versions, so we offer the
% user a chance to specify their own route. See the
% Execution Parameter "OpenURL".
%
% possible values for OpenURL:
%
% PC: rundll32 url.dll,FileProtocolHandler "((URL))"
% MAC: open -a safari '((URL))'
% MAC: open -a firefox '((URL))'
% LINUX: firefox '((URL))' &
% LINUX: mozilla '((URL))' &

% check for explicit solution provided by user
cache = getCache();
openURL = cache.pars.OpenURL;
if isempty(openURL)

	if ispc
		openURL = 'rundll32 url.dll,FileProtocolHandler "((URL))"';
	elseif ismac
		openURL = 'open -a safari ''((URL))''';
	else
		openURL = 'mozilla ''((URL))'' &';
	end

	msg = [ ...
		'You are trying to open the URL "' url '"' 10 10 ...
		'The Execution Parameter "OpenURL" is not set. Below is the default value of ' ...
		'this parameter for your system. When you click OK, this value will be saved ' ...
		'and then used to launch the file. You can modify it, if you know better, ' ...
		'and click OK, or you can click Cancel to not set it right now. You can ' ...
		'manage this, and other, parameters using "brahms_manager".' 10 ...
		];

	openURL = inputdlg(msg, 'OpenURL (BRAHMS)', 1, {openURL});
	if iscell(openURL) && ~isempty(openURL)
		openURL = openURL{1};
	end
	if isempty(openURL)
		return
	end

	brahms_utils('sep', 'user', 'OpenURL', openURL);

end


openURL_ = strrep(openURL, '((URL))', url);
[result, output] = system(openURL_);
if ~result return; end
disp(['Command: ' openURL]);
disp(['The above command is specified in parameter "OpenURL", but does not work.']);
disp(['Run <a href="matlab: brahms_manager(''ExecPar=OpenURL'')">brahms_manager</a> to correct this']);
error('Failed to open URL');

% % if they don't, though, we just fall back to the matlab
% % approach, which should work (eventually, when they fix it)
% result = web(url, '-browser');
% if result
% 	warning(['Failed to open URL: run "brahms_manager" and set parameter "OpenURL" to fix this']);
% end



%% openFolder

function openFolder(path)

% See the Execution Parameter "OpenFolder".

% check for explicit solution provided by user
cache = getCache();
openFolder = cache.pars.OpenFolder;
if isempty(openFolder)

	if ispc
		openFolder = 'rundll32 url.dll,FileProtocolHandler "((PATH))"';
	elseif ismac
		openFolder = 'open -a finder ''((PATH))''';
	else
		openFolder = 'nautilus ''((PATH))''';
	end

	msg = [ ...
		'You are trying to open the folder "' path '"' 10 10 ...
		'The Execution Parameter "OpenFolder" is not set. Below is the default value of ' ...
		'this parameter for your system. When you click OK, this value will be saved ' ...
		'and then used to launch the file. You can modify it, if you know better, ' ...
		'and click OK, or you can click Cancel to not set it right now. You can ' ...
		'manage this, and other, parameters using "brahms_manager".' 10 ...
		];

	openFolder = inputdlg(msg, 'OpenFolder (BRAHMS)', 1, {openFolder});
	if iscell(openFolder) && ~isempty(openFolder)
		openFolder = openFolder{1};
	end
	if isempty(openFolder)
		return
	end

	brahms_utils('sep', 'user', 'OpenFolder', openFolder);

end


openFolder_ = strrep(openFolder, '((PATH))', path);
[result, output] = system(openFolder_);
if ~result return; end
disp(['Command: ' openFolder]);
disp(['The above command is specified in parameter "OpenFolder", but does not work.']);
disp(['Run <a href="matlab: brahms_manager(''ExecPar=OpenFolder'')">brahms_manager</a> to correct this']);
error('Failed to open folder');




%% argument parser

function varargout = args(argsin, defaults)

% extract
if ~exist('defaults', 'var') defaults = {}; end
requiredCount = nargout - length(defaults);

% match up
for n = 1:nargout
	if length(argsin) >= n
		varargout{n} = argsin{n};
	else
		if n <= requiredCount
			error(['expects ' int2str(requiredCount) ' arguments']);
		end
		varargout{n} = defaults{n - requiredCount};
	end
end



%% string exploder

function c = explode(t, s)

if length(s) && s(end) ~= t
	s = [s t];
end
c = {};
f = [0 find(s == t)];
for n = 1:length(f)-1
	c{n} = s(f(n)+1:f(n+1)-1);
end



%% file writer

function writeto(filename, s)

fid = fopen(filename, 'a');
if fid == -1
	error(['error opening file "' filename '"']);
end
if length(s) && s(end) ~= 10
	s = [s 10];
end
fwrite(fid, s);
fclose(fid);

function writeoscmd(filename, cmd)

[a, b] = system(cmd);
if a b = '<not available>'; end
writeto(filename, ['OS "' cmd '": ' trimws(b)]);

function writeenv(filename, key)

val = getenv(key);
writeto(filename, ['$(' key '): ' trimws(val)]);

function writeenvsplit(filename, key)

c = ':';
if ispc c = ';'; end

val = getenv(key);
writeto(filename, ['$(' key '):']);
while 1
	f = find(val == c, 1);
	if isempty(f)
		writeto(filename, ['    ' val]);
		break;
	end
	writeto(filename, ['    ' val(1:f)]);
	val = val(f+1:end);
	if isempty(val)
		break
	end
end




%% trim whitespace

function s = trimws(s)

f = find(s ~= 10 & s ~= 13 & s ~= 32 & s ~= 9);
s = s(min(f):max(f));




%% build script template

function data = create_build_script(data)

% build script name
if ispc
	data.build_script = 'build.bat';
else
	data.build_script = 'build';
end

% if already exists, we're done
filename = [data.releasepath '/' data.build_script];
if exist(filename, 'file')
	return
end

% template
if isunix
	
	template = [...
	'((CMD))' 10 ...
	];

else
	
	template = [...
	'@echo off' 10 ...
	'((CMD))' 10 ...
	'if ERRORLEVEL 1 goto error' 10 ...
	'del *.obj' 10 ...
	'del *.manifest' 10 ...
	'del *.exp' 10 ...
	'del *.lib' 10 ...
	'exit /B' 10 ...
	10 ...
	':error' 10 ...
	'if "%1" EQU "--matlab" exit 1' 10 ...
	'exit /B 1' 10 ...
	];

end

% choose build parameters (and announce)
bits = brahms_utils('ArchBitsBRAHMS');
disp(['Writing build script for class "' data.cls '"...']);
disp(['Building for ' bits ' bit (based on installed BRAHMS release)']);
disp(' ')

% derived parameters
if ismac
	dos = [' -D__OSX__'];
elseif isunix
	dos = [' -D__GLN__'];
else
	dos = [' -D__WIN__'];
end
ucls = strrep(data.cls, '/', '_');

% common
if isunix
	fe = ' -o ';
 	libname = ['component.so'];
	inc = ' -I"?"';
	lib = ' -L"?"';
	linker = '';
	export = '';
	enginelib = ' -lbrahms-engine';
	cp = 'g++ -fPIC -Werror -pthread -O3 -ffast-math -shared';
	cc = 'gcc -fPIC -Werror -pthread -O3 -ffast-math -shared';
end

if ismac
	fe = ' -o ';
 	libname = ['component.dylib'];
	inc = ' -I"?"';
	lib = ' -L"?"';
	linker = '';
	export = '';
	enginelib = ' -lbrahms-engine';
	
	%% NOTE we assume we are building for 32-bit, here. 64-bit
	%% users must change arch from "i386" to "x86_64"
	cp = 'g++ -fPIC -Werror -O3 -ffast-math -dynamiclib -arch i386';
	cc = 'gcc -fPIC -Werror -O3 -ffast-math -dynamiclib -arch i386';
end

if ispc
	fe = ' -Fe';
 	libname = ['component.dll'];
	inc = ' -I"?"';
	lib = ' -LIBPATH:"?"';
	linker = ' -link';
	export = ' -EXPORT:EventHandler';
	enginelib = ' libbrahms-engine.lib';
	cp = 'cl -nologo -EHsc -Ox -MD -LD';
	cc = 'cl -nologo -Ox -MD -LD';
end

% C/C++
switch data.lang
	case '1199'
		srcname = ' component.cpp';
		cmd = cp;
	case '1266'
		srcname = ' component.c';
		cmd = cc;
end

% SIP
sip = brahms_utils('GetSystemMLInstallPath');

% augment cmd
cmd = [cmd dos srcname fe libname];
cmd = [cmd strrep(inc, '?', [sip '/BRAHMS/include'])];
cmd = [cmd strrep(inc, '?', [sip '/Namespace'])];
cmd = [cmd linker export];
cmd = [cmd strrep(lib, '?', [sip '/BRAHMS/bin'])];
cmd = [cmd enginelib];
cmd = [cmd 10];

% build into template
text = strrep(template, '((CMD))', cmd);
disp(['---------------- script'])
disp(text)
disp(['---------------- script'])
disp(' ')

% write build file
fid = fopen(filename, 'w');
if fid == -1
	error(['failed open "' filename '"']);
end
fwrite(fid, text);
fclose(fid);

% set u+x
if isunix
	system(['chmod u+x "' filename '"']);
end

disp(['A build script has been created in the component folder. It' 10 ...
			'will now be executed to compile your component. If it does' 10 ...
			'not work, you can modify it as required. If you prefer, you' 10 ...
			'can now run this script using the command:' 10 ...
			10 ...
			'brahms_utils rbs ''' data.cls '''' 10 ...
			10 ...
			'Find the script at "' data.releasepath '"' 10 ...
			'(or click "Open" in BRAHMS Manager).' 10 10]);



%% getClassReleasePath

function data = getClassReleasePath(cls, nodepath, lang)

data = [];
data.cls = cls;
data.nodepath = nodepath;
data.release = [];
data.lang = lang;
data.releasepath = '';

% for each release
path = [nodepath '/brahms/'];
rel = -1;
while true
	rel = rel + 1;
	relpath = [path int2str(rel)];
	if ~exist(relpath)
		break
	end
	releasefile = [relpath '/release.xml'];
	if ~exist(releasefile, 'file')
		error(['file not found "' releasefile '"']);
	end
	rml = sml_xml('file2xml', releasefile);
	lang = xml_getField(rml, 'Language');
	if ~isempty(data.lang) && ~strcmp(data.lang, lang)
		continue
	end
	data.lang = lang;
	data.releasepath = relpath;
end



%% isinlist

function b = isinlist(list, value)

b = false;
if ispc
	value = lower(value);
end
value(value == '\') = '/';

for n = 1:length(list)
	l = list{n};
	l(l == '\') = '/';
	if ispc
		s = strcmp(value, lower(l));
	else
		s = strcmp(value, l);
	end
	if s
		b = true;
		return
	end
end


%% warning

function dlg_warning(msg)

isoctave = brahms_utils('IsOctave');

if isoctave
	warning(msg);
else
	uiwait(warndlg(msg, 'Warning'));
end


%% inputstring

function o = breaklines(s)

o = '';

while length(s) > 60
	f = find(s == 32);
	if isempty(f)
		break
	end
	g = f(f<=60);
	if isempty(g)
		g = f(1);
	else
		g = g(end);
	end
	o = [o s(1:g-1) 10];
	s = s(g+1:end);
end

o = [o s];

function val = dlg_inputstring(val, msg, name)

isoctave = brahms_utils('IsOctave');

if isoctave
	disp(['________________________________________________________________' 10])
	disp(['Choose "' name '" (press ENTER to cancel)'])
	disp(breaklines(msg))
	val = input([10 '---> '], 's');
else
	val = inputdlg(msg, ['Choose "' name '"'], 1, {val});
	if length(val)
		val = val{1};
	end
end


%% confirm

function c = dlg_confirm(msg)

isoctave = brahms_utils('IsOctave');

if isoctave
else
	c = questdlg(msg, 'Confirm', 'OK', 'Cancel', 'OK');
	if strcmp(c, 'Cancel')
		c = false;
		return
	end
	c = true;
end

%% choose

function val = dlg_choose(vals, descs, val, msg, name)

isoctave = brahms_utils('IsOctave');

if ~isempty(val)
	
	val(val == '\') = '/';
	while length(val) && val(end) == '/'
		val = val(1:end-1);
	end
	if ~isinlist(vals, val)
		error(['"' val '" is not a valid value for "' name '"']);
	end
	
else

	if isoctave
		disp(['________________________________________________________________' 10])
		disp(['Choose "' name '" (press ENTER to cancel)'])
		disp(msg)
		for n = 1:length(vals)
			disp([int2str(n) ') ' descs{n}])
		end
		sel = 0;
		while true
			sel = input([10 '---> ']);
			if isempty(sel)
				return
			end
			if prod(size(sel)) ~= 1
				continue
			end
			if sel < 1 || sel > length(vals)
				disp(['choose a number between 1 and ' int2str(length(vals))])
				continue
			end
			break
		end
		val = vals{sel};
		disp(' ')
	else
		[sel, ok] = listdlg('ListString', descs, 'SelectionMode', 'single', ...
			'ListSize', [480 240], ...
			'PromptString', msg, 'Name', ['Choose "' name '"']);
		if ~ok
			return;
		end
		val = vals{sel};
	end

end




%% VALIDATE CLASS NAME

function s = validateClassName(c)

% no error
s = '';

% is empty
if isempty(c)
	s = 'is empty';
	return
end

% % must start with "dev/"
% if length(c) < 4 || ~strcmp(c(1:4), 'dev/')
% 	s = 'it does not start with "dev/"';
% 	return
% end

% find slashes, and validate positions
L = length(c);
f = find(c == '/');
if any(f == 1) || any(f == L)
	s = 'it begins or ends with forward slash';
	return
end
if any(diff(f) == 1)
	s = 'it contains double forward slash';
	return
end

% explode by slash
f = [0 f L+1];
N = length(f) - 1;
for n = 1:N
	p = c(f(n)+1:f(n+1)-1);
	s = validateClassNamePart(p);
	if ~isempty(s)
		return
	end
end


function s = validateClassNamePart(p)

% no error
s = '';

% analyze
alpha = (p >= 65 & p <= 90) | (p >= 97 & p <= 122);
numer = (p >= 48 & p <= 57);
under = (p == '_');

% % must start with alpha
% if ~alpha(1)
% 	s = ['part "' p '" does not start with a letter'];
% 	return
% end

% must continue with alpha/numer/under
a = alpha | numer | under;
if any(~a)
	s = ['part "' p '" contains characters that are not alphanumeric or underscore'];
	return
end








function text = specificate(text, data)

ucls = strrep(data.cls, '/', '_');
text = strrep(text, '__TEMPLATE_CLASS_STRING_C__', ['"' data.cls '"']);
text = strrep(text, '__TEMPLATE__CLASS_CPP__', [ucls '_' data.release]);
text = strrep(text, '__TEMPLATE__RELEASE_INT__', [data.release]);
text = strrep(text, '__TEMPLATE__REVISION_INT__', '1');
text = strrep(text, '__TEMPLATE__ADDITIONAL_C__', ['"Author=' data.username '\n" "URL=Not supplied\n"']);
text = strrep(text, '__TEMPLATE__FLAGS_C__', '(F_NOT_RATE_CHANGER)');

text = strrep(text, '__TEMPLATE__ADDITIONAL_M__', ['[''Author=' data.username '\n'' ''URL=Not supplied\n'']']);
text = strrep(text, '__TEMPLATE__FLAGS_M__', '[persist.constants.F_NOT_RATE_CHANGER]');

text = strrep(text, '__TEMPLATE__ADDITIONAL_PY__', ['(''Author=' data.username '\n'' ''URL=Not supplied\n'')']);
text = strrep(text, '__TEMPLATE__FLAGS_PY__', '(F_NOT_RATE_CHANGER)');


function text = specificate_test(text, data)

text = strrep(text, '__TEMPLATE_CLASS_STRING_M__', ['''' data.cls '''']);



function s = cuthere(s)

f = strfind(s, 'CUT HERE');
if isempty(f)
	return
end
s = s(f:end);
f = find(s == 10, 1);
if ~isempty(f)
	s = s(f+1:end);
end
