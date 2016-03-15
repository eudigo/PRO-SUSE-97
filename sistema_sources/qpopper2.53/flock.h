#ifndef HAVE_FLOCK
#ifndef _FLOCK_EMULATE_INCLUDED
#define _FLOCK_EMULATE_INCLUDED

#include <fcntl.h>

#if defined(F_SETLK) && defined(F_SETLKW)
# define FCNTL_FLOCK
#else
# define LOCKF_FLOCK
#endif /* F_SETLK && F_SETLKW */

/* These definitions are in <sys/file.h> on BSD 4.3 */

/*
 * Flock call.
 */
#define LOCK_SH		1	/* shared lock */
#define LOCK_EX		2	/* exclusive lock */
#define LOCK_NB		4	/* don't block when locking */
#define LOCK_UN		8	/* unlock */

extern int flock();

#endif /* _FLOCK_EMULATE_INCLUDED */
#endif /* HAVE_FLOCK */



















