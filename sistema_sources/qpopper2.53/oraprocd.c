#ifdef POPUSERSORACLE /* Definicoes */

char orapopuser[] = "popmgr";

static text *oraname = (text *) "gbl";       /* Nome do Usuario Oracle */
static text *orapass = (text *) "oronsuse";  /* Senha do Usuario */

/* Define comando SQL a ser usado no programa */

static text *seleuser = (text *) "SELECT password,bloqueado FROM pop WHERE userid = :userid";

#define TAM_MAX_AUTH_NAME 32

char userid[TAM_MAX_AUTH_NAME];         /* Nome do Usuario a ser Autenticado */

#define TAM_MAX_AUTH_PASS 32

char password[TAM_MAX_AUTH_PASS];       /* Senha do Usuario */

#define TAM_BLOQUEADO 1

char bloqueado[TAM_BLOQUEADO]; /* Status do Usuario: 0 - Liberado
                                                     1 - Bloqueado */

char bloqueadont[TAM_BLOQUEADO+1];

static OCIEnv    *envhp;  /* Environment */
static OCIServer *srvhp;  /* Server */
static OCIError  *errhp;  /* Error */
static OCISvcCtx *svchp;  /* Service */
static OCIStmt   *stmthp; /* Statement */

static OCIBind   *bindp = (OCIBind *) 0;   /* Bind (Input Variable) */

static OCIDefine *defnpass = (OCIDefine *) 0; /* Define (Output Variable) */
static OCIDefine *defnbloq = (OCIDefine *) 0; /* Define (Output Variable) */

static sword status;

#endif /* POPUSERSORACLE */
