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
#include <string.h>
#if HAVE_STRINGS_H
#include <strings.h>
#endif
#include <pwd.h>
#include <netinet/in.h> /* For IPPORT_RESERVED */
#include "popper.h"

/*
 * Use /etc/hosts.equiv and the users .rhost file to validate the user.
 */
int pop_rpop (p)
     POP     *   p;
{
  struct passwd  *   pw;

    if (p->ipport >= IPPORT_RESERVED || p->ipport < IPPORT_RESERVED/2)  {
      pop_log(p,POP_PRIORITY,
	      "RPOP command from %s (%s) on illegal port.",p->client,p->ipaddr);
        return (pop_msg(p,POP_FAILURE,
            "Permission denied.",p->user));
    }
    if (ruserok(p->client, 0, p->pop_parm[1], p->user) != 0)
        return (pop_msg(p,POP_FAILURE,
            "Permission denied.",p->user));
#ifdef NONAUTHFILE
    if (checknonauthfile(p->user) != 0)
        return (pop_msg(p,POP_FAILURE,
            "Permission denied.",p->user));
#endif
#ifdef AUTHFILE
    if (checkauthfile(p->user) != 0)
        return (pop_msg(p,POP_FAILURE,
            "Permission denied.",p->user));
#endif

    if ((pw = getpwnam(p->user)) == NULL)  /* "Can't happen" */
        return (pop_msg(p,POP_FAILURE,
            "Permission denied.",p->user));

    if (pw->pw_uid <= BLOCK_UID)
	return (pop_msg(p,POP_FAILURE, "Permission denied", p->user));

    /*  Build the name of the user's maildrop */
    if (genpath(p) < 0)
	return(pop_msg(p, POP_FAILURE, "Unable to create temporary drop name"));

    /*  Make a temporary copy of the user's maildrop */
    /*    and set the group and user id */
    if (pop_dropcopy(p, pw) != POP_SUCCESS) return (POP_FAILURE);

    /*  Get information about the maildrop */
    /* if (pop_dropinfo(p) != POP_SUCCESS) return(POP_FAILURE); */

    /*  Initialize the last-message-accessed number */
    p->last_msg = 0;

    /*  Authorization completed successfully */
    return (pop_msg (p,POP_SUCCESS,
        "%s has %d message(s) (%d octets).",
            p->user,p->msg_count,p->drop_size));
}

