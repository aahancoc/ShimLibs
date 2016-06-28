const char * SQL_ColName(sqlite3_stmt * stmt, int col);
const char * SQL_ColText(sqlite3_stmt * stmt, int col);
#ifdef SHIMLIBS_PROVIDES_JANSSON
json_t * SQL_GetJSON(sqlite3_stmt *stmt);
#endif
int SQL_GetNum(sqlite3_stmt *stmt);
char * SQL_GetStr(sqlite3_stmt *stmt);
unsigned char * SQL_GetBlob(sqlite3_stmt *stmt, int *len);
int SQL_HandleErrors(int lineno, int SQLResult);