/*
 *
 *	RADIUS
 *	Remote Authentication Dial In User Service
 *
 *
 *	Livingston Enterprises, Inc.
 *	6920 Koll Center Parkway
 *	Pleasanton, CA   94566
 *
 *	Copyright 1992-1996 Livingston Enterprises, Inc. All Rights Reserved.
 *
 *	This software source code is provided under license from
 *	Livingston Enterprises, Inc., the terms and conditions of which
 *	are set forth in an End User Agreement that is contained in
 *	both the product packaging, and electronically on the
 *	Livingston ftp site. This software may only be used in
 *	conjunction with Livingston (or Livingston authorized)
 *	products.  Livingston makes no warranties to any licensee
 *	concerning the applicability of the software to licensee's
 *	specific requirements or the suitability of the software for
 *	any intended use. Licensee shall not remove, modify or alter
 *	any copyright and/or other proprietary rights notice and must
 *	faithfully reproduce all such notices on any copies or
 *	modifications to this software that it makes.
 *	
 *	Livingston Enterprises, Inc. makes no representations about
 *	the suitability of this software for any purpose.  It is
 *	provided "as is" without express or implied warranty.
 *
 */

/*
 *	$Id: conf.h,v 1.6 1997/05/15 06:49:45 cdr Exp $ 12/22/94
 */

#if !defined(NDBM)
#define	NDBM		/* all supported platforms use NDBM now */
#endif

#if defined(__alpha) && defined(__osf__)
typedef unsigned int	UINT4;
#else
typedef unsigned long	UINT4;
#endif

#if defined(unixware) || defined(sys5) || defined(M_UNIX)
#include        <string.h>
#else   /* unixware */
#include        <strings.h>
#endif  /* unixware */

#if defined(bsdi)
#include        <machine/inline.h>
#include        <machine/endian.h>
#else	/* bsdi */
#include        <malloc.h>
/* extern char * sys_errlist[]; */
#endif	/* bsdi */

#if defined(aix)
#include	<sys/select.h>
#endif	/* aix 	*/

#if !defined(NDBM)
#define dbm_fetch(db,x) fetch(x)
#define dbm_close(db)   dbm_close()
#endif

/* #define PASSCHANGE	* comment this out to turn off radpass */

#define RADIUSERSORACLE /* Arquivo de Usuarios em Servidor Oracle */

#define LINUXSUSE       /* Ambiente: SuSE Linux 6.0 */
