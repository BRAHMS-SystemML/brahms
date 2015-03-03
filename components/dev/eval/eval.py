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
# $Id:: eval.py 2419 2009-11-30 18:33:48Z benjmitch                      $
# $Rev:: 2419                                                            $
# $Author:: benjmitch                                                    $
# $Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)                   $
#__________________________________________________________________________
#






import brahms
from numpy import *

# helpers
def numpyType2BrahmsType(t):

	if t == dtype('complex128'): return "DOUBLE/COMPLEX/"
	if t == dtype('complex64'): return "SINGLE/COMPLEX/"
	if t == dtype('float64'): return "DOUBLE/REAL/"
	if t == dtype('float32'): return "SINGLE/REAL/"
	if t == dtype('uint64'): return "UINT64/REAL/"
	if t == dtype('uint32'): return "UINT32/REAL/"
	if t == dtype('uint16'): return "UINT16/REAL/"
	if t == dtype('uint8'): return "UINT8/REAL/"
	if t == dtype('int64'): return "INT64/REAL/"
	if t == dtype('int32'): return "INT32/REAL/"
	if t == dtype('int16'): return "INT16/REAL/"
	if t == dtype('int8'): return "INT8/REAL/"
	
	raise Exception("type not found in numpyType2BrahmsType()")

	# don't need this, it's all in the dtype
	#iscmpx = iscomplexobj(result)
		


# event function
def brahms_process(persist, input):

	# nominal output
	output = {'event':{'response':S_NULL}}



	# switch on event type
	if input['event']['type'] == EVENT_MODULE_INIT:

		# provide component information
		output['info'] = {}
		output['info']['flags'] = F_NEEDS_ALL_INPUTS + F_NOT_RATE_CHANGER
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

		# create empty table
		persist['inputs'] = {}

		# access inputs
		ports = input['iif']['default']['ports']
		nports = len(ports)

		# for each input
		for p in range(0, nports):

			# access input
			port = ports[p]
			name = port['name']
			persist['inputs'][name] = port['data']

		# import
		for k in persist['inputs'].keys():
			persist['state']['function'] = persist['state']['function'].replace("$" + k, "persist['" + k + "']")
			exec("persist['" + k + "']=persist['inputs']['" + k + "']")

		# compute function
		result = eval(persist['state']['function'])

		# get data type of result
		struc = numpyType2BrahmsType(result.dtype)

		# get dimensions of result
		dims = shape(result)
		ndims = size(dims)
		sdims = ""
		for i in range(0,ndims):
			if i:
				sdims = "," + sdims
			sdims = str(dims[i]) + sdims
		struc += sdims

		# create output
		persist['hOutputPort'] = brahms.operation(
			persist['self'],
			OPERATION_ADD_PORT,
			'',
			'std/2009/data/numeric',
			struc,
			'out'
		)

		# ok
		output['event']['response'] = C_OK



	# switch on event type
	elif input['event']['type'] == EVENT_RUN_SERVICE:

		# changing from storing as persist['inputs']['varname'] to
		# persist['varname'] saved 14us a call (a call costs about
		# 175us now). that is to say, that indexing into these
		# arrays is a significant performance hit. if there was a
		# way of setting up the execution so it was "ready to go"
		# without any indexing, performance might improve considerably

		# in addition, we currently have to call ascontiguousarray() on
		# result before we pass it back to the bindings (if it was complex,
		# and we reduce it to real, for instance, it forms a non-contiguous
		# array). if the bindings could handle these sparse arrays, it would
		# be faster than converting it first.

		# compute function
		result = eval(persist['state']['function'])

		# set output
		brahms.operation(persist['self'], OPERATION_SET_CONTENT, persist['hOutputPort'], ascontiguousarray(result))

		# ok
		output['event']['response'] = C_OK



	# return
	return (persist, output)
