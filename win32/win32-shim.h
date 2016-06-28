BOOL CALLBACK SetFont(HWND child, LPARAM font);
void FixDlgFont(DLGTEMPLATE *pTemplate, unsigned short size);
HWND CreateDialogSysFont(int DlgIDD, DLGPROC DlgProc, HWND hParent, HINST CurrInstance);
void DialogBoxSysFont(int DlgIDD, DLGPROC DlgProc, HWND hParent, HINST CurrInstance);
BOOL CALLBACK ScrollByMove(HWND hCtl, LPARAM ScrollDelta);

HWND ProgDialog_Init(int max, const char *label);
void ProgDialog_Update(HWND ProgDialog, int Delta);
void ProgDialog_Kill(HWND ProgDialog);

void EnableWholeWindow(HWND hwnd, BOOL state);
char * SelectFolder(LPCSTR title);
char * SelectFile(HWND hwnd, int *nameOffset, const char *Filter);