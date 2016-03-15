/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */


/*
 * When adding each line length into the size of the message and the maildrop,
 * increase the character count by one for the <cr> added when sending the
 * message to the mail client.  All lines sent to the client are terminated
 * with a <cr><lf>.
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <flock.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_UNISTD_H
# include <sys/unistd.h>
#endif

#ifndef HAVE_INDEX
#  define	index(s,c)		strchr(s,c)
#  define	rindex(s,c)		strrchr(s,c)
# endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#include <sys/stat.h>
#include <sys/file.h>
#include <pwd.h>
#include <popper.h>

#include <md5.h>

#ifdef MAILOCK
# ifdef HAVE_MAILLOCK
#  include <maillock.h>
# endif
# define MAILUNLOCK() mailunlock()
#else 
# define MAILUNLOCK()
#endif

#define MIN_UIDL_LENGTH 5     /* For no reason */
#define MAX_UIDL_LENGTH 72    /* Including NL, Imposed by rfc1939 */

/* This macro comes from Mark Crispin's c-client code */

/* Validate line
 * Accepts: pointer to candidate string to validate as a From header
 *	    return pointer to end of date/time field
 *	    return pointer to offset from t of time (hours of ``mmm dd hh:mm'')
 *	    return pointer to offset from t of time zone (if non-zero)
 * Returns: t,ti,zn set if valid From string, else ti is NIL
 */

#define VALID(s,x,ti,zn) {						\
  ti = 0;								\
  if ((*s == 'F') && (s[1] == 'r') && (s[2] == 'o') && (s[3] == 'm') &&	\
      (s[4] == ' ')) {							\
    for (x = s + 5; *x && *x != '\n'; x++);				\
    if (x) {								\
      if (x - s >= 41) {						\
	for (zn = -1; x[zn] != ' '; zn--);				\
	if ((x[zn-1] == 'm') && (x[zn-2] == 'o') && (x[zn-3] == 'r') &&	\
	    (x[zn-4] == 'f') && (x[zn-5] == ' ') && (x[zn-6] == 'e') &&	\
	    (x[zn-7] == 't') && (x[zn-8] == 'o') && (x[zn-9] == 'm') &&	\
	    (x[zn-10] == 'e') && (x[zn-11] == 'r') && (x[zn-12] == ' '))\
	  x += zn - 12;							\
      }									\
      if (x - s >= 27) {						\
	if (x[-5] == ' ') {						\
	  if (x[-8] == ':') zn = 0,ti = -5;				\
	  else if (x[-9] == ' ') ti = zn = -9;				\
	  else if ((x[-11] == ' ') && ((x[-10]=='+') || (x[-10]=='-')))	\
	    ti = zn = -11;						\
	}								\
	else if (x[-4] == ' ') {					\
	  if (x[-9] == ' ') zn = -4,ti = -9;				\
	}								\
	else if (x[-6] == ' ') {					\
	  if ((x[-11] == ' ') && ((x[-5] == '+') || (x[-5] == '-')))	\
	    zn = -6,ti = -11;						\
	}								\
	if (ti && !((x[ti - 3] == ':') &&				\
		    (x[ti -= ((x[ti - 6] == ':') ? 9 : 6)] == ' ') &&	\
		    (x[ti - 3] == ' ') && (x[ti - 7] == ' ') &&		\
		    (x[ti - 11] == ' '))) ti = 0;			\
      }									\
    }									\
  }									\
}

/* You are not expected to understand this macro, but read the next page if
 * you are not faint of heart.
 *
 * Known formats to the VALID macro are:
 * 		From user Wed Dec  2 05:53 1992
 * BSD		From user Wed Dec  2 05:53:22 1992
 * SysV		From user Wed Dec  2 05:53 PST 1992
 * rn		From user Wed Dec  2 05:53:22 PST 1992
 *		From user Wed Dec  2 05:53 -0700 1992
 *		From user Wed Dec  2 05:53:22 -0700 1992
 *		From user Wed Dec  2 05:53 1992 PST
 *		From user Wed Dec  2 05:53:22 1992 PST
 *		From user Wed Dec  2 05:53 1992 -0700
 * Solaris	From user Wed Dec  2 05:53:22 1992 -0700
 *
 * Plus all of the above with `` remote from xxx'' after it. Thank you very
 * much, smail and Solaris, for making my life considerably more complicated.
 */


