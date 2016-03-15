#ifdef POPUSERSORACLE /* Funcoes */

/*************************************************************************
*
*	Function: orauser_error
*
*	Purpose: Grava no arquivo de log o erro informado pelo Oracle. 
*
*************************************************************************/

void
orauser_error(p,errhp, status)
POP *p;
OCIError *errhp;
sword    status;
{
   text errbuff[512];
   sb4  errcode = 0;

   switch (status)
   {
   case OCI_SUCCESS:
      break;
   case OCI_SUCCESS_WITH_INFO:
      pop_log(p,POP_FAILURE,"Error - OCI_SUCCESS_WITH_INFO\n");
      break;
   case OCI_NEED_DATA:
      pop_log(p,POP_FAILURE,"Error - OCI_NEED_DATA\n");
      break;
   case OCI_NO_DATA:
      pop_log(p,POP_FAILURE,"Error - OCI_NO_DATA\n");
      break;
   case OCI_ERROR:
      (void) OCIErrorGet((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode, errbuff, (ub4) sizeof(errbuff), OCI_HTYPE_ERROR);
      pop_log(p,POP_FAILURE,"Error - OCI_ERROR - %.*s\n",512,errbuff);
      break;
   case OCI_INVALID_HANDLE:
      pop_log(p,POP_FAILURE,"Error - OCI_INVALID_HANDLE\n");
      break;
   case OCI_STILL_EXECUTING:
      pop_log(p,POP_FAILURE,"Error - OCI_STILL_EXECUTING\n");
      break;
   case OCI_CONTINUE:
      pop_log(p,POP_FAILURE,"Error - OCI_CONTINUE\n");
      break;
   default:
      pop_log(p,POP_FAILURE,"Error - OCI_UNKNOWN_ERROR\n");
      break;
   }
}

/*************************************************************************
*
*	Function: orauser_close
*
*	Purpose: Libera Handles Alocados Previamente e Desconecta Servidor
*
*************************************************************************/

void
orauser_close(p)
POP *p;
{
   /* Libera Handle de Statement */
   if (stmthp)
      orauser_error(p,errhp, OCIHandleFree((dvoid *) stmthp, OCI_HTYPE_STMT));

   /* Desconecta do Servidor Oracle */
   if (errhp)
      (void) OCIServerDetach(srvhp, errhp, OCI_DEFAULT);

   /* Libera Handle de Servidor */
   if (srvhp)
      orauser_error(p,errhp, OCIHandleFree((dvoid *) srvhp, OCI_HTYPE_SERVER));

   /* Libera Handle de Servico */
   if (svchp)
      (void) OCIHandleFree((dvoid *) svchp, OCI_HTYPE_SVCCTX);

   /* Libera Handle de Erro */
   if (errhp)
      (void) OCIHandleFree((dvoid *) errhp, OCI_HTYPE_ERROR);
}

/*************************************************************************
*
*	Function: orauser_open
*
*	Purpose: Inicializa Processo e Ambiente OCI, Aloca Handles,
*                Conecta ao Servidor e Inicia Sessao de Usuario.
*
*************************************************************************/

int
orauser_open(p)
POP *p;
{
   OCISession *usrhp = (OCISession *) 0;

   /* Inicializa processo OCI */

   status = OCIInitialize((ub4) OCI_DEFAULT, (dvoid *) 0, (dvoid * (*)(dvoid *, size_t)) 0, (dvoid * (*)(dvoid *, dvoid *, size_t)) 0, (void (*)(dvoid *, dvoid *)) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Inicializa ambiente OCI */

   status = OCIEnvInit((OCIEnv **) &envhp, OCI_DEFAULT, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Aloca Handle de Erro - Handle e' vinculado a Environment */

   status = OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Aloca Handle de Servidor - Handle e' vinculado a Environment */

   status = OCIHandleAlloc((dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Aloca Handle de Servico - Handle e' vinculado a Environment */

   status = OCIHandleAlloc((dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Conecta ao Servidor Oracle */

   status = OCIServerAttach(srvhp, errhp, "gbl", strlen("gbl"), 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Atribui o Contexto de Servidor no Contexto de Servico */
   /* O Handle de Servidor e' vinculado a Service */

   status = OCIAttrSet((dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *) srvhp, (ub4) 0, OCI_ATTR_SERVER, (OCIError *) errhp);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Aloca Handle de Sessao - Handle e' vinculado a Environment */

   status = OCIHandleAlloc((dvoid *) envhp, (dvoid **) &usrhp, (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Informa, no Handle de Sessao, o Nome do Usuario Oracle */

   status = OCIAttrSet((dvoid *) usrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) oraname, (ub4) strlen((char *) oraname), (ub4) OCI_ATTR_USERNAME, errhp);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Informa, no Handle de Sessao, a Senha do Usuario */

   status = OCIAttrSet((dvoid *) usrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) orapass, (ub4) strlen((char *) orapass), (ub4) OCI_ATTR_PASSWORD, errhp);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Inicia Sessao de Usuario */
   /* O Handle de Sessao e' vinculado a Service */

   status = OCISessionBegin(svchp, errhp, usrhp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Inicializa contexto para Statement */

   status = OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) usrhp, (ub4) 0, (ub4) OCI_ATTR_SESSION, errhp);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Aloca Handle de Statement */

   status = OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp, OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Prepara a execucao do comando, atribuindo comando previamente definido */

   status = OCIStmtPrepare(stmthp, errhp, seleuser, (ub4) strlen((char *) seleuser), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Bind the input variable (userid) */
   /* O Handle de Bind e' alocado automaticamente, e vinculado a Statement */

   status = OCIBindByName(stmthp, &bindp, errhp, (text *) ":userid", strlen(":userid"), (ub1 *) userid, TAM_MAX_AUTH_NAME, SQLT_CHR, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Define the output variable (password) */
   /* O Handle de Define e' alocado automaticamente, e vinculado a Statement */

   status = OCIDefineByPos(stmthp, &defnpass, errhp, 1, (dvoid *) password, (sword) TAM_MAX_AUTH_PASS, SQLT_CHR, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   /* Define the output variable (bloqueado) */
   /* O Handle de Define e' alocado automaticamente, e vinculado a Statement */

   status = OCIDefineByPos(stmthp, &defnbloq, errhp, 2, (dvoid *) bloqueado, (sword) TAM_BLOQUEADO, SQLT_AFC, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   return (0);
}

/*************************************************************************
*
*	Function: orauser_find
*
*	Purpose: Pesquisa o Nome do Usuario na Base de Dados Oracle.
*
*************************************************************************/

int
orauser_find (p)
POP *p;
{
   char		*ptr;
   int		usernamelen;
   int          i;

/* 
 * Check for valid input, zero length names not permitted 
 */

   usernamelen=strlen(p->user);

   if (usernamelen < 1) {
      pop_log(p,POP_FAILURE,"orauser_find: zero length username rejected\n");
      return(-1);
   }

   strncpy(userid,p->user,TAM_MAX_AUTH_NAME);
   for (i = usernamelen; i < TAM_MAX_AUTH_NAME; i++) {
       userid[i] = ' ';
   }

   /* Executa o comando SQL - Statement */

   status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);

   if ((status) && (status != OCI_NO_DATA)) {
      orauser_error(p,errhp, status);
      orauser_close(p);
      return (-1);
   }

   if (status == OCI_NO_DATA) {
      return (-1);
   }

   status = OCIStmtFetch(stmthp, errhp, (ub4) 0, (ub2) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);
   if (status) {
      orauser_error(p,errhp,status);
      orauser_close(p);
      return(-1);
   }

   return(0);

}

#endif /* POPUSERSORACLE */
