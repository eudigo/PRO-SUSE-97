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
# include <string.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <flock.h>
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#include <sys/stat.h>
#include <sys/file.h>
#include <popper.h>

#ifdef MAILOCK
# include <maillock.h>
#endif

#if !defined(L_XTND)
#define L_XTND SEEK_END
#endif
#ifndef L_SET
#define L_SET SEEK_SET
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

extern int      errno;

#define		BBSIZE		4096

static char standard_error[] =
    "Error updating primary drop. Mail left in temporary maildrop (%d)";

/* 
 *  updt:   Apply changes to a user's POP maildrop
 */

int pop_updt (p)
POP     *   p;
{
    FILE                *   md;                     /*  Stream pointer for 
                                                        the user's maildrop */
    int                     mfd;                    /*  File descriptor for
                                                        above */
    char                    buffer[BBSIZE];         /*  Read buffer */

    char		    uidl_buf[128];	    /*  UIDL header */
    MsgInfoList         *   mp;                     /*  Pointer to message 
                                                        info list */
    register int            msg_num;                /*  Current message 
                                                        counter */
    register int            status_written;         /*  Status header field 
                                                        written */
    int                     nchar;                  /* Bytes read/written */

    long                    offset;                 /* New mail offset */

    char		*   result;		    /* fget and fputs status */

    int			    save_errno;		    /* Save the error value we
						       are trying to print. */
    struct stat             mybuf;                  /*  For fstat() */

#ifdef DEBUG
    if (p->debug) {
        pop_log(p,POP_DEBUG,"Performing maildrop update...");
        pop_log(p,POP_DEBUG,"Checking to see if all messages were deleted");
    }
#endif

    if (p->stats) {
        pop_log(p,POP_PRIORITY,"Stats: %s %d %d %d %d",
           p->user, p->msgs_deleted, p->bytes_deleted,
           p->msg_count - p->msgs_deleted,
           p->drop_size - p->bytes_deleted);
    }

    if (p->server_mode) {
	if (!p->dirty) {
#ifndef KEEP_TEMP_DROP
	    /* Added code in pop_dropcopy.c makes unlink ok now. */
	    /* s-dorner@uiuc.edu, 12/91 */
	    (void)unlink(p->temp_drop);
#endif
	    return(POP_SUCCESS);
	}

#ifdef MAILOCK
	/* Use SVR4 mail locking */
	if (maillock(p->user, 1) != 0)
	    return(pop_msg(p,POP_FAILURE,"maillock: '%s' (%d)",p->temp_drop,errno));
#endif
    }

    if (p->server_mode)
	fstat(fileno(p->drop), &mybuf);

    if ((p->msgs_deleted == p->msg_count) &&
	(!p->server_mode || (mybuf.st_size == p->spool_end))) {
	/* Truncate before close, to avoid race condition.  */
	(void)ftruncate (fileno(p->drop), (OFF_T)0);
#ifndef KEEP_TEMP_DROP
	/* Added code in pop_dropcopy.c makes unlink ok now. */
	/* s-dorner@uiuc.edu, 12/91 */
	(void)unlink(p->temp_drop);
#endif
	(void)fclose(p->drop);
	if (p->server_mode) {
	    (void)fclose(p->hold);
#ifdef MAILOCK
	    mailunlock();
#endif
	}
        return (POP_SUCCESS);
    }

    if (!p->server_mode) {

#ifdef DEBUG
	if (p->debug) 
	    pop_log(p,POP_DEBUG,"Opening mail drop \"%s\"", p->drop_name);
#endif

#ifdef MAILOCK
	/* Use SVR4 mail locking */
	if (maillock(p->user, 1) != 0)
	    return(pop_msg(p,POP_FAILURE,"maillock: '%s' (%d)",
							p->drop_name, errno));
#endif

	/*  Open the user's real maildrop */
	if ((mfd = open(p->drop_name,O_RDWR|O_CREAT,0660)) == -1 ||
	    (md = fdopen(mfd,"r+")) == NULL) {
#ifdef MAILOCK
	    mailunlock();
#endif
	    return pop_msg(p,POP_FAILURE, standard_error, errno);
	}
    } else {
#ifdef MAILOCK
	/* Use SVR4 mail locking */
	if (maillock(p->user, 1) != 0)
	    return(pop_msg(p,POP_FAILURE,"maillock: '%s' (%d)",
							p->drop_name, errno));
#endif
	mfd = fileno(p->drop);
    }

    /*  Lock the user's real mail drop */
    if ( flock(mfd,LOCK_EX) == -1 )
    {
        (void)fclose(md) ;
#ifdef MAILOCK
	mailunlock();
#endif
        return pop_msg(p,POP_FAILURE, "flock: '%s': %s (%d)", p->drop_name,
            (errno < sys_nerr) ? sys_errlist[errno] : "", errno);
    }

    if (!p->server_mode) {
	/* Go to the right places */
	(void)fseek(p->drop, 0, SEEK_END); 
	offset = ftell(p->drop); 

	/*  Append any messages that may have arrived during the session 
	    to the temporary maildrop */
	while ((nchar = read(mfd, buffer, BBSIZE)) > 0)
	    if ( nchar != write(fileno(p->drop), buffer, nchar) ) {
		nchar = -1;
		break ;
	    }
	if ( nchar != 0 ) {
	    save_errno = errno;
	    (void)fclose(md) ;
#ifdef MAILOCK
	    mailunlock();
#endif
	    (void)ftruncate(fileno(p->drop), (OFF_T)offset);
	    (void)fclose(p->drop) ;
#ifdef EDQUOT
	    if (save_errno == EDQUOT) {
		pop_msg(p, POP_FAILURE,
	"Overquota: appending messages from mailspool to temporary drop (%d)",
		errno);
	    } else
#endif
		pop_msg(p, POP_FAILURE,
	      "Error appending messages from mailspool to temporary drop (%d)",
		    errno);
	}

	fflush(md);
	rewind(md);
	(void)ftruncate(mfd, (OFF_T)0) ;
	(void)lseek(mfd, (OFF_T)0, L_SET);

	/* Synch stdio and the kernel for the POP drop */
	rewind(p->drop);
	(void)lseek(fileno(p->drop), (OFF_T)0, L_SET);

	/*  Transfer messages not flagged for deletion from the temporary 
	    maildrop to the new maildrop */
#ifdef DEBUG
	if (p->debug) 
	    pop_log(p,POP_DEBUG,"Creating new maildrop \"%s\" from \"%s\"",
		    p->drop_name,p->temp_drop);
#endif

    } else {
	/* Move this stuff later */
	fstat(fileno(p->drop), &mybuf);

	md = p->hold;	/* Really the temp drop */
	mfd = fileno(md);
    }

    if (!p->server_mode || (p->msgs_deleted != p->msg_count)) {

	for (msg_num = 0; msg_num < p->msg_count; ++msg_num) {

	    int inheader;
	    int body_lines;

	    /*  Get a pointer to the message information list */
	    mp = &p->mlp[msg_num];

	    if (mp->del_flag) {
#ifdef DEBUG
		if(p->debug)
		    pop_log(p,POP_DEBUG,
			"Message %d flagged for deletion.",mp->number);
#endif
		continue;
	    }

	    (void)fseek(p->drop, mp->offset, SEEK_SET);

#ifdef DEBUG
	    if(p->debug)
		pop_log(p,POP_DEBUG,"Copying message %d.",mp->number);
#endif

	    /* Put the From line separator */
	    fgets(buffer, MAXMSGLINELEN, p->drop);
	    if (fputs(buffer, md) == EOF)
		break;

	    sprintf(buffer, "%s %s", "X-UIDL:", mp->uidl_str);
	    if (fputs(buffer, md) == EOF)
		break;

	    for(status_written=0,inheader=1;
				    fgets(buffer,MAXMSGLINELEN,p->drop);) {

		if (inheader) { /* Header */

		    /*  A blank line signals the end of the header. */
		    if (*buffer == '\n') {
#ifndef NO_STATUS
			if (status_written == 0) {
			    if (mp->retr_flag) {
				(void)sprintf(buffer, "Status: RO\n\n");
			    } else {
				(void)sprintf(buffer, "Status: U\n\n");
			    }
			}
#endif

			inheader = 0;
			body_lines = 1;
			status_written = 0;

		    } else if (!strncasecmp(buffer, "X-UIDL:", 7)) {
			continue;	/* Skip all existing UIDL lines */

		    } else if (!strncasecmp(buffer,"Status:", 7)) {

			/*  Update the message status */
			if (mp->retr_flag)
			    sprintf(buffer, "Status: RO\n");
			status_written++;
		    }
		    /*  Save another header line */
		    if (fputs(buffer, md) == EOF)
			break;

		    } else { /* Body */ 
			if (++body_lines > mp->body_lines)
			    break;
			if (fputs(buffer, md) == EOF)
			    break;
		    }
	    }

	    if (ferror(md)) {
		break;
	    }

	    if (p->mmdf_separator) {
		fputs(p->mmdf_separator, md);
	    }
	}

    /* flush and check for errors now!  The new mail will writen
       without stdio,  since we need not separate messages */

	if (ferror(md)) {
	    save_errno = errno;

	    (void)ftruncate(mfd, (OFF_T)0) ;
	    (void)fclose(md) ;
#ifdef MAILOCK
	    mailunlock();
#endif
	    (void)fclose(p->drop) ;
#ifdef EDQUOT
	    if (save_errno == EDQUOT)
		return pop_msg(p, POP_FAILURE,
	    "Overquota copying messages to Mailspool. Temp drop unchanged (%d)",
		    save_errno);
	    else
#endif
		return pop_msg(p,POP_FAILURE, standard_error, errno);
	}

	(void)fflush(md) ;
	if (ferror(md)) {
	    int save_error = errno;

	    (void)ftruncate(mfd, (OFF_T)0) ;
	    (void)close(mfd) ;
#ifdef MAILOCK
	    mailunlock();
#endif
	    (void)fclose(p->drop);
#ifdef EDQUOT
	    if (save_errno == EDQUOT)
		return pop_msg(p, POP_FAILURE,
	    "Overquota copying messages to Mailspool. Temp drop unchanged (%d)",
		    save_errno);
	    else
#endif
		return pop_msg(p,POP_FAILURE, standard_error, errno);
	}

    } /* p->msgs_deleted != p->msg_count */

    if (p->server_mode) {
	if (mybuf.st_size > p->spool_end) {
	    /* Go to the right places */
	    (void)lseek(fileno(p->drop), (OFF_T)p->spool_end, L_SET); 

	    /*  Append any messages that may have arrived during the session 
		to the temporary maildrop */
	    while ((nchar = read(fileno(p->drop), buffer, BBSIZE)) > 0)
		if ( nchar != write(mfd, buffer, nchar) ) {
		    nchar = -1;
		    break ;
		}

	    if ( nchar != 0 ) {
#ifdef MAILOCK
		mailunlock();
#endif
		(void)ftruncate(mfd, (OFF_T)0) ;
		(void)close(mfd) ;
#ifdef EDQUOT
		if (errno == EDQUOT) {
		    pop_msg(p, POP_FAILURE,
	"Overquota: appending messages from mailspool to temporary drop (%d)",
		    errno);
		} else
#endif
		    pop_msg(p, POP_FAILURE,
	    "Error appending messages from mailspool to temporary drop (%d)",
			errno);
	    }
	}

	rewind(p->drop);
	(void)ftruncate(fileno(p->drop), (OFF_T)0);
	(void)lseek(fileno(p->drop), (OFF_T)0, L_SET);

	(void)lseek(mfd, (OFF_T)0, L_SET);

	while ((nchar = read(mfd, buffer, BBSIZE)) > 0)
	    if (nchar != write(fileno(p->drop), buffer, nchar)) {
		nchar = -1;
		break ;
	    }

	if ( nchar != 0 ) {
	    (void)ftruncate(fileno(p->drop), (OFF_T)0);
	    (void)fclose(p->drop);
#ifdef MAILOCK
	    mailunlock();
#endif
	    (void)fclose(md);
#ifdef EDQUOT
	    if (errno == EDQUOT) {
		pop_msg(p, POP_FAILURE,
		"Overquota: copying messages back to mailspool (%d)", errno);
	    } else
#endif
		pop_msg(p, POP_FAILURE,
	    "Error appending messages from temporary drop to mailspool (%d)",
		errno);
	}

	(void)fclose(md);
#ifdef MAILOCK
	mailunlock();
#endif
	(void)ftruncate(mfd, (OFF_T)0);
#ifndef KEEP_TEMP_DROP
	/* Added code in pop_dropcopy.c makes unlink ok now. */
	/* s-dorner@uiuc.edu, 12/91 */
	(void)fclose(p->drop);
#endif
	(void)unlink(p->temp_drop);
    } else {
	/* Go to start of new mail if any */
	(void)lseek(fileno(p->drop), (OFF_T)offset, L_SET);

	/* Copy over any new mail that arrived while processing the pop drop */
	while((nchar = read(fileno(p->drop), buffer, BBSIZE)) > 0)
	    if ( nchar != write(mfd, buffer, nchar) ) {
		nchar = -1;
		break ;
	    }
	if ( nchar != 0 ) {
	    int save_error = errno;

	    (void)ftruncate(mfd, (OFF_T)0) ;
	    (void)fclose(md) ;
#ifdef MAILOCK
	    mailunlock();
#endif
	    (void)fclose(p->drop) ;
#ifdef EDQUOT
	    if (save_errno == EDQUOT)
		return pop_msg(p, POP_FAILURE,
	    "Overquota copying messages to Mailspool. Temp drop unchanged (%d)",
		save_errno);
	    else
#endif
		return pop_msg(p,POP_FAILURE, standard_error, errno);
	}

	/*  Close the maildrop and empty temporary maildrop */
	(void)fclose(md);
#ifdef MAILOCK
	mailunlock();
#endif
	(void)ftruncate(fileno(p->drop), (OFF_T)0);
#ifndef KEEP_TEMP_DROP
	/* Added code in pop_dropcopy.c makes unlink ok now. */
	/* s-dorner@uiuc.edu, 12/91 */
	(void)unlink(p->temp_drop);
#endif
	(void)fclose(p->drop);
    }

    return(pop_quit(p));
}



