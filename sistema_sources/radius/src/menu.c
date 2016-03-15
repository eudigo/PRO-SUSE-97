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

/* don't look here for the version, run radiusd -v or look in version.c */
static char sccsid[] =
"$Id: menu.c,v 1.6 1997/05/15 06:49:49 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<netinet/in.h>

#include	<stdio.h>
#include	<netdb.h>
#include	<fcntl.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<signal.h>
#include	<errno.h>
#include	<sys/wait.h>

#include	"radius.h"

extern char	*radius_dir;
extern char	*radacct_dir;
extern char	*progname;

void
process_menu(authreq, activefd, pw_digest)
AUTH_REQ	*authreq;
int		activefd;
char		*pw_digest;
{
	VALUE_PAIR	*attr;
	VALUE_PAIR	*term_attr;
	VALUE_PAIR	*newattr;
	VALUE_PAIR	*get_attribute();
	VALUE_PAIR	*menu_pairs();
	char		menu_name[128];
	char		menu_input[32];
	int		i;
	char		state_value[128];
	void		send_accept();
	void		send_reject();

	if((attr = get_attribute(authreq->request, PW_STATE)) !=
		(VALUE_PAIR *)NULL && strncmp(attr->strvalue, "MENU=", 5) == 0){

		strcpy(menu_name, &attr->strvalue[5]);

		/* The menu input is in the Password Field */
		attr = get_attribute(authreq->request, PW_PASSWORD);
		if(attr == (VALUE_PAIR *)NULL) {
			*menu_input = '\0';
		}
		else {
			/*
			 * Decrypt the password in the request.
			 */
			memcpy(menu_input, attr->strvalue, AUTH_PASS_LEN);
			for(i = 0;i < AUTH_PASS_LEN;i++) {
				menu_input[i] ^= pw_digest[i];
			}
			menu_input[AUTH_PASS_LEN] = '\0';
		}
		attr = menu_pairs(menu_name, menu_input);
	}

	/* handle termination menu */
	if((term_attr = get_attribute(attr, PW_TERMINATION_MENU)) !=
							(VALUE_PAIR *)NULL) {

		/* Change this to a menu state */
		sprintf(state_value, "MENU=%s", term_attr->strvalue);
		term_attr->attribute = PW_STATE;
		strcpy(term_attr->strvalue, state_value);
		strcpy(term_attr->name, "Challenge-State");

		/* Insert RADIUS termination option */
		if((newattr = (VALUE_PAIR *)malloc(sizeof(VALUE_PAIR))) !=
						(VALUE_PAIR *)NULL) {

			/* Set termination values */
			newattr->attribute = PW_TERMINATION;
			newattr->type = PW_TYPE_INTEGER;
			newattr->lvalue = PW_TERM_RADIUS_REQUEST;
			strcpy(newattr->name, "Termination-Action");

			/* Insert it */
			newattr->next = term_attr->next;
			term_attr->next = newattr;
		}
	}

	if((term_attr = get_attribute(attr, PW_MENU)) != (VALUE_PAIR *)NULL &&
				strcmp(term_attr->strvalue, "EXIT") == 0) {
		send_reject(authreq, (char *)"", activefd);
		pairfree(attr);
	}
	else if(attr != (VALUE_PAIR *)NULL) {
		send_accept(authreq, attr, (char *)NULL, activefd);
		pairfree(attr);
	}
	else {
		send_reject(authreq, (char *)NULL, activefd);
	}
	pairfree(authreq->request);
	memset(authreq, 0, sizeof(AUTH_REQ));
	free(authreq);
	return;
}

char	*
get_menu(menu_name)
char	*menu_name;
{
	FILE	*fd;
	static	char menu_buffer[4096];
	int	mode;
	char	*ptr;
	int	nread;
	int	len;

	sprintf(menu_buffer, "%s/menus/%s", radius_dir, menu_name);
	if((fd = fopen(menu_buffer, "r")) == (FILE *)NULL) {
		return("\r\n*** User Menu is Not Available ***\r\n");
	}

	mode = 0;
	nread = 0;
	ptr = menu_buffer;
	*ptr = '\0';
	while(fgets(ptr, 4096 - nread, fd) != NULL && nread < 4096) {

		if(mode == 0) {
			if(strncmp(ptr, "menu", 4) == 0) {
				mode = 1;
			}
		}
		else {
			if(strncmp(ptr, "end\n", 4) == 0) {
				if(ptr > menu_buffer) {
					*(ptr - 2) = '\0';
				}
				else {
					*ptr = '\0';
				}
				return(menu_buffer);
			}
			len = strlen(ptr);
			ptr += len - 1;
			*ptr++ = '\r';
			*ptr++ = '\n';
			nread += len + 1;
		}
	}
	*ptr = '\0';
	return(menu_buffer);
}

VALUE_PAIR	*
menu_pairs(menu_name, menu_selection)
char	*menu_name;
char	*menu_selection;
{
	FILE	*fd;
	char 	buffer[4096];
	char	selection[32];
	int	mode;
	char	*ptr;
	int	nread;
	int	userparse();
	VALUE_PAIR	*reply_first;

	sprintf(buffer, "%s/menus/%s", radius_dir, menu_name);
	if((fd = fopen(buffer, "r")) == (FILE *)NULL) {
		return((VALUE_PAIR *)NULL);
	}

	/* Skip past the menu */
	mode = 0;
	nread = 0;
	while(fgets(buffer, sizeof(buffer), fd) != NULL) {
		if(mode == 0) {
			if(strncmp(buffer, "menu", 4) == 0) {
				mode = 1;
			}
		}
		else {
			if(strncmp(buffer, "end\n", 4) == 0) {
				break;
			}
		}
	}

	/* handle default */
	if(*menu_selection == '\0') {
		strcpy(selection, "<CR>");
	}
	else {
		strcpy(selection, menu_selection);
	}
	reply_first = (VALUE_PAIR *)NULL;

	/* Look for a matching menu entry */
	while(fgets(buffer, sizeof(buffer), fd) != NULL) {

		/* Terminate the buffer */
		ptr = buffer;
		while(*ptr != '\n' && *ptr != '\0') {
			ptr++;
		}
		if(*ptr == '\n') {
			*ptr = '\0';
		}

		if(strcmp(selection, buffer) == 0 ||
					strcmp("DEFAULT", buffer) == 0) {
			/* We have a match */
			while(fgets(buffer, sizeof(buffer), fd) != NULL) {
			    if(*buffer == ' ' || *buffer == '\t') {
				/*
				 * Parse the reply values
				 */
				if(userparse(buffer, &reply_first) != 0) {
					log_err("parse error for menu %s\n",menu_name);
					pairfree(reply_first);
					fclose(fd);
					return((VALUE_PAIR *)NULL);
				}
			    }
			    else {
				/* We are done */
				fclose(fd);
				return(reply_first);
			    }
			}
			fclose(fd);
			return(reply_first);
		}
	}
	fclose(fd);
	return((VALUE_PAIR *)NULL);
}
