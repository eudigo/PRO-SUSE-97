#ifdef RADIUSERSORACLE

static text *oraname = (text *) "scott"; /* Nome do Usuario Oracle */
static text *orapass = (text *) "tiger"; /* Senha do Usuario */

/* Define comando SQL a ser usado no programa */

static text *seleuser = (text *) "SELECT userpass,servicetype,authtype FROM users WHERE username = :username";

#define TAM_MAX_AUTH_NAME 32

char username[TAM_MAX_AUTH_NAME];   /* Nome do Usuario a ser Autenticado */

#define TAM_MAX_AUTH_PASS 13

char userpass[TAM_MAX_AUTH_PASS];   /* Senha do Usuario */

#define TAM_SRVC_TYPE 1

char servicetype[TAM_SRVC_TYPE]; /* Tipo de Servico: 1 - Login-User
                                                     2 - Framed-User
                                                     3 - Dialback-Login-User
                                                     4 - Dialback-Framed-User */
char servicetypent[TAM_SRVC_TYPE+1];

#define TAM_AUTH_TYPE 1

char authtype[TAM_AUTH_TYPE]; /* Tipo de Autenticacao: 0 - Local
                                                       1 - Unix
                                                       2 - SecureID
                                                       3 - Crypt
                                                       4 - Reject */
char authtypent[TAM_AUTH_TYPE+1];

static OCIEnv    *envhp;  /* Environment */
static OCIServer *srvhp;  /* Server */
static OCIError  *errhp;  /* Error */
static OCISvcCtx *svchp;  /* Service */
static OCIStmt   *stmthp; /* Statement */

static OCIBind   *bindp = (OCIBind *) 0;   /* Bind (Input Variable) */

static OCIDefine *defnpass = (OCIDefine *) 0; /* Define (Output Variable) */
static OCIDefine *defnsvct = (OCIDefine *) 0; /* Define (Output Variable) */
static OCIDefine *defnautt = (OCIDefine *) 0; /* Define (Output Variable) */

static sword status;

#endif /* RADIUSERSORACLE */
