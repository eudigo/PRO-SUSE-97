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
"$Id: version.c,v 1.20 1997/05/22 03:21:57 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include        <sys/types.h>
#include	<stdio.h>
#include	<syslog.h>
#include	"radius.h"

extern char	*progname;
extern int	radius_dbm;

/*  If you make any changes to this software please update this
 *  version number; contact support@livingston.com if you
 *  would like a range of versions allocated for your use.
 */

#define		VERSION		"Livingston RADIUS 2.0.1 97/5/22"

/*************************************************************************
 *
 *	Function: version
 *
 *	Purpose: Display the revision number for this program
 *
 *************************************************************************/

void
version()
{
	char buffer[1024];
	void build_version();

	build_version(buffer);
	fprintf(stderr, buffer);
	exit(-1);
}

void
log_version()
{
	char buffer[1024];
	void build_version();
	void log_msg();

	build_version(buffer);
	log_msg(LOG_INFO, buffer);
}

void
build_version(bp)
char * bp;
{

	sprintf(bp, "%s: %s ", progname, VERSION);

	/* here are all the conditional feature flags */
# if defined(SECURID)
	strcat(bp," SECURID");
# endif

# if defined(NDBM)
	strcat(bp," NDBM");
# endif

#if defined(NOSHADOW)
	strcat(bp," NOSHADOW");	/* system has no /usr/include/shadow.h */
#endif
#if defined(PASSCHANGE)
	strcat(bp," PASSCHANGE");	/* support deprecated radpass
					 * for changing passwords 
					 */
#endif

	/* here are all the system definitions compilation uses */
#if defined(__alpha)
	strcat(bp," __alpha");
#endif
#if defined(__osf__)
	strcat(bp," __osf__");
#endif
#if defined(aix)
	strcat(bp," aix");
#endif
#if defined(bsdi)
	strcat(bp," bsdi");
#endif
#if defined(hpux)
	strcat(bp," hpux");
#endif
#if defined(sun)
	strcat(bp," sun");
#endif
#if defined(sys5)
	strcat(bp," sys5");
#endif
#if defined(unixware)
	strcat(bp," unixware");
#endif
#if defined(M_UNIX)
	strcat(bp," M_UNIX");
#endif
	strcat(bp, radius_dbm ? " dbm_users" : " flat_users");
	strcat(bp,"\n");
}
