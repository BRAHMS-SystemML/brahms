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

	$Id:: link.h 1911 2009-05-18 20:20:20Z benjmitch           $
	$Rev:: 1911                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-05-18 21:20:20 +0100 (Mon, 18 May 2009)       $
________________________________________________________________

*/

#ifndef _ENGINE_SYSTEMML_LINK_H_
#define _ENGINE_SYSTEMML_LINK_H_

namespace brahms
{
    namespace systemml
    {
        class Link
        {
        public:
            Link(string name, string path, string src, string dst, string lag);

            void setSrc(string src);
            void setDst(string dst);

            string getSrc();
            string getDst();
            UINT32 getLag();

            string getSrcProcessName();
            string getDstProcessName();

            operator string() const;

            // private: (ideally)
            string name;
            Identifier src;
            Identifier dst;
            UINT32 lag;
            bool dstPortNameInferred;
        };
    }
}

#endif // _ENGINE_SYSTEMML_LINK_H_
