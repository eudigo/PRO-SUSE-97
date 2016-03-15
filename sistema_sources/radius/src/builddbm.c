/*
 *
 *	RADIUS
 *	Remote Authentication Dial In User Service
 *
 *
 *	Livingston Enterprises, Inc.
 *	4464 Willow Road
 *	Pleasanton, CA   94588
 *
 *	Copyright 1992-1996 Livingston Enterprises, Inc. All Rights Reserved.
 *
 *	This software is provided under license from Livingston
 *	Enterprises, Inc., the terms and conditions of which are set
 *	forth in a Software License Agreement that is contained in an
 *	End User Agreement contained in the product packaging, and
 *	electronically on the Livingston ftp site. This software may
 *	only be used in conjunction with Livingston (or Livingston
 *	authorized) products.  Livingston makes no warranties to any
 *	licensee concerning the applicability of the software to
 *	licensee's specific requirements or the suitability of the
 *	software for any intended use.  Licensee shall not remove,
 *	modify or alter any copyright and/or other proprietary rights
 *	notice and must faithfully reproduce all such notices on any
 *	copies or modifications to this software that it makes.
 *
 *	Livingston Enterprises, Inc. makes no representations about the
 *	suitability of this software for any purpose.  It is provided
 *	"as is" without express or implied warranty.
 *
 */

static char sccsid[] =
"$Id: builddbm.c,v 1.16 1997/06/05 08:20:45 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<netinet/in.h>

#include	<stdio.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>
#include        <errno.h>

#include	"radius.h"

#if defined(NDBM)
# include	<ndbm.h>
# include	<fcntl.h>
#else	/* not NDBM */
# include	<dbm.h>
#endif	/* NDBM */

extern int      errno;

char		*progname;
char		*radius_dir;
char		*radius_log;
int		debug_flag = 0;
int		radius_dbm = 1;	/* needed for version() */

#define FIND_MODE_NAME	0
#define FIND_MODE_REPLY	1
#define FIND_MODE_SKIP	2
#define FIND_MODE_FLUSH	3

FILE		*userfd;
int		default_count;	/* number of DEFAULT entries found */
int		linenum;	/* line in users file, for error messages */
int		lineuser;	/* line current user started on, for error messages */

