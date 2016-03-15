#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "popper.h"

/*
 *  xlst:   POP XTND function to list headers from messages
 */

pop_xlst (p)
POP     *   p;
{
    char                    buffer[MAXLINELEN];     /*  Read buffer */
    MsgInfoList         *   mp;         /*  Pointer to message info list */
    int min,max;
    int len = strlen(p->pop_parm[2]);

    /*  Convert the first parameter into an integer */
    if (p->parm_count==3)
      min = max = atoi(p->pop_parm[3]);
    else
    {
      min = 1;
      max = p->msg_count;
    }


    /*  Is requested message out of range? */
    if ((min < 1) || (min > p->msg_count))
        return (pop_msg (p,POP_FAILURE,"Message %d does not exist.",min));

    /* yes, we can do this */
    pop_msg (p,POP_SUCCESS,"xlst command accepted; headers coming.");

    for (;min<=max;min++)
    {
      /*  Get a pointer to the message in the message list */
      mp = &p->mlp[min-1];

      /*  Is the message flagged for deletion? */
      if (mp->del_flag) continue;

      /*  Position to the start of the message */
      (void)fseek(p->drop, (OFF_T)mp->offset, 0);

      /*  Skip the first line (the sendmail "From" line) */
      (void)fgets (buffer,MAXMSGLINELEN,p->drop);

      /*  scan until we fine the header or a blank line */
      while (fgets(buffer,MAXMSGLINELEN,p->drop)) {
          if (*buffer=='\n') break;
          if (!strncasecmp(buffer,p->pop_parm[2],len))
          {
            /* found it! */
            fprintf(p->output,"%d ",min);
            pop_sendline (p,buffer);
            while (fgets(buffer,MAXMSGLINELEN,p->drop))
            {
              if (*buffer!=' ' && *buffer!='\t') break;
              pop_sendline(p,buffer);
            }
            break;
          }
      }
  }

  /*  "." signals the end of a multi-line transmission */
  (void)fputs(".\r\n",p->output);
  (void)fflush(p->output);

  return(POP_SUCCESS);
}

