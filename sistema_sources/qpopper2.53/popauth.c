/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

/*  */
/* popauth.c - manipulate POP authorization DB */
#include <config.h>
#ifdef GDBM
# include <gdbm.h>
#else
# if HAVE_NDBM_H
#  include <ndbm.h>
# else
#  if HAVE_DBM_H
#   include <dbm.h>
#  endif
# endif
#endif
#include <pwd.h>
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifndef HAVE_BCOPY
#define bcopy(src,dest,len)	(void) (memcpy(dest,src,len))
#define bzero(dest,len)  	(void) (memset(dest, (char)NULL, len))
#define bcmp(b1,b2,n)		memcmp(b1,b2,n)
#endif
#ifndef HAVE_INDEX
# define index(s,c)		strchr(s,c)
# define rindex(s,c)		strrchr(s,c)
#endif
#include <flock.h>
#if HAVE_STRINGS_H
#include <strings.h>
#endif
#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef BSDI
#define BSD44_DBM
#endif

#ifndef HAVE_STRERROR
char *strerror();
#endif

#define	UID_T	uid_t

static struct swit {
	char *name;
} switches[] = {
#define	INITSW	0
    "init", 
#define	LISTSW	1
    "list", 
#define	USERSW	2
    "user", 
#define	DELESW	3
    "delete", 

    NULL,
};

static char   *program;

#ifdef __STDC__
static void
adios(const char *fmt, ...)
#else
static void
adios(va_alist)
va_dcl
#endif
{
	va_list ap;
#ifndef __STDC__
	char *fmt;
#endif

	(void) fprintf(stderr, "%s: ", program);
#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	(void) vfprintf(stderr, fmt, ap);
	(void) fprintf(stderr, "\n");
	va_end(fmt);
	exit(1);
	/*NOTREACHED*/
}

#ifndef HAVE_STRDUP
#include <stddef.h>

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

/*
 * Obscure password so a cleartext search doesn't come up with
 * something interesting.
 *
 */

char *
obscure(string)
char *string;
{
	unsigned char *cp, *newstr;

	cp = newstr = (unsigned char *)strdup(string);

	while (*cp) {
	    *cp++ ^= 0xff;
	}

	return((char *)newstr);
}

/* Use GNU_PASS for longer passwords on systems that support termios */

#ifndef GNU_PASS
char *getpass();
#else

/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* It is desireable to use this bit on systems that have it.
   The only bit of terminal state we want to twiddle is echoing, which is
   done in software; there is no need to change the state of the terminal
   hardware.  */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#ifndef TCSASOFT
#define TCSASOFT 0
#endif

#ifdef SSIZET
typedef SSIZET ssize_t;
#endif

char *
getpass (prompt)
#if defined(HPUX)
char *prompt;
#else
const char *prompt;
#endif
{
  FILE *in, *out;
  struct termios t;
  int echo_off;
  static char *buf = NULL;
  static size_t bufsize = 0;
  ssize_t nread;

  /* Try to write to and read from the terminal if we can.
     If we can't open the terminal, use stderr and stdin.  */

  in = fopen ("/dev/tty", "w+");
  if (in == NULL)
    {
      in = stdin;
      out = stderr;
    }
  else
    out = in;

  /* Turn echoing off if it is on now.  */

  if (tcgetattr (fileno (in), &t) == 0)
    {
      if (t.c_lflag & ECHO)
	{
	  t.c_lflag &= ~ECHO;
	  echo_off = tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t) == 0;
	  t.c_lflag |= ECHO;
	}
      else
	echo_off = 0;
    }
  else
    echo_off = 0;

  /* Write the prompt.  */
  fputs (prompt, out);
  fflush (out);

  /* Read the password.  */
#ifdef NO_GETLINE
  bufsize = 256;
  buf = (char *)malloc(256);
  nread = (fgets(buf, (size_t)bufsize, in) == NULL) ? 1 : strlen(buf);
  rewind(in);
  fputc('\n', out);
