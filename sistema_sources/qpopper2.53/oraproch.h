#define POPUSERSORACLE /* Arquivo de Usuarios em Servidor Oracle */

#define LINUXSUSE      /* Ambiente: SuSE Linux 6.0 */

#ifdef POPUSERSORACLE

#include <oci.h>

extern int orauser_open(POP *p);

extern void orauser_close(POP *p);

extern int orauser_find(POP *p);

#endif /* POPUSERSORACLE */