int newline = 1;

/*
 *  0 for not a from line
 *  1 for is a from line
 */

/* Valid UUCP From lines:
 *
 *	From Address [tty] date
 *	From [tty] date
 *
 *	"From [tty] date" is probably valid but I'm too lazy at this
 *	time to add the code.
 *
 */
int
isfromline(cp)
char	*cp;
{
    int ti, zn;
    char *x;

    /* If the previous line was not a newline then just return */
    /* From message separators are preceeded by a newline */ 
    if (*cp == '\n') {
	newline = 1;
	return(0);
    } else if (!newline) {
	return(0);
    } else
	newline = 0;

    VALID(cp, x, ti, zn);
    return(ti != 0);
}

/* Hashing to a spool directory helps reduce the lookup time for sites
 * with thousands of mail spool files.  Unix uses a linear list to
 * save directory information and the following methods attempt to
 * improve the performance.
 *
 * Method 1 - add the value of the first 4 chars mod 26 and open the
 *	      spool file in the directory 'a' - 'z'/user.
 *	      Brian Buhrow <buhrow@cats.ucsc.edu>
 *
 * Method 2 - Use the first 2 characters to determine which mail spool
 *	      to open.  Eg: /usr/spool/u/s/user.
 *	      Larry Schwimmer <rosebud@cyclone.stanford.edu>
 *
 * All these methods require that local mail delivery and client programs
 * use the same algorithm.  Only one method to a customer :-)
 */

#if (HASH_SPOOL == 1)

int
genpath(p)
POP *p;
{
    int seed = 0;
    char dirchar[4];

    if (!p->user || *p->user == '\0')
	return(NULL); /*bogus login name*/

    /*Now, perform the hash*/

    switch(strlen(p->user)) {
	   case 1:
	   seed = (p->user[0]);
	   break;
	   case 2:
	   seed = (p->user[0] + p->user[1]);
	   break;
	   case 3:
	   seed = (p->user[0] + p->user[1] + p->user[2]);
	   break;
	   case 4:
	   seed = (p->user[0] + p->user[1] + p->user[2]+p->user[3]);
	   break;
	   default:
	   seed = (p->user[0] + p->user[1] + p->user[2]+p->user[3]+p->user[4]);
	   break;
    }
    dirchar[0] = '/';
    dirchar[1] = (char)((seed % 26) + 'a');
    dirchar[2] = '/';
    dirchar[3] = '\0';
    strncpy(p->drop_name, POP_MAILDIR, sizeof(p->drop_name));
    strncat(p->drop_name, dirchar, sizeof(p->drop_name) - strlen(p->drop_name));
    strncat(p->drop_name, p->user, sizeof(p->drop_name) - strlen(p->drop_name));

    return(1);
}

#endif
#if (HASH_SPOOL == 2)

int
genpath(p)
POP *p;
{
    static char pathstr[256];

    if (!p->user || *p->user == '\0')
	return(NULL);
    
    sprintf(pathstr, "/%c/%c/%s",
		    *p->user, *(p->user+1) ? *(p->user+1) : *p->user, p->user);

    strncpy(p->drop_name, POP_MAILDIR, sizeof(p->drop_name));
    strncat(p->drop_name, pathstr, sizeof(p->drop_name) - strlen(p->drop_name));

    return(1);
}

#endif
#if (HASH_SPOOL != 1 && HASH_SPOOL != 2)

int
genpath(p)
POP *p;
{
    struct passwd *pwp;

#ifdef HOMEDIRMAIL
    if ((pwp = getpwnam(p->user)) == NULL) {
	pop_log(p, POP_FAILURE, "unable to retrieve users password entry");
	return(-1);
    }
    strncpy(p->drop_name, pwp->pw_dir, sizeof(p->drop_name));
    strncat(p->drop_name, "/.mail",sizeof(p->drop_name) - strlen(p->drop_name));
#else
    strncpy(p->drop_name, POP_MAILDIR, sizeof(p->drop_name));
    strncat(p->drop_name, "/", sizeof(p->drop_name) - strlen(p->drop_name));
    strncat(p->drop_name, p->user, sizeof(p->drop_name) - strlen(p->drop_name));
#endif

    return(1);
}

