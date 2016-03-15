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
#include <sys/file.h>

#ifdef POPSCO
# define __SCO_WAIT3__
# include <fcntl.h>
#endif

#include <sys/wait.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <popper.h>

/*
 *  xmit:   POP XTND function to receive a message from 
 *          a client and send it in the mail
 */

FILE *tmp; /* File descriptor for temporary file */
char temp_xmit[MAXDROPLEN]; /* Name of the temporary filedrop */


pop_xmit (p)
POP     *   p;
{
    int			    tfn;		    


    /*  Create a temporary file into which to copy the user's message */

    strncpy(temp_xmit, POP_TMPXMIT, sizeof(temp_xmit));
#ifdef DEBUG
    if(p->debug)
        pop_log(p,POP_DEBUG,
            "Creating temporary file for sending a mail message \"%s\"",
                temp_xmit);
#endif
    if (((tfn=mkstemp(temp_xmit)) == -1) ||
	((tmp=fdopen(tfn, "w+")) == NULL)) {	/* failure, bail out	*/
        return (pop_msg(p,POP_FAILURE,
            "Unable to create temporary message file \"%s\", errno = %d",
                temp_xmit, errno));
    }

    /*  Receive the message */
#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"Receiving mail message");
#endif
    p->xmitting = 1;
    return(pop_msg(p,POP_SUCCESS,"Start sending message."));
}

pop_xmit_recv(p,buffer)
POP  *p;
char *buffer;
{

    /*  Look for initial period */
#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"Receiving: \"%.1000s\"",buffer);
#endif
    if (*buffer == '.') {
        /*  Exit on end of message */
        if (strcmp(buffer,".\r\n") == 0) {
            (void) fclose(tmp);
            p->xmitting = 0;
            pop_xmit_exec(p);
        }
        /* sendmail will not remove escaped .. */
	else if (buffer[1] == '.') 	(void)fputs (&buffer[1], tmp);
	else 	(void)fputs (buffer, tmp);
    } else    (void)fputs (buffer, tmp);
}


    
pop_xmit_exec(p)
POP *p;
{

#ifdef NeXT
    union	wait	    stat;
#else
    int			    stat;
#endif
    PID_T                   id, pid;
    
#ifdef DEBUG
    if(p->debug)pop_log(p,POP_DEBUG,"Forking for \"%s\"",MAIL_COMMAND);
#endif
    /*  Send the message */
    switch (pid = fork()) {
        case 0:
	    /*  Open the log file */
	    (void)closelog();
#ifdef SYSLOG42
	    (void)openlog(p->myname,0);
#else
	    (void)openlog(p->myname,LOG_PID,POP_FACILITY);
#endif
	    pop_log(p, POP_DEBUG,
		    "Pop transmit from \"%s\" on \"%s\"", p->user, p->client);

            (void)fclose (p->input);
            (void)fclose (p->output);       
            (void)close(0);
            if (open(temp_xmit,O_RDONLY,0) < 0)
		(void)_exit(1);
            (void)execl (MAIL_COMMAND,"send-mail","-t","-i","-oem",NULLCP);
            (void)_exit(1);
        case -1:
            if (!p->debug) (void)unlink (temp_xmit);
            return (pop_msg(p,POP_FAILURE, "Unable to execute \"%s\" (%d)",
							MAIL_COMMAND, errno));
        default:

#ifdef NeXT
            while((id = wait(&stat)) >=0 && id != pid);
#else
            id = waitpid(pid, &stat, 0);
#endif
            if (!p->debug) (void)unlink (temp_xmit);

#ifdef NeXT
            if (!WIFEXITED (stat))
#else
            if ((!WIFEXITED (stat)) || (WEXITSTATUS (stat) != 0))
#endif
                return (pop_msg(p,POP_FAILURE,"Unable to send message"));

            return (pop_msg (p,POP_SUCCESS,"Message sent successfully"));
    }
}

pop_xmit_clean(p)
POP *p;
{
    fclose(tmp);
    if (!p->debug) (void)unlink (temp_xmit);
}


