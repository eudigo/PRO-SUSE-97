/*
 *
 *	RADIUS
 *	Remote Authentication Dial In User Service
 *
 *
 *	Livingston Enterprises, Inc.
 *	6920 Koll Center Parkway
 *	Pleasanton, CA   94566
 *
 *	Copyright 1992-1996 Livingston Enterprises, Inc. All Rights Reserved.
 *
 *	This software source code is provided under license from
 *	Livingston Enterprises, Inc., the terms and conditions of which
 *	are set forth in an End User Agreement that is contained in
 *	both the product packaging, and electronically on the
 *	Livingston ftp site. This software may only be used in
 *	conjunction with Livingston (or Livingston authorized)
 *	products.  Livingston makes no warranties to any licensee
 *	concerning the applicability of the software to licensee's
 *	specific requirements or the suitability of the software for
 *	any intended use. Licensee shall not remove, modify or alter
 *	any copyright and/or other proprietary rights notice and must
 *	faithfully reproduce all such notices on any copies or
 *	modifications to this software that it makes.
 *	
 *	Livingston Enterprises, Inc. makes no representations about
 *	the suitability of this software for any purpose.  It is
 *	provided "as is" without express or implied warranty.
 *
 */

/*
 *      $Id: radius.h,v 1.17 1997/05/15 06:49:49 cdr Exp $ %Y%
 */

#include "conf.h"

#define AUTH_VECTOR_LEN		16
#define AUTH_PASS_LEN		16
#define AUTH_MAXPASS_LEN	48
#define AUTH_STRING_LEN		128	/* maximum of 254 */


typedef struct pw_auth_hdr {
	u_char		code;
	u_char		id;
	u_short		length;
	u_char		vector[AUTH_VECTOR_LEN];
	u_char		data[2];
} AUTH_HDR;

#define AUTH_HDR_LEN			20
#define CHAP_VALUE_LENGTH		16

#define PW_AUTH_UDP_PORT		1645
#define PW_ACCT_UDP_PORT		1646

#define PW_TYPE_STRING			0
#define PW_TYPE_INTEGER			1
#define PW_TYPE_IPADDR			2
#define PW_TYPE_DATE			3


#define	PW_AUTHENTICATION_REQUEST	1
#define	PW_AUTHENTICATION_ACK		2
#define	PW_AUTHENTICATION_REJECT	3
#define	PW_ACCOUNTING_REQUEST		4
#define	PW_ACCOUNTING_RESPONSE		5
#ifdef PASSCHANGE
#define PW_PASSWORD_REQUEST		7
#define PW_PASSWORD_ACK			8
#define PW_PASSWORD_REJECT		9
#endif /* PASSCHANGE */
#define PW_ACCESS_CHALLENGE		11

#define	PW_USER_NAME			1
#define	PW_PASSWORD			2
#define	PW_CHAP_PASSWORD		3
#define	PW_CLIENT_ID			4
#define	PW_CLIENT_PORT_ID		5
#define	PW_USER_SERVICE_TYPE		6
#define	PW_FRAMED_PROTOCOL		7
#define	PW_FRAMED_ADDRESS		8
#define	PW_FRAMED_NETMASK		9
#define	PW_FRAMED_ROUTING		10
#define	PW_FRAMED_FILTER_ID		11
#define	PW_FRAMED_MTU			12
#define	PW_FRAMED_COMPRESSION		13
#define	PW_LOGIN_HOST			14
#define	PW_LOGIN_SERVICE		15
#define	PW_LOGIN_TCP_PORT		16
#ifdef PASSCHANGE
#define PW_OLD_PASSWORD			17
#endif
#define PW_PORT_MESSAGE			18
#define PW_DIALBACK_NO			19
#define PW_DIALBACK_NAME		20
#define PW_FRAMED_ROUTE			22
#define PW_FRAMED_IPXNET		23
#define PW_STATE			24
#define PW_TERMINATION			29

#define PW_ACCT_STATUS_TYPE		40
#define PW_ACCT_DELAY_TIME		41
#define PW_ACCT_INPUT_OCTETS		42
#define PW_ACCT_OUTPUT_OCTETS		43
#define PW_ACCT_SESSION_ID		44
#define PW_ACCT_AUTHENTIC		45
#define PW_ACCT_SESSION_TIME		46
#
#define PW_CONNECT_INFO			65
/*
 * Non-Protocol Attributes
 */
