/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

#ifdef POPSCO
# include <sys/security.h>
# include <sys/audit.h>
# include <prot.h>
# define VOIDSTAR
# ifdef SCOR5
#  define VOIDSTAR	(void (*)(int))
# endif
#else
# ifdef AIX
#  define VOIDSTAR	(void (*)(int))
# else
#  define VOIDSTAR	(void *)
# endif
#endif

#if defined(AUTH_SPECIAL)
# ifdef HAVE_SYS_SECURITY_H 
#  include <sys/security.h>
# endif
# ifdef HAVE_PROTO_H
#  include <prot.h>
# endif
#endif

#include <popper.h>

#ifndef HAVE_STRERROR
char * strerror();
#endif

extern  state_table *   pop_get_command();
int hangup = FALSE ;
int catchSIGHUP();
int poptimeout = 0;

int	pop_timeout = POP_TIMEOUT;

FILE *debuglog;

/* 
 *  popper: Handle a Post Office Protocol version 3 session
 */
main (argc, argv)
int         argc;
char    **  argv;
{
    POP                 p;
    state_table     *   s;
    char                message[MAXLINELEN];
    char            *   tgets();

#if defined(AUTH_SPECIAL) && defined(HAVE_SET_AUTH_PARAMETERS)
    (void) set_auth_parameters(argc, argv);
#endif

#ifdef AUX
    (void)set42sig();
#endif

    /* Set umask for better security */
#ifdef BINMAIL_IS_SETGID
    umask(0007);	/* Trust the mail delivery group */
#else
    umask(0077);	/* Trust no-one */
#endif

    (void) signal(SIGHUP,VOIDSTAR catchSIGHUP);
    (void) signal(SIGPIPE,VOIDSTAR catchSIGHUP);

    /*  Start things rolling */
    if (pop_init(&p,argc,argv) != POP_SUCCESS)
	exit(1);

    /*  Tell the user that we are listenting */
#ifdef APOP
    sprintf(p.md5str, "<%d.%d@%s>", getpid(), time((TIME_T *)0), p.myhost);
#else
    p.md5str[0] = '\0';
#endif

    pop_msg(&p, POP_SUCCESS, "QPOP (version %s) at %s starting.  %s",
						VERSION, p.myhost, p.md5str);

    /*  State loop.  The POP server is always in a particular state in 
        which a specific suite of commands can be executed.  The following 
        loop reads a line from the client, gets the command, and processes 
        it in the current context (if allowed) or rejects it.  This continues 
        until the client quits or an error occurs. */

    for (p.CurrentState=auth1;p.CurrentState!=halt&&p.CurrentState!=error;) {
#ifdef SETPROCTITLE
	setproctitle("%s@%s [%s]: cmd read", p.user, p.client, p.ipaddr);
#endif
        if (hangup) {
            pop_msg(&p,POP_FAILURE,"POP hangup",p.myhost);
#ifndef NOUPDATEONABORT
            if ((p.CurrentState != auth1) && (p.CurrentState != auth2) &&
								!pop_updt(&p))
                pop_msg(&p,POP_FAILURE,"POP mailbox update failed.",p.myhost);
#else
	    if ((p.CurrentState != auth1) && (p.CurrentState != auth2) &&
		!pop_restore(&p))
	      pop_msg(&p, POP_FAILURE,"POP mailbox restoration failed.",p.myhost);
#endif
            p.CurrentState = error;
        } else if (tgets(message,MAXLINELEN,p.input,pop_timeout) == NULL) {
	    if (poptimeout) {
                if (p.xmitting) pop_xmit_clean(&p);
		pop_msg(&p,POP_FAILURE,"POP timeout",p.myhost); 
            }
            
	    else
		pop_msg(&p,POP_FAILURE,"POP EOF received",p.myhost);
#ifndef NOUPDATEONABORT
            if ((p.CurrentState != auth1) && (p.CurrentState != auth2) &&
								!pop_updt(&p))
                pop_msg(&p,POP_FAILURE,"POP mailbox update failed!",p.myhost);
#else
            if ((p.CurrentState != auth1) && (p.CurrentState != auth2) &&
                !pop_restore(&p))
                pop_msg(&p,POP_FAILURE,"POP mailbox restoration failed!",p.myhost);
#endif
            p.CurrentState = error;
        } else if (p.xmitting) { /* After the xtnd xmit initiates the mail txfer */
            pop_xmit_recv(&p,message);
        } else if ((s = pop_get_command(&p,message)) != NULL) {
            if (s->function != NULL) {
#ifdef SETPROCTITLE
		int i;
		char command[10];

		for (i = 0; s->command[i]; i++)
		    command[i] = toupper(s->command[i]);
		command[i] = 0;
		setproctitle("%s@%s [%s]: %s",
				    p.user, p.client, p.ipaddr, command);
#endif
                p.CurrentState = s->result[(*s->function)(&p)];
            } else {
                p.CurrentState = s->success_state;
                pop_msg(&p,POP_SUCCESS,NULL);
            }
        }
    }

    /*  Say goodbye to the client */
    pop_msg(&p,POP_SUCCESS,"Pop server at %s signing off.",p.myhost);

#ifdef DEBUG
    /*  Log the end of activity */
    if (p.debug)
	pop_log(&p,POP_PRIORITY,
	    "(v%s) Ending request from \"%s\" at (%s) %s",
					VERSION,p.user,p.client,p.ipaddr);
#endif

    /*  Stop logging */
    closelog();

    return(0);
}

