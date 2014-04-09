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

	$Id:: link.cpp 2256 2009-10-31 15:45:54Z benjmitch         $
	$Rev:: 2256                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-10-31 15:45:54 +0000 (Sat, 31 Oct 2009)       $
________________________________________________________________

*/



#include "systemml.h"


namespace brahms
{
	namespace systemml
	{


		Link::Link(string p_name, string p_path, string p_src, string p_dst, string p_lag)
		{
			name = p_name;

			setSrc(p_path + p_src);
			setDst(p_path + p_dst);

			DOUBLE n = brahms::text::s2n(p_lag.c_str());
			lag = n;
			if (n == brahms::text::S2N_FAILED || n != lag)
				ferr << E_SYSTEM_FILE << "invalid lag in Link";

			//	make inferences, as necessary - currently we don't support set
			//	transport, so we only support the following:
			//
			//	port to port					A>P to B<Q
			//									A>>X>P to B<<Y<Q
			if (src.type == ST_OUTPUT_PORT && dst.type == ST_INPUT_PORT)
			{
				//	ok
				dstPortNameInferred = false;
			}

			//	port to set						A>P to B<<Y (to <<Y<P)
			//									A>>X>P to B<<Y (to <<Y<P)
			else if (src.type == ST_OUTPUT_PORT && dst.type == ST_INPUT_SET)
			{
				//	infer portname and change dst type
				dst.portName = src.portName;
				dst.type = ST_INPUT_PORT;
				dstPortNameInferred = true;
			}

			//	port to process					A>P to B (to <P)
			//									A>>X>P to B (to <P)
			else if (src.type == ST_OUTPUT_PORT && dst.type == ST_PROCESS)
			{
				//	infer portname and change dst type
				dst.portName = src.portName;
				dst.type = ST_INPUT_PORT;
				dstPortNameInferred = true;
			}

			//	otherwise, raise a hand-wavy error
			else
			{
				ferr << E_SYSTEMML << "Link protocol either invalid or not currently supported in \"" << string(*this) << "\"";
			}
		}

		void Link::setSrc(string p_src)
		{
			src.parse(p_src.c_str(), ST_NULL, "Src");

			//	check src is set to the only cases we currently handle
			if (src.type != ST_OUTPUT_PORT)
				ferr << E_SYSTEM_FILE << "Link Src must be an output port (currently, set links are not supported) (was \"" << p_src << "\")";
		}

		void Link::setDst(string p_dst)
		{
			dst.parse(p_dst.c_str(), ST_NULL, "Dst");

			//	check dst is set to the only cases we currently handle
			if (dst.type != ST_INPUT_PORT && dst.type != ST_INPUT_SET && dst.type != ST_PROCESS)
				ferr << E_SYSTEM_FILE << "Link Dst must be an input port, set, or process (was \"" << p_dst << "\")";
		}

		string Link::getSrc()
		{
			return string(src);
		}

		string Link::getDst()
		{
			return string(dst);
		}

		UINT32 Link::getLag()
		{
			return lag;
		}

		string Link::getSrcProcessName()
		{
			return src.processName;
		}

		string Link::getDstProcessName()
		{
			return dst.processName;
		}

		Link::operator string() const
		{
			return string(src) + " ==> " + string(dst);
		}


	}
}

