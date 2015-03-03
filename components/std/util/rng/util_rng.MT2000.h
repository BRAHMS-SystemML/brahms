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

	$Id:: util_rng.MT2000.h 2419 2009-11-30 18:33:48Z benjmitc#$
	$Rev:: 2419                                                $
	$Author:: benjmitch                                        $
	$Date:: 2009-11-30 18:33:48 +0000 (Mon, 30 Nov 2009)       $
________________________________________________________________

	This file also includes code from Marsaglia & Tsang 2000,
	the Ziggurat method for random number generation.

*/





////////////////	MARSAGLIA & TSANG 2000

/* Marsaglia & Tsang original (or nearly original) code block */

#define RANDOM_SHR3 (jz=jsr, jsr^=(jsr<<13), jsr^=(jsr>>17), jsr^=(jsr<<5),jz+jsr)
#define RANDOM_UNI (.5f + (INT32) RANDOM_SHR3 * .2328306e-9f)
#define RANDOM_RNOR (hz=RANDOM_SHR3, iz=hz&127, (((UINT32)abs(hz))<kn[iz])? hz*wn[iz] : nfix())
#define RANDOM_REXP (jz=RANDOM_SHR3, iz=jz&255, ( jz <ke[iz])? jz*we[iz] : efix())

/* Marsaglia & Tsang original (or nearly original) code block */

enum MT2000_Distribution
{
	MT2000_NULL = 0,
	MT2000_NORMAL,
	MT2000_EXPONENTIAL,
	MT2000_UNIFORM
};

class MT2000 : public RNG
{

public:

	MT2000()
	{
		distribution = MT2000_NULL;

		/*

			NOTE: generator is in an unknown state to begin with,
			but very likely to be all zeroes, and will thus generate
			all zeroes on use. We don't seed here, for performance
			reasons.

		*/
	}

	void select(const char* p_distribution)
	{
		string dist = p_distribution;

		if (dist == "normal") distribution = MT2000_NORMAL;
		else if (dist == "exponential") distribution = MT2000_EXPONENTIAL;
		else if (dist == "uniform") distribution = MT2000_UNIFORM;
		else
		{
			string e = "unrecognised distribution \"" + dist + "\"";
			throw e.c_str();
		}
	}

	void fill(DOUBLE* dst, UINT32 cnt, DOUBLE gain = 1.0, DOUBLE offset = 0.0)
	{
		switch(distribution)
		{
			case MT2000_NORMAL:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR * gain + offset;
				break;
			}

			case MT2000_EXPONENTIAL:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP * gain + offset;
				break;
			}

			case MT2000_UNIFORM:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI * gain + offset;
				break;
			}

			default:
				throw "select a generator before requesting data";
		}
	}

	void fill(SINGLE* dst, UINT32 cnt, SINGLE gain = 1.0, SINGLE offset = 0.0)
	{
		switch(distribution)
		{
			case MT2000_NORMAL:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_RNOR * gain + offset;
				break;
			}

			case MT2000_EXPONENTIAL:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_REXP * gain + offset;
				break;
			}

			case MT2000_UNIFORM:
			{
				if (gain == 1.0 && offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI;
				else if (gain == 1.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI + offset;
				else if (offset == 0.0)
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI * gain;
				else
					for (UINT32 i=0; i<cnt; i++) dst[i] = RANDOM_UNI * gain + offset;
				break;
			}

			default:
				throw "select a generator before requesting data";
		}
	}

	DOUBLE get()
	{
		switch(distribution)
		{
		case MT2000_NORMAL: return RANDOM_RNOR;
		case MT2000_EXPONENTIAL: return RANDOM_REXP;
		case MT2000_UNIFORM: return RANDOM_UNI;
		default: throw "select a generator before requesting data";
		}
	}



	/* Marsaglia & Tsang original (or nearly original) code block */

	void seed(const UINT32* seed, UINT32 count)
	{
		//	validate
		if (count != 1) berr << "expects scalar seed";
		if (!seed[0]) berr << "expects non-zero seed";
		UINT32 jsrseed = seed[0];
		
		const FLOAT64 m1 = 2147483648.0, m2 = 4294967296.;
		FLOAT64 dn=3.442619855899,tn=dn,vn=9.91256303526217e-3, q;
		FLOAT64 de=7.697117470131487, te=de, ve=3.949659822581572e-3;
		int i;
		jsr=jsrseed;

		//	Tables for RNOR
		q=vn/exp(-.5*dn*dn);
		kn[0]=(UINT32)((dn/q)*m1);
		kn[1]=0; wn[0]=(FLOAT64)(q/m1); wn[127]=(FLOAT64)(dn/m1);
		fn[0]=1.; fn[127]=(FLOAT64)exp(-.5*dn*dn);
		for(i=126;i>=1;i--)
		{
			dn=sqrt(-2.*log(vn/dn+exp(-.5*dn*dn)));
			kn[i+1]=(UINT32)((dn/tn)*m1);
			tn=dn;
			fn[i]=(FLOAT64)exp(-.5*dn*dn);
			wn[i]=(FLOAT64)(dn/m1);
		}

		//	Tables for REXP
		q = ve/exp(-de);
		ke[0]=(UINT32)((de/q)*m2); ke[1]=0;
		we[0]=(FLOAT64)(q/m2); we[255]=(FLOAT64)(de/m2);
		fe[0]=1.; fe[255]=(FLOAT64)exp(-de);
		for(i=254;i>=1;i--)
		{
			de=-log(ve/de+exp(-de));
			ke[i+1]=(UINT32)((de/te)*m2);
			te=de;
			fe[i]=(FLOAT64)exp(-de);
			we[i]=(FLOAT64)(de/m2);
		}
	}

	FLOAT64 nfix()
	{
		const FLOAT64 r = 3.442620f;
		FLOAT64 x=0.0, y=0.0;

		for(;;)
		{
			x=hz*wn[iz];

			if(iz==0)
			{
				do
				{
					x=(FLOAT64)(-log(RANDOM_UNI)*0.2904764f);
					y=(FLOAT64)(-log(RANDOM_UNI));
				}
				while (y+y<x*x);
				return (hz>0)? r+x : -r-x;
			}

			if (fn[iz]+RANDOM_UNI*(fn[iz-1]-fn[iz]) < exp(-.5*x*x)) return x;
			hz=RANDOM_SHR3;
			iz=hz&127;
			
			if (((UINT32)abs(hz))<kn[iz]) return (hz*wn[iz]);
		} 
	}

	FLOAT64 efix()
	{
		FLOAT64 x;
		for(;;)
		{
			if(iz==0) return (7.69711-log(RANDOM_UNI));          /* iz==0 */
			x=jz*we[iz]; if( fe[iz]+RANDOM_UNI*(fe[iz-1]-fe[iz]) < exp(-x) ) return (x);

			/* initiate, try to exit for(;;) loop */
			jz=RANDOM_SHR3;
			iz=(jz&255);
			if(jz<ke[iz]) return (jz*we[iz]);
		}
	}

	UINT32 iz, jz, kn[128], ke[256], jsr;
	INT32 hz;
	FLOAT64 wn[128], fn[128], we[256], fe[256];

	/* Marsaglia & Tsang original (or nearly original) code block */

	MT2000_Distribution distribution;

};

