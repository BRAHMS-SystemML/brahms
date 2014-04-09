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
		output['info']['flags'] = F_NOT_RATE_CHANGER
		output['info']['component'] = (__REL__, __REV__)
		output['info']['additional'] = 'Author=Ben Mitch\nURL=http://brahms.sourceforge.net\n'
		
		# ok
		output['event']['response'] = C_OK

	
		
	# switch on event type
	elif input['event']['type'] == EVENT_STATE_SET:

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_INIT_CONNECT:

		# on first call
		if input['event']['flags'] & F_FIRST_CALL:

			# create output
			persist['hOutputPort'] = brahms.operation(
				persist['self'],
				OPERATION_ADD_PORT,
				'',
				'std/2009/data/numeric',
				'DOUBLE/COMPLEX/2,3',
				'out'
			)

		# on first call
		if input['event']['flags'] & F_LAST_CALL:

			# validate input
			p = input['iif']['default']['ports']
			if len(p) != 1:
				output['error'] = 'expects one input'
				return (persist, output)
			if p[0]['class'] != 'std/2009/data/numeric':
				output['error'] = 'expects input to be data/numeric class'
				return (persist, output)
			if p[0]['structure'] != 'DOUBLE/COMPLEX/2,3':
				output['error'] = 'expects 2x3 complex input'
				return (persist, output)

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_RUN_SERVICE:

		# get input
		inp = input['iif']['default']['ports'][0]['data']

		inp[2][0] = 40+50j
		#print inp.shape

		# set output
		brahms.operation(persist['self'], OPERATION_SET_CONTENT, persist['hOutputPort'], inp)

		# ok
		output['event']['response'] = C_OK



	# return
	return (persist, output)
