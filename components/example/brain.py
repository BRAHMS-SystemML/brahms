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
# $Id:: brain.py 1751 2009-04-01 23:36:40Z benjmitch                     $
# $Rev:: 1751                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-04-02 00:36:40 +0100 (Thu, 02 Apr 2009)                   $
#__________________________________________________________________________
#

import brahms
import copy
import math
import random
import numpy

def brahms_process(persist, input):


	output = copy.deepcopy(persist['output'])


	if input['event']['type'] == EVENT_STATE_SET:

		# set initial pars
		persist['drive'] = 0
		persist['num_channels'] = int(persist['state']['numchannels']) # use parameter!
		persist['refractory_time'] = 0



	if input['event']['type'] == EVENT_INIT_CONNECT:
	
		# create output on first call
		if input['event']['flags'] & F_FIRST_CALL:

			# create spikes output (muscle drive)
			#
			# operation: addPort, default set, class numeric, dims and
			# type, output port name, sample rate
			persist['outputPortHandle'] = brahms.operation(
				persist['self'],
				OPERATION_ADD_PORT,
				'',
				'std/2009/data/spikes',
				str(persist['num_channels']),
				'out'
			)

		# processed
		output['event']['response'] = C_OK




	if input['event']['type'] == EVENT_RUN_SERVICE:

		# read input
		t_now = float(input['time']['now']) * input['time']['baseSampleRate']['period']
		head_tilt = input['iif']['default']['ports'][0]['data']
		if head_tilt > 1:
			persist['refractory_time'] = t_now + 0.1

		# run dynamics
		tau = 0.1
		target_drive = 1
		fS = input['time']['sampleRate']['rate']
		Lambda = math.exp(-1/(tau * fS))
		persist['drive'] = Lambda * persist['drive'] + (1-Lambda) * target_drive

		# refractory period (!!????!)
		if persist['refractory_time'] > t_now:
			persist['drive'] = 0

		# generate spiking output
		max_drive = 0.5
		prob_channel_on = max_drive * persist['drive']
		out = [chs for chs in xrange(persist['num_channels']) if random.random() < prob_channel_on]

		# update output
		brahms.operation(
			persist['self'],
			OPERATION_SET_CONTENT,
			persist['outputPortHandle'],
			numpy.array(out, numpy.int32)
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