#define PW_EXPIRATION			  21
#define PW_AUTHTYPE			1000
#define PW_MENU				1001
#define PW_TERMINATION_MENU		1002
#define PW_PREFIX			1003
#define PW_SUFFIX			1004
#define PW_GROUP			1005
#define PW_CRYPT_PASSWORD		1006
#define PW_CONNECT_RATE			1007

/*
 *	INTEGER TRANSLATIONS
 */

/*	USER TYPES	*/

#define	PW_LOGIN_USER			1
#define	PW_FRAMED_USER			2
#define	PW_DIALBACK_LOGIN_USER		3
#define	PW_DIALBACK_FRAMED_USER		4

/*	FRAMED PROTOCOLS	*/

#define	PW_PPP				1
#define	PW_SLIP				2

/*	FRAMED ROUTING VALUES	*/

#define	PW_NONE				0
#define	PW_BROADCAST			1
#define	PW_LISTEN			2
#define	PW_BROADCAST_LISTEN		3

/*	FRAMED COMPRESSION TYPES	*/

#define	PW_VAN_JACOBSON_TCP_IP		1

/*	LOGIN SERVICES	*/

#define	PW_TELNET			0
#define	PW_RLOGIN			1
#define	PW_TCP_CLEAR			2
#define	PW_PORTMASTER			3

/*	AUTHENTICATION LEVEL	*/

#define PW_AUTH_NONE			0
#define PW_AUTH_RADIUS			1
#define PW_AUTH_LOCAL			2

/*	STATUS TYPES	*/

#define PW_STATUS_START			1
#define PW_STATUS_STOP			2

/*	TERMINATION OPTIONS	*/

#define PW_TERM_DEFAULT			0
#define PW_TERM_RADIUS_REQUEST		1

/*	Internal Authentication Types	*/

#define PW_AUTHTYPE_LOCAL		0
#define PW_AUTHTYPE_UNIX		1
#define PW_AUTHTYPE_SECURID		2
#define PW_AUTHTYPE_CRYPT		3
#define PW_AUTHTYPE_REJECT		4

/* Default Database File Names */

#define RADIUS_DIR		"/etc/raddb"
#if defined(LINUXSUSE)
#define RADACCT_DIR		"/var/adm/radacct"
#else
#define RADACCT_DIR		"/usr/adm/radacct"
#endif

#define RADIUS_DICTIONARY	"dictionary"
#define RADIUS_CLIENTS		"clients"
#define RADIUS_CLIENT_CACHE	"clcache"
#define RADIUS_USERS		"users"
#define RADIUS_HOLD		"holdusers"

/* Server data structures */

typedef struct dict_attr {
	char			name[32];
	int			value;
	int			type;
	struct dict_attr	*next;
} DICT_ATTR;

typedef struct dict_value {
	char			attrname[32];
	char			name[32];
	int			value;
	struct dict_value	*next;
} DICT_VALUE;

typedef struct value_pair {
	char			name[32];
	int			attribute;
	int			type;
	UINT4			lvalue;	/* length of strvalue if present */
	char			strvalue[AUTH_STRING_LEN];
	struct value_pair	*next;
} VALUE_PAIR;

typedef struct auth_req {
	UINT4			ipaddr;
	u_short			udp_port;
	u_char			id;
	u_char			code;
	u_char			vector[16];
	u_char			secret[20];	/* max length supported is 16 */
	VALUE_PAIR		*request;
	int			child_pid;	/* Process ID of child */
	UINT4			timestamp;
	struct auth_req		*next;		/* Next active request */
} AUTH_REQ;

#define DEBUG	if(debug_flag)log_debug

#define SECONDS_PER_DAY		86400
#define CLEANUP_DELAY		5
#define MAX_REQUESTS		100
#if defined(SECURID)
#define MAX_REQUEST_TIME	120
#else 
#define MAX_REQUEST_TIME	30
#endif

#define RADIUS_MSG_KEY(pid)	(('r' << 24) + ((pid) & 0x00ffffff))

extern int debug_flag;
extern void debug_pair (VALUE_PAIR * pair);

void	log_debug(), log_err(), pairfree(), reqfree();

