#__________________________________________________________________________
#
# This file is part of BRAHMS
# Copyright (C) 2007 Ben Mitch(inson)
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
# You should have received a cop0.5,  0.5y of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#__________________________________________________________________________
#
# $Id:: rabbits.py 1751 2009-04-01 23:36:40Z benjmitch                   $
# $Rev:: 1751                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)                   $
#__________________________________________________________________________
#

import brahms
import copy
import numpy

def brahms_process(persist, input):


	output = copy.deepcopy(persist['output'])


	if input['event']['type'] == EVENT_INIT_CONNECT:
	
		# create output on first call
		if input['event']['flags'] & F_FIRST_CALL:

			# create scalar output (rabbit population)
			#
			# operation: addPort, default set, class numeric, dims and
			# type, output port name, sample rate
			persist['outputPortHandle'] = brahms.operation(
				persist['self'],
				OPERATION_ADD_PORT,
				'',
				'std/2009/data/numeric',
				'DOUBLE/REAL/1',
				'out'
			)

			# set initial persist
			persist['rabbits'] = 0.5

		# processed
		output['event']['response'] = C_OK



	elif input['event']['type'] == EVENT_INIT_POSTCONNECT:

		# processed
		output['event']['response'] = C_OK




	elif input['event']['type'] == EVENT_RUN_SERVICE:

		# read input
		foxes = input['iif']['default']['ports'][0]['data']

		# run dynamics
		T = 1 / input['time']['sampleRate']['rate']
		d_rabbits = 2 * persist['rabbits'] - 1.2 * persist['rabbits'] * foxes
		persist['rabbits'] = persist['rabbits'] + T * d_rabbits

		# update output
		brahms.operation(
			persist['self'],
			OPERATION_SET_CONTENT,
			persist['outputPortHandle'],
			numpy.array(persist['rabbits'], numpy.float64)
		)

		# processed
		output['event']['response'] = C_OK



	elif input['event']['type'] == EVENT_MODULE_INIT:

		# update output
		output['info']['component'] = (0, 1)
		output['info']['additional'] = 'Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n'

		# processed
		output['event']['response'] = C_OK



	return (persist, output)
