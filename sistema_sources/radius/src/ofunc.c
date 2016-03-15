#ifdef RADIUSERSORACLE

/*************************************************************************
*
*	Function: orauser_error
*
*	Purpose: Grava no arquivo de log o erro informado pelo Oracle. 
*
*************************************************************************/

void
orauser_error(errhp, status)
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
      log_err("Error - OCI_SUCCESS_WITH_INFO\n");
      break;
   case OCI_NEED_DATA:
      log_err("Error - OCI_NEED_DATA\n");
      break;
   case OCI_NO_DATA:
      log_err("Error - OCI_NO_DATA\n");
      break;
   case OCI_ERROR:
      (void) OCIErrorGet((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode, errbuff, (ub4) sizeof(errbuff), OCI_HTYPE_ERROR);
      log_err("Error - OCI_ERROR - %.*s\n",512,errbuff);
      break;
   case OCI_INVALID_HANDLE:
      log_err("Error - OCI_INVALID_HANDLE\n");
      break;
   case OCI_STILL_EXECUTING:
      log_err("Error - OCI_STILL_EXECUTING\n");
      break;
   case OCI_CONTINUE:
      log_err("Error - OCI_CONTINUE\n");
      break;
   default:
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
orauser_close(void)
{
   /* Libera Handle de Statement */
   if (stmthp)
      orauser_error(errhp, OCIHandleFree((dvoid *) stmthp, OCI_HTYPE_STMT));

   /* Desconecta do Servidor Oracle */
   if (errhp)
      (void) OCIServerDetach(srvhp, errhp, OCI_DEFAULT);

   /* Libera Handle de Servidor */
   if (srvhp)
      orauser_error(errhp, OCIHandleFree((dvoid *) srvhp, OCI_HTYPE_SERVER));

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
orauser_open(void)
{
   OCISession *usrhp = (OCISession *) 0;

   /* Inicializa processo OCI */

   (void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *) 0, (dvoid * (*)(dvoid *, size_t)) 0, (dvoid * (*)(dvoid *, dvoid *, size_t)) 0, (void (*)(dvoid *, dvoid *)) 0);

   /* Inicializa ambiente OCI */

   (void) OCIEnvInit((OCIEnv **) &envhp, OCI_DEFAULT, (size_t) 0, (dvoid **) 0);

   /* Aloca Handle de Erro - Handle e' vinculado a Environment */

   (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0);

   /* Aloca Handle de Servidor - Handle e' vinculado a Environment */

   (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0);

   /* Aloca Handle de Servico - Handle e' vinculado a Environment */

   (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0);

   /* Conecta ao Servidor Oracle */

   (void) OCIServerAttach(srvhp, errhp, "gbl", strlen("gbl"), 0);

   /* Atribui o Contexto de Servidor no Contexto de Servico */
   /* O Handle de Servidor e' vinculado a Service */

   (void) OCIAttrSet((dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *) srvhp, (ub4) 0, OCI_ATTR_SERVER, (OCIError *) errhp);

   /* Aloca Handle de Sessao - Handle e' vinculado a Environment */

   (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **) &usrhp, (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);

   /* Informa, no Handle de Sessao, o Nome do Usuario Oracle */

   (void) OCIAttrSet((dvoid *) usrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) oraname, (ub4) strlen((char *) oraname), (ub4) OCI_ATTR_USERNAME, errhp);

   /* Informa, no Handle de Sessao, a Senha do Usuario */

   (void) OCIAttrSet((dvoid *) usrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) orapass, (ub4) strlen((char *) orapass), (ub4) OCI_ATTR_PASSWORD, errhp);

   /* Inicia Sessao de Usuario */
   /* O Handle de Sessao e' vinculado a Service */

   orauser_error(errhp, OCISessionBegin(svchp, errhp, usrhp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT));

   /* Inicializa contexto para Statement */

   (void) OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) usrhp, (ub4) 0, (ub4) OCI_ATTR_SESSION, errhp);

   /* Aloca Handle de Statement */

   orauser_error(errhp, OCIHandleAlloc((dvoid *) envhp, (dvoid **) &stmthp, OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0));

   /* Prepara a execucao do comando, atribuindo comando previamente definido */

   orauser_error(errhp, OCIStmtPrepare(stmthp, errhp, seleuser, (ub4) strlen((char *) seleuser), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));

   /* Bind the input variable (username) */
   /* O Handle de Bind e' alocado automaticamente, e vinculado a Statement */

   if (status = OCIBindByName(stmthp, &bindp, errhp, (text *) ":username", strlen(":username"), (ub1 *) username, TAM_MAX_AUTH_NAME, SQLT_AFC, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT)) {
      orauser_error(errhp, status);
      orauser_close();
      return (-1);
   }

   /* Define the output variable (userpass) */
   /* O Handle de Define e' alocado automaticamente, e vinculado a Statement */

   if (status = OCIDefineByPos(stmthp, &defnpass, errhp, 1, (dvoid *) userpass, (sword) TAM_MAX_AUTH_PASS, SQLT_AFC, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT)) {
      orauser_error(errhp, status);
      orauser_close();
      return (-1);
   }

   /* Define the output variable (servicetype) */
   /* O Handle de Define e' alocado automaticamente, e vinculado a Statement */

   if (status = OCIDefineByPos(stmthp, &defnsvct, errhp, 2, (dvoid *) servicetype, (sword) TAM_SRVC_TYPE, SQLT_AFC, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT)) {
      orauser_error(errhp, status);
      orauser_close();
      return (-1);
   }

   /* Define the output variable (authtype) */
   /* O Handle de Define e' alocado automaticamente, e vinculado a Statement */

   if (status = OCIDefineByPos(stmthp, &defnautt, errhp, 3, (dvoid *) authtype, (sword) TAM_AUTH_TYPE, SQLT_AFC, (dvoid *) 0, (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT)) {
      orauser_error(errhp, status);
      orauser_close();
      return (-1);
   }

   db_index = -1;

   return (0);
}

