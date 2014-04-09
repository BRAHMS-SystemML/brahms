%
% systemml::subsref
%
%   This is a private function.
%
%
% __________________________________________________________
% This function is part of the "SystemML Toolbox".


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
% $Id:: subsref.m 2397 2009-11-18 21:20:07Z benjmitch                    $
% $Rev:: 2397                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-18 21:20:07 +0000 (Wed, 18 Nov 2009)                   $
%__________________________________________________________________________
%

function out = subsref(sys, ref)

switch ref(1).type
	
	case '.'
		switch ref(1).subs
			
% 			case 'time'
% 				if length(ref) > 1
% 					out = subsref(sys.time, ref(2:end));
% 				else
% 					out = sys.time;
% 				end
% 				return;
							
			case 'serialize'
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				serialize(sys, ref(2).subs{:});
				return;
							
			case 'unserialize'
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = unserialize(sys, ref(2).subs{:});
				return;
							
			case {'addprocess'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = addprocess(sys, ref(2).subs{:});
				return;

			case {'addsubsystem'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = addsubsystem(sys, ref(2).subs{:});
				return;

			case {'removeprocess'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = removeprocess(sys, ref(2).subs{:});
				return;

			case {'removesubsystem'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = removesubsystem(sys, ref(2).subs{:});
				return;

			case {'link'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = link(sys, ref(2).subs{:});
				return;

			case {'expose'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = expose(sys, ref(2).subs{:});
				return;

			case {'unlink'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = unlink(sys, ref(2).subs{:});
				return;

			case {'unexpose'}
				if ~strcmp(ref(2).type, '()')
					error('invalid reference');
				end
				out = unexpose(sys, ref(2).subs{:});
				return;
				
			case 'processes'
				% return a list of process names
				if length(ref) == 2
					if strcmp(ref(2).type, '{}') && length(ref(2).subs) == 1
						out = sys.proc{ref(2).subs{1}}.name;
						return
					end
				elseif length(ref) == 1
					out = {};
					for n = 1:length(sys.proc)
						out{n} = sys.proc{n}.name;
					end
					return
				end
				error('invalid reference');

			case 'links'
				% return a list of link names
				if length(ref) == 2
					if strcmp(ref(2).type, '{}') && length(ref(2).subs) == 1
						out = sys.link{ref(2).subs{1}}.name;
						return
					end
				elseif length(ref) == 1
					out = {};
					for n = 1:length(sys.link)
						out{n} = sys.link{n}.name;
					end
					return
				end
				error('invalid reference');

			case 'subsystems'
				% return a list of subsystem names
				out = {};
				for n = 1:length(sys.sub)
					out{n} = sys.sub{n}.name;
				end
				return

			otherwise
				% look for a process with that name
				for n = 1:length(sys.proc)
					if strcmp(sys.proc{n}.name, ref(1).subs)
						if length(ref) > 1
							out = subsref(sys.proc{n}, ref(2:end));
						else
							out = sys.proc{n};
						end
						return;
					end
				end
				
				% look for a sub-system with that name
				for n = 1:length(sys.sub)
					if strcmp(sys.sub{n}.name, ref(1).subs)
						if length(ref) > 1
							out = subsref(sys.sub{n}, ref(2:end));
						else
							out = sys.sub{n};
						end
						return;
					end
				end
				
		end
		
		error(['field not found "' ref(1).subs '"']);

	otherwise
		error('invalid reference')
		
end
