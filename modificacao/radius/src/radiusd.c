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
"$Id: radiusd.c,v 1.43 1997/06/05 08:20:48 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<sys/ipc.h>
#include	<netinet/in.h>
#include	<sys/stat.h>

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
#include	<syslog.h>
#include	<grp.h>

#if !defined(NOSHADOW)
#include	<shadow.h>
#endif /* !NOSHADOW */

#include	"radius.h"
#include	"users.h"

char		recv_buffer[4096];
char		send_buffer[4096];
char		*progname;
int		sockfd;
int		acctfd;
int		debug_flag;
int		spawn_flag;
int		acct_pid;
int		radius_dbm;
int		max_requests;
int		max_request_time;
char		*radius_dir;
char		*radacct_dir;
char		*radius_log;
u_short		radius_port;
UINT4		expiration_seconds;
UINT4		warning_seconds;
extern int	errno;
static AUTH_REQ	*first_request;
int		cleanup_pid;

char		cached_hostnm[128];
char		unknown[8];
UINT4		cached_ipaddr;

struct	sockaddr_in	rad_saremote;

void		sig_fatal();
void		sig_hup();
void		sig_cleanup();
void		process_menu();

char		*ipaddr2strp();
void		md5_calc();

int
main(argc, argv)
int	argc;
char	**argv;
{
	int			result;
	struct	sockaddr_in	salocal;
	struct	sockaddr_in	saremote;
	struct	sockaddr_in	*sin;
	struct	servent		*svp;
        u_short                 lport;
	char			argval;
	int			t;
	int			pid;
	fd_set			readfds;
	int			status;
	int			atoi();
	int			config_init();
	int			dict_init();
	int			update_clients();
	void			log_version();
	void			rad_exit();
	void			rad_request();
	void			usage();
	void			version();


	progname = *argv++;
	argc--;

	debug_flag = 0;
	spawn_flag = 1;
	radius_dbm = 0;
	max_requests = MAX_REQUESTS;
	max_request_time = MAX_REQUEST_TIME;
	radacct_dir = RADACCT_DIR;
	radius_dir = RADIUS_DIR;

	signal(SIGHUP, sig_hup);
	signal(SIGINT, sig_fatal);
	signal(SIGQUIT, sig_fatal);
	signal(SIGILL, sig_fatal);
	signal(SIGTRAP, sig_fatal);
	signal(SIGIOT, sig_fatal);
	signal(SIGFPE, sig_fatal);
	signal(SIGTERM, sig_fatal);
	signal(SIGCHLD, sig_cleanup);

	while(argc) {

		if(**argv != '-') {
			usage();
		}

		argval = *(*argv + 1);
		argc--;
		argv++;

		switch(argval) {

		case 'a':
			if(argc == 0) {
				usage();
			}
			radacct_dir = *argv;
			argc--;
			argv++;
			break;
		
		case 'b':	/* use dbm users file */
			radius_dbm = 1;
			break;

		case 'd':
			if(argc == 0) {
				usage();
			}
			radius_dir = *argv;
			argc--;
			argv++;
			break;

		case 'h':
			usage();
			break;

		case 'l':	/* change logging from syslog */
			if(argc == 0) {
				usage();
			}
			radius_log = *argv;
			argc--;
			argv++;
			break;
		
		case 'p':	/* set radius port */
			if(argc == 0) {
				usage();
			}
			radius_port = (u_short)atoi(*argv);
			argc--;
			argv++;
			break;

		case 'q':	/* set max queue size */
			if(argc == 0) {
				usage();
			}
			max_requests = (int)atoi(*argv);
			argc--;
			argv++;
			break;

		case 's':	/* Single process mode */
			spawn_flag = 0;
			break;

		case 't':	/* set max time out in seconds */
			if(argc == 0) {
				usage();
			}
			max_request_time = (int)atoi(*argv);
			argc--;
			argv++;
			break;


		case 'v':
			version();
			break;

		case 'x':
			debug_flag = 1;
			break;
		
		default:
			usage();
			break;
		}
	}

	if (debug_flag) {
		if (radius_log == (char *)NULL) {
			/*
			 * for backward compatibility
			 * send messages to users tty
			 */
			radius_log = "/dev/tty";
		} else if (strcmp(radius_log, "syslog") == 0) {
			/*
			 * allow user to override backward compatibility
			 * and send debug to syslog
			 */
			radius_log = (char *)NULL;
		}
	}

	/* Initialize the dictionary */
	if(dict_init() != 0) {
		rad_exit(-1);
	}

	/* Initialize Configuration Values */
	if(config_init() != 0) {
		rad_exit(-1);
	}

	/*
	 *	Disconnect from session
	 */
	if(debug_flag == 0) {
		pid = fork();
		if(pid < 0) {
			log_err("system error: could not fork at startup\n");
			rad_exit(-1);
		}
		if(pid > 0) {
			exit(0);
		}
	}

	/*
	 *	Disconnect from tty
	 */
	for (t = 32; t >= 3; t--) {
		close(t);
	}
	/* Show our stuff */
	log_version();

	if (radius_port) {
		lport = htons(radius_port);
	} else {
		svp = getservbyname ("radius", "udp");
		if (svp != (struct servent *) NULL) {
			lport = (u_short) svp->s_port;
		} else {
			lport = htons(PW_AUTH_UDP_PORT);
		}
	}
	DEBUG("using udp port %d for RADIUS\n", ntohs(lport));

	sockfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		log_err("auth socket error %s\n", sys_errlist[errno]);
		rad_exit(-1);
	}

	sin = (struct sockaddr_in *) & salocal;
        memset ((char *) sin, '\0', sizeof (salocal));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = lport;

	result = bind (sockfd, (struct sockaddr *)&salocal, sizeof (*sin));
	if (result < 0) {
		log_err("auth bind error %s\n", sys_errlist[errno]);
		rad_exit(-1);
	}

	/*
	 * Open Accounting Socket.
	 */
	if (radius_port) {
		lport = htons((radius_port + 1));
	} else {
		svp = getservbyname ("radacct", "udp");
		if (svp != (struct servent *) 0) {
			lport = (u_short) svp->s_port;
		} else {
			lport = htons(PW_ACCT_UDP_PORT);
		}
	}
	DEBUG("using udp port %d for RADIUS accounting\n",ntohs(lport));

	acctfd = socket (AF_INET, SOCK_DGRAM, 0);
	if (acctfd < 0) {
		log_err("acct socket error %s\n", sys_errlist[errno]);
		rad_exit(-1);
	}

	sin = (struct sockaddr_in *) & salocal;
        memset ((char *) sin, '\0', sizeof (salocal));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = lport;

	result = bind (acctfd, (struct sockaddr *)&salocal, sizeof (*sin));
	if (result < 0) {
		log_err("acct bind error %s\n", sys_errlist[errno]);
		rad_exit(-1);
	}

	/*
	 * If we are able to spawn processes, we will start a child
	 * to listen for Accounting-Requests.  If not, we will 
	 * listen for them ourself.
	 */
	if(spawn_flag) {
		acct_pid = fork();
		if(acct_pid < 0) {
			log_err("could not fork to spawn accounting daemon\n");
			rad_exit(-1);
		}
		if(acct_pid > 0) {
			close(acctfd);
			acctfd = -1;
		}
		else {
			close(sockfd);
			sockfd = -1;
		}
	}
		

	/*
	 *	Receive user requests
	 */
	sin = (struct sockaddr_in *) & saremote;

	if(sockfd != -1) {
		update_clients();
	}

	for(;;) {

		FD_ZERO(&readfds);
		if(sockfd >= 0) {
			FD_SET(sockfd, &readfds);
		}
		if(acctfd >= 0) {
			FD_SET(acctfd, &readfds);
		}

		status = select(32, &readfds, NULL, NULL, (struct timeval *)NULL);
		if(sockfd != -1) {
			update_clients();
		}

		if(status == -1) {
			if (errno == EINTR)
				continue;
			sig_fatal(101);
		}
		if(sockfd >= 0 && FD_ISSET(sockfd, &readfds)) {
			rad_request(sockfd);
		}
		if(acctfd >=0 && FD_ISSET(acctfd, &readfds)) {
			rad_request(acctfd);
		}
	}
}

/*************************************************************************
 *
 *	Function: rad_request
 *
 *	Purpose: Receive UDP client requests
 *
 *************************************************************************/

