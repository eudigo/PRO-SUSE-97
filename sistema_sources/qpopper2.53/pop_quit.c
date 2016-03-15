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
#include "popper.h"

/* 
 *  quit:   Terminate a POP session
 */

int pop_quit (p)
POP     *   p;
{
    /*  Release the message information list */
    if (p->mlp) free ((char *)p->mlp);

    if (p->CurrentState == auth2) {
	pop_log(p, POP_SUCCESS, "Possible probe of account %s from host %s",
							p->user, p->client);
    }

    return(POP_SUCCESS);
}
