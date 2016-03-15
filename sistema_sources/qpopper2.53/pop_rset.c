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
 *  rset:   Unflag all messages flagged for deletion in a POP maildrop
 */

int pop_rset (p)
POP     *   p;
{
    MsgInfoList     *   mp;         /*  Pointer to the message info list */
    register int        i;

    /*  Unmark all the messages */
    for (i = p->msg_count, mp = p->mlp; i > 0; i--, mp++) {
        mp->del_flag = FALSE; 
        mp->retr_flag = mp->orig_retr_state; 
    }
    
    /*  Reset the messages-deleted and bytes-deleted counters */
    p->msgs_deleted = 0;
    p->bytes_deleted = 0;
    
    /*  Reset the last-message-access flag */
    p->last_msg = 0;

    return (pop_msg(p,POP_SUCCESS,"Maildrop has %u messages (%u octets)",
        p->msg_count,p->drop_size));
}
