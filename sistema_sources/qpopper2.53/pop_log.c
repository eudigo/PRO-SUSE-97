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
#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "popper.h"

/* 
 *  log:    Make a log entry
 */

static char msgbuf[MAXLINELEN];

#ifdef __STDC__
pop_log(POP *p, int stat, const char *format,...)
#else
pop_log(va_alist)
va_dcl
#endif
{
    va_list     ap;
#ifndef __STDC__
    POP     *   p;
    int         stat;
    char    *   format;
#endif
#ifdef PYRAMID
    char    *   arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
#endif
    char    *   date_time;
    time_t	clock;


    /* Get the information in to msgbuf */
#ifdef __STDC__
    va_start(ap,format);
#else
    va_start(ap);
    p = va_arg(ap,POP *);
    stat = va_arg(ap,int);
    format = va_arg(ap,char *);
#endif
#ifdef HAVE_VPRINTF
        vsprintf(msgbuf,format,ap);
#else
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    arg6 = va_arg(ap, char *);
# ifdef PYRAMID
        (void)sprintf(msgbuf,format, arg1, arg2, arg3, arg4, arg5, arg6);
# else
        (void)sprintf (msgbuf,format,((int *)ap)[0],((int *)ap)[1],((int *)ap)[2],
                ((int *)ap)[3],((int *)ap)[4],((int *)ap)[5]);
# endif
    va_end(ap);
#endif

    if (p->debug && p->trace) {
	clock = time(0);
	date_time = (char *)ctime(&clock);
	date_time[strlen(date_time) - 1] = '\0';
	(void)fprintf(p->trace,"%s [%d] %s\n",date_time, getpid(), msgbuf);
        (void)fprintf(p->trace,"%s \n", date_time);
        (void)fflush(p->trace);
    }
    else {
        syslog (stat,"%s",msgbuf);
    }

    return(stat);
}
