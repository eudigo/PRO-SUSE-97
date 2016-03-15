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
#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "popper.h"

#define BUFSIZE 2048

/* 
 *  msg:    Send a formatted line to the POP client
 */

#ifdef __STDC__
pop_msg(POP *p, int stat, const char *format,...)
#else
pop_msg(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    POP             *   p;
    int                 stat;              /*  POP status indicator */
    char            *   format;            /*  Format string for the message */
#endif
    va_list             ap;
    register char   *   mp;
#ifdef PYRAMID
    char	    *   arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
#endif
    char                message[BUFSIZE];

#ifdef __STDC__
    va_start(ap,format);
#else
    va_start(ap);
    p = va_arg(ap, POP *);
    stat = va_arg(ap, int);
    format = va_arg(ap, char *);
#endif

#ifdef PYRAMID
	arg1 = va_arg(ap, char *);
	arg2 = va_arg(ap, char *);
	arg3 = va_arg(ap, char *);
	arg4 = va_arg(ap, char *);
	arg5 = va_arg(ap, char *);
	arg6 = va_arg(ap, char *);
#endif
    /*  Point to the message buffer */
    mp = message;

    /*  Format the POP status code at the beginning of the message */
    if (stat == POP_SUCCESS)
        (void)sprintf (mp,"%s ",POP_OK);
    else
        (void)sprintf (mp,"%s ",POP_ERR);

    /*  Point past the POP status indicator in the message message */
    mp += strlen(mp);

    /*  Append the message (formatted, if necessary) */
    if (format) {
#ifdef HAVE_VPRINTF
        vsprintf(mp,format,ap);
#else
# ifdef PYRAMID
	(void)sprintf(mp,format, arg1, arg2, arg3, arg4, arg5, arg6);
# else
	(void)sprintf(mp,format,((int *)ap)[0],((int *)ap)[1],((int *)ap)[2],
		      ((int *)ap)[3],((int *)ap)[4]);
# endif
#endif
    }
    va_end(ap);
    
    /*  Log the message if debugging is turned on */
#ifdef DEBUG
    if (p->debug && stat == POP_SUCCESS)
        pop_log(p,POP_DEBUG,"%.1023s",message);
#endif

    /*  Log the message if a failure occurred */
    if (stat != POP_SUCCESS) 
	pop_log(p,POP_PRIORITY,
               (isdigit (p->client[0]) ? "%s@[%s]: %s" : "%s@%s: %s"),
               (p->user ? p->user : "(null)"), p->client, message);

    /*  Append the <CR><LF> */
    (void)strcat(message, "\r\n");
        
    /*  Send the message to the client */
    (void)fputs(message,p->output);
    (void)fflush(p->output);

    return(stat);
}

