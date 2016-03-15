/*
 *
 *	RADIUS
 *	Remote Authentication Dial In User Service
 *
 *
 *	Livingston Enterprises, Inc.
 *	4464 Willow Road
 *	Pleasanton, CA   94588
 *
 *	Copyright 1992-1996 Livingston Enterprises, Inc. All Rights Reserved.
 *
 *	This software is provided under license from Livingston
 *	Enterprises, Inc., the terms and conditions of which are set
 *	forth in a Software License Agreement that is contained in an
 *	End User Agreement contained in the product packaging, and
 *	electronically on the Livingston ftp site. This software may
 *	only be used in conjunction with Livingston (or Livingston
 *	authorized) products.  Livingston makes no warranties to any
 *	licensee concerning the applicability of the software to
 *	licensee's specific requirements or the suitability of the
 *	software for any intended use.  Licensee shall not remove,
 *	modify or alter any copyright and/or other proprietary rights
 *	notice and must faithfully reproduce all such notices on any
 *	copies or modifications to this software that it makes.
 *
 *	Livingston Enterprises, Inc. makes no representations about the
 *	suitability of this software for any purpose.  It is provided
 *	"as is" without express or implied warranty.
 *
 */

static char sccsid[] =
"$Id: dict.c,v 1.10 1997/05/15 06:49:47 cdr Exp $ Copyright 1992 Livingston Enterprises Inc";

#include	<stdio.h>
#include	<sys/types.h>
#include	<pwd.h>
#include	<ctype.h>

#include	"radius.h"

extern char		*progname;
extern int		debug_flag;
extern char		*radius_dir;

static DICT_ATTR	*dictionary_attributes;
static DICT_VALUE	*dictionary_values;

/*************************************************************************
 *
 *	Function: dict_init
 *
 *	Purpose: Initialize the dictionary.  Read all ATTRIBUTES into
 *		 the dictionary_attributes list.  Read all VALUES into
 *		 the dictionary_values list.
 *
 *************************************************************************/