#else
  nread = __getline (&buf, &bufsize, in);
#endif
  if (nread < 0 && buf != NULL)
    buf[0] = '\0';
  else if (buf[nread - 1] == '\n')
    /* Remove the newline.  */
    buf[nread - 1] = '\0';

  /* Restore echoing.  */
  if (echo_off)
    (void) tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t);

  if (in != stdin)
    /* We opened the terminal; now close it.  */
    fclose (in);

  return buf;
}
#endif

/* ARGSUSED */

main (argc, argv)
int	argc;
char   *argv[];
{
    UID_T   myuid;
    int	    flags,
	    i,
	    delesw = 0,
	    initsw = 0,
	    insist,
	    listsw = 0,
	    popuser = 0;
    long    clock;
    char   *bp,
	   *cp,
	   *usersw = NULL,
	    buf[100],
	    obuf[100];
    struct  passwd *pw;
    datum   key,
	    value;
#ifdef GDBM
	GDBM_FILE db;
	char 	apop_file[BUFSIZ];
#else 
    DBM    *db;
    char    apop_dir[BUFSIZ];
#ifndef BSD44_DBM
    char    apop_pag[BUFSIZ];
#endif
#endif
    int     f;

    program = argv[0];
    argv++;
    argc--;

    while (argc > 0) {
	cp = argv[0];
	if (*cp == '-') {
	    int i, v;

	    i = 0;
	    v = -1;
	    for(i = 0; switches[i].name; i++) {
		if(strcmp(&cp[1], switches[i].name) == 0) {
			v = i;
			break;
		}
	    }
	    cp++;
	    switch (v) {
		default:
		    adios ("-%s unknown option", cp);
		case INITSW:
		    initsw = 1, listsw = 0; delesw = 0;
		    break;
		case LISTSW:
		    listsw = 1, initsw = 0; delesw = 0;
		    break;
		case DELESW:
		    delesw = 1, initsw = 0; listsw = 0;
		    if (argc < 2 || argv[1][0] == '-')
			adios ("missing argument to %s", argv[0]);
		    usersw = argv[1];
		    argc--;
		    argv++;
		    break;
		case USERSW:
		    if (argc < 2 || argv[1][0] == '-')
			adios ("missing argument to %s", argv[0]);
		    usersw = argv[1];
		    argc--;
		    argv++;
		    if (delesw)
			fprintf(stderr, "Warning: user '%s' will now be deleted\n", usersw);
		    break;
	    }
	}
	else
	    adios ("usage: %s [[-init]|[-list]|[-user name]|[-delete name]]", program);
	argc--;
	argv++;
    }


#ifndef	APOP
    adios ("not compiled with APOP option");
#else

    myuid = getuid();

    if ((pw = getpwnam (POPUID)) == NULL)
	adios ("\"%s\": user-id unknown", POPUID);

    if (pw->pw_uid == myuid)
	popuser = 1;

    if (myuid && !popuser && (delesw || initsw || listsw || (usersw != NULL)))
	adios("Only superuser or user '%s' can perform the requested function",
	    POPUID);

    if (myuid && initsw)
	adios("Only superuser can init the database");
#ifdef GDBM
	(void) strncpy(apop_file, APOP, sizeof(apop_file) - 1);
	apop_file[sizeof(apop_file)-1] = '\0';
#else
    (void) strncpy(apop_dir, APOP, sizeof(apop_dir) - 5);
#ifdef BSD44_DBM
    (void) strcat(apop_dir, ".db");
#else
    (void) strncpy(apop_pag, APOP, sizeof(apop_pag) - 5);
    (void) strcat(apop_pag, ".pag");
    (void) strcat(apop_dir, ".dir");
#endif
#endif

    if (delesw) {
	if (myuid && !popuser)
	    adios ("Only root or %s may delete entries", POPUID);

#ifdef GDBM
	if ((db = gdbm_open(apop_file,512,GDBM_WRITER,0,0)) == NULL)
#else
	if ((db = dbm_open (APOP, O_RDWR, 0)) == NULL)
#endif
	    adios ("%s: unable to open POP authorization DB", APOP);
	
	key.dsize = strlen (key.dptr = usersw) + 1;

#ifdef GDBM
	value = gdbm_fetch(db, key);
#else
	value = dbm_fetch(db, key);
#endif
	if (value.dptr == NULL)
	    adios("User '%s' not found in apop database", usersw);

#ifdef GDBM
	if (gdbm_delete(db,key) < 0)
#else
	if (dbm_delete(db, key) < 0)
#endif
	    adios("Unable to delete user '%s' from apop database", usersw);

#ifdef GDBM
	gdbm_close (db);
#else
	dbm_close (db);
#endif

	exit (0);
    }

    if (initsw) {
	struct stat st;

	setuid(myuid);

#ifdef GDBM
	if (stat (apop_file, &st) != -1) 
#else
	if (stat (apop_dir, &st) != -1) 
#endif
	{
	    char ibuf[30];
		
	    printf("Really initialize POP authorization DB? ");
	    if(fgets(ibuf, sizeof(ibuf), stdin) == NULL || ibuf[0] != 'y')
		exit (1);
#ifdef GDBM
		(void) unlink (apop_file);
#else
	    (void) unlink (apop_dir);
# ifndef BSD44_DBM
	    (void) unlink (apop_pag);
# endif
#endif
	}
#ifdef GDBM
	if ((db = gdbm_open (apop_file, 512, GDBM_WRCREAT, 0600, 0)) == NULL)
	    adios ("unable to create POP authorization DB: %s", 
	    	strerror(errno));
	gdbm_close (db);
	if (chown (apop_file, pw->pw_uid, pw->pw_gid) == -1)
	    adios ("error setting ownership of POP authorization for %s : %s", 
		apop_file, strerror(errno));
#else
	if ((db = dbm_open (APOP, O_RDWR | O_CREAT, 0600)) == NULL)
	    adios ("unable to create POP authorization DB: %s", 
	    	strerror(errno));
	dbm_close (db);
	if (chown (apop_dir, pw->pw_uid, pw->pw_gid) == -1 
#ifndef BSD44_DBM
	 || chown (apop_pag, pw->pw_uid, pw->pw_gid) == -1
#endif
	    )
	    adios ("error setting ownership of POP authorization DB: %s", 
		strerror(errno));
#endif

	exit (0);
    }

#ifdef GDBM
    if ((db = gdbm_open (apop_file, 512, GDBM_READER, 0, 0)) == NULL)
#else
    if ((db = dbm_open (APOP, O_RDONLY, 0)) == NULL)
#endif
		adios ("unable to open POP authorization DB: %s", strerror(errno));

#ifdef GDBM
    if ((f = open (apop_file, listsw ? O_RDONLY : O_RDWR)) == -1)
		adios ("%s: unable to open POP authorization DB", apop_file);
    if (flock (f, LOCK_SH) == -1)
		adios ("%s: unable to lock POP authorization DB", apop_file);
#else
    if ((f = open (apop_dir, listsw ? O_RDONLY : O_RDWR)) == -1)
		adios ("%s: unable to open POP authorization DB", apop_dir);
    if (flock (f, LOCK_SH) == -1)
		adios ("%s: unable to lock POP authorization DB", apop_dir);
#endif


    if (listsw) {
	if (usersw) {
	    key.dsize = strlen (key.dptr = usersw) + 1;
#ifdef GDBM
	    value = gdbm_fetch (db, key);
#else
	    value = dbm_fetch (db, key);
#endif
	    if (value.dptr == NULL)
		adios ("no such entry in POP authorization DB");
	    printf ("%s\n", key.dptr);
	}
	else
#ifdef GDBM
	    for (key = gdbm_firstkey (db); key.dptr; key = gdbm_nextkey (db,key)) 
#else
	    for (key = dbm_firstkey (db); key.dptr; key = dbm_nextkey (db)) 
#endif
		{
		printf ("%s", key.dptr);
#ifdef GDBM
		value = gdbm_fetch (db, key);
#else
		value = dbm_fetch (db, key);
#endif
		if (value.dptr == NULL)
		    printf (" - no information?!?\n");
		else {
		    printf ("\n");
		}
	    }

#ifdef GDBM
	gdbm_close (db);
#else
	dbm_close (db);
#endif

	exit (0);
    }

    if (usersw == NULL) {
	if ((pw = getpwuid(myuid)) == NULL)
	    adios("Sorry, don't know who uid %d is.", myuid);
	usersw = pw->pw_name;
    } else {
	if ((pw = getpwnam(usersw)) == NULL)
	    adios("Sorry, don't know who uid %s is.", usersw);
	usersw = pw->pw_name;
    }

    fprintf (stderr, "Changing POP password for %s.\n", usersw);

    key.dsize = strlen (key.dptr = usersw) + 1;
#ifdef GDBM
    value = gdbm_fetch (db, key);
#else
    value = dbm_fetch (db, key);
#endif
    if (myuid && !popuser && value.dptr != NULL) {
	if (((i = strlen(strncpy(obuf, getpass("Old password:"), sizeof(obuf)))) == 0) ||
		((value.dsize - 1) != i) ||
		(strncmp(obuf, value.dptr, i) &&
		     strncmp(obuf, obscure(value.dptr), i))) {
	    adios("Sorry, password entered incorrectly\n");
	}
    }
#ifdef GDBM
    gdbm_close (db);
#else
	dbm_close (db);
#endif

#ifdef	lint
    flags = 0;
#endif	/* lint */
    for (insist = 0; insist < 2; insist++) {
	int	i;
	char    c;

	if (insist)
	    printf ("Please use %s.\n",
		    flags == 1 ? "at least one non-numeric character"
		    : "a longer password");

	if (((i = strlen(strncpy(buf, getpass("New password:"), sizeof(buf)))) == 0) ||
	      !strncmp(buf, obuf, i)) {
	    fprintf (stderr, "Password unchanged.\n");
	    exit (1);
	}

	flags = 0;
	for (cp = buf; c = *cp++;)
	    if (c >= 'a' && c <= 'z')
		flags |= 2;
	    else
		if (c >= 'A' && c <= 'Z')
		    flags |= 4;
		else
		    if (c >= '0' && c <= '9')
			flags |= 1;
		    else
			flags |= 8;

	if ((flags >= 7 && i >= 4)
		|| ((flags == 2 || flags == 4) && i >= 6)
		|| ((flags == 3 || flags == 5 || flags == 6) && i >= 5))
	    break;
    }

    if (strcmp(buf, getpass("Retype new password:"))) {
	fprintf (stderr, "Mismatch - password unchanged.\n");
	exit (1);
    }

#ifdef GDBM
	if ((db = gdbm_open(apop_file, 512, GDBM_WRITER, 0, 0)) == NULL)
#else
    if ((db = dbm_open(APOP, O_RDWR, 0)) == NULL)
#endif
		adios("%s: unable to open POP authorization DB", APOP);

    if (flock(f, LOCK_EX) == -1)
	adios("%s: unable to lock POP authorization DB", 
#ifdef GDBM
	apop_file
#else
	apop_dir
#endif
	);

    key.dsize = strlen (key.dptr = usersw) + 1;

    value.dptr = obscure(buf);
    value.dsize = strlen(value.dptr) + 1;

#ifdef GDBM
    if (gdbm_store (db, key, value, GDBM_REPLACE))
	adios ("POP authorization DB may be corrupt?!?");
    gdbm_close (db);
#else
    if (dbm_store (db, key, value, DBM_REPLACE))
	adios ("POP authorization DB may be corrupt?!?");
    dbm_close (db);
#endif
#endif

    exit (0);
    /* NOTREACHED */
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
