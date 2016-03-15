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
#include <pwd.h>
#include<sys/stat.h>
#include <string.h>

# include <flock.h>
#ifndef HAVE_INDEX
# define index(s, c)	strchr(s, c)
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#ifdef GDBM
# include <gdbm.h>
#else
# if HAVE_NDBM_H
#  include <ndbm.h>
# endif
#endif

#include "popper.h"

/* 
 *  user:   Prompt for the user name at the start of a POP session
 */

int pop_user (p)
POP     *   p;
{
    /* If there is an APOP database entry then don't allow a cleartext
       password over the net */
# ifdef APOP
#ifdef GDBM
    char apop_file[BUFSIZ];
	GDBM_FILE db;
#else
    char apop_dir[BUFSIZ];
    DBM    *db;
#endif
    int	   fid;
    struct passwd *pw;
    struct stat st;
    datum	key, value;
# endif

#ifdef KERBEROS
    if (p->kerberos && strcmp(p->pop_parm[1], p->user)) {
	pop_log(p, LOG_WARNING, "%s: auth failed: %s.%s@@%s vs %s",
		p->client, kdata.pname, kdata.pinst, kdata.prealm, 
		p->pop_parm[1]);
        return(pop_msg(p,POP_FAILURE,
		       "Wrong username supplied (%s vs. %.128s).", p->user,
		       p->pop_parm[1]));
    }
#endif

    /*  Save the user name */
    (void)strncpy(p->user, p->pop_parm[1], sizeof(p->user));
    p->user[sizeof(p->user)-1] = 0;

# ifdef APOP_ONLY
	return(pop_auth_fail(p, POP_FAILURE,
	    "You must use APOP authentication to connect to this server"));
# endif

# ifdef APOP

	/* If this call fails then the database is not accessable (doesn't
	   exist?) in which case we can ignore an APOP user trying to
	   access the popper with a cleartext password.
         */

    if (((pw = getpwnam(p->user)) != NULL) &&
#ifdef GDBM
	((db = gdbm_open(APOP, 512, GDBM_READER, 0, 0)) != NULL))
#else
	((db = dbm_open(APOP, O_RDONLY, 0)) != NULL)) 
#endif
    {

#ifdef GDBM
        (void) strncpy(apop_file, APOP, sizeof(apop_file) - 1);
	apop_file[sizeof(apop_file) - 1] = '\0';
#else
	(void) strncpy(apop_dir, APOP, sizeof(apop_dir) - 5);
# if defined(BSD44_DBM)
	(void) strcat(apop_dir, ".db");
# else
	(void) strcat(apop_dir, ".dir");
# endif
#endif
#ifdef GDBM
	if (stat (apop_file, &st) != -1 && (st.st_mode & 0777) != 0600) 
#else
	if (stat (apop_dir, &st) != -1 && (st.st_mode & 0777) != 0600) 
#endif
	{
#ifdef GDBM
	    gdbm_close (db);
#else
	    dbm_close (db);
#endif
	    return(pop_auth_fail(p, POP_FAILURE,
		"POP authorization DB has wrong mode (0%o)",st.st_mode & 0777));
	}
#ifdef GDBM
	fid = open(apop_file, O_RDONLY);
#else
	fid = open(apop_dir, O_RDONLY);
#endif
	if(fid == -1) {
	    int e = errno;
#ifdef GDBM
	    gdbm_close (db);
#else
	    dbm_close (db);
#endif
	    return(pop_auth_fail(p, POP_FAILURE,
		    "unable to lock POP authorization DB (%s)", strerror(e)));
	}
	if (flock (fid, LOCK_SH) == -1) {
	    int e = errno;
	    (void) close(fid);
#ifdef GDBM
	    gdbm_close (db);
#else
	    dbm_close (db);
#endif 
	    return(pop_auth_fail(p, POP_FAILURE,
		    "unable to lock POP authorization DB (%s)", strerror(e)));
	}
	key.dsize = strlen (key.dptr = p->user) + 1;
#ifdef GDBM
	value = gdbm_fetch (db, key);
	gdbm_close(db);
#else
	value = dbm_fetch (db, key);
	dbm_close (db);
#endif
	(void) close(fid);

	if (value.dptr != NULL) {
	    return(pop_auth_fail(p, POP_FAILURE,
		"You must use APOP to connect to this server"));
	}
    }
#endif /* APOP */

    /*  Tell the user that the password is required */
    return (pop_msg(p,POP_SUCCESS,"Password required for %s.",p->user));
}