#endif /* HASH_SPOOL */


/* Open and check the .user.pop file and gather info before copying over
 * the users mailbox.
 */
int init_dropinfo(p)
POP *p;
{
    MsgInfoList         *   mp;         /* Pointer to message info list */
    int			    msg_num;	/* Current message number */
    int			    nchar;
    int			    inheader;
    int			    uidl_found;
    int			    expecting_trailer;
    int			    content_length, content_nchar, cont_len;
    char                    buffer[MAXLINELEN];		/*  Read buffer */
    MD5_CTX		    mdContext;
    unsigned char	    digest[16];

#ifdef DEBUG
    if(p->debug)
	pop_log(p,POP_DEBUG, "Checking for old .%s.pop file", p->user);
#endif

    /*  Allocate memory for message information structures */
    p->mlp = (MsgInfoList *)calloc((unsigned)ALLOC_MSGS,sizeof(MsgInfoList));
    if (p->mlp == NULL){
        return pop_msg (p,POP_FAILURE,
            "Can't build message list for '%s': Out of memory", p->user);
    }

    p->msg_count = 0;
    p->drop_size = 0;

#ifdef NULLKLUDGE
  /* Kludge to get around NULLs at the beginning of the mailspool */
  while ((tempchar = getc(p->drop)) == 0);
  ungetc(tempchar, p->drop);
#endif

    if (fgets(buffer, MAXMSGLINELEN, p->drop) == NULL) {
#ifdef DEBUG
	if(p->debug)
	    pop_log(p,POP_DEBUG, "Old .%s.pop file not found, errno (%d)",
								p->user, errno);
#endif
	return(POP_SUCCESS);
    }

    /* Is this an MMDF file? */
    if (*buffer == MMDF_SEP_CHAR) {
	p->mmdf_separator = (char *)strdup(buffer);
    } else if (!isfromline(buffer)) {
	return pop_msg (p,POP_FAILURE,
	  "Unable to process From lines (envelopes), change recognition modes.");
    }

    newline = 1;
    rewind(p->drop);

    inheader = 0;
    msg_num = 0;
    uidl_found = 0;
    expecting_trailer = 0;
    content_length = 0;
    content_nchar = 0;
    cont_len = 0;
    p->msg_count = ALLOC_MSGS;

#ifdef NULLKLUDGE
  /* Kludge to get around NULLs at the beginning of the mailspool */
  while ((tempchar = getc(p->drop)) == 0);
  ungetc(tempchar, p->drop);
#endif

    for (mp=p->mlp - 1; fgets(buffer, MAXMSGLINELEN, p->drop);) {
	nchar = strlen(buffer);

	if ((content_nchar >= content_length) &&
	    (p->mmdf_separator ? !strcmp(p->mmdf_separator, buffer) :
	    isfromline(buffer))) {

	    if (expecting_trailer) {
		/* skip over the MMDF trailer */
		expecting_trailer = 0;
		continue;
	    }

	    MD5Init (&mdContext);
	    MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));

	    if (!inheader) {
		if (++msg_num > p->msg_count) {
		    p->mlp = (MsgInfoList *) realloc(p->mlp,
			(p->msg_count += ALLOC_MSGS) * sizeof(MsgInfoList));
		    if (p->mlp == NULL){
			p->msg_count = 0;
			return pop_msg (p, POP_FAILURE,
			    "Can't build message list for '%s': Out of memory",
				    p->user);
		    }
		    mp = p->mlp + msg_num - 2;
		}
#ifdef DEBUG
		if(p->debug && msg_num != 1)
		    pop_log(p, POP_DEBUG,
	    "Msg %d uidl %s at offset %d is %d octets long and has %u lines.",
	    mp->number, mp->uidl_str, mp->offset, mp->length, mp->lines);
		else
		    pop_log(p,POP_DEBUG, "Found top of first message");
#endif
		++mp;

	    } else {
		pop_log(p,POP_DEBUG,
		    "Msg %d corrupted, ignoring previous header information.",
		     mp->number);
	    }
	    mp->number = msg_num;
	    mp->length = 0;
	    mp->lines = 0;
	    mp->body_lines = 0;
	    mp->offset = ftell(p->drop) - nchar;
	    mp->del_flag = FALSE;
	    mp->retr_flag = FALSE;
	    mp->orig_retr_state = FALSE;
	    mp->uidl_str = "\n";
#ifdef DEBUG
	    if(p->debug)
		pop_log(p,POP_DEBUG, "Msg %d being added to list", mp->number);
#endif
	    inheader = 1;
	    uidl_found = 0;
	    content_nchar = 0;
	    content_length = 0;
	    cont_len = 0;
	    if (p->mmdf_separator)
		expecting_trailer = 1;
	    
	    continue;	/* Don't count message separator in total size */
	}
	
	if (inheader) {
	    if (*buffer == '\n') {
		inheader = 0;
		content_length = cont_len;
		mp->body_lines = 1;  /* Count newline as the first body line */
		if (!uidl_found) {
		    char	*cp;
		    int		i;

		    MD5Final (digest, &mdContext);
		    cp = mp->uidl_str = (char *)malloc((DIG_SIZE * 2) + 2);

		    for (i = 0; i < DIG_SIZE; i++, cp+=2) {
			sprintf(cp, "%02x", digest[i]);
		    }
		    *cp++ = '\n';
		    *cp   = '\0';

		    mp->length += strlen("X-UIDL: ") + strlen(mp->uidl_str) + 1;
		    p->drop_size += strlen("X-UIDL: ") + strlen(mp->uidl_str)+1;

	/* New UIDs do not dirty the mailspool if NO_STATUS is set.  They
	   are just recalculated each time the popper is run or LMOS is
	   set and the mail spool is updated.
	 */
#ifndef NO_STATUS
		    p->dirty = 1;
#endif
		}

	    } else if (CONTENT_LENGTH && !strncmp(buffer, "Content-Length:", 15)) {
		cont_len = atoi(buffer + 15);
		MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));
		continue;	/* not part of the message size */
	    } else if (!uidl_found && (!strncasecmp("Received:", buffer, 9) ||
				       !strncasecmp("Date:", buffer, 5) ||
				       !strncasecmp("Message-Id:",buffer, 11) ||
				       !strncasecmp("Subject:",buffer, 8)
				       )) {
		MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));
	    } else if (!strncasecmp("X-UIDL:", buffer, 7)) {
		if (!uidl_found) {
		    char *cp;
		    int len;

		    /* Skip over header string */
		    cp = &buffer[7];
                    while (*cp && (*cp == ' ' || *cp == '\t')) cp++;
		    if( (len = strlen(cp)) > MIN_UIDL_LENGTH && len < MAX_UIDL_LENGTH ) {
			uidl_found++;
			mp->uidl_str = (char *)strdup(cp);
			mp->length += nchar + 1;
			p->drop_size += nchar + 1;
		    }
		}
		continue;  /* Do not include this value in the message size */
	    } else if ((strncasecmp(buffer,"Status:",7) == 0)) {
		if (index(buffer, 'R') != NULL) {
		    mp->retr_flag = TRUE;
		    mp->orig_retr_state = TRUE;
		}
	    }
	} else {
	    content_nchar += nchar;
	    mp->body_lines++;
	}

	mp->length += nchar + 1;
	p->drop_size += nchar + 1;
	mp->lines++;
    }

    p->msg_count = msg_num;

    return(POP_SUCCESS);
}


