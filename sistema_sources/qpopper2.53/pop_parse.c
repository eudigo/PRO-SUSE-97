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
#include "popper.h"

/* 
 *  parse:  Parse a raw input line from a POP client 
 *  into null-delimited tokens
 */

pop_parse(p,buf)
POP         *   p;
char        *   buf;        /*  Pointer to a message containing 
                                the line from the client */
{
    char            *   mp;
    register int        i;
    
    /*  Loop through the POP command array */
    for (mp = buf, i = 0; ; i++) {
    
        /*  Skip leading spaces and tabs in the message */
        while (isspace(*mp))mp++;

        /*  Are we at the end of the message? */
        if (*mp == 0) break;

        /*  Have we already obtained the maximum allowable parameters? */
        if (i >= MAXPARMCOUNT) {
            pop_msg(p,POP_FAILURE,"Too many arguments supplied.");
            return(-1);
        }

        /*  Point to the start of the token */
        p->pop_parm[i] = mp;

        /*  Search for the first space character (end of the token) */
        while (!isspace(*mp) && *mp) mp++;

        /*  Delimit the token with a null */
        if (*mp) *mp++ = 0;

 	if (i == 0) {
 	  /*  Convert the first token (POP command) to lower case */
 	  pop_lower(p->pop_command);
 	  /*
 	   * This is kinda gross.  Passwords have to be parsed diffrently
 	   * as they may contain spaces.  If you think of a cleaner way,
 	   * do it.  The "p->pop_command[0] == 'p'" is so save a call to
 	   * strcmp() on ever call to pop_parse();  This parsing keeps
	   * leading and trailing speces behind for the password command.
 	   */
 	  if(p->pop_command[0] == 'p' && strcmp(p->pop_command,"pass") == 0) {
 	    if (*mp != 0) {
 	      p->pop_parm[1] = mp;
              if (strlen(mp) > 0) {
                mp = mp + strlen(mp) - 1;
                while (*mp == 0xa || *mp == 0xd) *mp-- = 0;
              }
 	      return(1);
 	    } else
 	      return (-1);
 	  }
 	}
    }

    /*  Were any parameters passed at all? */
    if (i == 0) return (-1);

    /*  Return the number of tokens extracted minus the command itself */
    return (i-1);
    
}
