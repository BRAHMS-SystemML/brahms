/*
 * Reproduces the behaviour of the old brahms-1065.h, which was
 * constructed by concatenating brahms-c++-common.h,
 * brahms-c++-legacy.h and the in-source "stub" brahms-1065.h.
 */

// The first section, the H section, gets processed the *first* time
// this file is included.
#ifndef INCLUDED_OVERLAY_1
# define INCLUDED_OVERLAY_1

#include "BrahmsConfig.h"
# define __BINDING__ 1065
# define __REV__ VERSION_BRAHMS_REV
# include "brahms-c++-common.h"
# include "brahms-c++-legacy.h"
# include "brahms-1065_hdr.h"

#else // INCLUDED_OVERLAY_1 is defined

// The second section, the CPP section, gets processed the *second*
// time this file is included.
# ifndef INCLUDED_OVERLAY_2
#  define INCLUDED_OVERLAY_2
#  include "brahms-1065_impl.h"
# endif // INCLUDED_OVERLAY_2

#endif // INCLUDED_OVERLAY_1