/*************************************************************************
*
*	Function: orauser_find
*
*	Purpose: Pesquisa o Nome do Usuario na Base de Dados Oracle.
*                Cria as estruturas de pares "Atributo-Valor"  para
*                autenticar (check) e autorizar/configurar (reply)
*                este usuario a partir das informacoes armazenadas
*                em seu registro.
*
*************************************************************************/

int
orauser_find(req_name, auth_name, check_pairs, reply_pairs)
char *req_name;
char *auth_name;
VALUE_PAIR **check_pairs;
VALUE_PAIR **reply_pairs;
{
   char		buffer[MAXBUF];
   char		*ptr;
   int		req_namelen;
   int		mode;
   int          i;
   int		user_auth_name();
   VALUE_PAIR	*check_first;
   VALUE_PAIR	*reply_first;
   VALUE_PAIR   *pair;
   VALUE_PAIR   *link;
   datum	contentd;

/* 
 * Check for valid input, zero length names not permitted 
 */

   mode = FIND_MODE_NAME;

   ptr=req_name;
   while (*ptr != '\0') {
      if (*ptr == ' ' || *ptr == '\t') {
#ifdef SPACECHOP        /* Trunca o nome */
         *ptr = '\0';
#else                   /* Erro: Rejeitado */
         log_err("user_find: space in username <%s> rejected\n",req_name);
         return(-1);
#endif
      } else {
         ptr++;
      }
   }

   req_namelen=strlen(req_name);

   if (req_namelen < 1) {
      log_err("user_find: zero length username rejected\n");
      return(-1);
   }

   check_first = (VALUE_PAIR *)NULL;
   reply_first = (VALUE_PAIR *)NULL;

   for (;;) {
      if (db_index == -1) {
         strncpy(username,req_name,TAM_MAX_AUTH_NAME);
         for (i = req_namelen; i < TAM_MAX_AUTH_NAME; i++) {
             username[i] = ' ';
         }
      } else if (db_index == 0) {
         snprintf(username,TAM_MAX_AUTH_NAME,"DEFAULT");
         for (i = strlen(username); i < TAM_MAX_AUTH_NAME; i++) {
             username[i] = ' ';
         }
      } else {
         snprintf(username,TAM_MAX_AUTH_NAME,"DEFAULT%d",db_index);
         for (i = strlen(username); i < TAM_MAX_AUTH_NAME; i++) {
             username[i] = ' ';
         }
      }
      db_index++;

      /* Executa o comando SQL - Statement */

      if ((status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) 0, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT)) && (status != OCI_NO_DATA)) {
         orauser_error(errhp, status);
         orauser_close();
         return (-1);
      }

      if (status == OCI_NO_DATA) { /* Se Usuario nao Encontrado */

            log_err("Comando SELECT nao encontrou o Usuario: %s\n",username);

         if (db_index == 0) { /* E e' o nome que consta na requisicao */
            /*
             * the request id failed
             * lets try the defaults
             */

             continue;
         }
         return(-1);
      }

      if (status = OCIStmtFetch(stmthp, errhp, (ub4) 1, (ub2) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT)) {
         orauser_error(errhp,status);
         orauser_close();
         return(-1);
      }

      log_err("Comando SELECT encontrou o Usuario: %s\n",username);

      /*
       * Parse the check values
       *

      if (strlen(userbuff) > MAXBUF) {
         log_err("user_find: user record for user %s is too big, %d exceeds %d\n", req_name,contentd.dsize,MAXBUF);
         return(-1);
      }
      memcpy(buffer,contentd.dptr,contentd.dsize);
      buffer[contentd.dsize] = '\0';
      ptr = buffer;

      if (userparse(ptr, &check_first) != 0) {
         log_err("user_find: unable to parse check-items in dbm entry for user %s\n", req_name);
         pairfree(check_first);
         return(-1);
      }
  
       */
      
      /* PASSWORD */

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      check_first = pair;
      sprintf(pair->name,"Password");
      pair->attribute = PW_CRYPT_PASSWORD;
      pair->type = PW_TYPE_STRING;
      for (i = 0; i < TAM_MAX_AUTH_PASS; i++) {
          if (userpass[i] == ' ') userpass[i] = '\0';
      };
      strcpy(pair->strvalue,userpass);
      pair->lvalue = strlen(userpass);

      link = pair;

      /* AUTH-TYPE = LOCAL */

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      sprintf(pair->name,"Auth-Type");
      pair->attribute = PW_AUTHTYPE;
      pair->type = PW_TYPE_INTEGER;
      for (i = 0; i < TAM_AUTH_TYPE; i++) {
          authtypent[i] = authtype[i];
      };
      authtypent[TAM_AUTH_TYPE] = '\0';
      pair->lvalue = (UINT4) atoi(authtypent);
      pair->next = (VALUE_PAIR *)NULL;

      /*
       * set authentication name
       */

      if (user_auth_name(req_name, auth_name, check_first) != 0) {
         pairfree(check_first);
         check_first = (VALUE_PAIR *)NULL;
         continue;
      }

      break;

   } /* for */

