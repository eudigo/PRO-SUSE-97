/*
 * Copyright (c) 1997 by Qualcomm Incorporated.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#include <popper.h>

/* 
 *  auth  : optional command ; Not supported.
 */

int pop_auth (p)
POP     *   p;
{
    /*  Tell the user that this command is not supported */
    return (pop_msg(p,POP_FAILURE,"This command is not supported yet"));
}



