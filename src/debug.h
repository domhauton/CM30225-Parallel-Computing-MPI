/*
@(#)File:           $RCSfile: debug.h,v $
@(#)Version:        $Revision: 3.6 $
@(#)Last changed:   $Date: 2008/02/11 06:46:37 $
@(#)Purpose:        Definitions for the debugging system
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1990-93,1997-99,2003,2005,2008
@(#)Product:        :PRODUCT:
*/

#ifndef PARALLEL_COMPUTATION_CW1_DEBUG_H
#define PARALLEL_COMPUTATION_CW1_DEBUG_H
#define DEBUG 0

#ifdef DEBUG
#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr,##__VA_ARGS__); } while (0)
#else
#define debug_print(msg) (void)0
#endif //DEBUG

#endif //PARALLEL_COMPUTATION_CW1_DEBUG_H