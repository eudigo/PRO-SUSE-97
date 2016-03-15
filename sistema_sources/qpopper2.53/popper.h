/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

/*  LINTLIBRARY */

/* 
 *  Header file for the POP programs
 */
#define BINMAIL_IS_SETGID 1
#ifdef POPSCO
# include <sys/syslog.h>
#else
# include <syslog.h>
#endif
#include <sys/socket.h>
#include "version.h"

#ifdef ISC
# include <sys/types.h>
# include <sys/fcntl.h>
# include <net/errno.h>
#endif

#ifdef		HAVE_SYS_PARAM_H
#include <sys/param.h>
# if (defined(BSD) && (BSD >= 199103))
#  define	HAVE_UNISTD_H
#  define	HAVE_VPRINTF
#  define	BIND43
# endif
#endif

#ifdef BSDI
#define BSD44_DBM
#endif

#ifdef BULLDB
# undef	DBM		/* used by mts.c and ndbm.h */
# ifdef GDBM
#  include <gdbm.h>
# else
#  include <ndbm.h>
# endif
#endif

#define NULLCP          ((char *) 0)
#define SPACE           32
#define TAB             9
#define TRUE            1
#define FALSE           0
#define NEWLINE         '\n'

#define MAXUSERNAMELEN  65
#define MAXDROPLEN      64
#define MAXLINELEN      1024
#define MAXMSGLINELEN   MAXLINELEN
#define MAXCMDLEN       4
#define MAXPARMCOUNT    5
#define MAXPARMLEN      10
#define ALLOC_MSGS  20

#ifndef OSF1
# define POP_FACILITY    LOG_LOCAL0
#else
# define POP_FACILITY    LOG_MAIL
#endif
#define POP_PRIORITY    LOG_NOTICE
#define POP_DEBUG       LOG_DEBUG
#define POP_LOGOPTS     LOG_PID

#ifdef POPSCO
# define L_SET  0
# define L_XTND 2
#endif

			/* ^A */
#define	MMDF_SEP_CHAR	'\001'

/* For performance reasons large sites should change the pop tempdrop
 * location to another directory other than the mail spool location.
 * The more files referenced in a directory, the slower the access is
 */

/*
 * The default string for HOMEDIRMAIL is "%s/.mail".  You need to change
 * the value in pop_dropcopy.h in routine genpath() if you wish to modify
 * the mail drop location.
 */

/* Define his path to default the bulletin dir.
 *	-DBULLDIR= "/usr/mail/bulletins"
 * in the make file works well or you can define BULLDIR before the #ifdef.
 */

#ifndef BULLDIR
#define BULLDIR		NULL
#endif

			/* Defines the minimum number of bulletins a new user
			   receives */
#define NEWBULLCNT	1

#ifndef CONTENT_LENGTH
#define CONTENT_LENGTH 0
#endif

/* POP_TMPSIZE needs to be big enough to hold the string
 * defined by POP_TMPDROP.  POP_DROP and POP_TMPDROP
 * must be in the same filesystem.
 */
#define POP_TMPSIZE	256
#define POP_OK          "+OK"
#define POP_ERR         "-ERR"
#define POP_SUCCESS     1
#define POP_FAILURE     0
#define POP_TERMINATE   '.'
#define POP_TIMEOUT	120	/* timeout connection after this many secs */

		    /* UID's <= this value are not allowed to access email */
#define BLOCK_UID	10

#define DIG_SIZE	16

		/* Set these to the types your OS returns if they are not
		   already typedefed for you */
#define OFF_T		off_t
#define PID_T		pid_t
#define UID_T		uid_t
#define GID_T		gid_t
#define	TIME_T		time_t
#define SIZE_T		size_t

#ifdef NeXT
#undef PID_T
#define PID_T		int
#endif

#ifdef CHECK_SHELL
#define WILDCARD_SHELL	"/POPPER/ANY/SHELL/"
#endif

extern int              errno;

#if !(defined(BSD) && (BSD >= 199306)) && !defined(__USE_BSD)
extern int              sys_nerr;
extern char         *   sys_errlist[];
#ifndef __linux__
extern char         *   sys_siglist[];
#endif
#endif

extern int		pop_timeout;

extern int		hangup;

#define pop_command         pop_parm[0]     /*  POP command is first token */
#define pop_subcommand      pop_parm[1]     /*  POP XTND subcommand is the 
                                                second token */

typedef enum {                              /*  POP processing states */
    auth1,                                  /*  Authorization: waiting for 
                                                USER command */
    auth2,                                  /*  Authorization: waiting for 
                                                PASS command */
    trans,                                  /*  Transaction */
    update,                                 /*  Update:  session ended, 
                                                process maildrop changes */
    halt,                                   /*  (Halt):  stop processing 
                                                and exit */
    error                                   /*  (Error): something really 
                                                bad happened */
} state;

typedef struct {                                /*  State information for 
                                                    each POP command */
    state       ValidCurrentState;              /*  The operating state of 
                                                    the command */
    char   *    command;                        /*  The POP command */
    int         min_parms;                      /*  Minimum number of parms 
                                                    for the command */
    int         max_parms;                      /*  Maximum number of parms 
                                                    for the command */
    int         (*function) ();                 /*  The function that process 
                                                    the command */
    state       result[2];                      /*  The resulting state after 
                                                    command processing */
#define success_state   result[0]               /*  State when a command 
                                                    succeeds */
} state_table;

