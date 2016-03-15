/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

/*
 * bullcopy
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <pwd.h>

#if HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#ifndef HAVE_BCOPY
# define bcopy(src,dest,len)	(void) (memcpy(dest,src,len))
#endif

#include <ctype.h>
#include <sys/types.h>
#include <popper.h>
#include <flock.h>

static int sequence = 0;
static time_t timestamp;
static char *errmesg = "Unable to copy Bulletin %s to pop dropbox %s (%d)";

typedef struct _file_list file_list;

struct _file_list {
    char *bull_name;
    long bull_value;
    file_list *next;
};
/*
 * List in the ascending order of the bulletin value
 */
file_list *
insert_list(p, head, name)
POP *p;
file_list *head;
char *name;
{
    long new_bull;
    file_list *new_rec, *current;

    new_bull = atol(name);

    new_rec = (file_list *)malloc(sizeof(file_list));
    new_rec->next = NULL;
    new_rec->bull_value = new_bull;
    new_rec->bull_name = (char *)malloc(strlen(name) + 1);
    strcpy(new_rec->bull_name, name);

    current = head;
	
    if (!head) {
	return(new_rec);
    } else {
	if (head->bull_value > new_bull) {
	    new_rec->next = head;
	    head = new_rec;
	    return(head);
	}
    }

    while (current->next) {
	if (current->next->bull_value > new_bull)
	    break;
	current = current->next;
    }

    new_rec->next = current->next;
    current->next = new_rec;
    return(head);
}

/*
 *  pop_bull: Append any new bulletins to the end of the user's
 *  temporary maildrop.
 */

pop_bull (p, pwp)                  
POP *p;
struct passwd *pwp;
{
   char popBullName[256];
   FILE *popBull;
   DIR *dirp;
   file_list *list = NULL;
   struct dirent *dp;
   long maxBullNumber = 0;
   long bullNumber;
   long lastBullSent;
   char buffer[MAXMSGLINELEN];
   int res;
   int bullcount = 0;
   int save_count;
#ifdef BULLDB
   datum bull_count;
   datum name;
#endif

   /* Construct full path name of .popbull file. */
   sprintf(popBullName, "%s/.popbull", pwp->pw_dir);

   /* Scan bulletin directory and compute the maximum current
      bulletin number. */
   dirp = opendir(p->bulldir);
   if (dirp == NULL) {
      pop_log(p, POP_PRIORITY,
         "Unable to open bulletin directory '%s'. (%d)", p->bulldir, errno);
      return POP_FAILURE;
   }

   while ((dp = readdir(dirp)) != NULL) {
      if (!isdigit(*dp->d_name)) continue;
      bullNumber = atol(dp->d_name);
      if (bullNumber > maxBullNumber) maxBullNumber = bullNumber;
      list = insert_list(p, list, dp->d_name);
   }
   closedir(dirp); 

   timestamp = time(0);

