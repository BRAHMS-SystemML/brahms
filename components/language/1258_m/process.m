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
% $Id:: process.m 2324 2009-11-08 00:47:49Z benjmitch                    $
% $Rev:: 2324                                                            $
% $Author:: benjmitch                                                    $
% $Date:: 2009-11-08 00:47:49 +0000 (Sun, 08 Nov 2009)                   $
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
		output.info.flags = persist.constants.F_NEEDS_ALL_INPUTS + persist.constants.F_NOT_RATE_CHANGER;
		output.info.component = [__REL__ __REV__];
		output.info.additional = 'Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n';
		
		% ok
		output.event.response = persist.constants.C_OK;

	
		
	case persist.constants.EVENT_STATE_SET
		
		switch input.event.continuation

			case 0

				% say hello
				output.operations{end+1} = {
					persist.constants.OPERATION_BOUT,
					'Hello from Matlab',
					persist.constants.D_INFO
					};

				% initalise state
				persist.accum = 0;
				
				% get random seed from framework
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_RANDOM_SEED
					};

				% create an RNG utility
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_UTILITY_OBJECT,
					'std/2009/util/rng',
					0
					'rng'
					};

				% request event continuation
				output.event.continuation = 1;

			case 1

				% get select function from RNG utility
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_UTILITY_FUNCTION,
					input.objects.rng.id,
					'select'
					};

				% get seed function from RNG utility
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_UTILITY_FUNCTION,
					input.objects.rng.id,
					'seed'
					};

				% get fill function from RNG utility
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_UTILITY_FUNCTION,
					input.objects.rng.id,
					'fill'
					};

				% get get function from RNG utility
				output.operations{end+1} = {
					persist.constants.OPERATION_GET_UTILITY_FUNCTION,
					input.objects.rng.id,
					'get'
					};

				% request event continuation
				output.event.continuation = 2;

			case 2

				% select the MT2000.normal generator
				output.operations{end+1} = {
					persist.constants.OPERATION_CALL_UTILITY_FUNCTION,
					input.objects.rng.select.id,
					{'MT2000.normal'}
					};

				% seed the generator
				output.operations{end+1} = {
					persist.constants.OPERATION_CALL_UTILITY_FUNCTION,
					input.objects.rng.seed.id,
					{persist.seed}
					};

		end

		% ok
		output.event.response = persist.constants.C_OK;

		
	
	case persist.constants.EVENT_INIT_CONNECT
		
		% validate input
		p = input.iif.default.ports;
		if length(p) ~= 1
			output.error = 'expects one input';
			return
		end
		if ~strcmp(p.class, 'std/2009/data/numeric')
			output.error = 'expects input to be data/numeric class';
			return
		end
		if ~strcmp(p.structure, 'DOUBLE/REAL/1')
			output.error = 'expects scalar real double input';
			return
		end
		
		% create real double output
		output.operations{end+1} = {
			persist.constants.OPERATION_ADD_PORT
			''
			'std/2009/data/numeric'
			['DOUBLE/REAL/' int2str(persist.state.count)]
			'out'
			};
		
		% ok
		output.event.response = persist.constants.C_OK;



	case persist.constants.EVENT_RUN_SERVICE

		switch input.event.continuation

			case 0

				% get input
				in = input.iif.default.ports.data;
				
				% process
				persist.accum = 0.9 * persist.accum + in;

				% get values from the generator
				output.operations{end+1} = {
					persist.constants.OPERATION_CALL_UTILITY_FUNCTION,
					input.objects.rng.fill.id,
					{[], uint32(persist.state.count), persist.state.noise, 0}
					};

				% request event continuation
				output.event.continuation = 1;
				
			case 1	

				% receive random values
				rnd = input.objects.rng.fill.output{1};

				% spread to multiple channels and add some noise
				myOutput = persist.accum * ones(persist.state.count, 1) + rnd;

				% set output
				output.operations{end+1} = ...
					{persist.constants.OPERATION_SET_CONTENT, input.oif.default.out, myOutput};


		end

		% ok
		output.event.response = persist.constants.C_OK;



end