/* 
 *  use to be dropinfo:   Extract information about the POP maildrop and store 
 *  it for use by the other POP routines.
 *
 *  Now, the copy and info collection are done at the same time.
 */

do_drop_copy(p, mfd, dfd)
int	mfd, dfd;
POP	*p;
{
    char                    buffer[MAXLINELEN];     /*  Read buffer */
    MsgInfoList         *   mp;                     /*  Pointer to message 
                                                        info list */
    int                     nchar;                  /*  Bytes written/read */
    int			    inheader;		    /*  Header section flag */
    int			    uidl_found;		    /*  UIDL header flag */
    int			    msg_num;
    int			    expecting_trailer;
    int			    content_length, content_nchar, cont_len;
    MD5_CTX		    mdContext;
    unsigned char	    digest[16];

    FILE		    *mail_drop;		    /*  Streams for fids */

    /*  Acquire a stream pointer for the maildrop */
    if ( (mail_drop = fdopen(mfd,"r")) == NULL ) {
        (void)close(mfd) ;
        return pop_msg(p,POP_FAILURE,"Cannot assign stream for %s (%d)",
            p->drop_name, errno);
    }

    rewind (mail_drop);

#ifdef NULLKLUDGE
  /* Kludge to get around NULLs at the beginning of the mailspool */
  while ((tempchar = getc(p->drop)) == 0);
  ungetc(tempchar, p->drop);
#endif

    if (fgets(buffer, MAXMSGLINELEN, mail_drop) == NULL) {
	return(POP_SUCCESS);
    }

    /* Is this an MMDF file? */
    if (*buffer == MMDF_SEP_CHAR) {
	if (!p->mmdf_separator)
	    p->mmdf_separator = (char *)strdup(buffer);
    } else if (!isfromline(buffer)) {
	return pop_msg (p,POP_FAILURE,
	 "Unable to process From lines (envelopes), change recognition modes.");
    }

    newline = 1;
    rewind (mail_drop);

    /*  Scan the file, loading the message information list with 
        information about each message */

    inheader = 0;
    uidl_found = 0;
    expecting_trailer = 0;
    msg_num = p->msg_count;
    content_length = 0;
    content_nchar = 0;
    cont_len = 0;
    p->msg_count = (((p->msg_count - 1) / ALLOC_MSGS) + 1) * ALLOC_MSGS;

#ifdef NULLKLUDGE
  /* Kludge to get around NULLs at the beginning of the mailspool */
  while ((tempchar = getc(p->drop)) == 0);
  ungetc(tempchar, p->drop);
#endif

    for (mp = p->mlp + msg_num - 1; fgets(buffer,MAXMSGLINELEN,mail_drop);) {

	nchar = strlen(buffer);

	if (fputs(buffer, p->drop) == EOF) {
#ifdef EDQUOT
	    if (errno == EDQUOT)
		return pop_msg (p,POP_FAILURE,
		    "Unable to copy mail spool file, quota exceeded (%d)",
			errno);
#endif
	    return pop_msg (p,POP_FAILURE,
		"Unable to copy mail spool file to temp pop dropbox %s (%d)",
		    p->temp_drop, errno);
	}

	if ((content_nchar >= content_length) &&
	    (p->mmdf_separator ? !strcmp(p->mmdf_separator, buffer) :
	    isfromline(buffer))) {

	    if (expecting_trailer) {
		expecting_trailer = 0;
		continue;
	    }

	    MD5Init (&mdContext);
	    MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));

	    if (!inheader) {
		if (++msg_num > p->msg_count) {
		    p->mlp=(MsgInfoList *) realloc(p->mlp,
			(p->msg_count+=ALLOC_MSGS)*sizeof(MsgInfoList));
		    if (p->mlp == NULL) {
			(void)close (mfd);
			(void)close (dfd);
			p->msg_count = 0;
			return pop_msg (p,POP_FAILURE,
			    "Can't build message list for '%s': Out of memory",
				p->user);
		    }
		    mp = p->mlp + msg_num - 2;
		}
#ifdef DEBUG
		if(p->debug && msg_num != 1)
		    pop_log(p,POP_DEBUG,
	    "Msg %d uidl %s at offset %d is %d octets long and has %u lines.",
	    mp->number,mp->uidl_str,mp->offset,mp->length,mp->lines);
#endif
		++mp;

	    } else {
		pop_log(p,POP_DEBUG,
		    "Msg %d corrupted, ignoring previous header information.",
		     mp->number);
	    }

            mp->number = msg_num;
            mp->length = 0;
            mp->lines = 0;
            mp->body_lines = 0;
            mp->offset = ftell(p->drop) - nchar;
            mp->del_flag = FALSE;
            mp->retr_flag = FALSE;
            mp->orig_retr_state = FALSE;
	    mp->uidl_str = "\n";

#ifdef DEBUG
            if(p->debug)
                pop_log(p,POP_DEBUG, "Msg %d being added to list", mp->number);
#endif
	    inheader = 1;
	    content_length = 0;
	    content_nchar = 0;
	    cont_len = 0;
	    uidl_found = 0;
	    if (p->mmdf_separator)
		expecting_trailer = 1;

	    continue;	/* Do not include From separator in message size */
        } 

	if (inheader) {
	    if (*buffer == '\n') {
		inheader = 0;
		mp->body_lines = 1;
		content_length = cont_len;

		if (!uidl_found) {
		    char *cp;
		    int  i;

		    MD5Final (digest, &mdContext);
		    cp = mp->uidl_str = (char *)malloc((DIG_SIZE * 2) + 2);

		    for (i = 0; i < DIG_SIZE; i++, cp+=2) {
			sprintf(cp, "%02x", digest[i]);
		    }
		    *cp++ = '\n';
		    *cp   = '\0';

		    mp->length += strlen("X-UIDL: ") + strlen(mp->uidl_str) + 1;
		    p->drop_size += strlen("X-UIDL: ") + strlen(mp->uidl_str)+1;

	/* New UIDs do not dirty the mailspool if NO_STATUS is set.  They
	   are just recalculated each time the popper is run or LMOS is
	   set and the mail spool is updated.
	 */
#ifndef NO_STATUS
		    p->dirty = 1;
#endif
		}

	    } else if (CONTENT_LENGTH && !strncmp(buffer, "Content-Length:", 15)) {
		cont_len = atoi(buffer + 15);
		MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));
		continue;  /* Not included in message size */

	    } else if (!uidl_found && (!strncasecmp("Received:", buffer, 9) ||
				       !strncasecmp("Date:", buffer, 5) ||
				       !strncasecmp("Message-Id:",buffer, 11) ||
				       !strncasecmp("Subject:",buffer, 8)
				       )) {
		MD5Update(&mdContext,(unsigned char *)buffer,strlen(buffer));
	    } else if (!strncasecmp("X-UIDL:", buffer, 7)) {
		if (!uidl_found) {
		    int len;
		    char *cp;

		    uidl_found++;

		    /* Skip over header */
		    cp = &buffer[7];
                    while (*cp && (*cp == ' ' || *cp == '\t')) cp++;
		    if( (len = strlen(cp)) > MIN_UIDL_LENGTH && len < MAX_UIDL_LENGTH ) {
			uidl_found++;
			mp->uidl_str = (char *)strdup(cp);
			mp->length += nchar + 1;
			p->drop_size += nchar + 1;
		    }
		}
		continue;  /* Do not include this value in the message size */
	    } else if (!strncasecmp(buffer,"Status:",7)) {
		if (index(buffer, 'R') != NULL) {
		    mp->retr_flag = TRUE;
		    mp->orig_retr_state = TRUE;
		}
	    }
	} else {
	    content_nchar += nchar;
	    mp->body_lines++;
	}

        mp->length += nchar + 1;
        p->drop_size += nchar + 1;
        mp->lines++;
    }

    p->msg_count = msg_num;

