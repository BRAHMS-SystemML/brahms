#__________________________________________________________________________
#
# This file is part of BRAHMS
# Copyright (C) 2007 Ben Mitchinson
# URL: http://sourceforge.net/projects/brahms
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#__________________________________________________________________________
#
# $Id:: process.py 2324 2009-11-08 00:47:49Z benjmitch                   $
# $Rev:: 2324                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-11-08 00:47:49 +0000 (Sun, 08 Nov 2009)                   $
#__________________________________________________________________________
#






import brahms
import copy
import numpy


# event function
def brahms_process(persist, input):



	# nominal output (must use deepcopy)
	output = copy.deepcopy(persist['output'])



	# switch on event type
	if input['event']['type'] == EVENT_MODULE_INIT:

		# provide component information
		output['info']['flags'] = F_NEEDS_ALL_INPUTS + F_NOT_RATE_CHANGER
		output['info']['component'] = (__REL__, __REV__)
		output['info']['additional'] = 'Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n'
		
		# ok
		output['event']['response'] = C_OK

	
		
	# switch on event type
	elif input['event']['type'] == EVENT_STATE_SET:

		# say hello
		brahms.operation(
			persist['self'],
			OPERATION_BOUT,
			'Hello from Python',
			D_INFO
			)

		# set state
		persist['accum'] = 0

		# get random seed from framework
		seed = brahms.operation(
			persist['self'],
			OPERATION_GET_RANDOM_SEED
		)

		# create an RNG utility
		rng = brahms.operation(
			persist['self'],
			OPERATION_GET_UTILITY_OBJECT,
			'std/2009/util/rng',
			0,
		)

		# get select function from RNG utility
		fselect = brahms.operation(
			persist['self'],
			OPERATION_GET_UTILITY_FUNCTION,
			rng,
			'select'
		)

		# get seed function from RNG utility
		fseed = brahms.operation(
			persist['self'],
			OPERATION_GET_UTILITY_FUNCTION,
			rng,
			'seed')

		# get fill function from RNG utility
		persist['ffill'] = brahms.operation(
			persist['self'],
			OPERATION_GET_UTILITY_FUNCTION,
			rng,
			'fill'
		)

		# select the MT2000.normal generator
		brahms.operation(
			persist['self'],
			OPERATION_CALL_UTILITY_FUNCTION,
			fselect,
			('MT2000.normal', )
		)

		# seed the generator
		brahms.operation(
			persist['self'],
			OPERATION_CALL_UTILITY_FUNCTION,
			fseed,
			(seed, )
		)

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_INIT_CONNECT:

		# validate input
		p = input['iif']['default']['ports']
		if len(p) != 1:
			output['error'] = 'expects one input'
			return (persist, output)
		if p[0]['class'] != 'std/2009/data/numeric':
			output['error'] = 'expects input to be data/numeric class'
			return (persist, output)
		if p[0]['structure'] != 'DOUBLE/REAL/1':
			output['error'] = 'expects scalar real double input'
			return (persist, output)

		# create output
		persist['hOutputPort'] = brahms.operation(
			persist['self'],
			OPERATION_ADD_PORT,
			'',
			'std/2009/data/numeric',
			'DOUBLE/REAL/' + str(int(persist['state']['count'][0])),
			'out'
		)

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_RUN_SERVICE:

		# get input
		inp = input['iif']['default']['ports'][0]['data']

		# process
		persist['accum'] = 0.9 * persist['accum'] + inp

		# get values from the generator
		rnd = brahms.operation(
			persist['self'],
			OPERATION_CALL_UTILITY_FUNCTION,
			persist['ffill'],
			(
				numpy.array([]),
				numpy.array(persist['state']['count'], numpy.uint32),
				numpy.array(persist['state']['noise']),
				numpy.array(0.0)
			)
		)

		# spread to multiple channels and add some noise
		myOutput = persist['accum'] * numpy.ones(persist['state']['count'], numpy.float64) + rnd

		# set output
		brahms.operation(persist['self'], OPERATION_SET_CONTENT, persist['hOutputPort'], myOutput)

		# ok
		output['event']['response'] = C_OK



	# return
	return (persist, output)
