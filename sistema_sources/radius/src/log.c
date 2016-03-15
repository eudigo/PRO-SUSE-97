/*
 *
 *	RADIUS
 *	Remote Authentication Dial In User Service
 *
 *
 *	Livingston Enterprises, Inc.
 *	4464 Willow Road
 *	Pleasanton, CA   94588
 *
 *	Copyright 1992-1996 Livingston Enterprises, Inc. All Rights Reserved.
 *
 *	This software is provided under license from Livingston
 *	Enterprises, Inc., the terms and conditions of which are set
 *	forth in a Software License Agreement that is contained in an
 *	End User Agreement contained in the product packaging, and
 *	electronically on the Livingston ftp site. This software may
 *	only be used in conjunction with Livingston (or Livingston
 *	authorized) products.  Livingston makes no warranties to any
 *	licensee concerning the applicability of the software to
 *	licensee's specific requirements or the suitability of the
 *	software for any intended use.  Licensee shall not remove,
 *	modify or alter any copyright and/or other proprietary rights
 *	notice and must faithfully reproduce all such notices on any
 *	copies or modifications to this software that it makes.
 *
 *	Livingston Enterprises, Inc. makes no representations about the
 *	suitability of this software for any purpose.  It is provided
 *	"as is" without express or implied warranty.
 *
 */

static char sccsid[] =
"$Id: log.c,v 1.7 1997/05/15 06:49:48 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<stdio.h>
#include	<syslog.h>
#include	<time.h>
#include	<stdarg.h>
#include	<sys/types.h>

extern char	*radius_log;
extern char	*progname;

void		log_msg();

/*************************************************************************
 *
 *	Function: log_err
 *
 *	Purpose: Log the error message
 *
 *************************************************************************/

void
log_err(char * fmt, ...)
{
	va_list	args;

	va_start(args, fmt);
	log_msg(LOG_ERR, fmt, args);
	va_end(args);
}

/*************************************************************************
 *
 *	Function: log_debug
 *
 *	Purpose: Log the debug message
 *
 *************************************************************************/

void
log_debug(char * fmt, ...)
{
	va_list	args;
	
	va_start(args, fmt);
	log_msg(LOG_DEBUG, fmt, args);
	va_end(args);
}

/*************************************************************************
 *
 *	Function: log_msg
 *
 *	Purpose: Log the priority message
 *
 *************************************************************************/

void
log_msg(priority, fmt, args)
int priority;
char *fmt;
va_list args;
{
	FILE	*msgfd;
	time_t	timeval;
	pid_t	getpid();
#ifdef	VSYSLOG
	char buffer[1024];
#endif /* VSYSLOG */

	if (radius_log) {
		/*
		 * use users option logfile [-l <logfile>]
		 */
		if((msgfd = fopen(radius_log, "a")) == (FILE *)NULL) {
			fprintf(stderr, "%s: could not open %s for logging\n",
					progname, radius_log);
			return;
		}
		timeval = time(0);
		fprintf(msgfd, "%-24.24s: [%d] ", ctime(&timeval), getpid());
		vfprintf(msgfd, fmt, args);
		fclose(msgfd);
	} else {
		/*
		 * use syslog
		 */
		openlog("radius", LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_AUTH);
#ifdef VSYSLOG
		vsprintf(buffer, fmt, args);
		syslog(priority, buffer);
#else /* not VSYSLOG */
		vsyslog(priority, fmt, args);
#endif /* not VSYSLOG */
		closelog();
	}
	return;
}
