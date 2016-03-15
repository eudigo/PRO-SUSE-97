/*
 *
 *	RADIUS Accounting
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
"$Id: acct.c,v 1.13 1997/05/15 06:49:44 cdr Exp $  Copyright 1994 Livingston Enterprises Inc";

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<netinet/in.h>

#include	<stdio.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<signal.h>
#include	<errno.h>
#include	<sys/wait.h>

#include	"radius.h"

#define SIGN_NOCLIENT	-1
#define SIGN_MATCH	0
#define SIGN_ZERO	1
#define SIGN_NOMATCH	2

extern char		recv_buffer[4096];
extern char		send_buffer[4096];
extern char		*progname;
extern int		debug_flag;
extern char		*radacct_dir;
extern char		*radius_dir;
extern UINT4		expiration_seconds;
extern UINT4		warning_seconds;
extern int		errno;

UINT4			calctime();

void
rad_accounting(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	FILE		*outfd;
	char		*ip_hostname();
	char		*client_hostname();
	char		clientname[128];
	char		buffer[512];
	VALUE_PAIR	*pair;
	long		curtime;
	int		retsig;
	int		calc_acctreq();
	void		send_acct_reply();
	void		reqfree();
	void		fprint_attr_val();

	/* Verify the client */
	retsig=calc_acctreq(authreq);

	if (retsig == SIGN_NOCLIENT) {
		/* We do not respond when this fails */
		log_err("accounting: unknown client %s ignored\n",
					ip_hostname(authreq->ipaddr));
		reqfree(authreq);
		return;
	} else if (retsig == SIGN_NOMATCH) {
		DEBUG("accounting: client %s sent accounting-request with invalid request authenticator\n",ip_hostname(authreq->ipaddr));
	}

	strcpy(clientname, client_hostname(authreq->ipaddr));

	/*
	 * Create a directory for this client.
	 */
	sprintf(buffer, "%s/%s", radacct_dir, clientname);
	mkdir(buffer, 0755);

	/*
	 * Write Detail file.
	 */
	sprintf(buffer, "%s/%s/detail", radacct_dir, clientname);
	if((outfd = fopen(buffer, "a")) == (FILE *)NULL) {
		log_err("accounting: could not append to file %s\n", buffer);
		/* do not respond if we cannot save record */
	} else {
		/* Post a timestamp */
		curtime = time(0);
		fputs(ctime(&curtime), outfd);

		/* Write each attribute/value to the log file */
		pair = authreq->request;
		while(pair != (VALUE_PAIR *)NULL) {
			fputs("\t", outfd);
			fprint_attr_val(outfd, pair);
			fputs("\n", outfd);
			pair = pair->next;
		}
		/* print the seconds since epoch for easier processing */
		sprintf(buffer,"\tTimestamp = %ld\n",curtime);
		fputs(buffer,outfd);

		/* To be strictly compliant with the RADIUS Accounting RFC we
		   should only accept packets that returned SIGN_MATCH to
		   indicate the Request-Authenticator is valid, but to provide
		   compatibility with NASes that haven't implemented that yet
		   we accept no authenticator (all 0's) or mistaken ones, but
		   flag them as such.  A future release may be more strict.
		  */
		switch(retsig) {
			case SIGN_MATCH:
				/* fputs("\tRequest-Authenticator = Verified\n",outfd); */
				break;
			case SIGN_NOMATCH:
				fputs("\tRequest-Authenticator = Unverified\n",outfd);
				break;
			case SIGN_ZERO:
				fputs("\tRequest-Authenticator = None\n",outfd);
				break;
			default:
				break;
		}
		fputs("\n", outfd);
		fclose(outfd);
		/* let NAS know it is OK to delete from buffer */
		send_acct_reply(authreq, (VALUE_PAIR *)NULL,activefd);
	}

	reqfree(authreq);
}

/*************************************************************************
 *
 *	Function: send_acct_reply
 *
 *	Purpose: Reply to the request with an ACKNOWLEDGE.  Also attach
 *		 reply attribute value pairs (not that there should be any)
 *
 *************************************************************************/

