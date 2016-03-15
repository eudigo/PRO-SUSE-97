/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

#if HAVE_SYS_NETINET_IN_H
# include <sys/netinet/in.h>
#endif
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif

#ifndef HAVE_INDEX
# define index(s,c) strchr(s,c)
# define rindex(s,c) strrchr(s,c)
#endif
#ifndef HAVE_BCOPY
# define bcopy(src,dest,len)   (void) (memcpy(dest, src, len))
# define bzero(dest,len)       (void) (memset(dest, (char)NULL, len))
# define bcmp(b1,b2,n)         memcmp(b1,b2,n)
#endif

#include <popper.h>

/* CNS Kerberos IV */
#ifdef KERBEROS
AUTH_DAT kdata;
#endif

extern int      errno;
/*
#ifdef POPSCO
extern struct state	_res;
#endif
*/

#ifndef HAVE_STRDUP
#include <stddef.h>
#include <stdlib.h>

char *
strdup(str)
        char *str;
{
    int len;
    char *copy;

    len = strlen(str) + 1;
    if (!(copy = malloc((u_int)len)))
	return((char *)NULL);
    bcopy(str, copy, len);
    return(copy);
}
#endif

authenticate(p, addr)
     POP     *p;
     struct sockaddr_in *addr;
{

#ifdef KERBEROS
    Key_schedule schedule;
    KTEXT_ST ticket;
    char instance[INST_SZ];  
    char version[9];
    int auth;
  
    if (p->kerberos) {
	strcpy(instance, "*");
	auth = krb_recvauth(0L, 0, &ticket, KERBEROS_SERVICE, instance,
			    addr, (struct sockaddr_in *) NULL,
			    &kdata, "", schedule, version);
	
	if (auth != KSUCCESS) {
	    pop_msg(p, POP_FAILURE, "Kerberos authentication failure: %s", 
		    krb_err_txt[auth]);
	    pop_log(p, LOG_WARNING, "%s: (%s.%s@%s) %s", p->client, 
		    kdata.pname, kdata.pinst, kdata.prealm, krb_err_txt[auth]);

	    return(POP_FAILURE);
	}

# ifdef DEBUG
	if (p->debug)
	    pop_log(p, POP_DEBUG, "%s.%s@%s (%s): ok", kdata.pname, 
		kdata.pinst, kdata.prealm, inet_ntoa(addr->sin_addr));
# endif /* DEBUG */

	strncpy(p->user, kdata.pname, sizeof(p->user));

    }
#endif /* KERBEROS */

    return(POP_SUCCESS);
}

/* 
 *  init:   Start a Post Office Protocol session
 */