typedef struct {                                /*  Table of extensions */
    char   *    subcommand;                     /*  The POP XTND subcommand */
    int         min_parms;                      /*  Minimum number of parms for
                                                    the subcommand */
    int         max_parms;                      /*  Maximum number of parms for
                                                    the subcommand */
    int         (*function) ();                 /*  The function that processes 
                                                    the subcommand */
} xtnd_table;

typedef struct {                                /*  Message information */
    int         number;                         /*  Message number relative to 
                                                    the beginning of list */
    long        length;                         /*  Length of message in 
                                                    bytes */
    int         lines;                          /*  Number of (null-terminated)
						    lines in the message */
    int         body_lines;			/*  Number of (null-terminated)
   						    lines in the body */
    long        offset;                         /*  Offset from beginning of 
                                                    file */
    int         del_flag;                       /*  Flag indicating if message 
                                                    is marked for deletion */
    int         retr_flag;                      /*  Flag indicating if message 
                                                    was retrieved */
    int		orig_retr_state;		/*  What was the state at the
						    start of this session.
						    Used for RSET cmd. */
    char	*uidl_str;			/*  Cache of the UIDL str for
						    faster access */
} MsgInfoList;

typedef struct  {                               /*  POP parameter block */
    int                 debug;                  /*  Debugging requested */
    int                 xmitting;               /*  =1 xtnd xmit started */
    int                 stats;                  /*  Stats requested */
    int			dirty;			/*  Any mailbox changes? */
    int			kerberos;		/*  Flag to enable kerberos
						    authentication */
    int			server_mode;		/*  Default at compile time */
    char            *   myname;                 /*  The name of this POP 
                                                    daemon program */
    char            *   myhost;                 /*  The name of our host 
                                                    computer */
    char            *   client;                 /*  Canonical name of client 
                                                    computer */
    char            *   ipaddr;                 /*  Dotted-notation format of 
                                                    client IP address */
    unsigned short      ipport;                 /*  Client port for privileged 
                                                    operations */
    char                user[MAXUSERNAMELEN];   /*  Name of the POP user */
    state               CurrentState;           /*  The current POP operational                                                     state */
    MsgInfoList     *   mlp;                    /*  Message information list */
    int                 msg_count;              /*  Number of messages in 
                                                    the maildrop */
    int                 msgs_deleted;           /*  Number of messages flagged 
                                                    for deletion */
    int                 last_msg;               /*  Last message touched by 
                                                    the user */
    long                bytes_deleted;          /*  Number of maildrop bytes 
                                                    flagged for deletion */
    char                drop_name[MAXDROPLEN];  /*  The name of the user's 
                                                    maildrop */
    char                temp_drop[MAXDROPLEN];  /*  The name of the user's 
                                                    temporary maildrop */
    long                drop_size;              /*  Size of the maildrop in
                                                    bytes */
    long		spool_end;		/*  Offset of the end of the
						    mailspool */
    FILE            *   drop;                   /*  (Temporary) mail drop */
    FILE            *   input;                  /*  Input TCP/IP communication 
                                                    stream */
    FILE            *   output;                 /*  Output TCP/IP communication                                                     stream */
    FILE            *   trace;                  /*  Debugging trace file */
    FILE            *   hold;                   /*  In SERVER_MODE, this value
						    holds the drop FILE */
    char            *   pop_parm[MAXPARMCOUNT]; /*  Parse POP parameter list */
    int                 parm_count;             /*  Number of parameters in 
                                                    parsed list */
    char            *   bulldir;                /*  Bulletin directory */
#ifdef BULLDB
#ifdef GDBM
	GDBM_FILE 		bull_db;
#else
    DBM		    *	bull_db;		/*  Central Bulletin Database */
#endif
#endif
    char	    *	mmdf_separator;		/*  string between messages */
    char		md5str[BUFSIZ];		/*  String used with the shared
						    secret to create the md5
						    digest */
} POP;

#ifdef KERBEROS

#ifndef KERBEROS_SERVICE
# define KERBEROS_SERVICE	"rcmd"
#endif

#ifdef SOLARIS2
# include <kerberos/krb.h>
#else
# ifdef BSDI
#  include <kerberosIV/des.h>
#  include <kerberosIV/krb.h>
# else
#  include <krb.h>
# endif
#endif
extern AUTH_DAT kdata;
#endif /* KERBEROS */

#if defined(AUTHFILE)
extern int  checkauthfile();
#endif
#if defined(NONAUTHFILE)
extern int  checknonauthfile();
#endif

extern int  pop_dele();
extern int  pop_last();
extern int  pop_list();
extern int  pop_pass();
extern int  pop_rpop();
extern int  pop_apop();
extern int  pop_auth();
extern int  pop_quit();
extern int  pop_rset();
extern int  pop_send();
extern int  pop_stat();
extern int  pop_updt();
extern int  pop_user();
extern int  pop_xtnd();
extern int  pop_xmit();
extern int  pop_xmit_recv();
extern int  pop_xmit_exec();
extern int  pop_xlst();
extern int  pop_uidl();
extern int  pop_euidl();
extern int  isfromline();
extern int  genpath();

extern char *pwerrmsg;

#define pop_auth_fail	pop_msg
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif
/*
extern int  pop_auth_fail();
*/

#define POPUSERSORACLE /* Arquivo de Usuarios em Servidor Oracle */

#define LINUXSUSE      /* Ambiente: SuSE Linux 6.0 */

#ifdef POPUSERSORACLE

#include <oci.h>

extern int orauser_open(POP *p);

extern void orauser_close(POP *p);

extern int orauser_find(POP *p);

#endif /* POPUSERSORACLE */
