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

#include <popper.h>

/* 
 *  get_subcommand: Extract a POP XTND subcommand from a client input line
 */

static xtnd_table subcommands[] = {
        "xmit",     0,  0,  pop_xmit,
	"xlst",	    1,  2,  pop_xlst,
        NULL
};

xtnd_table *pop_get_subcommand(p)
POP     *   p;
{
    xtnd_table      *   s;

    /*  Search for the POP command in the command/state table */
    for (s = subcommands; s->subcommand; s++) {

        if (strcmp(s->subcommand,p->pop_subcommand) == 0) {

            /*  Were too few parameters passed to the subcommand? */
            if ((p->parm_count-1) < s->min_parms) {
                pop_msg(p,POP_FAILURE,
                    "Too few arguments for the %.128s %.128s command.",
                        p->pop_command,p->pop_subcommand);
                return((xtnd_table *)0);
            }
            

            /*  Were too many parameters passed to the subcommand? */
            if ((p->parm_count-1) > s->max_parms) {
                pop_msg(p,POP_FAILURE,
                    "Too many arguments for the %.128s %.128s command.",
                        p->pop_command,p->pop_subcommand);
                return((xtnd_table *)0);
            }
            

            /*  Return a pointer to the entry for this subcommand 
                in the XTND command table */
            return (s);
        }
    }
    /*  The client subcommand was not located in the XTND command table */
    pop_msg(p,POP_FAILURE,
            "Unknown command: \"%.128s %.128s\".",p->pop_command,p->pop_subcommand);
    return((xtnd_table *)0);
}
