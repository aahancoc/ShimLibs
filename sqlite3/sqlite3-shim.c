#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include "sqlite3-shim.h"
#include "../alertmsg.h"
#define SHIMLIBS_PROVIDES_SQLITE3
///SQLite helper Functions

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_ColName
 *  Description:  Returns the name of a column given it's index number
 * =====================================================================================
 */
const char * SQL_ColName(sqlite3_stmt * stmt, int col){
	return sqlite3_column_name(stmt, col) == NULL ?
		"" : (char *)sqlite3_column_name(stmt, col);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_ColText
 *  Description:  Returns the value of an item given it's column index number
 * =====================================================================================
 */
const char * SQL_ColText(sqlite3_stmt * stmt, int col){	
	return sqlite3_column_text(stmt, col) == NULL ?
		"" : (char *)sqlite3_column_text(stmt, col);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_GetJSON
 *  Description:  Converts the result of an SQL statement to a easily searchable
 *                or savable JSON array. Requires Jansson.
 * =====================================================================================
 */
#ifdef SHIMLIBS_PROVIDES_JANSSON
json_t * SQL_GetJSON(sqlite3_stmt *stmt)
{
	int i, errorNo;
	json_t *result = json_array();
	CURRERROR = errNOERR;
	
	///Compose the JSON array
	errorNo = sqlite3_step(stmt);
        while (errorNo == SQLITE_ROW){
		//For each row of results, create a new object
		json_t *tempobj = json_object();
		
		for(i = sqlite3_column_count(stmt) - 1; i >= 0; i--){
			//For each result key:value pair, insert into tempobj
			//key "colname" with value "coltext".
			json_t *value;
			char *colname = strdup(SQL_ColName(stmt, i));
			char *coltext = strdup(SQL_ColText(stmt, i));
			
			if(!colname || !coltext){
				CURRERROR = errCRIT_FUNCT;
				AlertMsg("SQL->JSON ERROR!", coltext);
				free(colname);
				free(coltext);
				return json_array();
			}
			
			
			if(strcmp(coltext, "") == 0){
				value = json_null();
			} else {
				value = json_string(coltext);
			}
			
			if(value == NULL){
				CURRERROR = errCRIT_FUNCT;
				AlertMsg("There was an error fetching data from"
					"the mod database. The application will"
					"now quit to prevent data corruption",
					"SQL->JSON ERROR!");
				free(colname);
				free(coltext);
				return json_array();
			}
			 
			json_object_set_new(tempobj, colname, value);
			
			free(colname);
			free(coltext);
		}
		//Add object to array
		json_array_append_new(result, tempobj);
		
		errorNo = sqlite3_step(stmt);
        };
	
	if(errorNo != SQLITE_DONE){
		CURRERROR = errCRIT_DBASE;
		result = json_array();
	}
	
	//End array
	sqlite3_reset(stmt);
	
	return result;
}
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_GetNum
 *  Description:  Returns the first item from an SQL query as a number
 *                (ex: the value of a COUNT statement)
 * =====================================================================================
 */
int SQL_GetNum(sqlite3_stmt *stmt)
{
	int errorNo, result = -1;
	CURRERROR = errNOERR;
	
        errorNo = sqlite3_step(stmt);
        if (errorNo == SQLITE_ROW) {
		result = sqlite3_column_int(stmt, 0);
        } else if (errorNo == SQLITE_DONE){
		result = -1;
	} else {
		CURRERROR = errCRIT_DBASE;
		result = -1;
	}
	
	sqlite3_reset(stmt);
	return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_GetStr
 *  Description:  Returns the first item from an SQL query as a string
 *                (ex: the value of a SELECT statement)
 * =====================================================================================
 */
char * SQL_GetStr(sqlite3_stmt *stmt)
{
	int errorNo;
	char *result = NULL;
	CURRERROR = errNOERR;
	
        errorNo = sqlite3_step(stmt);
        if (errorNo == SQLITE_ROW) {
		result = strdup(SQL_ColText(stmt, 0));
        } else if (errorNo == SQLITE_DONE){
		result = strdup("");
	} else {
		CURRERROR = errCRIT_DBASE;
	}
	
	sqlite3_reset(stmt);
	return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_GetBlob
 *  Description:  Returns the first item from an SQL query as a blob.
 *                UNTESTED! PROBABLY UNSAFE!
 * =====================================================================================
 */
unsigned char * SQL_GetBlob(sqlite3_stmt *stmt, int *len)
{
	int errorNo;
	unsigned char *result = NULL;
	CURRERROR = errNOERR;
	
        errorNo = sqlite3_step(stmt);
        if (errorNo == SQLITE_ROW) {
		*len = sqlite3_column_bytes(stmt, 0);
		result = calloc(*len, sizeof(unsigned char));
		memcpy(result, sqlite3_column_blob(stmt, 0), *len);
        } else if (errorNo == SQLITE_DONE){
		result = NULL;
	} else {
		CURRERROR = errCRIT_DBASE;
	}
	
	sqlite3_reset(stmt);
	return result;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SQL_HandleErrors
 *  Description:  In the event of an error, handle and attempt to recover from it.
 * =====================================================================================
 */
int SQL_HandleErrors(int lineno, int SQLResult)
{
	char *message = NULL;
	
	if(
		SQLResult == SQLITE_OK ||
		SQLResult == SQLITE_DONE ||
		SQLResult == SQLITE_ROW
	){
		//No error, continue.
		return 0;
	}

	asprintf(&message,
		"Internal Database Error!\n"
		"Error code %d\n"
		"Line %d\n"
		, SQLResult, lineno);
	AlertMsg(message, "SQLite Error!");
	free(message);
	return -1;
}