/*   for (link = check_first; link != (VALUE_PAIR *)NULL; link = link->next) {
       log_err("Nome: %s, Atributo: %d, Tipo: %d, Lvalue: %d, StrValue: %s\n",link->name,link->attribute,link->type,link->lvalue,link->strvalue);
   };*/

/*

   while (*ptr != '\n' && *ptr != '\0') {
      ptr++;
   }
   if (*ptr == '\0') {

       * no reply-items * 

      *check_pairs = check_first;
      *reply_pairs = (VALUE_PAIR *)NULL;
      return(0);
   }

   ptr++;

    *
    * Parse the reply values
    *

   if (userparse(ptr, &reply_first) != 0) {
      log_err("user_find: unable to parse reply-items in dbm entry for user %s\n", req_name);
      pairfree(check_first);
      pairfree(reply_first);
      return(-1);
   }

*/
      
      /* USER-SERVICE-TYPE */

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      reply_first = pair;
      sprintf(pair->name,"Service-Type");
      pair->attribute = PW_USER_SERVICE_TYPE;
      pair->type = PW_TYPE_INTEGER;
      for (i = 0; i < TAM_SRVC_TYPE; i++) {
          servicetypent[i] = servicetype[i];
      };
      servicetypent[TAM_SRVC_TYPE] = '\0';
      pair->lvalue = (UINT4) atoi(servicetypent);

      link = pair;

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      /* FRAMED-PROTOCOL */

      sprintf(pair->name,"Framed-Protocol");
      pair->attribute = PW_FRAMED_PROTOCOL;
      pair->type = PW_TYPE_INTEGER;
      pair->lvalue = PW_PPP; 

      link = pair;

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      /* FRAMED-ADDRESS */

      sprintf(pair->name,"Framed-Address");
      pair->attribute = PW_FRAMED_ADDRESS;
      pair->type = PW_TYPE_IPADDR;
      pair->lvalue = get_ipaddr("255.255.255.254"); 

      link = pair;

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      /* FRAMED-ROUTING */

      sprintf(pair->name,"Framed-Routing");
      pair->attribute = PW_FRAMED_ROUTING;
      pair->type = PW_TYPE_INTEGER;
      pair->lvalue = PW_NONE;

      link = pair;

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      /* FRAMED-MTU */

      sprintf(pair->name,"Framed-MTU");
      pair->attribute = PW_FRAMED_MTU;
      pair->type = PW_TYPE_INTEGER;
      pair->lvalue = (UINT4) 1500;

      link = pair;

      if ((pair=(VALUE_PAIR *)malloc(sizeof(VALUE_PAIR)))==(VALUE_PAIR *)NULL) {
         log_err("userparse: system error: out of memory\n");
         return (-1);
      }

      link->next = pair;

      /* FRAMED-COMPRESSION */

      sprintf(pair->name,"Framed-Compression");
      pair->attribute = PW_FRAMED_COMPRESSION;
      pair->type = PW_TYPE_INTEGER;
      pair->lvalue = PW_VAN_JACOBSON_TCP_IP;

      pair->next = (VALUE_PAIR *)NULL;

/*    for (link = reply_first; link != (VALUE_PAIR *)NULL; link = link->next) {
          log_err("Nome: %s, Atributo: %d, Tipo: %d, Lvalue: %d, StrValue: %s\n",link->name,link->attribute,link->type,link->lvalue,link->strvalue);
      };*/

   /* Update the callers pointers */
   if (reply_first != (VALUE_PAIR *)NULL) {
      *check_pairs = check_first;
      *reply_pairs = reply_first;
      return(0);
   }

   return(-1);

}

#endif /* RADIUSERSORACLE */