void
send_acct_reply(authreq, reply, activefd)
AUTH_REQ	*authreq;
VALUE_PAIR	*reply;
int		activefd;
{
	AUTH_HDR		*auth;
	u_short			total_length;
	struct	sockaddr_in	saremote;
	struct	sockaddr_in	*sin;
	u_char			*ptr;
	int			len;
	UINT4			lvalue;
	u_char			digest[16];
	int			secretlen;
	char			*client_hostname();
	char			*ipaddr2strp();
	void			md5_calc();

	auth = (AUTH_HDR *)send_buffer;

	/* Build standard header */
	auth->code = PW_ACCOUNTING_RESPONSE;
	auth->id = authreq->id;
	memcpy(auth->vector, authreq->vector, AUTH_VECTOR_LEN);

	DEBUG("Sending Accounting-Response for id %d to %s (%s)\n",
		authreq->id, client_hostname(authreq->ipaddr),
		ipaddr2strp(authreq->ipaddr));

	total_length = AUTH_HDR_LEN;

	/* Load up the configuration values for the user */
	ptr = auth->data;
	while(reply != (VALUE_PAIR *)NULL) {
		debug_pair(reply);
		*ptr++ = reply->attribute;

		switch(reply->type) {

		case PW_TYPE_STRING:
			len = strlen(reply->strvalue);
			*ptr++ = len + 2;
			strcpy((char *)ptr, reply->strvalue);
			ptr += len;
			total_length += len + 2;
			break;
			
		case PW_TYPE_INTEGER:
		case PW_TYPE_IPADDR:
			*ptr++ = sizeof(UINT4) + 2;
			lvalue = htonl(reply->lvalue);
			memcpy(ptr, &lvalue, sizeof(UINT4));
			ptr += sizeof(UINT4);
			total_length += sizeof(UINT4) + 2;
			break;

		default:
			break;
		}

		reply = reply->next;
	}

	auth->length = htons(total_length);

	/*
	 * The Authenticator field in an Accounting-Response packet is
	 * called the Response Authenticator, and contains a one-way MD5
	 * hash calculated over a stream of octets consisting of the
	 * Accounting-Response Code, Identifier, Length, the Request
	 * Authenticator field from the Accounting-Request packet being
	 * replied to, and the response attributes if any, followed by the
	 * shared secret.  The resulting 16 octet MD5 hash value is stored
	 * in the Authenticator field of the Accounting-Response packet.
	 */

	/* Calculate the response digest */
	secretlen = strlen((const char *)authreq->secret);
	memcpy(send_buffer + total_length, authreq->secret, secretlen);
	md5_calc(digest, (char *)auth, total_length + secretlen);
	memcpy(auth->vector, digest, AUTH_VECTOR_LEN);
	memset(send_buffer + total_length, 0, secretlen);

	sin = (struct sockaddr_in *) &saremote;
        memset ((char *) sin, 0, sizeof (saremote));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(authreq->ipaddr);
	sin->sin_port = htons(authreq->udp_port);

	/* Send it to the client */
	sendto(activefd, (char *)auth, (int)total_length, (int)0,
			(struct sockaddr *) sin, sizeof(struct sockaddr_in));
}

/*************************************************************************
 *
 *	Function: calc_acctreq
 *
 *	Purpose: Validates the requesting client NAS.  Calculates 
 *		 the accounting-request signature based on the 
 *		 clients private key.
 *	Returns: -1 Client not found
 *		  0 signature matched expected value
 *		  1 signature was all 0's (for backwards compatibility)
 *		  2 signature was non-zero and did not match
 *
 *************************************************************************/

int
calc_acctreq(authreq)
AUTH_REQ	*authreq;
{
	u_char	buffer[1024];
	u_char	secret[20];
	u_char	digest[16];
	char	hostnm[256];
	char	*ip_hostname();
	int	secretlen;
	int	len;
	int	find_client();
	int	build_packet();
	void	md5_calc();

	/*
	 * Validate the requesting IP address -
	 * Not secure, but worth the check for accidental requests
	 * find_client() logs an error message if needed
	 */
	if(find_client(authreq->ipaddr, secret, hostnm) != 0) {
		return(SIGN_NOCLIENT);
	}

	/*
	 * The NAS and RADIUS accounting server share a secret.
	 * The Request Authenticator field in Accounting-Request packets
	 * contains a one-way MD5 hash calculated over a stream of octets
	 * consisting of the Code + Identifier + Length + 16 zero octets +
	 * request attributes + shared secret (where + indicates
	 * concatenation).  The 16 octet MD5 hash value is stored in the
	 * Authenticator field of the Accounting-Request packet.
 	 */

	memset(buffer, 0, sizeof(buffer));
	if (memcmp(buffer,authreq->vector,AUTH_VECTOR_LEN) == 0) {
		/* client did not sign accounting-request */
		return(SIGN_ZERO);
	}
	len = build_packet(authreq,buffer);
	memset(buffer+4, 0, AUTH_VECTOR_LEN);
	secretlen = strlen((char *)secret);
	strncpy((char *)authreq->secret, (char *)secret,20);
	memcpy(buffer+len,secret,secretlen);
	md5_calc(digest, buffer, len+secretlen);
	memset(buffer, 0, len+secretlen);
	memset(secret, 0, secretlen);
	if (memcmp(digest,authreq->vector,AUTH_VECTOR_LEN) == 0) {
		return(SIGN_MATCH);
	} else {
		return(SIGN_NOMATCH);
	}
		
}



/*************************************************************************
 *
 *	Function: build_packet
 *
 *	Purpose: build a RADIUS Packet from an authreq structure,
 *		 except for the authenticator
 *
 *************************************************************************/

int
build_packet(authreq, buffer)
AUTH_REQ	*authreq;
u_char		*buffer;
{
	u_short			total_length;
	u_char			*ptr;
	u_short			len;
	UINT4			lvalue;
	VALUE_PAIR		*req;

	ptr=buffer;
	*ptr++ = authreq->code;
	*ptr = authreq->id;
	memset(buffer+2, 0, AUTH_VECTOR_LEN+2);
	total_length = AUTH_HDR_LEN;

	/* Load up the attributes */
	ptr = buffer + total_length;
	req = authreq->request;
	while(req != (VALUE_PAIR *)NULL) {
		*ptr++ = req->attribute;

		switch(req->type) {

		case PW_TYPE_STRING:
			len = strlen(req->strvalue);
			if (len >= AUTH_STRING_LEN) {
				len = AUTH_STRING_LEN - 1;
			}
			*ptr++ = len + 2;
			memcpy(ptr, req->strvalue,len);
			ptr += len;
			total_length += len + 2;
			break;
		
		case PW_TYPE_INTEGER:
		case PW_TYPE_IPADDR:
			*ptr++ = sizeof(UINT4) + 2;
			lvalue = htonl(req->lvalue);
			memcpy(ptr, &lvalue, sizeof(UINT4));
			ptr += sizeof(UINT4);
			total_length += sizeof(UINT4) + 2;
			break;

		default:
			break;
		}
	
		req = req->next;
	}

	len = htons(total_length);
	memcpy(buffer+2, &len, 2);

	return(total_length);
}