int
dict_init()
{
	FILE	*dictfd;
	char	dummystr[64];
	char	namestr[64];
	char	valstr[64];
	char	attrstr[64];
	char	typestr[64];
	int	line_no;
	DICT_ATTR	*attr;
	DICT_VALUE	*dval;
	char	buffer[256];
	char	dictfile[256];
	int	value;
	int	type;
	int	atoi();

	sprintf(dictfile, "%s/%s", radius_dir, RADIUS_DICTIONARY);
	if((dictfd = fopen(dictfile, "r")) == (FILE *)NULL) {
		log_err("could not read dictionary file %s\n",dictfile);
		return(-1);
	}

	line_no = 0;
	while(fgets(buffer, sizeof(buffer), dictfd) != (char *)NULL) {
		line_no++;
		
		/* Skip empty space */
		if(*buffer == '#' || *buffer == '\0' || *buffer == '\n') {
			continue;
		}

		if(strncmp(buffer, "ATTRIBUTE", 9) == 0) {

			/* Read the ATTRIBUTE line */
			if(sscanf(buffer, "%s%s%s%s", dummystr, namestr,
					valstr, typestr) != 4) {
				log_err("invalid attribute on line %d of dictionary file %s\n", line_no,dictfile);
				return(-1);
			}

			/*
			 * Validate all entries
			 */
			if((int)strlen(namestr) > 31) {
				log_err("attribute name too long on line %d of dictionary %s\n",
					line_no, dictfile);
				return(-1);
			}

			if(!isdigit(*valstr)) {
				log_err("attribute has non-numeric value on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}
			value = atoi(valstr);

			if(strcmp(typestr, "string") == 0) {
				type = PW_TYPE_STRING;
			}
			else if(strcmp(typestr, "integer") == 0) {
				type = PW_TYPE_INTEGER;
			}
			else if(strcmp(typestr, "ipaddr") == 0) {
				type = PW_TYPE_IPADDR;
			}
			else if(strcmp(typestr, "date") == 0) {
				type = PW_TYPE_DATE;
			}
			else {
				log_err("attribute has unknown type on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}

			/* Create a new attribute for the list */
			if((attr = (DICT_ATTR *)malloc(sizeof(DICT_ATTR))) ==
					(DICT_ATTR *)NULL) {
				log_err("ran out of memory after reading line %d of dictionary %s\n",line_no,dictfile);
				return(-1);
			}
			strcpy(attr->name, namestr);
			attr->value = value;
			attr->type = type;

			/* Insert it into the list */
			attr->next = dictionary_attributes;
			dictionary_attributes = attr;
		}
		else if(strncmp(buffer, "VALUE", 5) == 0) {

			/* Read the VALUE line */
			if(sscanf(buffer, "%s%s%s%s", dummystr, attrstr,
						namestr, valstr) != 4) {
				log_err("Invalid value entry on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}

			/*
			 * Validate all entries
			 */
			if((int)strlen(attrstr) > 31) {
				log_err("attribute name too long on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}

			if((int)strlen(namestr) > 31) {
				log_err("value name too long on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}

			if(!isdigit(*valstr)) {
				log_err("value has non-numeric value on line %d of dictionary %s\n", line_no, dictfile);
				return(-1);
			}
			value = atoi(valstr);

			/* Create a new VALUE entry for the list */
			if((dval = (DICT_VALUE *)malloc(sizeof(DICT_VALUE))) ==
					(DICT_VALUE *)NULL) {
				log_err("ran out of memory after reading line %d of dictionary %s\n",line_no,dictfile);
				return(-1);
			}
			strcpy(dval->attrname, attrstr);
			strcpy(dval->name, namestr);
			dval->value = value;

			/* Insert it into the list */
			dval->next = dictionary_values;
			dictionary_values = dval;
		}
	}
	fclose(dictfd);
	return(0);
}

/*************************************************************************
 *
 *	Function: dict_attrget
 *
 *	Purpose: Return the full attribute structure based on the
 *		 attribute id number.
 *
 *************************************************************************/

DICT_ATTR	*
dict_attrget(attribute)
int	attribute;
{
	DICT_ATTR	*attr;

	attr = dictionary_attributes;
	while(attr != (DICT_ATTR *)NULL) {
		if(attr->value == attribute) {
			return(attr);
		}
		attr = attr->next;
	}
	return((DICT_ATTR *)NULL);
}

/*************************************************************************
 *
 *	Function: dict_attrfind
 *
 *	Purpose: Return the full attribute structure based on the
 *		 attribute name.
 *
 *************************************************************************/

DICT_ATTR	*
dict_attrfind(attrname)
char	*attrname;
{
	DICT_ATTR	*attr;

	attr = dictionary_attributes;
	while(attr != (DICT_ATTR *)NULL) {
		if(strcmp(attr->name, attrname) == 0) {
			return(attr);
		}
		attr = attr->next;
	}
	return((DICT_ATTR *)NULL);
}

/*************************************************************************
 *
 *	Function: dict_valfind
 *
 *	Purpose: Return the full value structure based on the
 *		 value name.
 *
 *************************************************************************/

DICT_VALUE	*
dict_valfind(valname)
char	*valname;
{
	DICT_VALUE	*val;

	val = dictionary_values;
	while(val != (DICT_VALUE *)NULL) {
		if(strcmp(val->name, valname) == 0) {
			return(val);
		}
		val = val->next;
	}
	return((DICT_VALUE *)NULL);
}

/*************************************************************************
 *
 *	Function: dict_valget
 *
 *	Purpose: Return the full value structure based on the
 *		 actual value and the associated attribute name.
 *
 *************************************************************************/

DICT_VALUE	*
dict_valget(value, attrname)
UINT4	value;
char	*attrname;
{
	DICT_VALUE	*val;

	val = dictionary_values;
	while(val != (DICT_VALUE *)NULL) {
		if(strcmp(val->attrname, attrname) == 0 &&
						val->value == value) {
			return(val);
		}
		val = val->next;
	}
	return((DICT_VALUE *)NULL);
}
