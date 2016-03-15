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
 *	$Id: users.h,v 1.5 1997/05/15 06:49:53 cdr Exp $ %Y%
 */

# ifdef NDBM               /* Arquivo de Usuarios em Formato NDBM */
#	include	<fcntl.h>
#	include	<ndbm.h>
# else /* not NDBM */      /* Arquivo de Usuarios em Formato DBM */
#	include	<dbm.h>
# endif /* NDBM */

# define USER_DESC	DBM

extern USER_DESC *user_open  ( void);
extern void       user_close ( USER_DESC *user_desc);
extern int        user_find  ( char *req_name,
                               char *auth_name,
                               VALUE_PAIR **check_pairs,
                               VALUE_PAIR **reply_pairs,
                               USER_DESC *userfd );

#ifdef RADIUSERSORACLE /* Arquivo de Usuarios em Servidor Oracle */

#include <oci.h>

extern int  orauser_open  ( void );
extern void orauser_close ( void );
extern int  orauser_find  ( char *req_name,
                            char *auth_name,
                            VALUE_PAIR **check_pairs,
                            VALUE_PAIR **reply_pairs );

#endif /* RADIUSERSORACLE */
