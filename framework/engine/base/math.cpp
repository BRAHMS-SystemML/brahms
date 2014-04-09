/*
________________________________________________________________

	This file is part of BRAHMS
	Copyright (C) 2007 Ben Mitchinson
	URL: http://brahms.sourceforge.net

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
________________________________________________________________

	Subversion Repository Information (automatically updated on commit)

	$Id:: math.cpp 2278 2009-11-01 21:23:08Z benjmitch         $
	$Rev:: 2278                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-01 21:23:08 +0000 (Sun, 01 Nov 2009)       $
________________________________________________________________

*/



namespace brahms
{
	namespace math
	{
		//	Greatest common divisor (textbook Euclidean algorithm)
		//	by Tak-Shing
		//
		//	Quadratic time complexity O(n^2) in number of bits of the
		//	larger integer.  Compare exponential time complexity for
		//	factorization: even with the fastest quadratic sieve it is
		//	O(exp(sqrt(n log n))) in number of bits.

		UINT64 gcd(UINT64 a, UINT64 b)
		{
			return b ? gcd(b, a % b) : a;
		}

		INT32 unitIndex(INT32 zeroBasedIndex)
		{
			/*

				This function just adds one - we use it to
				indicate the semantic purpose of the operation,
				that is to change from zero-based indexing
				into unity-based indexing for display to the user.

			*/

			//	just add one
			return zeroBasedIndex + 1;
		}
	}
}