   /* Open the user's .popbull file and read the number of the last
      bulletin sent to this user. If the file doesn't exist, create
      it and seed it with the current max bulletin number. Note that
      new users do not get sent old bulletins. */

#ifdef BULLDB
   name.dptr = p->user;
   name.dsize = strlen(p->user) + 1;
#ifdef GDBM
   bull_count = gdbm_fetch(p->bull_db, name);
#else
   bull_count = dbm_fetch(p->bull_db, name);
#endif
   if (bull_count.dptr == NULL) {
      /* If the database does not have a value, check the users .popbull
       * file.  If it's not empty, then use the value there, otherwise,
       * create a new value.
       */
       popBull = fopen(popBullName, "r");
       if ((popBull == NULL) || (fgets(buffer,MAXMSGLINELEN,popBull) == NULL) ||
            !isdigit(*buffer)) {
	   if ((lastBullSent = (maxBullNumber - NEWBULLCNT)) < 0)
	      lastBullSent = 0;
       } else {
	   lastBullSent = atol(buffer);
       }
       if (popBull)
	   fclose(popBull);

       bull_count.dptr = (char *)&lastBullSent;
       bull_count.dsize = sizeof(lastBullSent);

#ifdef GDBM
       gdbm_store(p->bull_db, name, bull_count, GDBM_REPLACE);
#else
       /* Block while waiting for a lock to update the entry */
       if (flock(dbm_dirfno(p->bull_db), LOCK_EX) == -1)
	  return(pop_msg(p, POP_FAILURE, "Bulletin database lock failed"));
       dbm_store(p->bull_db, name, bull_count, DBM_REPLACE);
#endif
       flock(dbm_dirfno(p->bull_db), LOCK_UN);
       if (NEWBULLCNT <= 0) {
	     return POP_SUCCESS;
       }
   } 
   else
	bcopy(bull_count.dptr, &lastBullSent, bull_count.dsize);
#else
   popBull = fopen(popBullName, "r");
   if ((popBull == NULL) || (fgets(buffer, MAXMSGLINELEN, popBull) == NULL) ||
       !isdigit(*buffer)) {
       if (popBull != NULL) fclose(popBull);
       popBull = fopen(popBullName, "w");
       if (popBull == NULL) {
	   pop_log(p, POP_PRIORITY, "Unable to create .popbull file (%d)", errno);
	   return POP_FAILURE;
       }

      if ((lastBullSent = (maxBullNumber - NEWBULLCNT)) < 0)
	  lastBullSent = 0;

      fprintf(popBull, "%ld\n", lastBullSent);
      fclose(popBull);

      if (NEWBULLCNT <= 0)
	  return POP_SUCCESS;
   } 
   else lastBullSent = atol(buffer);
#endif /* BULLDB */

   /* If there aren't any new bulletins for this user, return. */
   if (lastBullSent >= maxBullNumber) {
       return POP_SUCCESS;
   }

   (void) chmod(popBullName, 0600);   /* Only needs to be read by me */


   /* Append the new bulletins to the end of the user's temporary 
      mail drop. */

   res = POP_SUCCESS;
   save_count = p->msg_count;

   while (list) {
       if (list->bull_value > lastBullSent)
           if ((res = CopyOneBull(list->bull_name, p)) != POP_SUCCESS) {
	      p->msg_count = save_count;
	      break;
	   }
	list = list->next;
   }

   if (res == POP_SUCCESS) {
#ifdef BULLDB
       bull_count.dptr = (char *)&maxBullNumber;
       bull_count.dsize = sizeof(maxBullNumber);
       if (flock(dbm_dirfno(p->bull_db), LOCK_EX) == -1) {
	   p->msg_count = save_count;
	  return(pop_msg(p, POP_FAILURE, "Bulletin database lock failed"));
       }
#ifdef GDBM
       gdbm_store(p->bull_db, name, bull_count, GDBM_REPLACE);
#else
       dbm_store(p->bull_db, name, bull_count, DBM_REPLACE);
#endif
       flock(dbm_dirfno(p->bull_db), LOCK_UN);
#else
       /* Update the user's .popbull file. */
       popBull = fopen(popBullName, "w");
       if (popBull == NULL) {
	   p->msg_count = save_count;
	   pop_log(p, POP_PRIORITY, "Unable to open .popbull file. (%d)", errno);
	   return POP_FAILURE;
       }
       fprintf(popBull, "%ld\n", maxBullNumber);
       fclose(popBull); 
#endif
    }

   return(res);
}

extern int newline;

/*
 *  CopyOneBull: Append a single bulletin file to the end of the
 *  temporary maildrop file.
 */