void
rad_request(fd)
int fd;
{
	int			salen;
	int			result;
	struct	sockaddr_in	*sin;
	AUTH_REQ		*authreq;
	AUTH_REQ		*radrecv();
	void			radrespond();
#ifdef SECURID
	u_char			*ptr;
	int			length;
	int			attribute;
	int			attrlen;
	int			child_pid;
	AUTH_HDR		*auth;
	AUTH_REQ		*curreq;
	key_t			msg_key;
	int			msg_id;
	char			buf[AUTH_STRING_LEN + 2];
	int			atoi();
#endif
	salen = sizeof (rad_saremote);
	sin = (struct sockaddr_in *) & rad_saremote;
	result = recvfrom (fd, (char *) recv_buffer,
		(int) sizeof(recv_buffer),
		(int) 0, (struct sockaddr *)&rad_saremote, & salen);
	if (result < AUTH_HDR_LEN) {
		log_err("rad_request: runt packet of %d bytes\n",result);
		return;
	}

#ifdef SECURID

	if (spawn_flag == 0) {
		authreq = radrecv(
			ntohl(sin->sin_addr.s_addr),
			ntohs(sin->sin_port),
			recv_buffer, result);
		radrespond(authreq, fd);
		return;
	}
	/*
	 * We need to see if this is a challenge response
	 */
	auth = (AUTH_HDR *)recv_buffer;
	ptr = auth->data;
	length = ntohs(auth->length);
	if (length > result) {
		log_err("rad_request: request from client %s claimed length %d, only %d bytes found\n",
			ipaddr2strp((u_long)ntohl(sin->sin_addr.s_addr)), 
			length, result);
		return;
	}
	length -= AUTH_HDR_LEN;
	child_pid = -1;

	while(length > 0) {
		attribute = *ptr++;
		attrlen = *ptr++;
		if(attrlen < 2) {
			break;
		}
		attrlen -= 2;
		if ((attribute == PW_STATE) && (attrlen <= AUTH_STRING_LEN)) {
			memcpy(buf, ptr, attrlen);
			buf[attrlen] = '\0';
			DEBUG("rad_request: PW_STATE<%s>\n", buf);
			/*
			 * the format for SECURID state string is
			 *
			 *	SECURID_xxxx=n
			 *
			 * xxxx is commands: next or npin or wait
			 * n is the child pid
			 */
			if (strncmp(buf, "SECURID_", 8) == 0) {
				child_pid = (int)atoi(&buf[13]);
			}
			break;
		}
		ptr += attrlen;
		length -= attrlen + 2;
	}

	if (child_pid == -1) {
		authreq = radrecv(
			ntohl(sin->sin_addr.s_addr),
			ntohs(sin->sin_port),
			recv_buffer, result);
		radrespond(authreq, fd);
		return;
	}
	DEBUG("rad_request: challenge_response id %d child %d\n",
			auth->id, child_pid);
	curreq = first_request;
	while(curreq != (AUTH_REQ *)NULL) {
		if (curreq->child_pid == child_pid) {
			break;
		}
		curreq = curreq->next;
	}
	if (curreq == (AUTH_REQ *)NULL) {
		log_err("rad_request: child %d not found\n", child_pid);
		return;
	}
	if (curreq->ipaddr != ntohl(sin->sin_addr.s_addr)) {
		log_err("rad_request: error: mismatched IP addresses in request %x != %x for ID %d %d\n",
			curreq->ipaddr,
			ntohl(sin->sin_addr.s_addr),
			curreq->id, auth->id);
		return;
	}
	if (curreq->id == auth->id) {
		/* This is a duplicate request - just drop it */
		log_err("rad_request: dropped duplicate ID %d\n", auth->id);
		return;
	}
	msg_key = RADIUS_MSG_KEY(child_pid);
	if ((msg_id = msgget(msg_key, 0600)) == -1) {
		log_err("rad_request: error: msgget for key %x for id %d returned error %d\n",msg_key, msg_id, errno);
		return;
	}
	if (msgsnd(msg_id, recv_buffer, result, IPC_NOWAIT) == -1) {
		log_err("rad_request: error: msgsnd for key %x for id %d returned error %d\n", msg_key, msg_id, errno);
		return;
	}
	curreq->id = auth->id;

#else /* not SECURID */

	authreq = radrecv(
		ntohl(sin->sin_addr.s_addr),
		ntohs(sin->sin_port),
		recv_buffer, result);
	radrespond(authreq, fd);

#endif /* not SECURID */

}

/*************************************************************************
 *
 *	Function: radrecv
 *
 *	Purpose: Receive UDP client requests, build an authorization request
 *		 structure, and attach attribute-value pairs contained in
 *		 the request to the new structure.
 *
 *************************************************************************/

AUTH_REQ	*
radrecv(host, udp_port, buffer, length)
UINT4	host;
u_short	udp_port;
u_char	*buffer;
int	length;
{
	u_char		*ptr;
	AUTH_HDR	*auth;
	int		totallen;
	int		attribute;
	int		attrlen;
	DICT_ATTR	*attr;
	DICT_ATTR	*dict_attrget();
	UINT4		lvalue;
	VALUE_PAIR	*first_pair;
	VALUE_PAIR	*prev;
	VALUE_PAIR	*pair;
	AUTH_REQ	*authreq;
	void		rad_exit();

	if (length < AUTH_HDR_LEN) {            /* too short to be real */
                return ((AUTH_REQ *)NULL);
        }

	/*
	 * Pre-allocate the new request data structure
	 */

	if((authreq = (AUTH_REQ *)malloc(sizeof(AUTH_REQ))) ==
						(AUTH_REQ *)NULL) {
		log_err("radrecv: fatal system error: out of memory, exiting\n");
		rad_exit(-1);
	}

	auth = (AUTH_HDR *)buffer;
	totallen = ntohs(auth->length);
	if (totallen > length) {	/* truncated packet, ignore */
		log_err("radrecv: request from client %s claimed length %d, only %d bytes found\n", ipaddr2strp((u_long)host), totallen, length);
		free(authreq);
		return((AUTH_REQ *)NULL);
	}

	DEBUG("radrecv: Request from host %s code=%d, id=%d, length=%d\n",
		ipaddr2strp((u_long)host), auth->code, auth->id, totallen);

	/*
	 * Fill header fields
	 */
	authreq->ipaddr = host;
	authreq->udp_port = udp_port;
	authreq->id = auth->id;
	authreq->code = auth->code;
	memcpy(authreq->vector, auth->vector, AUTH_VECTOR_LEN);

	/*
	 * Extract attribute-value pairs
	 */
	ptr = auth->data;
	length = totallen - AUTH_HDR_LEN;
	first_pair = (VALUE_PAIR *)NULL;
	prev = (VALUE_PAIR *)NULL;

	while(length > 0) {

		attribute = *ptr++;
		attrlen = *ptr++;
		if(attrlen < 2) {
			length = 0;
			continue;
		}
		attrlen -= 2;
		if((attr = dict_attrget(attribute)) == (DICT_ATTR *)NULL) {
			DEBUG("received unknown attribute %d\n", attribute);
		}
		else if ( attrlen >= AUTH_STRING_LEN ) {
			DEBUG("attribute %d too long, length of %d >= %d\n", attribute, attrlen, AUTH_STRING_LEN);
		}
		else {
			if((pair = (VALUE_PAIR *)malloc(sizeof(VALUE_PAIR))) ==
						(VALUE_PAIR *)NULL) {
				log_err("radrecv: fatal system error: out of memory, exiting\n");
				rad_exit(-1);
			}
			strcpy(pair->name, attr->name);
			pair->attribute = attr->value;
			pair->type = attr->type;
			pair->next = (VALUE_PAIR *)NULL;

			switch(attr->type) {

			case PW_TYPE_STRING:
				memcpy(pair->strvalue, ptr, attrlen);
				pair->strvalue[attrlen] = '\0';
				pair->lvalue = attrlen;
				debug_pair(pair);
				if(first_pair == (VALUE_PAIR *)NULL) {
					first_pair = pair;
				}
				else {
					prev->next = pair;
				}
				prev = pair;
				break;
			
			case PW_TYPE_INTEGER:
			case PW_TYPE_IPADDR:
				memcpy(&lvalue, ptr, sizeof(UINT4));
				pair->lvalue = ntohl(lvalue);
				debug_pair(pair);
				if(first_pair == (VALUE_PAIR *)NULL) {
					first_pair = pair;
				}
				else {
					prev->next = pair;
				}
				prev = pair;
				break;
			
			default:
				DEBUG("    %s (Unknown Type %d)\n",
					attr->name,attr->type);
				free(pair);
				break;
			}

		}
		ptr += attrlen;
		length -= attrlen + 2;
	}
	authreq->request = first_pair;
	return(authreq);
}

/*************************************************************************
 *
 *	Function: radrespond
 *
 *	Purpose: Respond to supported requests:
 *
 *		 PW_AUTHENTICATION_REQUEST - Authentication request from
 *				a client network access server.
 *
 *		 PW_ACCOUNTING_REQUEST - Accounting request from
 *				a client network access server.
 *
 *		 PW_PASSWORD_REQUEST - User request to change a password.
 *
 *************************************************************************/