#ifdef DEBUG
    if(p->debug && msg_num > 0) {
        register    i;
        for (i = 0, mp = p->mlp; i < p->msg_count; i++, mp++)
            pop_log(p,POP_DEBUG,
	    "Msg %d uidl %s at offset %d is %d octets long and has %u lines.",
	    mp->number,mp->uidl_str,mp->offset,mp->length,mp->lines);
    }
#endif

    if (fflush(p->drop) == EOF)
        return pop_msg(p,POP_FAILURE,"Flush of temp pop dropbox %s failed (%d)",
	    p->temp_drop, errno);

    return(POP_SUCCESS);
}

/* 
 *  dropcopy:   Make a temporary copy of the user's mail drop and 
 *  save a stream pointer for it.
 */

pop_dropcopy(p, pwp)
POP     *   p;
struct passwd	*	pwp;
{
    int                     mfd;                    /*  File descriptor for 
                                                        the user's maildrop */
    int                     dfd;                    /*  File descriptor for 
                                                        the SERVER maildrop */
    FILE		    *tf;		    /*  The temp file */
    int			    tfn;		    
    char                    buffer[MAXLINELEN];     /*  Read buffer */
    long                    offset;                 /*  Old/New boundary */
    int                     nchar;                  /*  Bytes written/read */
    struct stat             mybuf;                  /*  For fstat() */

