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
# $Id:: component.py 2406 2009-11-19 20:53:19Z benjmitch                 $
# $Rev:: 2406                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-11-19 20:53:19 +0000 (Thu, 19 Nov 2009)                   $
#__________________________________________________________________________
#

# CUT HERE

# This is a newly created BRAHMS Process. It is a non-native
# process, and does not need to be built - BRAHMS can run it
# as it stands.





import brahms
import numpy


# event function
def brahms_process(persist, input):

	# nominal output
	output = {'event':{'response':S_NULL}}



	# switch on event type
	if input['event']['type'] == EVENT_MODULE_INIT:

		# provide component information
		output['info'] = {}
		output['info']['component'] = (__TEMPLATE__RELEASE_INT__, __TEMPLATE__REVISION_INT__)
		output['info']['additional'] = __TEMPLATE__ADDITIONAL_PY__
		output['info']['flags'] = __TEMPLATE__FLAGS_PY__
		
		# ok
		output['event']['response'] = C_OK

	
		
	# switch on event type
	elif input['event']['type'] == EVENT_STATE_SET:

		# EXAMPLE
		#
		#print persist['state']['parameter_1']
		#print persist['state']['parameter_2']

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_INIT_CONNECT:

		# on first call
		if input['event']['flags'] & F_FIRST_CALL:

			# EXAMPLE: access input port "X"
			#
			#index = input['iif']['default']['index']['X']
			#port = input['iif']['default']['ports'][index]
			
			# EXAMPLE: check it is valid (2 by 4 matrix)
			#
			#if not port['structure'] == 'DOUBLE/REAL/2,4':
			#	output['error'] = "input 'X' is invalid (" + port['structure'] + ")";
			#	return(persist, output)

			# EXAMPLE: store handle to its data object
			#
			#persist['X'] = port['data']

			# do nothing
			pass

		# on last call
		if input['event']['flags'] & F_LAST_CALL:

			# EXAMPLE: create scalar output "Y" (and store its handle in hOutputPort)
			#
			#persist['hOutputPort'] = brahms.operation(
			#	persist['self'],
			#	OPERATION_ADD_PORT,
			#	'',
			#	'std/2009/data/numeric',
			#	'DOUBLE/REAL/1',
			#	'Y'
			#)

			# do nothing
			pass

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_RUN_SERVICE:

		# EXAMPLE: get data from input
		#
		#X = persist['X']

		# do computation...

		# EXAMPLE: set value of output
		#
		#brahms.operation(
		#	persist['self'],
		#	OPERATION_SET_CONTENT,
		#	persist['hOutputPort'],
		#	ascontiguousarray(result)
		#)

		# ok
		output['event']['response'] = C_OK



	# return
	return (persist, output)