void
radrespond(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	char		*ip_hostname();
	int		rad_authenticate();
	void		rad_accounting();
	void		rad_spawn_child();
#ifdef PASSCHANGE
	void		rad_passchange();
#endif

        if (authreq == (AUTH_REQ *)NULL) {
                return;
        }

	switch(authreq->code) {

	case PW_AUTHENTICATION_REQUEST:
		if(spawn_flag) {
			rad_spawn_child(authreq, activefd);
		}
		else {
			rad_authenticate(authreq, activefd);
		}
		break;
	
	case PW_ACCOUNTING_REQUEST:
		rad_accounting(authreq, activefd);
		break;
	
#ifdef PASSCHANGE
	case PW_PASSWORD_REQUEST:
		rad_passchange(authreq, activefd);
		break;
#endif
	
	default:
		log_err("unknown request type %d from %s ignored\n",
			authreq->code, ip_hostname(authreq->ipaddr)); 
		reqfree(authreq);
		break;
	}
}

/*************************************************************************
 *
 *	Function: rad_spawn_child
 *
 *	Purpose: Spawns child processes to perform password authentication
 *		 and respond to RADIUS clients.  This functions also
 *		 cleans up complete child requests, and verifies that there
 *		 is only one process responding to each request (duplicate
 *		 requests are filtered out.
 *
 *************************************************************************/

int rad_spawned_child_pid;

void
rad_spawn_child(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	AUTH_REQ	*curreq;
	AUTH_REQ	*prevreq;
	UINT4		curtime;
	int		request_count;
	char		*client_hostname();
	int		child_pid;
	void		child_authenticate();
	void		clean_child();
#ifdef SECURID
	key_t		msg_key;
	int		msg_id;
#endif

	curtime = (UINT4)time(0);
	request_count = 0;
	curreq = first_request;
	prevreq = (AUTH_REQ *)NULL;
	while(curreq != (AUTH_REQ *)NULL) {
		if(curreq->child_pid == -1 &&
				curreq->timestamp + CLEANUP_DELAY <= curtime) {
			/* Request completed, delete it */
			if(prevreq == (AUTH_REQ *)NULL) {
				first_request = curreq->next;
				pairfree(curreq->request);
				free(curreq);
				curreq = first_request;
			}
			else {
				prevreq->next = curreq->next;
				pairfree(curreq->request);
				free(curreq);
				curreq = prevreq->next;
			}
		}
		else if(curreq->ipaddr == authreq->ipaddr &&
					curreq->id == authreq->id) {
			/* This is a duplicate request - just drop it */
			log_err("dropping duplicate request for id %d from %s\n",
				authreq->id,client_hostname(authreq->ipaddr));
			reqfree(authreq);
			return;
		}
		else {
			if(curreq->timestamp + max_request_time <= curtime &&
						curreq->child_pid != -1) {
				/* This request seems to have hung - kill it */
				child_pid = curreq->child_pid;
				log_err("sending SIGHUP signal to unresponsive child process %d\n",
								child_pid);
				curreq->child_pid = -1;
				kill(child_pid, SIGHUP);
#ifdef SECURID
				/*
				 * delete childs message queue
				 */
				msg_key = RADIUS_MSG_KEY(child_pid);
				if ((msg_id = msgget(msg_key, 0600)) != -1) {
					msgctl(msg_id, IPC_RMID, 0);
				}
#endif /* SECURID */
			}
			prevreq = curreq;
			curreq = curreq->next;
			request_count++;
		}
	}

	/* This is a new request */
	if(request_count > max_requests) {
		request_count--;
		log_err("dropping request for id %d from %s; %d requests already in queue\n",
			authreq->id,client_hostname(authreq->ipaddr),request_count);
		reqfree(authreq);
		return;
	}

	/* Add this request to the list */
	authreq->next = (AUTH_REQ *)NULL;
	authreq->child_pid = -1;
	authreq->timestamp = curtime;

	if(prevreq == (AUTH_REQ *)NULL) {
		first_request = authreq;
	}
	else {
		prevreq->next = authreq;
	}

	/* fork our child */
	cleanup_pid = -1;
	rad_spawned_child_pid = fork();
	if(rad_spawned_child_pid < 0) {
		log_err("system error: fork failed with error %d for request %d from %s\n",
			errno,authreq->id, client_hostname(authreq->ipaddr));
		reqfree(authreq);
		return;
	}
	if(rad_spawned_child_pid == 0) {
		/* This is the child, it should go ahead and respond */
		child_authenticate(authreq, activefd);
		exit(0);
	}

	/* Register the Child */
	authreq->child_pid = rad_spawned_child_pid;

	/*
	 * If cleanup_pid is not -1, then we received a SIGCHLD between
	 * the time we forked and the time we got here, so clean up after it
	 */
	if(cleanup_pid != -1) {
		clean_child(cleanup_pid);
		cleanup_pid = -1;
	}
}

void
clean_child(pid)
int	pid;
{
	AUTH_REQ	*curreq;

	curreq = first_request;
	while(curreq != (AUTH_REQ *)NULL) {
		if(curreq->child_pid == pid) {
			curreq->child_pid = -1;
			curreq->timestamp = (UINT4)time(0);
			return;
		}
		curreq = curreq->next;
	}
	cleanup_pid = (int)pid;
	return;
}

void
sig_cleanup(sig)
int	sig;
{
	int		status;
        pid_t		pid;
 
        for (;;) {
		pid = waitpid((pid_t)-1,&status,WNOHANG);
		signal(SIGCHLD, sig_cleanup);
                if (pid <= 0)
                        return;

#if defined (aix)
		kill(pid, SIGKILL);
#endif

		if(pid == acct_pid) {
			sig_fatal(100);
		}
		clean_child(pid);
        }
}

#ifdef PASSCHANGE

/*************************************************************************
 *
 *	Function: rad_passchange
 *
 *	Purpose: Change a users password
 *
 *************************************************************************/

