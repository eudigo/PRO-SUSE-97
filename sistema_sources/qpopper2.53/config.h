/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if `sys_siglist' is declared by <signal.h>.  */
#define SYS_SIGLIST_DECLARED 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* #undef DEBUG */
/* #undef APOP */
/* #undef POPUID */
#define GDBM 1
/*
 * Define this macro if your system supports special authorization
 * mechanisms like shadow passowrds or special crypt programs.
 */
#define AUTH_SPECIAL 1
#define MAIL_COMMAND "/usr/sbin/sendmail"
#define POP_MAILDIR "/usr/spool/mail"
/* #undef BULLDIR */
#define POP_DROP "/usr/spool/mail/.%s.pop"
#define POP_TMPDROP "/usr/spool/mail/tmpXXXXXX"
#define POP_TMPXMIT "/usr/spool/mail/xmitXXXXXX"
/* #undef MAILOCK */
/* 
 * Define to run in SERVER_MODE. Take a look at
 * <http://www.eudora.com/freeware/qpop_faq.html>
 * to know more about SERVER_MODE.
 */
#define SERVER_MODE 1

/* The number of bytes in a unsigned long int.  */
#define SIZEOF_UNSIGNED_LONG_INT 4

/* Define if you have the bcopy function.  */
#define HAVE_BCOPY 1

/* Define if you have the flock function.  */
#define HAVE_FLOCK 1

/* Define if you have the ftruncate function.  */
#define HAVE_FTRUNCATE 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getusershell function.  */
#define HAVE_GETUSERSHELL 1

/* Define if you have the index function.  */
#define HAVE_INDEX 1

/* Define if you have the pw_encrypt function.  */
/* #undef HAVE_PW_ENCRYPT */

/* Define if you have the set_auth_parameters function.  */
/* #undef HAVE_SET_AUTH_PARAMETERS */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the <dbm.h> header file.  */
#define HAVE_DBM_H 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <gdbm.h> header file.  */
#define HAVE_GDBM_H 1

/* Define if you have the <maillock.h> header file.  */
/* #undef HAVE_MAILLOCK_H */

/* Define if you have the <ndbm.h> header file.  */
#define HAVE_NDBM_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <net/errno.h> header file.  */
/* #undef HAVE_NET_ERRNO_H */

/* Define if you have the <netinet/in.h> header file.  */
#define HAVE_NETINET_IN_H 1

/* Define if you have the <prot.h> header file.  */
/* #undef HAVE_PROT_H */

/* Define if you have the <shadow.h> header file.  */
#define HAVE_SHADOW_H 1

/* Define if you have the <strings.h> header file.  */
#define HAVE_STRINGS_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/fcntl.h> header file.  */
#define HAVE_SYS_FCNTL_H 1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/netinet/in.h> header file.  */
/* #undef HAVE_SYS_NETINET_IN_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/security.h> header file.  */
/* #undef HAVE_SYS_SECURITY_H */

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/syslog.h> header file.  */
#define HAVE_SYS_SYSLOG_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <sys/unistd.h> header file.  */
#define HAVE_SYS_UNISTD_H 1

/* Define if you have the <syslog.h> header file.  */
#define HAVE_SYSLOG_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the cposix library (-lcposix).  */
/* #undef HAVE_LIBCPOSIX */
