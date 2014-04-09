%
% sml_log::merge


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
% $Id:: merge.m 2397 2009-11-18 21:20:07Z benjmitch                      $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%


function log = merge(add, log)

if isempty(log)
	log = add;
	return
end

if ~isa(log, 'sml_log')
	error(['can only merge sml_log with another sml_log (or empty)']);
end

if isempty(add.fields)
	return
end

f = fieldnames(add.fields);
for i = 1:length(f)
	if isfield(log.fields, f{i})
		log.fields.(f{i}) = merge(add.fields.(f{i}), log.fields.(f{i}));
	else
		log.fields.(f{i}) = add.fields.(f{i});
	end
end