    if (genpath(p) < 0)
	return(pop_msg(p, POP_FAILURE, "Unable to create temporary drop name"));

    /*  Create a temporary maildrop into which to copy the updated maildrop */
    (void)sprintf(p->temp_drop, POP_DROP, p->user);

#ifdef DEBUG
    if(p->debug)
        pop_log(p,POP_DEBUG,"Creating temporary maildrop '%s'",
            p->temp_drop);
#endif

#ifdef BULLDB
    if (p->bulldir) {
	char bull_db[MAXLINELEN];

#ifdef BULLDBDIR
	sprintf(bull_db, "%s/bulldb", BULLDBDIR);
#else
	sprintf(bull_db, "%s/bulldb", p->bulldir);
#endif
#ifdef GDBM
		if ((p->bull_db = gdbm_open (bull_db, 512, GDBM_WRCREAT, 0600, 0)) == NULL)
#else
        if ((p->bull_db = dbm_open (bull_db, O_RDWR | O_CREAT, 0600)) == NULL) 
#endif
		{
pop_log(p,POP_PRIORITY,"[g]dbm_open failed : %s (%d)",sys_errlist[errno],errno);
	    return(pop_msg(p, POP_FAILURE,
	       "Unable to open Bulletin database, contact your administrator"));
		}
    }
#endif

