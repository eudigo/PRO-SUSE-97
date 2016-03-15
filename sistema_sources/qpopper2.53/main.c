#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <varargs.h>
#include <signal.h>

#define SERV_TCP_PORT 110

char *pname = NULL;
pid_t pgrpid;
int clean=0;

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];
void reaper();
void cleanup();

main(argc, argv)
int argc;
char *argv[];
{
	int sockfd, newsockfd, clilen, childpid;
	struct sockaddr_in cli_addr, serv_addr;

	pname = argv[0];

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_dump("Server : Can't open stream socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_TCP_PORT);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		err_dump("Server : Can't bind local address");
	
	listen(sockfd, 5);

	signal(SIGCHLD, reaper);
	signal(SIGTERM, cleanup);

	pgrpid = setpgrp();

	for(;;) {

		if(clean)
		{
			close(sockfd);
			exit(0);
		}
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if(newsockfd < 0)
		{
			if(errno == EINTR)
				continue;
			err_dump("Server: accept error");
		}
		
		if((childpid = fork()) < 0)
			err_dump("Server : fork error");
		
		else if(childpid == 0) {
			close(sockfd);
			dup2(newsockfd,0);
			dup2(newsockfd,1);
			dup2(newsockfd,2);
			close(newsockfd);
			printf("Starting the popper server\n");
			popper(argc,argv);
			exit(0);
		}

		close(newsockfd);
	}
}

err_dump(va_alist)
va_dcl
{
	va_list args;
	char *fmt;
	va_start(args);
	if(pname != NULL)
		fprintf(stderr, "%s: ", pname);
	fmt = va_arg(args, char *);
	vfprintf(stderr, fmt, args);
	va_end(args);

	my_perror();

	fflush(stdout);
	fflush(stderr);

	abort();
	exit(1);
}

my_perror()
{
	char *sys_err_str();
	fprintf(stderr, " %s\n", sys_err_str());
}

char *sys_err_str()
{
	static char msgstr[200];
	if(errno != 0)
		if(errno > 0 && errno < sys_nerr)
			sprintf(msgstr,"(%s)", sys_errlist[errno]);
		else
			sprintf(msgstr,"(errno = %d)", errno);
	else
		msgstr[0] = '\0';
	
	return(msgstr);
}

void reaper()
{
	int stts;
	while(wait3(&stts, WNOHANG, (struct rusage *)0) >= 0);
	signal(SIGCHLD, reaper);
}

void cleanup()
{
	killpg(pgrpid,SIGHUP);
	clean = 1;
}

