/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */


#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>

/* 
 *  lower:  Convert a string to lowercase
 */

pop_lower (buf)
char        *   buf;
{
    char        *   mp;

    for (mp = buf; *mp; mp++)
        if (isupper(*mp) && isupper(*mp)) *mp = (char)tolower((int)*mp);
}
