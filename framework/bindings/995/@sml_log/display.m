%
% sml_log::display


%__________________________________________________________________________
%
% This file is part of the SystemML Toolbox
% Copyright (C) 2007 Ben Mitch(inson)
% URL: http://sourceforge.net/projects/systemml
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
% $Id:: display.m 2397 2009-11-18 21:20:07Z benjmitch                    $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function display(log)

disp([10 'ans =' 10]);

sz = size(log);
if prod(sz) ~= 1
	f = fieldnames(log);
	sz = sprintf('%ux', sz);
	if length(sz)
		sz = sz(1:end-1);
	else
		sz = '0x0';
	end
	disp([sz ' sml_log with fields:'])
	disp(f)
	return
end

if isstruct(log.fields)
	f = fieldnames(log.fields);
else
	f = {};
end
lmax = 0;
for i = 1:length(f)
	l = length(f{i});
	if l > lmax lmax = l; end
end

for i = 1:length(f)
	l = length(f{i});
	v = log.fields.(f{i});
	if isnumeric(v)
		cls = class(v);
		sz = size(v);
		if ~isreal(v)
			dsp = [xsize(sz) ' complex ' cls];
		else
			dsp = [xsize(sz) ' ' cls];
		end
	else
		switch class(v)
			case 'sml_log'
				dsp = 'sml_log';
			case 'struct'
				if isfield(v, 'sml_log____interface_to_file')
					if v.complex
						dsp = [xsize(v.size) ' complex ' v.matcls ' (read-on-demand)'];
					else
						dsp = [xsize(v.size) ' ' v.matcls ' (read-on-demand)'];
					end
				else
					dsp = [xsize(size(v)) ' struct'];
				end
			otherwise
				dsp = ['display case not coded for class "' class(v) '"'];
		end
	end
	disp(['    ' repmat(' ', 1, lmax-l) f{i} ': [' dsp ']'])
end

disp(' ')



function s = xsize(s)

s = sprintf('%ix', s);
s = s(1:end-1);




