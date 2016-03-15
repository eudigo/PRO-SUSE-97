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
"$Id: securid.c,v 1.9 1997/06/05 08:20:48 cdr Exp $ Copyright 1995 Livingston Enterprises Inc";

#include	<stdio.h>
#include	<errno.h>

#include	<sys/types.h>

#include	"radius.h"
#include	"sdi_athd.h"
#include	"sdi_defs.h"
#include	"sdi_size.h"
#include	"sdi_type.h"
#include	"sdacmvls.h"
#include	"sdconf.h"

union config_record configure;

struct SD_CLIENT	sd_dat;

#define CANCELLED	1

int
securid(username, password, authreq, user_reply, activefd)
char		*username;
char		*password;
AUTH_REQ	*authreq;
VALUE_PAIR	*user_reply;
int		activefd;
{
	struct SD_CLIENT	*sd_p;
	int			retcode;
	VALUE_PAIR		*attr;
	VALUE_PAIR		*get_attribute();
	static int		securid_active;
	int			challenge;
	char			msg[256];
	char			pintype[16];
	char			pinsize[16];
	char			state_value[128];
	pid_t			getpid();
	void			send_accept();
	void			send_reject();
	void			send_challenge();

	sd_p = &sd_dat;
	if(securid_active != 1) {
		/* clear struct */
		memset((u_char *)sd_p, 0, sizeof(sd_dat));
		/*  accesses sdconf.rec  */
		if (creadcfg()) {
			log_err("securid: error reading sdconf.rec\n");
			reqfree(authreq);
			pairfree(user_reply);
			return(0);
		}

		if(sd_init(sd_p)) {
			log_err("securid: cannot initialize connection to SecurID server\n");
			reqfree(authreq);
			pairfree(user_reply);
			return(0);
		}
		securid_active = 1;
	}

	/*
	 * In some cases, SecurID will require two Cardcodes to properly
	 * authenticate.  When it wants a second one, we will store the
	 * SD_CLIENT data so we have state.  The scond time around (after
	 * the challenge) we will have what we need to use sd_next().
	 * This keeps us stateless.
	 */

	attr = get_attribute(authreq->request, PW_STATE);
	if (attr != (VALUE_PAIR *)NULL) {
		if (strncmp(attr->strvalue, "SECURID_NEXT=", 13) == 0){
			strcpy(sd_p->username, username);
			retcode = sd_next(password, sd_p);
			DEBUG("securid: sd_next retcode=%d\n", retcode);
		} else if (strncmp(attr->strvalue, "SECURID_NPIN=", 13) == 0){
			strcpy(sd_p->username, username);
			retcode = sd_pin(password, 0, sd_p);
			DEBUG("securid: sd_pin retcode=%d\n", retcode);
		} else if (strncmp(attr->strvalue, "SECURID_WAIT=", 13) == 0){
			send_reject(authreq,"Log in with new PIN and code.\r\n",activefd);
			reqfree(authreq);
			pairfree(user_reply);
			sd_close();
			securid_active = 0;
			return(0);
		} else {
			log_err("securid: unexpected STATE=\"%s\"\n",attr->strvalue);
			retcode = -1;
		}
	} else {
/* The following line removed in 2.0.1 to enable SDI to function properly */
	/*	memset(sd_p, 0, sizeof(sd_dat)); */
		retcode = sd_check(password, username, sd_p);
		DEBUG("securid: sd_check retcode=%d\n", retcode);
	}

	challenge = 0;
	switch (retcode) {

	case ACM_OK:
		send_accept(authreq, user_reply, (char *)NULL, activefd);
		break;

	case ACM_ACCESS_DENIED:
		send_reject(authreq, (char *)NULL, activefd);
		break;

	case ACM_NEXT_CODE_REQUIRED:
		challenge = 1;
		sprintf(state_value, "SECURID_NEXT=%lu", (UINT4)getpid());
		send_challenge(authreq, "Enter next Cardcode: ", 
			state_value, activefd);
		break;

	case ACM_NEW_PIN_REQUIRED:
		if (sd_p->user_selectable == CANNOT_CHOOSE_PIN) {
		    	if (sd_pin(sd_p->system_pin, 0, sd_p) == ACM_NEW_PIN_ACCEPTED) {
				challenge = 1;
				sprintf(msg,"%s is your new PIN.  Press RETURN to disconnect, wait for token\r\n code to change, log in with new PIN and code.\r\n",sd_p->system_pin);
 				sprintf(state_value, "SECURID_WAIT=%u", (UINT4)getpid());
				send_challenge(authreq,msg,state_value,activefd);
			}
			else {
				send_reject(authreq,"PIN rejected. Please try again.\r\n",activefd);
			}
    		}
		else if (sd_p->user_selectable == USER_SELECTABLE ||
			 sd_p->user_selectable == MUST_CHOOSE_PIN) {
			challenge = 1;
			sprintf(state_value, "SECURID_NPIN=%u", (UINT4)getpid());
			if (sd_p->alphanumeric)
				strcpy(pintype, "characters");
			else 
				strcpy(pintype, "digits");
			if (sd_p->min_pin_len == sd_p->max_pin_len)
				sprintf(pinsize, "%d", sd_p->min_pin_len);
			else 
				sprintf(pinsize, "%d to %d",
				sd_p->min_pin_len, sd_p->max_pin_len);
			sprintf(msg,"Enter your new PIN, containing %s %s:",
				pinsize,pintype); 
			send_challenge(authreq, msg, state_value, activefd);
		}
		else {
			log_err("securid: New Pin required but user select has unknown value %d, sending reject\n",sd_p->user_selectable);
			send_reject(authreq,(char *)NULL,activefd);
		}
		break;

	case ACM_NEW_PIN_ACCEPTED:
		send_reject(authreq,"New Pin Accepted.\r\nWait for next card code and then login.\r\n", activefd);
		break;
	case -1:
		send_reject(authreq, (char *)NULL, activefd);
		break;
	default:
		log_err("securid: SecurID server returned unknown code %d for user %s\n", retcode, username);
		send_reject(authreq, (char *)NULL, activefd);
		break;
	}
	reqfree(authreq);
	pairfree(user_reply);
/* The following line added in 2.0.1 so that the connection between RADIUS
 * and SDI is broken after each request, as it should according to SDI.
 */
	if(challenge==0) {
		sd_close();
		securid_active = 0;
	}
 
	return(challenge);
}

int
__ansi_fflush(f)
FILE *f;
{
	return fflush(f);
}
