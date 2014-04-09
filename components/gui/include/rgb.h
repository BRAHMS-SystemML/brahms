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

	$Id:: rgb.h 1766 2009-04-10 17:02:40Z benjmitch            $
	$Rev:: 1766                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-04-10 18:02:40 +0100 (Fri, 10 Apr 2009)       $
________________________________________________________________

*/



////////////////	RGB support

inline UINT8 float2byte(DOUBLE val)
{
	return max(min(floor(val * 255) + 0.5, 255.0), 0.0);
}

struct RGBbyte
{
	RGBbyte()
	{
		r = 0.0;
		g = 0.0;
		b = 0.0;
	}

	RGBbyte(UINT8 r, UINT8 g, UINT8 b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	RGBbyte(DOUBLE r, DOUBLE g, DOUBLE b)
	{
		this->r = float2byte(r);
		this->g = float2byte(g);
		this->b = float2byte(b);
	}

	RGBbyte operator*(DOUBLE rhs)
	{
		RGBbyte ret;
		ret.r = max(min(((DOUBLE)r) * rhs, 255.0), 0.0);
		ret.g = max(min(((DOUBLE)g) * rhs, 255.0), 0.0);
		ret.b = max(min(((DOUBLE)b) * rhs, 255.0), 0.0);
		return ret;
	}

	RGBbyte operator+(RGBbyte rhs)
	{
		RGBbyte ret;
		ret.r = max(min(((DOUBLE)r) + ((DOUBLE)rhs.r), 255.0), 0.0);
		ret.g = max(min(((DOUBLE)g) + ((DOUBLE)rhs.g), 255.0), 0.0);
		ret.b = max(min(((DOUBLE)b) + ((DOUBLE)rhs.b), 255.0), 0.0);
		return ret;
	}

	operator string()
	{
		stringstream ss;
		ss << ((DOUBLE)r) << ", " << ((DOUBLE)g) << ", " << ((DOUBLE)b);
		return ss.str();
	}

	UINT8 r;
	UINT8 g;
	UINT8 b;
};



////////////////	HSV to RGB conversion

//	credit: http://www.tecgraf.puc-rio.br/~mgattass/color/HSVtoRGB.htm
//	(modified to output UINT8 values)

void HSVtoRGB(double h, double s, double v, UINT8* r, UINT8* g, UINT8* b)
{
	//	wrap 1.0 onto 0.0
	if (h >= 1.0) h -= 1.0;

	if ( s == 0 )
	{
		*r = v;
		*g = v;
		*b = v;
	}
	else
	{
		double var_h = h * 6;
		double var_i = floor( var_h );
		double var_1 = v * ( 1 - s );
		double var_2 = v * ( 1 - s * ( var_h - var_i ) );
		double var_3 = v * ( 1 - s * ( 1 - ( var_h - var_i ) ) );

		if      ( var_i == 0 ) { *r = float2byte(v)     ; *g = float2byte(var_3) ; *b = float2byte(var_1); }
		else if ( var_i == 1 ) { *r = float2byte(var_2) ; *g = float2byte(v)     ; *b = float2byte(var_1); }
		else if ( var_i == 2 ) { *r = float2byte(var_1) ; *g = float2byte(v)     ; *b = float2byte(var_3); }
		else if ( var_i == 3 ) { *r = float2byte(var_1) ; *g = float2byte(var_2) ; *b = float2byte(v);     }
		else if ( var_i == 4 ) { *r = float2byte(var_3) ; *g = float2byte(var_1) ; *b = float2byte(v);     }
		else                   { *r = float2byte(v)     ; *g = float2byte(var_1) ; *b = float2byte(var_2); }
	}
}