int pop_restore (p)
POP *p;
{
  FILE    *md;
  int     mfd;
  char    buffer[BBSIZE];
  int     nchar;
  long    offset;
  int     save_errno;

#ifdef DEBUG
    if (p->debug)
        pop_log(p,POP_DEBUG,"Performing maildrop restoration...");
#endif

    if (p->server_mode)
    {
#ifndef KEEP_TEMP_DROP        
        (void) unlink(p->temp_drop);
#endif        
        return(POP_SUCCESS);
    }

#ifdef DEBUG
    if (p->debug)
        pop_log(p,POP_DEBUG,"Opening mail drop \"%s\"", p->drop_name);
#endif

#ifdef MAILOCK
    if (maillock(p->user, 1))
        return pop_msg(p,POP_FAILURE,"maillock: '%s' (%d)",p->drop_name, errno);
#endif

    if ((mfd = open(p->drop_name,O_RDWR | O_CREAT,0660)) == -1 ||
        (md = fdopen(mfd, "r+")) == NULL)
    {
#ifdef MAILOCK
        mailunlock();
#endif
        return pop_msg(p,POP_FAILURE, standard_error, errno);
    }

    if (flock(mfd, LOCK_EX) == -1)
    {
        (void) fclose(md);
#ifdef MAILOCK
        mailunlock();
#endif
        return pop_msg(p,POP_FAILURE, "flock: '%s': %s (%d)", p->drop_name,
            (errno < sys_nerr) ? sys_errlist[errno]: "", errno);
    }

    (void)fseek(p->drop, 0, SEEK_END);
    offset = ftell(p->drop);

    /* Append any messages that may have arrived during the session 
         to the temporary mail drop */
    while ((nchar = read(mfd,buffer, BBSIZE)) > 0)
        if ( nchar != write(fileno(p->drop), buffer, nchar))
        {
            nchar = -1;
            break;
        }
        if ( nchar != 0)
        {
	    save_errno = errno;
            (void) fclose(md);
#ifdef MAILOCK
            mailunlock();
#endif
            (void)ftruncate(fileno(p->drop), (OFF_T)offset);
            (void)fclose(p->drop);
#ifdef EDQUOT
            if (save_errno == EDQUOT)
            {
                pop_msg(p, POP_FAILURE, 
                  "Overquota: appending messages to temporary drop (%d)",
                  errno);
            }
            else
#endif
            pop_msg(p, POP_FAILURE,
                "Error appending messages from mailspool to temporary drop (%d)",
                 errno);
       }

       fflush(md);
       rewind(md);
       (void)ftruncate(mfd, (OFF_T)0);
       (void)lseek(mfd, (OFF_T)0, L_SET);

       rewind(p->drop);
       (void)lseek(fileno(p->drop), (OFF_T)0, L_SET);
       
       while((nchar = read(fileno(p->drop), buffer, BBSIZE)) > 0)
            if (nchar != write(mfd, buffer, nchar))
            {
                nchar = -1;
                break;
            }
       if (nchar != 0)
       {
           save_errno = errno;
           (void) ftruncate(mfd, (OFF_T)0);
           (void) fclose(md);
#ifdef MAILOCK
           mailunlock();
#endif
           (void)fclose(p->drop);
#ifdef EDQUOT
           if (save_errno == EDQUOT)
               return pop_msg(p, POP_FAILURE,
                  "Overquota copying messages to Mailspool. Temp drop unchanged (%d)",
                   save_errno);
           else
#endif
           return pop_msg(p,POP_FAILURE, standard_error, errno);
       }

       (void) fclose(md);
#ifdef MAILOCK
       mailunlock();
#endif
       (void) ftruncate(fileno(p->drop), (OFF_T)0);
#ifndef KEEP_TEMP_DROP
       (void) unlink(p->temp_drop);
#endif
       (void) fclose(p->drop);

       return pop_quit(p);
}








