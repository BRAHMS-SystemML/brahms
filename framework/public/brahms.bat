


::	On a windows system, this wrapper allows you to call BRAHMS with
::	the loader path PATH conveniently set. There are three possible
::	outcomes:
::
::		1) Lockfile deleted, error level 0 (success).
::		2) Lockfile deleted, error level 1 (exception).
::		3) Lockfile not deleted (abort).
::
::	Usage:
::
::		brahms ...
::
::	The first form will pass all arguments straight through to the
::	execute function, after setting the loader path appropriately.
::
::	From Matlab:
::
::		brahms matlab ...
::
::	Called as above, the wrapper will exit the shell on failure,
::	which forces Matlab to acknowledge the failure return code, rather
::	than just setting ERRORLEVEL, which Matlab will ignore.
::
::
::
::________________________________________________________________
::
::	This file is part of BRAHMS
::	Copyright (C) 2007 Ben Mitchinson
::	URL: http:sourceforge.net/projects/abrg-brahms
::
::	This program is free software; you can redistribute it and/or
::	modify it under the terms of the GNU General Public License
::	as published by the Free Software Foundation; either version 2
::	of the License, or (at your option) any later version.
::
::	This program is distributed in the hope that it will be useful,
::	but WITHOUT ANY WARRANTY; without even the implied warranty of
::	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
::	GNU General Public License for more details.
::
::	You should have received a copy of the GNU General Public License
::	along with this program; if not, write to the Free Software
::	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
::________________________________________________________________
::
::	Subversion Repository Information (automatically updated on commit)
::
::	$Id:: brahms.bat 2276 2009-11-01 17:30:15Z benjmitch       $
::	$Rev:: 2276                                                $
::	$Author:: benjmitch                                        $
::	$Date:: 2009-11-01 17:30:15 +0000 (Sun, 01 Nov 2009)       $
::________________________________________________________________
::




::	NB This script is not really used in anger any more - the PATH variable
::	should have the bin folder on it after installation, and the matlab
::	bindings now call brahms-execute directly. However, we need to keep
::	it so that "brahms ..." is just a transparent (effectively) wrapper
::	for "brahms-execute ...". Around R2134 I reduced it to a very simplistic
::	wrapper; see an earlier revision for the original form.
::
::	The original form made sure that the bin folder was on PATH, and behaved
::	slightly differently on error if called with an argument "matlab". We no
::	longer need the PATH addition, since this is part of the install, and we
::	no longer need the special matlab behaviour, since just allowing the script
::	to drop out propagates the ERRORLEVEL correctly both in the shell and from
::	Matlab.


@echo off



:: call BRAHMS executable
brahms-execute %1 %2 %3 %4 %5 %6 %7 %8


