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
"$Id: util.c,v 1.9 1997/05/15 06:49:54 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

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
 *	Function: ip_hostname
 *
 *	Purpose: Return a printable host name (or IP address in dot notation)
 *		 for the supplied IP address.
 *
 *************************************************************************/

char	*
ip_hostname(ipaddr)
UINT4	ipaddr;
{
	struct	hostent *hp;
	static char	hstname[128];
	UINT4	n_ipaddr;
	void	ipaddr2str();

	n_ipaddr = htonl(ipaddr);
	hp = gethostbyaddr((char *)&n_ipaddr, sizeof (struct in_addr), AF_INET);
	if (hp == 0) {
		ipaddr2str(hstname, ipaddr);
		return(hstname);
	}
	return(hp->h_name);
}

/*************************************************************************
 *
 *	Function: get_ipaddr
 *
 *	Purpose: Return an IP address in host long notation from a host
 *		 name or address in dot notation.
 *
 *************************************************************************/

UINT4
get_ipaddr(host)
char	*host;
{
	struct hostent *hp;
	UINT4		ipstr2long();
	int		good_ipaddr();

	if(good_ipaddr(host) == 0) {
		return(ipstr2long(host));
	}
	else if((hp = gethostbyname(host)) == (struct hostent *)NULL) {
		return((UINT4)0);
	}
	return(ntohl(*(UINT4 *)hp->h_addr));
}

/*************************************************************************
 *
 *	Function: good_ipaddr
 *
 *	Purpose: Check for valid IP address in standard dot notation.
 *
 *************************************************************************/

int
good_ipaddr(addr)
char	*addr;
{
	int	dot_count;
	int	digit_count;

	dot_count = 0;
	digit_count = 0;
	while(*addr != '\0' && *addr != ' ') {
		if(*addr == '.') {
			dot_count++;
			digit_count = 0;
		}
		else if(!isdigit(*addr)) {
			dot_count = 5;
		}
		else {
			digit_count++;
			if(digit_count > 3) {
				dot_count = 5;
			}
		}
		addr++;
	}
	if(dot_count != 3) {
		return(-1);
	}
	else {
		return(0);
	}
}

/*************************************************************************
 *
 *	Function: ipaddr2str
 *
 *	Purpose: Return an IP address in standard dot notation for the
 *		 provided address in host long notation.
 *
 *************************************************************************/

void
ipaddr2str(buffer, ipaddr)
char	*buffer;
UINT4	ipaddr;
{
	int	addr_byte[4];
	int	i;
	UINT4	xbyte;

	for(i = 0;i < 4;i++) {
		xbyte = ipaddr >> (i*8);
		xbyte = xbyte & (UINT4)0x000000FF;
		addr_byte[i] = xbyte;
	}
	sprintf(buffer, "%u.%u.%u.%u", addr_byte[3], addr_byte[2],
		addr_byte[1], addr_byte[0]);
}

/*************************************************************************
 *
 *	Function: ipaddr2strp
 *
 *	Purpose: Return an IP address in standard dot notation for the
 *		 provided address in host long notation.
 *
 *************************************************************************/

char	*
ipaddr2strp(ipaddr)
UINT4	ipaddr;
{
	int	addr_byte[4];
	int	i;
	UINT4	xbyte;
	static char buffer[32];

	for(i = 0;i < 4;i++) {
		xbyte = ipaddr >> (i*8);
		xbyte = xbyte & (UINT4)0x000000FF;
		addr_byte[i] = xbyte;
	}
	sprintf(buffer, "%u.%u.%u.%u", addr_byte[3], addr_byte[2],
		addr_byte[1], addr_byte[0]);
	return(buffer);
}
/*************************************************************************
 *
 *	Function: pairfree
 *
 *	Purpose: Release the memory used by a list of attribute-value
 *		 pairs.
 *
 *************************************************************************/

void
pairfree(pair)
VALUE_PAIR	*pair;
{
	VALUE_PAIR	*next;

	while(pair != (VALUE_PAIR *)NULL) {
		next = pair->next;
		free(pair);
		pair = next;
	}
}



/*************************************************************************
 *
 *	Function: ipstr2long
 *
 *	Purpose: Return an IP address in host long notation from
 *		 one supplied in standard dot notation.
 *
 *************************************************************************/

UINT4
ipstr2long(ip_str)
char	*ip_str;
{
	char	buf[6];
	char	*ptr;
	int	i;
	int	count;
	UINT4	ipaddr;
	int	cur_byte;
	int	atoi();

	ipaddr = (UINT4)0;
	for(i = 0;i < 4;i++) {
		ptr = buf;
		count = 0;
		*ptr = '\0';
		while(*ip_str != '.' && *ip_str != '\0' && count < 4) {
			if(!isdigit(*ip_str)) {
				return((UINT4)0);
			}
			*ptr++ = *ip_str++;
			count++;
		}
		if(count >= 4 || count == 0) {
			return((UINT4)0);
		}
		*ptr = '\0';
		cur_byte = atoi(buf);
		if(cur_byte < 0 || cur_byte > 255) {
			return((UINT4)0);
		}
		ip_str++;
		ipaddr = ipaddr << 8 | (UINT4)cur_byte;
	}
	return(ipaddr);
}

