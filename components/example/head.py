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
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#__________________________________________________________________________
#
# $Id:: head.py 1751 2009-04-01 23:36:40Z benjmitch                      $
# $Rev:: 1751                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)                   $
#__________________________________________________________________________
#

import brahms
import copy
import math
import numpy

def brahms_process(persist, input):


	output = copy.deepcopy(persist['output'])


	if input['event']['type'] == EVENT_INIT_CONNECT:

		# create output on first call
		if input['event']['flags'] & F_FIRST_CALL:

			# create scalar output (muscle position)
			#
			# operation: addPort, default set, class numeric, dims and
			# type, output port name, sample rate
			persist['outputPortHandle'] = brahms.operation(
				persist['self'],
				OPERATION_ADD_PORT,
				'',
				'std/2009/data/numeric',
				'DOUBLE/REAL/1',
				'tilt'
			)

			# set initial persist
			persist['position'] = 0

		# processed
		output['event']['response'] = C_OK



	if input['event']['type'] == EVENT_INIT_POSTCONNECT:

		# can get structure of input now
		persist['num_channels'] = int(input['iif']['default']['ports'][0]['structure'])

		# processed
		output['event']['response'] = C_OK




	if input['event']['type'] == EVENT_RUN_SERVICE:

		# read input (spikes)
		drive = copy.copy(input['iif']['default']['ports'][0]['data'])

		# convert to activity
		drive = sum(drive) / persist['num_channels']

		# run dynamics
		tau = 0.2
		fS = input['time']['sampleRate']['rate']
		Lambda = math.exp(-1/(tau * fS))
		T = 1 / fS
		persist['position'] = Lambda * persist['position'] + T * drive

		# update output
		brahms.operation(
			persist['self'],
			OPERATION_SET_CONTENT,
			persist['outputPortHandle'],
			numpy.array(persist['position'], numpy.float64)
		)

		# processed
		output['event']['response'] = C_OK



	if input['event']['type'] == EVENT_MODULE_INIT:

		# update output
		output['info']['component'] = (0, 1)
		output['info']['additional'] = 'Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n'

		# processed
		output['event']['response'] = C_OK



	return (persist, output)