CopyOneBull(name, p)
char *name;
POP *p;
{
    FILE *bull;
    char buffer[MAXMSGLINELEN];
    int in_header = 1;
    int first_line = 1;
    int nchar;
    int msg_num;
    int receivedhdrs = 0;
    char bullName[256];
    MsgInfoList *mp;			/* Pointer to message info list */

    msg_num = p->msg_count;
    p->msg_count = (((p->msg_count - 1) / ALLOC_MSGS) + 1) * ALLOC_MSGS;

    sprintf(bullName, "%s/%s", p->bulldir, name);
    bull = fopen(bullName, "r");
    if (bull == NULL) {
       pop_log(p, POP_PRIORITY,
	      "Unable to open bulletin file %s (%d)", name, errno);
       return POP_FAILURE;
    }

    newline = 1;

    if ((fgets(buffer, MAXMSGLINELEN, bull) != NULL) && !(isfromline(buffer))) {
	pop_log(p, POP_PRIORITY,
	    "Bulletin %s does not start with a valid \"From \" separator",name);
	fclose(bull);
	return POP_FAILURE;
    }

    /* Just and appended message, no Status or UIDL updates here */
#ifndef NO_STATUS
    p->dirty = 1;
#endif

    mp = p->mlp + msg_num - 1;

    if (++msg_num > p->msg_count) {
	p->mlp=(MsgInfoList *) realloc(p->mlp,
	    (p->msg_count += ALLOC_MSGS)*sizeof(MsgInfoList));
	if (p->mlp == NULL) {
	    p->msg_count = 0;
	    return pop_msg (p,POP_FAILURE,
	     "Bull: Can't build message list for '%s': Out of memory",
	     p->user);
	}

	mp = p->mlp + msg_num - 2;
    }

    ++mp;
    mp->number = msg_num;
    mp->length = 0;
    mp->lines = 0;
    mp->body_lines = 0;
    mp->offset = ftell(p->drop);
    mp->del_flag = FALSE;
    mp->retr_flag = FALSE;

    if (fputs(buffer, p->drop) == EOF) {
	return(pop_msg(p, POP_FAILURE, errmesg, name, p->temp_drop, errno));
    }

    p->drop_size += strlen(buffer);
    mp->lines++;

    sprintf(buffer, "X-UIDL: %ld.%05d\n", timestamp, sequence);
    if (fputs(buffer, p->drop) == EOF) {
	return(pop_msg(p, POP_FAILURE, errmesg, name, p->temp_drop, errno));
    }

    mp->length += strlen(buffer);
    p->drop_size += strlen(buffer);
    mp->lines++;

#ifdef DEBUG
    if(p->debug)
	pop_log(p,POP_DEBUG,
		"Bull msg %d being added to list, offset %d",
		mp->number, mp->offset);
#endif
    sprintf(buffer, "%ld.%05d\n", timestamp, sequence++);

    mp->uidl_str = (char *)strdup(buffer);

    first_line = 0;

    while (fgets(buffer, MAXMSGLINELEN, bull) != NULL) {
	nchar = strlen(buffer);

	if (in_header) { /* Header */
	    if (!strncasecmp(buffer, "X-UIDL:", 7)) {
		continue;	/* Skip any UIDLs */

	    } else if (strncasecmp(buffer, "To:", 3) == 0) {
		sprintf(buffer,"To: %s@%s\n", p->user, p->myhost);
		nchar = strlen(buffer);

	    } else if (strncasecmp(buffer, "Status:", 7) == 0) {
		continue;

	    } else if (*buffer == '\n') {
		in_header = 0;
		mp->body_lines = 0;	/* Reset to zero when in the body */
	    }
	}

	mp->length += nchar;
	p->drop_size += nchar;
	mp->lines++;
	mp->body_lines++;

	if (fputs(buffer, p->drop) == EOF) {
	    return(pop_msg(p,POP_FAILURE,errmesg,name,p->temp_drop, errno));
	}
    }

    fflush(p->drop);

    p->msg_count = msg_num;
    fclose(bull);

#ifdef DEBUG
    if(p->debug && msg_num != 1)
	pop_log(p,POP_DEBUG,
	"Bull msg %d uidl %s at offset %d is %d octets long and has %u lines.",
	mp->number,mp->uidl_str,mp->offset,mp->length,mp->lines);
#endif

    return POP_SUCCESS;
}