pop_init(p, argcount, argmessage)
POP     *       p;
int             argcount;
char    **      argmessage;
{

    struct sockaddr_in      cs;                 /*  Communication parameters */
    struct hostent      *   ch;                 /*  Client host information */
    int                     errflag = 0;
    int                     c;
    int                     len;
    extern char         *   optarg;
    int                     options = 0;
    int                     sp = 0;             /*  Socket pointer */
    char                *   trace_file_name;
    struct hostent	*   hp = NULL;

    /*  Initialize the POP parameter block */
    bzero ((char *)p,(int)sizeof(POP));

    /*  Initialize maildrop status variables in the POP parameter block */
    p->msgs_deleted = 0;
    p->last_msg = 0;
    p->bytes_deleted = 0;
    p->drop_size = 0;
    p->mmdf_separator = NULL;
    p->bulldir = BULLDIR;
    p->dirty = 0;
    p->xmitting = 0;
    p->kerberos = 0;
#ifdef SERVER_MODE
    p->server_mode = 1;
#else
    p->server_mode = 0;
#endif

    /*  Save my name in a global variable */
    p->myname = argmessage[0];

    /*  Get the name of our host */
    if ((p->myhost = (char *)malloc(MAXHOSTNAMELEN+1)) == NULL) {
	perror("malloc");
	exit(1);	
    }
    (void)gethostname(p->myhost,MAXHOSTNAMELEN);
    if (hp = gethostbyname(p->myhost)) {
	if (! index(hp->h_name, '.')) {		/* FQN not returned */
	    /*
	     * SVR4 resolver is stupid and returns h_name as whatever
	     * you gave gethostbyname.  Thus do a reverse lookup
	     * on the first address and hope for the best.
	     */
	    u_long x = *(u_long *)hp->h_addr_list[0];
	    if ((hp = gethostbyaddr((char *)&x, 4, AF_INET)) != NULL) {
	        (void) strncpy (p->myhost, hp->h_name, MAXHOSTNAMELEN);
	        p->myhost[MAXHOSTNAMELEN] = '\0';
	    }
	}
	else {
	    (void) strncpy (p->myhost, hp->h_name, MAXHOSTNAMELEN);
	    p->myhost[MAXHOSTNAMELEN] = '\0';
	}
    }


    /*  Open the log file */
#ifdef SYSLOG42
    (void)openlog(p->myname,0);
#else
    (void)openlog(p->myname,POP_LOGOPTS,POP_FACILITY);
#endif

    /*  Process command line arguments */
    while ((c = getopt(argcount,argmessage,"dkst:T:b:")) != EOF)
        switch (c) {

            /*  Debugging requested */
            case 'd':
                p->debug++;
                options |= SO_DEBUG;
#ifdef DEBUG
		if (p->debug)
		    pop_log(p,POP_PRIORITY,"Debugging turned on");
#endif
                break;

#ifdef KERBEROS
	    case 'k':
		p->kerberos++;
	    break;
#endif	/* KERBEROS */

            /* Stats requested */
            case 's':
                p->stats++;
                break;

	    /* Bulletins requested */
            case 'b':
                p->bulldir = optarg;
                break;

            /*  Debugging trace file specified */
            case 't':
                p->debug++;

#ifdef DEBUG
		if (p->trace)
		    pop_log(p,POP_PRIORITY,
		    "Tracing session and debugging information in file \"%s\"",
			    trace_file_name);
#endif

                if ((p->trace = fopen(optarg,"a+")) == NULL) {
                    pop_log(p,POP_PRIORITY,
                        "Unable to open trace file \"%s\", err = %d",
                            optarg,errno);
                    exit(1);
		}
                trace_file_name = optarg;
                break;

	    /*  Timeout value passed.  Default changed */
	    case 'T':
		pop_timeout = atoi(optarg);
		break;

            /*  Unknown option received */
            default:
                errflag++;
        }

    /*  Exit if bad options specified */
    if (errflag) {
#ifdef KERBEROS
        (void)fprintf(stderr,"Usage: %s [-d] [-k] [-s] [-t trace-file] [-T timeout] [-b bulldir]\n",argmessage[0]);
#else
        (void)fprintf(stderr,"Usage: %s [-d] [-s] [-t trace-file] [-T timeout] [-b bulldir]\n",argmessage[0]);
#endif
        exit(1);
    }

    /*  Get the address and socket of the client to whom I am speaking */
    len = sizeof(cs);
    if (getpeername(sp,(struct sockaddr *)&cs,&len) < 0){
        pop_log(p,POP_PRIORITY,
            "Unable to obtain socket and address of client, err = %d",errno);
        exit(1);
    }

    /*  Save the dotted decimal form of the client's IP address 
        in the POP parameter block */
    p->ipaddr = (char *)strdup(inet_ntoa(cs.sin_addr));

    /*  Save the client's port */
    p->ipport = ntohs(cs.sin_port);

    /*  Get the canonical name of the host to whom I am speaking */
    ch = gethostbyaddr((char *) &cs.sin_addr, sizeof(cs.sin_addr), AF_INET);
    if (ch == NULL){
        pop_log(p,POP_PRIORITY,
            "(v%s) Unable to get canonical name of client, err = %d",
	    VERSION, errno);
        p->client = p->ipaddr;
    }
    /*  Save the cannonical name of the client host in 
        the POP parameter block */
    else {

#ifndef BIND43
        p->client = (char *)strdup(ch->h_name);
#else

# ifndef SCOR5
#       include <arpa/nameser.h>
#       include <resolv.h>
# endif

        /*  Distrust distant nameservers */

#if !(defined(BSD) && (BSD >= 199103)) && !defined(OSF1) && !defined(HPUX10)
# if (!defined(__RES)) || (__RES < 19940415)
#  ifdef SCOR5
	extern struct __res_state	_res;
#  else
        extern struct state     _res;
#  endif
# endif
#endif
        struct hostent      *   ch_again;
        char            *   *   addrp;
	char			h_name[MAXHOSTNAMELEN + 1];

        /*  We already have a fully-qualified name */
#ifdef RES_DEFNAMES
        _res.options &= ~RES_DEFNAMES;
#endif

	strncpy(h_name, ch->h_name, sizeof(h_name));

        /*  See if the name obtained for the client's IP 
            address returns an address */
        if ((ch_again = gethostbyname(h_name)) == NULL) {
            pop_log(p,POP_PRIORITY,
                "Client at \"%s\" resolves to an unknown host name \"%s\"",
                    p->ipaddr, h_name);
            p->client = p->ipaddr;
        }
        else {
            /*  Save the host name (the previous value was 
                destroyed by gethostbyname) */
            p->client = (char *)strdup(ch_again->h_name);

            /*  Look for the client's IP address in the list returned 
                for its name */
            for (addrp=ch_again->h_addr_list; *addrp; ++addrp)
                if (bcmp(*addrp,&(cs.sin_addr),sizeof(cs.sin_addr)) == 0) break;

            if (!*addrp) {
                pop_log (p,POP_PRIORITY,
                    "Client address \"%s\" not listed for its host name \"%s\"",
                        p->ipaddr,h_name);
                p->client = p->ipaddr;
            }
        }

#ifdef RES_DEFNAMES
	/* 
	 *  Must restore nameserver options since code in crypt uses
	 *  gethostbyname call without fully qualified domain name!
	 */
	_res.options |= RES_DEFNAMES;
#endif

#endif /* BIND43 */
    }

    /*  Create input file stream for TCP/IP communication */
    if ((p->input = fdopen(sp,"r")) == NULL){
        pop_log(p,POP_PRIORITY,
            "Unable to open communication stream for input, err = %d",errno);
        exit (1);
    }

    /*  Create output file stream for TCP/IP communication */
    if ((p->output = fdopen(sp,"w")) == NULL){
        pop_log(p,POP_PRIORITY,
            "Unable to open communication stream for output, err = %d",errno);
        exit (1);
    }

#ifdef DEBUG
    if (p->debug)
	pop_log(p,POP_PRIORITY,
	    "(v%s) Servicing request from \"%s\" at %s",
		VERSION,p->client,p->ipaddr);
#endif

    return(authenticate(p, &cs));
}

