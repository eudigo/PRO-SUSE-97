/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#ifndef HAVE_INDEX
# define index(s,c)		strchr(s,c)
#endif
#if HAVE_STRINGS_H
#include <strings.h>
#endif

#include "popper.h"

/*
 *  uidl:   POP UIDL function to list messages by message-ids
 */

pop_uidl (p)
POP     *   p;
{
    char                    buffer[MAXLINELEN];     /*  Read buffer */
    char		    *nl, *bp;
    MsgInfoList         *   mp;         /*  Pointer to message info list */
    int msg_id = 0, x;
    int len = 0;

    if (p->parm_count == 1) {
      len = strlen(p->pop_parm[1]);

      /*  Convert the parameter into an integer */
      msg_id = atoi(p->pop_parm[1]);
    }

    /*  Is requested message out of range? */
    if (len > 0 && msg_id == 0)
    {
      return (pop_msg (p,POP_FAILURE,"Parameter must be a number (range 1 to %d)", p->msg_count));
    }

    if (len > 0 && (msg_id < 1 || msg_id > p->msg_count))
      return (pop_msg (p,POP_FAILURE,
	      "Message out of range.  %d messages in mail drop.",p->msg_count));

    if (msg_id > 0) {
      /*  Get a pointer to the message in the message list */
      mp = &p->mlp[msg_id-1];

      if (mp->del_flag) {
        return (pop_msg (p,POP_FAILURE,
			    "Message %d has been marked for deletion.",msg_id));
      } else {

	sprintf(buffer, "%d %s", msg_id, mp->uidl_str);
        if (nl = index(buffer, NEWLINE)) *nl = 0;
	return (pop_msg (p,POP_SUCCESS, buffer));
      }
    } else {
	/* yes, we can do this */
	pop_msg (p,POP_SUCCESS,"uidl command accepted.");

	for (x = 1; x <= p->msg_count; x++)
	{
	    /*  Get a pointer to the message in the message list */
	    mp = &p->mlp[x-1];

	    /*  Is the message flagged for deletion? */
	    if (mp->del_flag) continue;

	    sprintf(buffer, "%d %s", x, mp->uidl_str);
	    pop_sendline(p, buffer);
        }
    }

    /*  "." signals the end of a multi-line transmission */
    (void)fputs(".\r\n",p->output);
    (void)fflush(p->output);

    return(POP_SUCCESS);
}

/*
 *  euidl:   POP EUIDL function to list messages by message-ids and adds
 *	     message size and From: header text as well.  This is to help
 *	     the Newton do some pre-filtering before downloading messages.
 */

char *
from_hdr(p, mp)
     POP         *p;
     MsgInfoList *mp;
{
  static char buf[MAXLINELEN];
  char *cp;

    fseek(p->drop, mp->offset, 0);
    while (fgets(buf, sizeof(buf), p->drop) != NULL) {
      if (buf[0] == '\n') break;    /* From header not found */
      if (!strncasecmp("From:", buf, 5)) {
	cp = index(buf, ':');
	while (*++cp && (*cp == ' ' || *cp == '\t'));
	return(cp);
      }
    }
    return("");
}

pop_euidl (p)
POP     *   p;
{
    char                    buffer[MAXLINELEN];     /*  Read buffer */
    char		    *nl, *bp;
    MsgInfoList         *   mp;         /*  Pointer to message info list */
    int msg_id = 0, x;
    int len = 0;

    if (p->parm_count == 1) {
      len = strlen(p->pop_parm[1]);

      /*  Convert the parameter into an integer */
      msg_id = atoi(p->pop_parm[1]);
    }

    /*  Is requested message out of range? */
    if (len > 0 && msg_id == 0)
    {
      return (pop_msg (p,POP_FAILURE,"Parameter must be a number (range 1 to %d)", p->msg_count));
    }

    if (len > 0 && (msg_id < 1 || msg_id > p->msg_count))
      return (pop_msg (p,POP_FAILURE,
	      "Message out of range.  %d messages in mail drop.",p->msg_count));

    if (msg_id > 0) {
      /*  Get a pointer to the message in the message list */
      mp = &p->mlp[msg_id-1];

      if (mp->del_flag) {
        return (pop_msg (p,POP_FAILURE,
			    "Message %d has been marked for deletion.",msg_id));
      } else {

	sprintf(buffer, "%d %s", msg_id, mp->uidl_str);
        if (nl = index(buffer, NEWLINE)) *nl = 0;
	sprintf(buffer, "%s %d %.128s", buffer, mp->length, from_hdr(p, mp));
	return (pop_msg (p,POP_SUCCESS, buffer));
      }
    } else {
	/* yes, we can do this */
	pop_msg (p,POP_SUCCESS,"uidl command accepted.");

	for (x = 1; x <= p->msg_count; x++)
	{
	    /*  Get a pointer to the message in the message list */
	    mp = &p->mlp[x-1];

	    /*  Is the message flagged for deletion? */
	    if (mp->del_flag) continue;

	    sprintf(buffer, "%d %s", x, mp->uidl_str);
	    if (nl = index(buffer, NEWLINE)) *nl = 0;	    
	    sprintf(buffer, "%s %d %.128s", buffer, mp->length, from_hdr(p, mp));
	    pop_sendline(p, buffer);
        }
    }

    /*  "." signals the end of a multi-line transmission */
    (void)fputs(".\r\n",p->output);
    (void)fflush(p->output);

    return(POP_SUCCESS);
}