jmp_buf env;

/*
 *  There seems to be a problem with the AIX fgets.  This is suppose to
 *  fix it.
 */
char *
myfgets(str, size, fp)
char *str;
int size;
FILE *fp;
{
	char *cp;
	char ch;
	int nbytes;
	int found_nl = 0;

	cp = str;

	while (--size > 0) {
	    if ((nbytes = read(fileno(fp), cp, 1)) <= 0)
		break;

	    if (*cp == '\n') {
		*++cp = '\0';
		found_nl++;
		break;
	    }
	    ++cp;
	}

	if ((nbytes <= 0) || (cp == str)) {
	    return(NULL);
	} else {
	    if (!found_nl)
		while ((read(fileno(fp), &ch, 1) == 1) && ch != '\n');
	    return(str);
	}
}

/*
 * fgets, but with a timeout
 */
char *tgets(str,size,fp,timeout)
char *str;
int size;
FILE *fp;
int timeout;
{
  int ring();
  (void) signal(SIGALRM, VOIDSTAR ring);
  alarm(timeout);
  if (setjmp(env))
    str = NULL;
  else
    str = myfgets(str,size,fp);
/*    str = fgets(str,size,fp); */
  alarm(0);
  signal(SIGALRM,SIG_DFL);
  return(str);
}

int ring()
{
  poptimeout = 1;
  longjmp(env,1);
}
  

#ifdef STRNCASECMP
/*
 *  Perform a case-insensitive string comparision
 */
strncasecmp(str1,str2,len)
register char   *   str1;
register char   *   str2;
register int        len;
{
    register int    i;
    char            a,
                    b;

    for (i=len-1;i>=0;i--){
        a = str1[i];
        b = str2[i];
        if (isupper(a)) a = tolower(str1[i]);
        if (isupper(b)) b = tolower(str2[i]);
        if (a > b) return (1);
        if (a < b) return(-1);
    }
    return(0);
}
#endif

int catchSIGHUP()
{
    extern int hangup ;

    hangup = TRUE ;

    /* This should not be a problem on BSD systems */
    signal(SIGHUP,  VOIDSTAR catchSIGHUP);
    signal(SIGPIPE,  VOIDSTAR catchSIGHUP);
}

#ifndef HAVE_STRERROR
char *
strerror(e)
	int e;
{
	extern char *sys_errlist[];
	extern int sys_nerr;

	if(e < sys_nerr)
		return(sys_errlist[e]);
	else
		return("unknown error");
}
#endif

#ifdef POPSCO
/*
 * Ftruncate() for non-BSD systems.
 *
 * This module gives the basic functionality for ftruncate() which
 * truncates the given file descriptor to the given length.
 * ftruncate() is a Berkeley system call, but does not exist in any
 * form on many other versions of UNIX such as SysV. Note that Xenix
 * has chsize() which changes the size of a given file descriptor,
 * so that is used if M_XENIX is defined.
 *
 * Since there is not a known way to support this under generic SysV,
 * there is no code generated for those systems.
 *
 * SPECIAL NOTE: On Xenix, using this call in the BSD library
 * will REQUIRE the use of -lx for the extended library since chsize()
 * is not in the standard C library.
 *
 * By Marc Frajola, 3/27/87
 */

#include <fcntl.h>

ftruncate(fd,length)
    int fd;			/* File descriptor to truncate */
    off_t length;		/* Length to truncate file to */
{
    int status;			/* Status returned from truncate proc */

    status = chsize(fd,length);
/*
    status = -1;
    NON-XENIX SYSTEMS CURRENTLY NOT SUPPORTED
*/

    return(status);
}
#endif

#ifndef HAVE_FTRUNCATE
/* ftruncate emulations that work on some System V's.
   This file is in the public domain.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <fcntl.h>

#ifdef F_CHSIZE

int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  return fcntl (fd, F_CHSIZE, length);
}

#else /* not F_CHSIZE */
#ifdef F_FREESP

/* By William Kucharski <kucharsk@netcom.com>.  */

#include <sys/stat.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  struct flock fl;
  struct stat filebuf;

  if (fstat (fd, &filebuf) < 0)
    return -1;

  if (filebuf.st_size < length)
    {
      /* Extend file length. */
      if (lseek (fd, (length - 1), SEEK_SET) < 0)
	return -1;

      /* Write a "0" byte. */
      if (write (fd, "", 1) != 1)
	return -1;
    }
  else
    {

      /* Truncate length. */

      fl.l_whence = 0;
      fl.l_len = 0;
      fl.l_start = length;
      fl.l_type = F_WRLCK;	/* write lock on file space */

      /* This relies on the *undocumented* F_FREESP argument to fcntl,
	 which truncates the file so that it ends at the position
	 indicated by fl.l_start.  Will minor miracles never cease?  */

      if (fcntl (fd, F_FREESP, &fl) < 0)
	return -1;
    }

  return 0;
}

#else /* not F_CHSIZE nor F_FREESP */
#ifdef HAVE_CHSIZE

int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  return chsize (fd, length);
}

#else /* not F_CHSIZE nor F_FREESP nor HAVE_CHSIZE */

#include <errno.h>
#ifndef errno
extern int errno;
#endif

int
ftruncate (fd, length)
     int fd;
     off_t length;
{
  errno = EIO;
  return -1;
}

#endif /* not HAVE_CHSIZE */
#endif /* not F_FREESP */
#endif /* not F_CHSIZE */
#endif /*! HAVE_FTRUNCATE */

