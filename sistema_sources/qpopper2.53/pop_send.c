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
#include <string.h>
#ifndef HAVE_BCOPY
# define bcopy(src,dest,len)	(void) (memcpy(dest,src,len))
# define bzero(dest,len)  	(void) (memset(dest, (char)NULL, len))
# define bcmp(b1,b2,n)		(void) (memcmp(b1,b2,n))
#endif
#ifndef HAVE_INDEX
# define index(s,c)		strchr(s,c)
# define rindex(s,c)		strrchr(s,c)
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#include <popper.h>

/* 
 *  send:   Send the header and a specified number of lines 
 *          from a mail message to a POP client.
 */

pop_send(p)
POP     *   p;
{
    MsgInfoList         *   mp;         /*  Pointer to message info list */
    register int            msg_num;
    register int            msg_lines;
    register int	    uidl_sent = 0;
    char                    buffer[MAXMSGLINELEN];

    /*  Convert the first parameter into an integer */
    msg_num = atoi(p->pop_parm[1]);

    /*  Is requested message out of range? */
    if ((msg_num < 1) || (msg_num > p->msg_count))
        return (pop_msg (p,POP_FAILURE,"Message %d does not exist.",msg_num));

    /*  Get a pointer to the message in the message list */
    mp = &p->mlp[msg_num-1];

    /*  Is the message flagged for deletion? */
    if (mp->del_flag)
        return (pop_msg (p,POP_FAILURE,
            "Message %d has been deleted.",msg_num));

    /*  If this is a TOP command, get the number of lines to send */
    if (strcmp(p->pop_command,"top") == 0) {
        /*  Convert the second parameter into an integer */
        msg_lines = atoi(p->pop_parm[2]) + 1;
	msg_lines = msg_lines > mp->body_lines ? mp->body_lines : msg_lines; 
    }
    else {
	/* NO_STATUS does not dirty the mailspool if a status is changed */
#ifndef NO_STATUS
        /*  Assume that a RETR (retrieve) command was issued */
	if (mp->retr_flag != TRUE)
	    p->dirty = 1;
#endif

        msg_lines = mp->body_lines;
        /*  Flag the message as retreived */
        mp->retr_flag = TRUE;
    }
    
    /*  Display the number of bytes in the message */
    pop_msg(p,POP_SUCCESS,"%u octets",mp->length);

    /*  Position to the start of the message */
    (void)fseek(p->drop, mp->offset, 0);

    /*  Skip the first line (the sendmail "From" or MMDF line) */
    (void)fgets (buffer,MAXMSGLINELEN,p->drop);

    /*  Send the header of the message followed by a blank line */
    while (fgets(buffer, MAXMSGLINELEN, p->drop)) {
	if (!strncasecmp(buffer, "Content-Length:", 15) ||
	    !strncasecmp(buffer, "X-UIDL:", 7)) {	/* Skip UIDLs */
	    continue;	/* Content-Length is MTA dependent, don't send to MUA */
	}

	if (!uidl_sent && (*buffer=='\n' || !strncasecmp(buffer,"Status:",7))) {
	    char uidl_buf[MAXMSGLINELEN];

	    sprintf(uidl_buf, "%s %s", "X-UIDL:", mp->uidl_str);
	    pop_sendline(p, uidl_buf);
	    uidl_sent++;
	}

	pop_sendline(p, buffer);

        /*  A single newline (blank line) signals the end of the header.
	    pop_sendline turns \n into \0 */

	if (*buffer == '\0')
	    break;

	if (hangup)
          return(pop_msg(p, POP_FAILURE, "SIGHUP or SIGPIPE flagged"));
    }

    /*  Send the message body */
    while(fgets(buffer, MAXMSGLINELEN, p->drop)) {

        /*  Decrement the lines sent (for a TOP command) */
        if (--msg_lines <= 0) break;

        pop_sendline(p,buffer);

	if (hangup)
          return(pop_msg(p, POP_FAILURE, "SIGHUP or SIGPIPE flagged"));
    }

    /*  "." signals the end of a multi-line transmission */
    /*  Must be an fputs because pop_sendline inserts an additional . */
    (void)fputs(".\r\n", p->output);
    (void)fflush(p->output);

    return(POP_SUCCESS);
}

/*
 *  sendline:   Send a line of a multi-line response to a client.
 */
pop_sendline(p,buffer)
POP         *   p;
char        *   buffer;
{
    char        *   bp;

    /*  Byte stuff lines that begin with the temirnation octet */
    if (*buffer == POP_TERMINATE) (void)fputc(POP_TERMINATE,p->output);

    /*  Look for a <NL> in the buffer */
    if (bp = index(buffer,NEWLINE)) *bp = 0;

    /*  Send the line to the client */
    (void)fputs(buffer,p->output);

#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"Sending line \"%.1000s\"",buffer);
#endif

    /*  Put a <CR><NL> if a newline was removed from the buffer */
    if (bp) (void)fputs ("\r\n",p->output);
}