int
main(argc,argv)
int argc;
char **argv;
{
	char	name[128];
	char	content[1024];
	char	argval;
	int	usercount;	/* number of users stored */
	int	errcount;	/* number of users not stored, usually dups */ 
	datum	named;
	datum	contentd;
#ifdef	NDBM
	DBM	*db;
	int	xx;
#else	/* not NDBM */
	int	fd;
#endif	/* NDBM */
	int	user_read();
	void	usage();
	void	version();


	/* Parse arguments */

	progname = *argv++;
	argc--;
	radius_dir = ".";	/* default looks in current directory */

	while(argc) {

		if(**argv != '-') {
			usage();
		}

		argval = *(*argv + 1);
		argc--;
		argv++;

		switch(argval) {

		case 'd':
			if(argc == 0) {
				usage();
			}
			radius_dir = *argv;
			argc--;
			argv++;
			break;

		case 'h':
			usage();
			break;

                case 'l':       /* change logging from syslog */
                        if(argc == 0) {
                                usage();
                        }
                        radius_log = *argv;
                        argc--;
                        argv++;
                        break;

		case 'v':
			version();
			break;

		case 'x':
			debug_flag = 1;
			break;
		
		default:
			usage();
			break;
		}
	}


        if (debug_flag) {
                if (radius_log == (char *)NULL) {
                        /*
                         * for backward compatibility
                         * send messages to users tty
                         */
                        radius_log = "/dev/tty";
                } else if (strcmp(radius_log, "syslog") == 0) {
                        /*
                         * allow user to override backward compatibility
                         * and send debug to syslog
                         */
                        radius_log = (char *)NULL;
                }
        }


	/* Open Database */

	errno = 0;
	if (chdir(radius_dir) < 0) {
		fprintf(stderr, "%s: unable to change to directory %s - %s\n",progname,sys_errlist[errno]);
		exit(-1);
	}
#ifdef NDBM
	errno = 0;
	if((db = dbm_open("users", O_RDWR | O_CREAT | O_TRUNC, 0600))
							== (DBM *)NULL) {
		fprintf(stderr, "%s: dbm_open failed - %s\n", progname, sys_errlist[errno]);
		exit(-1);
	}

#else /* not NDBM */

	errno = 0;
	if((fd = open("users.pag", O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0) {
		fprintf(stderr,"%s: Couldn't open users.pag for writing - %s\n",progname,sys_errlist[errno]);
		exit(-1);
	}
	close(fd);

	errno = 0;
	if((fd = open("users.dir", O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0) {
		fprintf(stderr,"%s: Couldn't open users.dir for writing - %s\n",progname,sys_errlist[errno]);
		exit(-1);
	}
	close(fd);

	errno = 0;
	if(dbminit("users") != 0) {
		fprintf(stderr, "%s: dbminit failed - %s\n",progname, sys_errlist[errno]);
		exit(-1);
	}

#endif /* not NDBM */


	/* read through users file putting entries into database */

	default_count = 0;
	errcount = 0;
	usercount = 0;
	while(user_read(name, content) == 0) {
		named.dptr = name;
		named.dsize = strlen(name);
		contentd.dptr = content;
		contentd.dsize = strlen(content);
#ifdef NDBM
		if((xx = dbm_store(db, named, contentd, DBM_INSERT)) != 0)
#else /* not NDBM */
		if(store(named, contentd) != 0)
#endif /* not NDBM */

		{
			fprintf(stderr, "%s: could not store %s from line %d, check for duplicate\n", progname,name,lineuser);
			errcount++;
		} else {
			usercount++;
		}
	}
#ifdef NDBM
	dbm_close(db);
#else /* not NDBM */
	dbmclose();
#endif /* not NDBM */


	/* report results */

	printf("%s: %d user%s stored in DBM file",progname,usercount,usercount==1?"":"s");
	if (default_count > 0) {
		printf(" including %d DEFAULT entries\n",default_count);
	} else {
		printf("\n");
	}
	if (errcount > 0) {
		printf("%s: %d user%s not written to DBM file, check for duplicates\n",progname,errcount,errcount==1?"":"s");
	}
	exit(0);
}

/*************************************************************************
 *
 *	Function: user_read
 *
 *	Purpose: Return each user in the database - name is key content
 *		 is 2 strings - check values, and reply values seperated
 *		 by a newline.
 *
 *************************************************************************/

int
user_read(name, content)
char	*name;
char	*content;
{
	static char	buffer[256];
	char		*ptr;
	int		mode;
	char 		*base_name = name;
	extern int	linenum;
	extern int	lineuser;

	/*
	 * Open the user table
	 */
	if(userfd == (FILE *)NULL) {
		if((userfd = fopen(RADIUS_USERS, "r")) == (FILE *)NULL) {
			fprintf(stderr, "%s: could not open %s for reading\n",
					progname, buffer);
			exit(-1);
		}
		linenum = 0;
		*buffer = '\0';
	}

	mode = FIND_MODE_NAME;

	while(*buffer || (fgets(buffer, sizeof(buffer), userfd) != (char *)NULL)) {
		linenum++;	/* track line number for error messages */
		/* skip comments */
		if (*buffer == '#') {
			*buffer = '\0';
			continue;
		}
		if(mode == FIND_MODE_NAME) {
			/*
			 * Find the entry starting with the users name
			 */
			if(*buffer != '\t' && *buffer != ' '
						&& *buffer != '\n'
						&& *buffer != '\r') {
				ptr = buffer;
				while(*ptr != ' ' && *ptr != '\t' &&
								*ptr != '\0') {
					*name++ = *ptr++;
				}
				*name = '\0';
				if(*ptr == '\0') {
					continue;
				}
				if (strncmp(base_name, "DEFAULT",7) == 0) {
					if (default_count > 0) {
						sprintf(base_name, "DEFAULT%d",
							default_count);
					}  else {
						strcpy(base_name, "DEFAULT");
					}
					default_count++;
				}
				ptr++;
				while(*ptr == ' ' || *ptr == '\t') {
					ptr++;
				}
				strcpy(content, ptr);
				content += strlen(content);
				mode = FIND_MODE_REPLY;
				lineuser = linenum;
			}
			*buffer = '\0';
		}
		else {
			if(*buffer == ' ' || *buffer == '\t') {
				ptr = buffer;
				while(*ptr == ' ' || *ptr == '\t') {
					ptr++;
				}
				strcpy(content, ptr);
				content += strlen(content) - 1;
				/* strip trailing white space and comma */
				while(*content == ' ' || *content == '\t' ||
					*content == '\n' || *content == ',' ) {
					content--;
				}
				content++;
				*content = ',';
				content++;
				*content = '\0';
				*buffer = '\0';
			}
			else {
				/* We are done, leave buffer for next call */
				if (*buffer == '\n') {
					*buffer = '\0';
				} else {
					linenum--;
				}
				if(*(content - 1) == ',') {
					*(content-1) = '\0';
				}
				return(0);
			}
		}
	}
	if (mode == FIND_MODE_REPLY) {	/* return last entry */
		*buffer = '\0';
		if(*(content - 1) == ',') {
			*(content-1) = '\0';
		}
		return (0);
	}
	fclose(userfd);
	return(-1);
}


/*************************************************************************
 *
 *	Function: usage
 *
 *	Purpose: Display the syntax for starting this program.
 *
 *************************************************************************/

void
usage()
{
	fprintf(stderr, "Usage: %s", progname);
	fprintf(stderr, " [-d <db_dir>]");
	fprintf(stderr, " [-h]");
	fprintf(stderr, " [-v]");
	fprintf(stderr, " [-x]\n");
	exit(-1);
}
