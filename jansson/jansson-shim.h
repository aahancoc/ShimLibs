json_t * JSON_Load(const char *fpath);
signed long JSON_GetInt(json_t *root, const char *name);
double JSON_GetDouble(json_t *root, const char *name);
char * JSON_GetStr(json_t *root, const char *name);
int JSON_GetStrLen(json_t *root, const char *name);