    /* Here we work to make sure the user doesn't cause us to remove or
     * write over existing files by limiting how much work we do while
     * running as root.
     */

#ifdef BINMAIL_IS_SETGID
# if BINMAIL_IS_SETGID > 1
    pwp->pw_gid = (gid_t)BINMAIL_IS_SETGID;
# else
    if (!stat(POP_MAILDIR, &mybuf))
	pwp->pw_gid = mybuf.st_gid;
# endif
#endif

    /* Now we run as the user. */
    (void) setgid((GID_T)pwp->pw_gid);
    (void) setgroups(1,(GID_T *)&pwp->pw_gid); /* Set the supplementary groups list */
    (void) setuid((UID_T)pwp->pw_uid);

#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"uid = %d, gid = %d",getuid(),getgid());
#endif

    if ((dfd = open(p->temp_drop, O_RDWR | O_CREAT, 0600)) == -1) {
        pop_log(p, POP_PRIORITY,
            "Unable to open temporary maildrop '%s': %s (%d)",p->temp_drop,
                (errno < sys_nerr) ? sys_errlist[errno] : "", errno) ;
        return pop_msg(p,POP_FAILURE,
		"System error, can't open temporary file, do you own it?");
    }

    fstat(dfd, &mybuf);
    if (mybuf.st_uid != pwp->pw_uid) {
	close(dfd);
	return(pop_msg(p, POP_FAILURE, "Temporary drop file not owned by %s.",
	    p->user));
    }
#ifdef NeXT
    if (mybuf.st_mode & (0x7)) {
#else
    if (mybuf.st_mode & (S_IWOTH | S_IROTH)) {
#endif
	close(dfd);
	return(pop_msg(p, POP_FAILURE,
	  "Your temporary file %s is accessable by others.  This must be fixed",
	    p->temp_drop));
    }
    /* Make sure the mailspool is not a hard link */
    if (mybuf.st_nlink != 1) {
	close(dfd);
	return(pop_msg(p, POP_FAILURE,
	    "Your temporary file appears to have more than one link."));
    }

    /* If the temporary popdrop is not empty, revert to regular mode. */
    if (mybuf.st_size != 0)
	p->server_mode = 0;

#if defined(S_ISREG)
    /* Make sure the file is not a symbolic link reference */
    lstat(p->temp_drop, &mybuf);
    if (!S_ISREG(mybuf.st_mode)) {
	close(dfd);
	return pop_msg(p, POP_FAILURE,
	"Your temporary drop file %s is not type 'regular file'", p->temp_drop);
    }
#endif

    /*  Lock the temporary maildrop */
    if ( flock (dfd, LOCK_EX|LOCK_NB) == -1 ) {
	switch(errno) {
	    case EWOULDBLOCK:
		return pop_msg(p,POP_FAILURE,
		     "%s lock busy!  Is another session active? (%d)",
							p->temp_drop, errno);
		/* NOTREACHED */
	    default:
		return pop_msg(p,POP_FAILURE,"flock: '%s': %s (%d)",
	    p->temp_drop, (errno < sys_nerr) ? sys_errlist[errno] : "", errno);
		/* NOTREACHED */
	}
    }
    
#ifndef KEEP_TEMP_DROP
    /* check for race conditions involving unlink.  See pop_updt.c */
    /* s-dorner@uiuc.edu, 12/91 */
    {
      struct stat byName, byFd;
      if (stat(p->temp_drop, &byName) || fstat(dfd, &byFd) ||
	  byName.st_ino != byFd.st_ino)
      {
        return pop_msg(p,POP_FAILURE,
		"Maildrop being unlocked; try again.");
      }
    }
#endif
    
    /* If in server mode and the temporary popdrop has any data in it
       then revert back to the original way of dealing with pop drops.
     */

    /*  Acquire a stream pointer for the temporary maildrop */
    if ( (p->drop = fdopen(dfd,"r+")) == NULL ) {
        (void)close(dfd) ;
        return pop_msg(p,POP_FAILURE,"Cannot assign stream for %s (%d)",
            p->temp_drop, errno);
    }

    if (!p->server_mode) {
	/* In server mode this file is used as a process lock and a temporary
	   copy file later on */

	if (init_dropinfo(p) != POP_SUCCESS)
	    return(POP_FAILURE);

	/* Sync with stdio */
	(void)fseek(p->drop, 0L, SEEK_END);
	offset = ftell(p->drop);
    }

#ifdef MAILOCK
    /*  Lock the maildrop */
    if (maillock(p->user,1) != 0)
            return pop_msg(p,POP_FAILURE, "maillock: '%s'", p->temp_drop);
#endif

    /*  Open the user's maildrop, If this fails,  no harm in assuming empty */
    if ((mfd = open(p->drop_name, O_RDWR)) > 0) {
        /* Lock the maildrop */
        if (flock (mfd,LOCK_EX) == -1)
	{
            (void)close(mfd) ;
	    MAILUNLOCK();
            return pop_msg(p,POP_FAILURE, "flock: '%s': %s (%d)", p->temp_drop,
                (errno < sys_nerr) ? sys_errlist[errno] : "", errno);
        }

	if (!p->server_mode) {
	    /* New routine to copy and get dropbox info all at the same time */
	    nchar = do_drop_copy(p, mfd, dfd);

	    if ( nchar != POP_SUCCESS ) {
		/* pop_dropcopy has integrated the info gathering pass into
		   the copy pass so now the information of the dropfile is
		   inconsistant if there is an error.  Now we exit instead
		   of trying to continue with what is available.
		*/
		(void)ftruncate(dfd, (OFF_T)offset) ;
		return(nchar);
	    } else {
		/* Mail transferred!  Zero the mail drop NOW,  that we
		   do not have to do gymnastics to figure out what's new
		   and what is old later */
		(void)ftruncate(mfd, (OFF_T)0) ;
	    }

	    /*  Close the actual mail drop */
	    (void)close (mfd);
	} else {
	    /* Save the temporary drop FILE and fid values */
	    p->hold = p->drop;
	    if ((p->drop = fdopen(mfd,"r+")) == NULL) {
		pop_msg(p,POP_FAILURE,"Cannot assign stream for %s (%d)",
		    p->drop_name, errno);
		goto bailout;
	    }

	    if (init_dropinfo(p) != POP_SUCCESS)
		goto bailout;

	    dfd = mfd;
	}
    } 

    /* Recalculate offset */
    (void)fseek(p->drop, 0L, SEEK_END);
    offset = ftell(p->drop);
    if ((p->bulldir != NULL) && (pop_bull(p, pwp) != POP_SUCCESS)) {
	/* Return pop drop back to what it was before the bulletins */
	(void)ftruncate(dfd, (OFF_T)offset);
    }
#ifdef BULLDB
    if(p->bulldir != NULL && p->bull_db != NULL) {
#ifdef GDBM
        gdbm_close(p->bull_db);
#else
        dbm_close(p->bull_db);
#endif
	p->bull_db = NULL;
    }
#endif
    (void)fseek(p->drop, 0L, SEEK_END);
    p->spool_end = ftell(p->drop);

    MAILUNLOCK();
    if (p->server_mode)
        flock(mfd, LOCK_UN);

    return(POP_SUCCESS);
 bailout:
    MAILUNLOCK();
    flock(mfd, LOCK_UN);
    flock(dfd, LOCK_UN);
    close(mfd);
    close(dfd);
    return( POP_FAILURE );
}