void
rad_passchange(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	VALUE_PAIR	*namepair;
	VALUE_PAIR	*check_item;
	VALUE_PAIR	*newpasspair;
	VALUE_PAIR	*oldpasspair;
	VALUE_PAIR	*curpass;
	VALUE_PAIR	*user_check;
	VALUE_PAIR	*user_reply;
	VALUE_PAIR	*get_attribute();
	USER_DESC	*user_desc;
	char		pw_digest[16];
	char		string[64];
	char		passbuf[AUTH_PASS_LEN];
	char		auth_name[AUTH_STRING_LEN + 2];
	int		i;
	int		secretlen;
	char		*ip_hostname();
	int		user_update();
	int		set_expiration();
	void		send_pwack();
	void		send_reject();

	/* Get the username */
	namepair = get_attribute(authreq->request,PW_USER_NAME);

	if(namepair == (VALUE_PAIR *)NULL) {
		log_err("passchange: from %s - no user name supplied\n",
			ip_hostname(authreq->ipaddr));
		reqfree(authreq);
		return;
	}

	/*
	 * Open the user table
	 */
	if((user_desc = user_open()) == (USER_DESC *)NULL) {
		/* error already logged by user_open() */
		reqfree(authreq);
		return;
	}

	/*
	 * Look the user up in the database
	 */
	if (user_find(	namepair->strvalue,
			auth_name,
			&user_check,
			&user_reply,
			user_desc) != 0) {
		log_err("passchange: from %s - user %s not found\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		send_reject(authreq, (char *)NULL, activefd);
		reqfree(authreq);
		user_close(user_desc);
		return;
	}
	user_close(user_desc);

	/*
	 * Validate the user -
	 *
	 * We have to unwrap this in a special way to decrypt the
	 * old and new passwords.  The MD5 calculation is based
	 * on the old password.  The vector is different.  The old
	 * password is encrypted using the encrypted new password
	 * as its vector.  The new password is encrypted using the
	 * random encryption vector in the request header.
	 */

	/* Extract the attr-value pairs for the old and new passwords */
	check_item = authreq->request;
	while(check_item != (VALUE_PAIR *)NULL) {
		if(check_item->attribute == PW_PASSWORD) {
			newpasspair = check_item;
		}
		else if(check_item->attribute == PW_OLD_PASSWORD) {
			oldpasspair = check_item;
		}
		check_item = check_item->next;
	}

	/* Verify that both encrypted passwords were supplied */
	if(newpasspair == (VALUE_PAIR *)NULL ||
					oldpasspair == (VALUE_PAIR *)NULL) {
		/* Missing one of the passwords */
		log_err("passchange: from %s - missing password in request for user %s\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		send_reject(authreq, (char *)NULL, activefd);
		pairfree(user_check);
		pairfree(user_reply);
		reqfree(authreq->request);
		return;
	}

	/* Get the current password from the database */
	curpass = user_check;
	while(curpass != (VALUE_PAIR *)NULL) {
		if(curpass->attribute == PW_PASSWORD) {
			break;
		}
		curpass = curpass->next;
	}
	if((curpass == (VALUE_PAIR *)NULL) || curpass->strvalue == (char *)NULL) {
		/* Missing our local copy of the password */
		log_err("passchange: from %s - no password found for user %s\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		send_reject(authreq, (char *)NULL, activefd);
		pairfree(user_check);
		pairfree(user_reply);
		reqfree(authreq);
		return;
	}
	if(strcmp(curpass->strvalue,"UNIX") == 0) {
		/* Can not change passwords that are not in users file */
		log_err("passchange: from %s - system password change not allowed for user %s\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		send_reject(authreq, (char *)NULL, activefd);
		pairfree(user_check);
		pairfree(user_reply);
		reqfree(authreq);
		return;
	}

	/* Decrypt the old password */
	secretlen = strlen(curpass->strvalue);
	memcpy(string, curpass->strvalue, secretlen);
	memcpy(string + secretlen, newpasspair->strvalue, AUTH_VECTOR_LEN);
	md5_calc(pw_digest, string, AUTH_VECTOR_LEN + secretlen);
	memcpy(passbuf, oldpasspair->strvalue, AUTH_PASS_LEN);
	for(i = 0;i < AUTH_PASS_LEN;i++) {
		passbuf[i] ^= pw_digest[i];
	}

	/* Did they supply the correct password ??? */
	if(strncmp(passbuf, curpass->strvalue, AUTH_PASS_LEN) != 0) {
		log_err("passchange: from %s - incorrect password for user %s\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		send_reject(authreq, (char *)NULL, activefd);
		pairfree(user_check);
		pairfree(user_reply);
		reqfree(authreq);
		return;
	}

	/* Decrypt the new password */
	memcpy(string, curpass->strvalue, secretlen);
	memcpy(string + secretlen, authreq->vector, AUTH_VECTOR_LEN);
	md5_calc(pw_digest, string, AUTH_VECTOR_LEN + secretlen);
	memcpy(passbuf, newpasspair->strvalue, AUTH_PASS_LEN);
	for(i = 0;i < AUTH_PASS_LEN;i++) {
		passbuf[i] ^= pw_digest[i];
	}

	/* Update the users password */
	strncpy(curpass->strvalue, passbuf, AUTH_PASS_LEN);

	/* Add a new expiration date if we are aging passwords */
	if(expiration_seconds != (UINT4)0) {
		set_expiration(user_check, expiration_seconds);
	}

	/* Update the database */
	if(user_update(namepair->strvalue, user_check, user_reply) != 0) {
		send_reject(authreq, (char *)NULL, activefd);
		log_err("passchange: unable to update password for user %s\n",
			namepair->strvalue);
	}
	else {
		send_pwack(authreq, activefd);
	}
	pairfree(user_check);
	pairfree(user_reply);
	reqfree(authreq);
	return;
}
#endif	/* PASSCHANGE */

/*************************************************************************
 *
 *	Function: child_authenticate
 *
 *	Purpose: Process and reply to an authentication request
 *
 *************************************************************************/

void
child_authenticate(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	int			rad_authenticate();
#ifdef SECURID
	key_t			msg_key;
	int			msg_id;
	int			length;
	struct	sockaddr_in	*sin;

	msg_key = RADIUS_MSG_KEY(getpid());
#endif /* SECURID */
	for (;;) {
		if (rad_authenticate(authreq, activefd) == 0) {
			break;
		}
#ifdef SECURID
		if ((msg_id = msgget(msg_key, IPC_CREAT | 0600)) == -1) {
			log_err("child_authenticate: msgget for key %x for id %d returned error: %s\n", msg_key, msg_id, sys_errlist[errno]);
			break;
		}
		if ((length = msgrcv(msg_id, recv_buffer, 
				sizeof recv_buffer - sizeof(long),
				0, 0)) == -1) {
			log_err("child_authenticate: msgrcv for msgid %d returned error: %s\n", msg_id, sys_errlist[errno]);
			break;
		}
		if (msgctl(msg_id, IPC_RMID, 0) == -1) {
			log_err("child_authenticate: msgctl for msgid %d returned error: %s\n", msg_id, sys_errlist[errno]);
		}
		sin = (struct sockaddr_in *) &rad_saremote;
		authreq = radrecv(
			ntohl(sin->sin_addr.s_addr),
			ntohs(sin->sin_port),
			recv_buffer, length);
#else /* not SECURID */
		break;
#endif /* not SECURID */
	}
}

/*************************************************************************
 *
 *	Function: rad_authenticate
 *
 *	Purpose: Process and reply to an authentication request
 *
 *************************************************************************/

int
rad_authenticate(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	USER_DESC	*user_desc;
	VALUE_PAIR	*attr;
	VALUE_PAIR	*auth_item;
	VALUE_PAIR	*check_item;
	VALUE_PAIR	*get_attribute();
	VALUE_PAIR	*namepair;
	VALUE_PAIR	*password_item;
	VALUE_PAIR	*user_check;
	VALUE_PAIR	*user_reply;
	char		auth_name[AUTH_STRING_LEN + 2];
	char		hold_vector[AUTH_VECTOR_LEN];
	char		pw_digest[16];
	char		string[AUTH_STRING_LEN + 2];
	char		umsg[AUTH_STRING_LEN + 2];
	char		*client_hostname();
	char		*crypt();
	char		*encpw;
	char		*ip_hostname();
	char		*ptr;
	char		*user_msg;
	int		authtype;
	int		i;
	int		j;
	int		passlen;
	int		result;
	int		retval;
	int		speed;
	int		atoi();
	int		calc_digest();
	int		pw_expired();
	int		unix_group();
	int		unix_pass();
	void		calc_next_digest();
	void		send_accept();
	void		send_reject();
#ifdef SECURID
	int		securid();
#endif

	/* Get the username from the request */
	namepair = get_attribute(authreq->request, PW_USER_NAME);
	if((namepair == (VALUE_PAIR *)NULL) || 
	   (strlen(namepair->strvalue) <= (size_t)0)) {
		log_err("auth: access-request from %s ignored; no user name\n",
			ip_hostname(authreq->ipaddr));
		reqfree(authreq);
		return(0);
	}

	/* Verify the client and calculate the MD5 Password Digest */
	if(calc_digest(pw_digest, authreq) != 0) {
		/* We do not respond when this fails */
		log_err("auth: access-request from unknown client %s ignored; user name %s\n",
			ip_hostname(authreq->ipaddr), namepair->strvalue);
		reqfree(authreq);
		return(0);
	}

	/*
	 * If the request is processing a menu, service it here.
	 */
	if((attr = get_attribute(authreq->request, PW_STATE)) !=
		(VALUE_PAIR *)NULL && strncmp(attr->strvalue, "MENU=", 5) == 0){
		process_menu(authreq, activefd, pw_digest);
		return(0);
	}

	/*
	 * Open the user table
	 */
#ifdef RADIUSERSORACLE
  	if (orauser_open() != 0) {
#else /* not RADIUSERSORACLE */
	if((user_desc = user_open()) == (USER_DESC *)NULL) {
#endif /* RADIUSERSORACLE */
		reqfree(authreq);
		return(0);
	}

	for (;;) {
		/* Get the user from the database */
#ifdef RADIUSERSORACLE
  		if ((result = orauser_find(	namepair->strvalue,
						auth_name,
						&user_check,
						&user_reply)) != 0) {
#else /* not RADIUSERSORACLE */
		if ((result = user_find(	namepair->strvalue,
						auth_name,
						&user_check,
						&user_reply,
						user_desc)) != 0) {
#endif /* RADIUSERSORACLE */
			log_err("auth: access-request from %s (%s) denied for unknown user %s\n",
				client_hostname(authreq->ipaddr),
				ipaddr2strp(authreq->ipaddr),
				namepair->strvalue);
			send_reject(authreq, (char *)NULL, activefd);
			reqfree(authreq);
#ifdef RADIUSERSORACLE
  			orauser_close();
#else /* not RADIUSERSORACLE */
			user_close(user_desc);
#endif
			return(0);
		}

		/* Validate the user */

		/* Look for matching check items */
		password_item = (VALUE_PAIR *)NULL;
		authtype = PW_AUTHTYPE_LOCAL;
		user_msg = (char *)NULL;
		check_item = user_check;
		while(result == 0 && check_item != (VALUE_PAIR *)NULL) {

			auth_item = get_attribute(authreq->request,
							check_item->attribute);
			
			switch(check_item->attribute) {

			case PW_PREFIX:
			case PW_SUFFIX:
				break;

			case PW_EXPIRATION:
				/*
				 * Check expiration date if we are
				 * doing password aging.
				 */

				/* Has this user's password expired */
				retval = pw_expired(check_item->lvalue);
				if(retval < 0) {
					result = -2;
 					sprintf(umsg,"Password Has Expired\r\n");
					user_msg = umsg;
				}
				else {
					if(retval > 0) {
						sprintf(umsg,
				  "Password Will Expire in %d Days\r\n",
						  retval);
						user_msg = umsg;
					}
				}
				break;
			
			case PW_PASSWORD:
				if(strcmp(check_item->strvalue, "UNIX") == 0) {
					authtype = PW_AUTHTYPE_UNIX;
				}
				else {
					password_item = check_item;
				}
				break;

			case PW_AUTHTYPE:
				authtype = check_item->lvalue;
				break;
			
			case PW_GROUP:
				if(!unix_group(auth_name, check_item->strvalue)) {
					result = -1;
				}
				break;

			case PW_CRYPT_PASSWORD:
				authtype = PW_AUTHTYPE_CRYPT;
				password_item = check_item;
				break;
			
			case PW_CONNECT_RATE:
				auth_item = get_attribute(authreq->request,
							PW_CONNECT_INFO);
				if (auth_item != (VALUE_PAIR *)NULL) {
					speed = atoi(auth_item->strvalue);
					if (speed > check_item->lvalue) {
						result = -1;
					}
				}
				break;

			default:
				if(auth_item == (VALUE_PAIR *)NULL) {
					result = -1;
					break;
				}

				switch(check_item->type) {

				case PW_TYPE_STRING:
					if(strcmp(check_item->strvalue,
						  auth_item->strvalue) != 0) {
						result = -1;
					}
					break;

				case PW_TYPE_INTEGER:
				case PW_TYPE_IPADDR:
					if(check_item->lvalue
							!= auth_item->lvalue) {
						result = -1;
					}
					break;

				default:
					result = -1;
					break;
				}
				break;
			}
			check_item = check_item->next;
		}
		if (result != -1) {
			break;
		}
	}

#ifdef RADIUSERSORACLE
  	orauser_close();
#else /* not RADIUSERSORACLE */
	user_close(user_desc);
#endif

	/*
	 * At this point we have validated all normal comparisons
	 * for the user.  All that is left is the actual authentication.
	 * Authentication will be done based on the authentication type
	 * previously specified.
	 */

	if(result == 0) {
		
		/*
		 * Decrypt the password in the request.
		 */
		if((auth_item = get_attribute(authreq->request,
					PW_PASSWORD)) != (VALUE_PAIR *)NULL) {

			passlen = auth_item->lvalue;
			if(passlen > AUTH_MAXPASS_LEN) {
				passlen = AUTH_MAXPASS_LEN;
			}
			memcpy(string, auth_item->strvalue, AUTH_MAXPASS_LEN);
			ptr = string;
			for(i = 0;i < passlen;i += AUTH_PASS_LEN) {
				/*
				 * Store the vector to be used in next segment
				 * of the encrypted password.
				 */
				memcpy(hold_vector, ptr, AUTH_VECTOR_LEN);

				/* Decrypt from the digest */
				for(j = 0;j < AUTH_PASS_LEN;j++) {
					*ptr ^= pw_digest[j];
					ptr++;
				}

				/* Calculate the next digest if necessary */
				if(i < passlen) {
					calc_next_digest(pw_digest,
						authreq->secret, hold_vector);
				}
			}
			*ptr = '\0';
		}
		else {
			string[0] = '\0';
		}

		switch(authtype) {

		case PW_AUTHTYPE_LOCAL:
		default:
			/*
			 * The local authentication type supports normal
			 * password comparison and the Three-Way CHAP.
			 */
			if(password_item == (VALUE_PAIR *)NULL) {
				if(string[0] != '\0') {
					result = -1;
				}
			}
			else if(auth_item != (VALUE_PAIR *)NULL) {
				if(strcmp(password_item->strvalue,
								string) != 0) {
                                        log_err("SENHA INCORRETA: REQUISICAO: %s - USERS: %s\n",string,password_item->strvalue);
					result = -1;
				} else {
                                         log_err("SENHA CORRETA: %s\n",password_item->strvalue);
                                }
			}
			/*
			 * Check to see if we have a CHAP password.
			 */
			else if((auth_item = get_attribute(authreq->request,
				PW_CHAP_PASSWORD)) != (VALUE_PAIR *)NULL) {

				/* Use MD5 to verify */
				ptr = string;
				*ptr++ = *auth_item->strvalue;
				strcpy(ptr, password_item->strvalue);
				ptr += strlen(password_item->strvalue);
				memcpy(ptr, authreq->vector, AUTH_VECTOR_LEN);
				md5_calc(pw_digest, string,
					1 + CHAP_VALUE_LENGTH +
					strlen(password_item->strvalue));
				/* Compare them */
				if(memcmp(pw_digest, auth_item->strvalue + 1,
						CHAP_VALUE_LENGTH) != 0) {
					result = -1;
				}
			}
			else {
				result = -1;
			}
			break;

		case PW_AUTHTYPE_UNIX:
			if(auth_item != (VALUE_PAIR *)NULL) {
				if(unix_pass(auth_name, string) != 0) {
					result = -1;
				}
			}
			else {
				result = -1;
			}
			break;
		
#ifdef SECURID
		case PW_AUTHTYPE_SECURID:
			if(auth_item != (VALUE_PAIR *)NULL) {
				pairfree(user_check);
				return( securid(auth_name, string,
					authreq, user_reply, activefd) );
			}
			else {
				result = -1;
			}
			break;
#endif /* SECURID */
		case PW_AUTHTYPE_CRYPT:
			/* password is stored encrypted in string */
			if(password_item == (VALUE_PAIR *)NULL) {
				if(string[0] != '\0') {
					result = -1;
				}
			}
			else if(auth_item != (VALUE_PAIR *)NULL) {
				encpw = crypt(string,password_item->strvalue);
				if(strcmp(encpw,password_item->strvalue) != 0) {
					result = -1;
				}
			}
			else {
				result = -1;
			}
			break;
		case PW_AUTHTYPE_REJECT:
			result = -1;
			break;
		}
	}
	if(result != 0) {
                log_err("PEDIDO DE AUTENTICACAO REJEITADO\n");
		send_reject(authreq, user_msg, activefd);
	}
	else {
                log_err("PEDIDO DE AUTENTICACAO ACEITO\n");
		send_accept(authreq, user_reply, user_msg, activefd);
	}
	reqfree(authreq);
	pairfree(user_check);
	pairfree(user_reply);
	return(0);
}

/*************************************************************************
 *
 *	Function: send_reject
 *
 *	Purpose: Reply to the request with a REJECT.  Also attach
 *		 any user message provided.
 *
 *************************************************************************/

void
send_reject(authreq, msg, activefd)
AUTH_REQ	*authreq;
char		*msg;
int		activefd;
{
	AUTH_HDR		*auth;
	struct	sockaddr_in	saremote;
	struct	sockaddr_in	*sin;
	char			*client_hostname();
	char			digest[AUTH_VECTOR_LEN];
	int			secretlen;
	int			total_length;
	u_char			*ptr;
	int			len;
	int			block_len;

	auth = (AUTH_HDR *)send_buffer;

	/* Build standard response header */
#ifdef PASSCHANGE
	if(authreq->code == PW_PASSWORD_REQUEST) {
		auth->code = PW_PASSWORD_REJECT;
	}
	else {
		auth->code = PW_AUTHENTICATION_REJECT;
	}
#else /* not PASSCHANGE */
	auth->code = PW_AUTHENTICATION_REJECT;
#endif /* PASSCHANGE */
	auth->id = authreq->id;
	memcpy(auth->vector, authreq->vector, AUTH_VECTOR_LEN);
	total_length = AUTH_HDR_LEN;
	ptr = auth->data;

	/* Append the user message */
	if(msg != (char *)NULL) {
		len = strlen(msg);
		do {
			if(len > AUTH_STRING_LEN) {
				block_len = AUTH_STRING_LEN;
			}
			else {
				block_len = len;
			}

			*ptr++ = PW_PORT_MESSAGE;
			*ptr++ = block_len + 2;
			memcpy(ptr, msg, block_len);
			msg += block_len;
			ptr += block_len;
			total_length += block_len + 2;
			len -= block_len;
		} while(len > 0);
	}

	/* Set total length in the header */
	auth->length = htons(total_length);

	/* Calculate the response digest */
	secretlen = strlen((const char *)authreq->secret);
	memcpy(send_buffer + total_length, authreq->secret, secretlen);
	md5_calc(digest, (char *)auth, total_length + secretlen);
	memcpy(auth->vector, digest, AUTH_VECTOR_LEN);
	memset(send_buffer + total_length, 0, secretlen);

	sin = (struct sockaddr_in *) &saremote;
        memset ((char *) sin, '\0', sizeof (saremote));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(authreq->ipaddr);
	sin->sin_port = htons(authreq->udp_port);

	DEBUG("Sending Reject of id %d to %s (%s)\n",
		authreq->id, client_hostname(authreq->ipaddr),
		ipaddr2strp(authreq->ipaddr));
	
	/* Send it to the user */
	sendto(activefd, (char *)auth, (int)total_length, (int)0,
			(struct sockaddr *) &saremote, sizeof(struct sockaddr_in));
}

/*************************************************************************
 *
 *	Function: send_challenge
 *
 *	Purpose: Reply to the request with a CHALLENGE.  Also attach
 *		 any user message provided and a state value.
 *
 *************************************************************************/

void
send_challenge(authreq, msg, state, activefd)
AUTH_REQ	*authreq;
char		*msg;
char		*state;
int		activefd;
{
	AUTH_HDR		*auth;
	struct	sockaddr_in	saremote;
	struct	sockaddr_in	*sin;
	char			*client_hostname();
	char			digest[AUTH_VECTOR_LEN];
	int			secretlen;
	int			total_length;
	u_char			*ptr;
	int			len;
	int			block_len;

	auth = (AUTH_HDR *)send_buffer;

	/* Build standard response header */
	auth->code = PW_ACCESS_CHALLENGE;
	auth->id = authreq->id;
	memcpy(auth->vector, authreq->vector, AUTH_VECTOR_LEN);
	total_length = AUTH_HDR_LEN;
	ptr = auth->data;

	/* Append the user message */
	if(msg != (char *)NULL && (len = strlen(msg)) > 0) {
		while(len > 0) {
			if(len > AUTH_STRING_LEN) {
				block_len = AUTH_STRING_LEN;
			}
			else {
				block_len = len;
			}

			*ptr++ = PW_PORT_MESSAGE;
			*ptr++ = block_len + 2;
			memcpy(ptr, msg, block_len);
			msg += block_len;
			ptr += block_len;
			total_length += block_len + 2;
			len -= block_len;
		}
	}

	/* Append the state info */
	if((state != (char *)NULL) && (strlen(state) > (size_t)0)) {
		len = strlen(state);
		*ptr++ = PW_STATE;
		*ptr++ = len + 2;
		memcpy(ptr, state, len);
		ptr += len;
		total_length += len + 2;
	}

	/* Set total length in the header */
	auth->length = htons(total_length);

	/* Calculate the response digest */
	secretlen = strlen((const char *)authreq->secret);
	memcpy(send_buffer + total_length, authreq->secret, secretlen);
	md5_calc(digest, (char *)auth, total_length + secretlen);
	memcpy(auth->vector, digest, AUTH_VECTOR_LEN);
	memset(send_buffer + total_length, 0, secretlen);

	sin = (struct sockaddr_in *) &saremote;
        memset ((char *) sin, '\0', sizeof (saremote));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(authreq->ipaddr);
	sin->sin_port = htons(authreq->udp_port);

	DEBUG("Sending Challenge of id %d to %s (%s)\n",
		authreq->id, client_hostname(authreq->ipaddr),
		ipaddr2strp(authreq->ipaddr));
	
	/* Send it to the user */
	sendto(activefd, (char *)auth, (int)total_length, (int)0,
			(struct sockaddr *) &saremote, sizeof(struct sockaddr_in));
}

#ifdef PASSCHANGE
/*************************************************************************
 *
 *	Function: send_pwack
 *
 *	Purpose: Reply to the request with an ACKNOWLEDGE.
 *		 User password has been successfully changed.
 *
 *************************************************************************/

void
send_pwack(authreq, activefd)
AUTH_REQ	*authreq;
int		activefd;
{
	AUTH_HDR		*auth;
	struct	sockaddr_in	saremote;
	struct	sockaddr_in	*sin;
	char			*ip_hostname();
	char			digest[AUTH_VECTOR_LEN];
	int			secretlen;

	auth = (AUTH_HDR *)send_buffer;

	/* Build standard response header */
	auth->code = PW_PASSWORD_ACK;
	auth->id = authreq->id;
	memcpy(auth->vector, authreq->vector, AUTH_VECTOR_LEN);
	auth->length = htons(AUTH_HDR_LEN);

	/* Calculate the response digest */
	secretlen = strlen((const char *)authreq->secret);
	memcpy(send_buffer + AUTH_HDR_LEN, authreq->secret, secretlen);
	md5_calc(digest, (char *)auth, AUTH_HDR_LEN + secretlen);
	memcpy(auth->vector, digest, AUTH_VECTOR_LEN);
	memset(send_buffer + AUTH_HDR_LEN, 0, secretlen);

	sin = (struct sockaddr_in *) &saremote;
        memset ((char *) sin, '\0', sizeof (saremote));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(authreq->ipaddr);
	sin->sin_port = htons(authreq->udp_port);

	DEBUG("Sending PW Ack of id %d to %lx (%s)\n",
		authreq->id, (u_long)authreq->ipaddr,
		ip_hostname(authreq->ipaddr));
	
	/* Send it to the user */
	sendto(activefd, (char *)auth, (int)AUTH_HDR_LEN, (int)0,
			(struct sockaddr *)&saremote,
			sizeof(struct sockaddr_in));
}
#endif /* PASSCHANGE */

/*************************************************************************
 *
 *	Function: send_accept
 *
 *	Purpose: Reply to the request with an ACKNOWLEDGE.  Also attach
 *		 reply attribute value pairs and any user message provided.
 *
 *************************************************************************/

void
send_accept(authreq, reply, msg, activefd)
AUTH_REQ	*authreq;
VALUE_PAIR	*reply;
char		*msg;
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
	VALUE_PAIR		*menu_attr;
	VALUE_PAIR		*get_attribute();
	char			*get_menu();
	int			block_len;
	char			state_value[100];

	/* Check to see if the response is a menu */
	if((menu_attr = get_attribute(reply, PW_MENU)) != (VALUE_PAIR *)NULL) {
		msg = get_menu(menu_attr->strvalue);
		sprintf(state_value, "MENU=%s", menu_attr->strvalue);
		send_challenge(authreq, msg, state_value, activefd);
		return;
	}

	auth = (AUTH_HDR *)send_buffer;

	/* Build standard header */
	auth->code = PW_AUTHENTICATION_ACK;
	auth->id = authreq->id;
	memcpy(auth->vector, authreq->vector, AUTH_VECTOR_LEN);

	total_length = AUTH_HDR_LEN;

	DEBUG("Sending Accept of id %d to %s (%s)\n",
		authreq->id, client_hostname(authreq->ipaddr),
		ipaddr2strp(authreq->ipaddr));

	/* Load up the configuration values for the user */
	ptr = auth->data;
	while(reply != (VALUE_PAIR *)NULL) {
		debug_pair(reply);
		*ptr++ = reply->attribute;

		switch(reply->type) {

		case PW_TYPE_STRING:
			len = strlen(reply->strvalue);
			if (len >= AUTH_STRING_LEN) {
				len = AUTH_STRING_LEN - 1;
			}
			*ptr++ = len + 2;
			memcpy(ptr, reply->strvalue,len);
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

	/* Append the user message */
	if(msg != (char *)NULL && (len = strlen(msg)) > 0) {
		while(len > 0) {
			if(len > AUTH_STRING_LEN) {
				block_len = AUTH_STRING_LEN;
			}
			else {
				block_len = len;
			}

			*ptr++ = PW_PORT_MESSAGE;
			*ptr++ = block_len + 2;
			memcpy(ptr, msg, block_len);
			msg += block_len;
			ptr += block_len;
			total_length += block_len + 2;
			len -= block_len;
		}
	}

	auth->length = htons(total_length);

	/* Append secret and calculate the response digest */
	secretlen = strlen((const char *)authreq->secret);
	memcpy(send_buffer + total_length, authreq->secret, secretlen);
	md5_calc(digest, (char *)auth, total_length + secretlen);
	memcpy(auth->vector, digest, AUTH_VECTOR_LEN);
	memset(send_buffer + total_length, 0, secretlen);

	sin = (struct sockaddr_in *) &saremote;
        memset ((char *) sin, '\0', sizeof (saremote));
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = htonl(authreq->ipaddr);
	sin->sin_port = htons(authreq->udp_port);

	/* Send it to the user */
	sendto(activefd, (char *)auth, (int)total_length, (int)0,
			(struct sockaddr *)&saremote,
			sizeof(struct sockaddr_in));
}

/*************************************************************************
 *
 *	Function: unix_pass
 *
 *	Purpose: Check the users password against the standard UNIX
 *		 password table.
 *
 *************************************************************************/

int
unix_pass(name, passwd)
char	*name;
char	*passwd;
{
	struct passwd	*pwd;
	struct passwd	*getpwnam();
	char		*encpw;
	char		*crypt();
	char		*encrypted_pass;
#if !defined(NOSHADOW)
#if defined(M_UNIX)
	struct passwd	*spwd;
#else
	struct spwd	*spwd;
#endif
#endif /* !NOSHADOW */
	
	/* Get encrypted password from password file */
	if((pwd = getpwnam(name)) == NULL) {
		DEBUG("unix_pass: getpwnam for <%s> failed\n", name);
		return(-1);
	}

	encrypted_pass = pwd->pw_passwd;

#if !defined(NOSHADOW)
	if(strcmp(pwd->pw_passwd, "x") == 0 ||
	   strcmp(pwd->pw_passwd, "*") == 0 ||
	   strcmp(pwd->pw_passwd, "*NP*") == 0) {
		if((spwd = getspnam(name)) == NULL) {
			return(-1);
		}
#if defined(M_UNIX)
		encrypted_pass = spwd->pw_passwd;
#else
		encrypted_pass = spwd->sp_pwdp;
#endif	/* M_UNIX */
	}
#endif	/* !NOSHADOW */

	/* Run encryption algorythm */
	encpw = crypt(passwd, encrypted_pass);

	/* Check it */
	if(strcmp(encpw, encrypted_pass)) {
		DEBUG("unix_pass: password for <%s> failed\n", name);
		return(-1);
	}
	return(0);
}

/*************************************************************************
 *
 *	Function: unix_group
 *
 *	Purpose: Check the user's membership to the standard UNIX
 *		 group table.
 *
 *************************************************************************/

int
unix_group(name, group)
char	*name;
char	*group;
{
	struct passwd	*pwd;
	struct passwd	*getpwnam();
	char		**gr_mem;
	struct group	*gr_ent;
	
	/* Get encrypted password from password file */
	if((pwd = getpwnam(name)) == NULL) {
		DEBUG("unix_group: getpwnam for <%s> failed\n", name);
		return(0);
	}

	if((gr_ent = getgrnam(group)) == NULL) {
		DEBUG("unix_group: getgrnam(%s) for <%s> failed\n", group,name);
		return(0);
	}

	/* Check the immediate group */
	if(pwd->pw_gid == gr_ent->gr_gid) {
		return(1);
	}
	/* Search for this user */
	gr_mem = gr_ent->gr_mem;
	while(*gr_mem != NULL) {
		if(strcmp(*gr_mem, name) == 0) {
			return(1);
		}
		gr_mem++;
	}
	return(0);
}

/*************************************************************************
 *
 *	Function: calc_digest
 *
 *	Purpose: Validates the requesting client NAS.  Calculates the
 *		 digest to be used for decrypting the users password
 *		 based on the clients private key.
 *
 *************************************************************************/

int
calc_digest(digest, authreq)
u_char		*digest;
AUTH_REQ	*authreq;
{
	u_char	buffer[128];
	u_char	secret[64];
	char	hostnm[256];
	char	*ip_hostname();
	int	secretlen;
	int	find_client();

	/*
	 * Validate the requesting IP address -
	 * Not secure, but worth the check for accidental requests
	 * find_client() logs an error message if needed
	 */
	if(find_client(authreq->ipaddr, secret, hostnm) != 0) {
		return(-1);
	}

	/* Use the secret to setup the decryption digest */
	memset(buffer, 0, sizeof(buffer));
	secretlen = strlen((const char *)secret);
	strcpy((char *)buffer, (const char *)secret);
	memcpy(buffer + secretlen, authreq->vector, AUTH_VECTOR_LEN);
	md5_calc(digest, buffer, secretlen + AUTH_VECTOR_LEN);
	strcpy((char *)authreq->secret, (const char *)secret);
	memset(buffer, 0, sizeof(buffer));
	memset(secret, 0, secretlen);
	return(0);
}

/*************************************************************************
 *
 *	Function: calc_next_digest
 *
 *	Purpose: Calculates the digest to be used for decrypting the 
 *	users password past the first 16 octets based on the clients
 *	private key.
 *
 *************************************************************************/

void
calc_next_digest(digest, secret, vector)
u_char	*digest;
u_char	*secret;
u_char	*vector;
{
	u_char	buffer[128];
	int	secretlen;

	/* Use the secret to setup the decryption digest */
	memset(buffer, 0, sizeof(buffer));
	secretlen = strlen((const char *)secret);
	strcpy((char *)buffer, (const char *)secret);
	memcpy(buffer + secretlen, vector, AUTH_VECTOR_LEN);
	md5_calc(digest, buffer, secretlen + AUTH_VECTOR_LEN);
	memset(buffer, 0, sizeof(buffer));
}

/*************************************************************************
 *
 *	Function: client_hostname
 *
 *	Purpose: Return the cached client name if we have one.  Otherwise
 *		 use the regular ip_hostname() function.
 *
 *************************************************************************/

char	*
client_hostname(ipaddr)
UINT4	ipaddr;
{
	u_char	secret[64];
	char	hostnm[128];
	char	*ip_hostname();
	int	find_client();

	/* Look at the last used entry first */
	if(ipaddr == cached_ipaddr) {
		return(cached_hostnm);
	}
	if(find_client(ipaddr, secret, hostnm) != 0) {
		memset(secret, 0, sizeof(secret));
		return(cached_hostnm);	/* set by find_client() */
	}
	return(ip_hostname(ipaddr));
}

/*************************************************************************
 *
 *	Function: find_client
 *
 *	Purpose: Retrieve the client name and secret from the temporary
 *		 DBM client database.
 *
 *************************************************************************/

int
find_client(ipaddr, secret, hostnm)
UINT4	ipaddr;
char	*secret;
char	*hostnm;
{
	datum		named;
	datum		contentd;
	DBM		*db;
	char		ip_str[32];
	char		buffer[256];
	void		ipaddr2str();

	/* Find the client in the database */
	sprintf((char *)buffer, "%s/%s", radius_dir, RADIUS_CLIENT_CACHE);
	/* Find the client in the database */
# ifdef NDBM
        if((db = dbm_open(buffer, O_RDONLY, 0600)) == (DBM *)NULL) {
# else /* not NDBM */
        if(dbminit(buffer) != 0) {
# endif /* NDBM */
		log_err("could not read %s to find clients\n", buffer);
		return(-1);
	}

	ipaddr2str(ip_str, ipaddr);
	named.dptr = ip_str;
	named.dsize = strlen(ip_str);

	contentd = dbm_fetch(db, named);
	if(contentd.dsize == 0 || contentd.dptr == NULL) {
		dbm_close(db);
		log_err("client %s not found in client cache\n", ip_str);
		return(-1);
	}
	memcpy(buffer,contentd.dptr,contentd.dsize);
        buffer[contentd.dsize] = '\0';

	if(sscanf((const char *)buffer, "%s%s", hostnm, secret) != 2) {
		dbm_close(db);
		log_err("client cache entry for %s could not be parsed\n", ip_str);
		return(-1);
	}

	/* Build a cached hostname entry for client_hostname() to use */
	strcpy(cached_hostnm, hostnm);
	cached_ipaddr = ipaddr;

	dbm_close(db);
	return(0);
}

/*************************************************************************
 *
 *	Function: update_clients
 *
 *	Purpose: Check last modified time on clients file and build a
 *		 new temporary DBM client database if the file has been
 *		 changed.
 *
 *************************************************************************/

int
update_clients()
{
	static time_t	last_update_time;
	struct stat 	statbuf;
	struct stat 	statbuf2;
	datum		named;
	datum		contentd;
	DBM		*db;
	FILE		*clientfd;
	u_char		buffer[256];
	u_char		oldcache[256];
	u_char		newcache[256];
	u_char		secret[64];
	char		hostnm[128];
	char		ip_str[64];
	int		nclients;
	int		rcode;
	int		s1;
	int		s2;
	UINT4		ipaddr;
	UINT4		get_ipaddr();
	void		ipaddr2str();

	nclients = 0;
	rcode = 0;

	/* Check last modified time of clients file */
	sprintf((char *)buffer, "%s/%s", radius_dir, RADIUS_CLIENTS);
	if(stat(buffer, &statbuf) != 0) {
		log_err("Error: clients file %s not found\n", buffer);
		return(-1);
	}
	if(statbuf.st_mtime == last_update_time) {
		/* nothing to update */
		return(0);
	}
	cached_ipaddr = 0;

	/* Open the standard clients file */
	if((clientfd = fopen((const char *)buffer, "r")) == (FILE *)NULL) {
		log_err("Error: could not read clients file %s\n", buffer);
		return(-1);
	}

	/* Open and truncate the clients DBM cache file */
	sprintf((char *)oldcache, "%s/%s", radius_dir, RADIUS_CLIENT_CACHE);
	sprintf((char *)newcache, "%s.lock", oldcache);
#if defined(NDBM)
	if((db = dbm_open(newcache, O_RDWR | O_CREAT | O_TRUNC, 0600))
							== (DBM *)NULL) {
#else
        if(dbminit(newcache) != 0) {
#endif
		log_err("Error: could not create temporary client cache file %s\n",newcache);
		return(-1);
	}

	while(fgets((char *)buffer, sizeof(buffer), clientfd)
						!= (char *)NULL) {
		if(*buffer == '#') {
			continue;
		}
		if(sscanf((const char *)buffer, "%s%s", hostnm, secret) != 2) {
			continue;
		}
		if((ipaddr = get_ipaddr(hostnm)) != (UINT4)0) {
			ipaddr2str(ip_str, ipaddr);
			named.dptr = ip_str;
			named.dsize = strlen(ip_str);
			contentd.dptr = (char *)buffer;
			contentd.dsize = strlen(buffer);
#if defined(NDBM)
			if(dbm_store(db, named, contentd, DBM_INSERT) != 0) {
#else /* not NDBM */
                	if(store(named, contentd) != 0) {
#endif /* NDBM */

				log_err("could not cache client datum for host %s\n", hostnm);
				rcode = -1;
			} else {
				nclients++;
			}
		}
	}
	dbm_close(db);
	fclose(clientfd);
	s1 = strlen(newcache);
	strcat(newcache,".db");
	if (stat(newcache,&statbuf2) == 0) {
		strcat(oldcache,".db");
		if (rename(newcache,oldcache) != 0) {
			log_err("Error: could not move client cache file %s to %s, error %d\n",newcache,oldcache,errno);
			return(-1);
		} else {
			DEBUG("updated client cache with %d clients\n",nclients);
		}
	} else {
		newcache[s1] = '\0';
		s2 = strlen(oldcache);
		strcat(newcache,".pag");
		strcat(oldcache,".pag");
		if (rename(newcache,oldcache) != 0) {
			log_err("Error: could not move client cache file %s to %s, error %d\n",newcache,oldcache,errno);
			return(-1);
		}
		newcache[s1] = '\0';
		oldcache[s2] = '\0';
		strcat(newcache,".dir");
		strcat(oldcache,".dir");
		if (rename(newcache,oldcache) != 0) {
			log_err("Error: could not move client cache file %s to %s, error %d\n",newcache,oldcache,errno);
			rcode = -1;
		} else {
			DEBUG("updated client cache with %d clients\n",nclients);
		}
	}
	if (rcode == 0) {
		last_update_time = statbuf.st_mtime;
	}
	return(rcode);
}

/*************************************************************************
 *
 *	Function: debug_pair
 *
 *	Purpose: Print the Attribute-value pair to the desired File.
 *
 *************************************************************************/

void
debug_pair(pair)
VALUE_PAIR * pair;
{
	void	fprint_attr_val();

	if(debug_flag) {
		fprint_attr_val(-1, pair);
	}
}

/*************************************************************************
 *
 *	Function: usage
 *
 *	Purpose: Display the syntax for starting this program.
 *
 *************************************************************************/

void
usage()
{
	fprintf(stderr, "Usage: %s", progname);
	fprintf(stderr, " [-a <acct_dir>]");
	fprintf(stderr, " [-b]");
	fprintf(stderr, " [-d <db_dir>]");
	fprintf(stderr, " [-h]");
	fprintf(stderr, " [-l <log_file>]");
	fprintf(stderr, " [-p <udp_port>]");
	fprintf(stderr, " [-q <max outstanding requests>]");
	fprintf(stderr, " [-s]");
	fprintf(stderr, " [-t <seconds>]");
	fprintf(stderr, " [-v]");
	fprintf(stderr, " [-x]\n");
	exit(-1);
}

/*************************************************************************
 *
 *	Function: config_init
 *
 *	Purpose: intializes configuration values:
 *
 *		 expiration_seconds - When updating a user password,
 *			the amount of time to add to the current time
 *			to set the time when the password will expire.
 *			This is stored as the VALUE Password-Expiration
 *			in the dictionary as number of days.
 *
 *		warning_seconds - When acknowledging a user authentication
 *			time remaining for valid password to notify user
 *			of password expiration.
 *
 *************************************************************************/

int
config_init()
{
	DICT_VALUE	*dval;
	DICT_VALUE	*dict_valfind();

	if((dval = dict_valfind("Password-Expiration")) == (DICT_VALUE *)NULL) {
		expiration_seconds = (UINT4)0;
	}
	else {
		expiration_seconds = dval->value * (UINT4)SECONDS_PER_DAY;
	}
	if((dval = dict_valfind("Password-Warning")) == (DICT_VALUE *)NULL) {
		warning_seconds = (UINT4)0;
	}
	else {
		warning_seconds = dval->value * (UINT4)SECONDS_PER_DAY;
	}
	strcpy(unknown,"unknown");	/* for client caching error return */
	return(0);
}

/*************************************************************************
 *
 *	Function: set_expiration
 *
 *	Purpose: Set the new expiration time by updating or adding
		 the Expiration attribute-value pair.
 *
 *************************************************************************/

int
set_expiration(user_check, expiration)
VALUE_PAIR	*user_check;
UINT4		expiration;
{
	VALUE_PAIR	*exppair;
	VALUE_PAIR	*prev;
	struct timeval	tp;
	struct timezone	tzp;

	if(user_check == (VALUE_PAIR *)NULL) {
		return(-1);
	}

	/* Look for an existing expiration entry */
	exppair = user_check;
	prev = (VALUE_PAIR *)NULL;
	while(exppair != (VALUE_PAIR *)NULL) {
		if(exppair->attribute == PW_EXPIRATION) {
			break;
		}
		prev = exppair;
		exppair = exppair->next;
	}
	if(exppair == (VALUE_PAIR *)NULL) {
		/* Add a new attr-value pair */
		if((exppair = (VALUE_PAIR *)malloc(sizeof(VALUE_PAIR))) ==
					(VALUE_PAIR *)NULL) {
			log_err("setexp: system error: out of memory\n");
			return(-1);
		}
		/* Initialize it */
		strcpy(exppair->name, "Expiration");
		exppair->attribute = PW_EXPIRATION;
		exppair->type = PW_TYPE_DATE;
		*exppair->strvalue = '\0';
		exppair->lvalue = (UINT4)0;
		exppair->next = (VALUE_PAIR *)NULL;

		/* Attach it to the list. */
		prev->next = exppair;
	}

	/* calculate a new expiration */
	gettimeofday(&tp, &tzp);
	exppair->lvalue = tp.tv_sec + expiration;
	return(0);
}

/*************************************************************************
 *
 *	Function: pw_expired
 *
 *	Purpose: Tests to see if the users password has expired.
 *
 *	Return: Number of days before expiration if a warning is required
 *		otherwise 0 for success and -1 for failure.
 *
 *************************************************************************/

int
pw_expired(exptime)
UINT4	exptime;
{
	struct timeval	tp;
	struct timezone	tzp;
	UINT4		exp_remain;
	int		exp_remain_int;

	if(expiration_seconds == (UINT4)0) {	/* expiration not enabled */
		return(0);
	}

	gettimeofday(&tp, &tzp);
	if(tp.tv_sec > exptime) {
		return(-1);
	}
	if(warning_seconds != (UINT4)0) {
		if(tp.tv_sec > exptime - warning_seconds) {
			exp_remain = exptime - tp.tv_sec;
			exp_remain /= (UINT4)SECONDS_PER_DAY;
			exp_remain_int = exp_remain;
			return(exp_remain_int);
		}
	}
	return(0);
}

/*************************************************************************
 *
 *	Function: get_attribute
 *
 *	Purpose: Retrieve a specific value-pair from a list of value-pairs.
 *
 *************************************************************************/

VALUE_PAIR	*
get_attribute(value_list, attribute)
VALUE_PAIR	*value_list;
int		attribute;
{
	while(value_list != (VALUE_PAIR *)NULL) {
		if(value_list->attribute == attribute) {
			return(value_list);
		}
		value_list = value_list->next;
	}
	return((VALUE_PAIR *)NULL);
}

/*************************************************************************
 *
 *      Function: reqfree
 *
 *      Purpose: Release the memory used by an AUTH_REQ structure
 *
 *************************************************************************/

void 
reqfree(authreq)
AUTH_REQ        *authreq;
{
        if (authreq != (AUTH_REQ *)NULL) {
                pairfree(authreq->request);
                memset(authreq, 0, sizeof(AUTH_REQ));
                free(authreq);
        }
}


void
sig_fatal(sig)
int	sig;
{
	void rad_exit();

	if(acct_pid > 0) {
		kill(acct_pid, SIGKILL);
	}

	log_err("exit on signal %d\n", sig);
	rad_exit(1);
}

void
sig_hup(sig)
int	sig;
{
	return;
}

void
rad_exit(rc)
int	rc;
{
#ifdef SECURID

	AUTH_REQ	*curreq;
	key_t		msg_key;
	int		msg_id;

	if (rad_spawned_child_pid == 0) {
		/*
		 * child clean up
		 */
		msg_key = RADIUS_MSG_KEY(getpid());
		if ((msg_id = msgget(msg_key, 0600)) != -1) {
			msgctl(msg_id, IPC_RMID, 0);
		}
	} else {
		/*
		 * parent clean up
		 */
		curreq = first_request;
		while(curreq != (AUTH_REQ *)NULL) {
			msg_key = RADIUS_MSG_KEY(curreq->child_pid);
			if ((msg_id = msgget(msg_key, 0600)) != -1) {
				msgctl(msg_id, IPC_RMID, 0);
			}
			curreq = curreq->next;
		}
	}

#endif /* SECURID */

	exit(rc);
}
