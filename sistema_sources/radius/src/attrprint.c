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
"@(#)$Id: attrprint.c,v 1.8 1997/05/15 06:49:44 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>

#include	<stdio.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>

#include	"radius.h"

/*************************************************************************
 *
 *	Function: fprint_attr_val
 *
 *	Purpose: Write a printable version of the attribute-value
 *		 pair to the supplied File.
 *
 *************************************************************************/

void
fprint_attr_val(fd, pair)
FILE		*fd;
VALUE_PAIR	*pair;
{
	DICT_VALUE	*dict_valget();
	DICT_VALUE	*dval;
	char		buffer[32];
	u_char		*ptr;
	char		prtbuf[256];
	void		ipaddr2str();
	void		fprint_attr_putc();

	switch(pair->type) {

	case PW_TYPE_STRING:
		sprintf(prtbuf, "%s = \"", pair->name);
		ptr = (u_char *)pair->strvalue;
		while(*ptr != '\0') {
			if(!(isprint(*ptr))) {
				sprintf(buffer, "\\%03o", *ptr);
				strcat(prtbuf, buffer);
			}
			else {
				fprint_attr_putc(*ptr, prtbuf);
			}
			ptr++;
		}
		fprint_attr_putc('"', prtbuf);
		break;
			
	case PW_TYPE_INTEGER:
		dval = dict_valget(pair->lvalue, pair->name);
		if(dval != (DICT_VALUE *)NULL) {
			sprintf(prtbuf, "%s = %s", pair->name, dval->name);
		}
		else {
			sprintf(prtbuf, "%s = %ld", pair->name, pair->lvalue);
		}
		break;

	case PW_TYPE_IPADDR:
		ipaddr2str(buffer, pair->lvalue);
		sprintf(prtbuf, "%s = %s", pair->name, buffer);
		break;

	case PW_TYPE_DATE:
		strftime(buffer, sizeof(buffer), "%b %e %Y",
					gmtime((time_t *)&pair->lvalue));
		sprintf(prtbuf, "%s = \"%s\"", pair->name, buffer);
		break;

	default:
		sprintf(prtbuf, "Unknown type %d", pair->type);
		break;
	}
	if (fd == (FILE *)-1) {
		/*
		 * send to debug log
		 */
		log_debug("%s\n", prtbuf);
	} else {
		fputs(prtbuf, fd);
	}
}

void
fprint_attr_putc(cc, buf)
u_char cc;
u_char * buf;
{
	int len;

	len = strlen((const char *)buf);
	buf[len] = cc;
	buf[len+1] = (u_char)0;
}
