#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "win32.h"
#include "../alertmsg.h"
#include "progres.h"
//Win32 shims for... weird stuff no sane mortal should do.

#ifndef SHIMLIBS_PROVIDES_ALERTMSG
	#define SHIMLIBS_PROVIDES_ALERTMSG
	void AlertMsg (const char *Message, const char *Title)
	{
		MessageBox(0, Message, Title, MB_ICONEXCLAMATION|MB_OK);
	}
#endif

//Callback to set font for all children of a window
//Use with EnumChildWindows
//Taken from http://stackoverflow.com/a/17075471
BOOL CALLBACK SetFont(HWND child, LPARAM font)
{
	SendMessage(child, WM_SETFONT, font, TRUE);
	return TRUE;
}

//Fix a dialog resource so that it uses the given font size
//Win95-compatible DPI compliance! Probably not a good thing!
void FixDlgFont(DLGTEMPLATE *pTemplate, unsigned short size)
{
	unsigned short *fontSize = pTemplate; //Readdress template
	
	//Look up https://blogs.msdn.microsoft.com/oldnewthing/20040621-00/?p=38793/
	//for details on what exactly I'm doing. It's weird and hacky.
	//(Also, I know thenewoldthing isn't official documentation, but
	//it's the best I've got. Sorry Raymond Chen.)
	
	//Skip past header (gotta divide by 2 because we're going by shorts)
	fontSize += sizeof(DLGTEMPLATE)/2;
	
	//After the header comes the menu.
	//Menu can be either a numeral, a string, or a null byte.
	//Test first byte
	
	if(*fontSize == 0x0000){
		//Nothing!
		fontSize++;
	} else if (*fontSize == 0xFFFF || *fontSize == 0xFF00){
		//Technically it's supposed to be 0xFF00, but Borland's
		//resource compiler uses 0xFFFF. Not gonna argue.
		
		//This means it's a 16-bit short, so we gotta go ahead 2 shorts.
		fontSize += 2;
	} else {
		//String. Skip past it.
		fontSize += wcslen(fontSize);
		//Plus null byte
		fontSize++;
	}
	
	//Then dialog class. Same thing
	if(*fontSize == 0x0000){
		fontSize++;
	} else if (*fontSize == 0xFFFF || *fontSize == 0xFF00){
		fontSize += 2;
	} else {
		fontSize += wcslen(fontSize);
		fontSize++;
	}
	
	//Then title. Same thing.
	if(*fontSize == 0x0000){
		fontSize++;
	} else if (*fontSize == 0xFFFF || *fontSize == 0xFF00){
		fontSize += 2;
	} else {
		fontSize += wcslen(fontSize);
		fontSize++;
	}
	
	//Then set font size!
	*fontSize = size;

	return;
}

//Like CreateDialog(), but patches the font size to be the same as the system
//font. That way you HiDPI folks won't go blind staring at 8pt Tahoma on
//your 4K monitor.
HWND CreateDialogSysFont(int DlgIDD, DLGPROC DlgProc, HWND hParent, HINST CurrInstance)
{
	HWND hDialog = NULL;
	char *strIDD = NULL;
	HRSRC res;
	
	void *pTemplate = NULL;
	void *mainTemplate = NULL;
	int pTemplateSize = 0;
	
	NONCLIENTMETRICS *MetricsConfig = NULL;
	unsigned short DlgFontSize;
	
	//Get dialog font size
	MetricsConfig = malloc(sizeof(NONCLIENTMETRICS));
	MetricsConfig->cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(
		SPI_GETNONCLIENTMETRICS,
		MetricsConfig->cbSize,
		MetricsConfig, 0
	);
	DlgFontSize = (unsigned short)MetricsConfig->lfMessageFont.lfHeight;
	free(MetricsConfig);
	
	//Create string for IDD_MAIN
	asprintf(&strIDD, "#%u", DlgIDD); 
	
	//Get dialog from resource file
	res = FindResource(NULL, strIDD, RT_DIALOG);
	pTemplate = LoadResource(NULL, res);
	pTemplate = LockResource(pTemplate);
	
	//Duplicate template
	pTemplateSize = SizeofResource(NULL, res);
	mainTemplate = calloc(pTemplateSize, 1);
	memcpy(mainTemplate, pTemplate, pTemplateSize);
	
	//Fix template
	FixDlgFont(mainTemplate, DlgFontSize);
	
	//Make dialog
	hDialog = CreateDialogIndirect(CurrInstance, mainTemplate, hParent, DlgProc);
	
	free(strIDD);
	free(mainTemplate);
	return hDialog;
}

//Literally the same as above, but modal
void DialogBoxSysFont(int DlgIDD, DLGPROC DlgProc, HWND hParent, HINST CurrInstance)
{
	char *strIDD = NULL;
	HRSRC res;
	void *pTemplate = NULL;
	void *mainTemplate = NULL;
	int pTemplateSize = 0;
	
	NONCLIENTMETRICS *MetricsConfig = NULL;
	unsigned short DlgFontSize;
	
	//Get dialog font size
	MetricsConfig = malloc(sizeof(NONCLIENTMETRICS));
	MetricsConfig->cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(
		SPI_GETNONCLIENTMETRICS,
		MetricsConfig->cbSize,
		MetricsConfig, 0
	);
	DlgFontSize = (unsigned short)MetricsConfig->lfMessageFont.lfHeight;
	free(MetricsConfig);
	
	//Create string for IDD_MAIN
	asprintf(&strIDD, "#%u", DlgIDD);
	
	//Get dialog from resource file
	res = FindResource(NULL, strIDD, RT_DIALOG);
	pTemplate = LoadResource(NULL, res);
	pTemplate = LockResource(pTemplate);
	
	//Duplicate template
	pTemplateSize = SizeofResource(NULL, res);
	mainTemplate = calloc(pTemplateSize, 1);
	memcpy(mainTemplate, pTemplate, pTemplateSize);
	
	//Fix template
	FixDlgFont(mainTemplate, DlgFontSize);
	
	//Make dialog
	DialogBoxIndirect(CurrInstance, mainTemplate, hParent, DlgProc);
	
	free(strIDD);
	free(mainTemplate);
	return;
}

//Scroll a window by moving it's contents!
//Because, unsurprisingly, ScrollWindowEx() is worthless.
BOOL CALLBACK ScrollByMove(HWND hCtl, LPARAM ScrollDelta)
{
	RECT rectCtl;
	GetWindowRect(hCtl, &rectCtl);
	MapWindowPoints(HWND_DESKTOP, GetParent(hCtl), (LPPOINT) &rectCtl, 2);
	
	MoveWindow(
		hCtl,
		rectCtl.left,                  	//X
		rectCtl.top + ScrollDelta,     	//Y
		(rectCtl.right - rectCtl.left),	//Width
		(rectCtl.bottom - rectCtl.top),	//Height
		TRUE
	);
	
	return TRUE;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ProgDialog_Init
 *  Description:  Returns a dialog or other visible indicator with a progress
 *                bar from 0 to a given maximum value and a label.
 * =====================================================================================
 */
HWND ProgDialog_Init(int max, const char *label)
{
	HWND ProgDialog;
	//Create progress window in modeless mode
	ProgDialog = CreateDialogSysFont(IDD_PROGRESS, Dlg_Generic, NULL);
	if(ProgDialog == NULL){
		CURRERROR = errCRIT_FUNCT;
		return NULL;
	}
	
	//Set progress bar maximum
	SendMessage(
		GetDlgItem(ProgDialog, IDC_PROGBAR),
		PBM_SETRANGE, 0,
		MAKELPARAM(0, max)
	);

	//Set text label
	SetWindowText(ProgDialog, label);
	return ProgDialog;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ProgDialog_Update
 *  Description:  Modifies the progress indicator in the dialog/etc. returned
 *                by ProgDialog_Init by the amount Delta
 * =====================================================================================
 */
void ProgDialog_Update(HWND ProgDialog, int Delta)
{
	HWND ProgBar;
	
	//Get progress bar control.
	ProgBar = GetDlgItem(ProgDialog, IDC_PROGBAR);
	
	//Update bar
	SendMessage(ProgBar, PBM_DELTAPOS, Delta, 0);
	//Weird Aero hack so you can actually see progress. Curse you Aero.
	SendMessage(ProgBar, PBM_DELTAPOS, -1, 0);
	SendMessage(ProgBar, PBM_DELTAPOS, 1, 0);
	
	return;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ProgDialog_Kill
 *  Description:  Destroys the dialog/etc. returned by ProgDialog_Init
 * =====================================================================================
 */
void ProgDialog_Kill(HWND ProgDialog)
{
	SendMessage(ProgDialog, WM_CLOSE, 0, 0);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  EnableWholeWindow
 *  Description:  Enables or disables all elements in a window and repaints it.
 *                Useful for preventing user actions during installation or work
 * =====================================================================================
 */
void EnableWholeWindow(HWND hwnd, BOOL state)
{
	HWND hCtl = GetWindow(hwnd,GW_CHILD);
	//Disable all controls, but not main window.
	//This prevents window from flying into BG.
	while(hCtl){
		EnableWindow(hCtl, state);
		hCtl = GetWindow(hCtl, GW_HWNDNEXT);
	}
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE || RDW_UPDATENOW || RDW_ALLCHILDREN);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SelectFolder
 *  Description:  Wrapper around SHBrowseForFolder that is designed to
 *                get only valid folder names (not My Computer, for instance).
 * =====================================================================================
 */
char * SelectFolder(LPCSTR title)
{
	BROWSEINFO bi = {0}; // For init directory selector
	LPITEMIDLIST pidl;
	char *path = malloc(MAX_PATH);
	CURRERROR = errNOERR;
	bi.lpszTitle = title;
		
	if(path == NULL){
		AlertMsg("Malloc Fail #3", "");
		CURRERROR = errCRIT_MALLOC;
		return strdup("");
	}
	strcpy(path, "");
	

	
	pidl = SHBrowseForFolder ( &bi );
	//If user hits Cancel, stop nagging user
	if(pidl == 0){
		CURRERROR = errUSR_ABORT;
		free(path);
		return strdup("");
	}
		
	//Get the name of the folder
	if (!SHGetPathFromIDList ( pidl, path )){
		//User chose invalid folder
		CURRERROR = errWNG_BADDIR;
		free(path);
		return strdup("");
	}
	return path;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SelectFile
 *  Description:  Simple wrapper around GetOpenFileName.
 * =====================================================================================
 */
char * SelectFile(HWND hwnd, int *nameOffset, const char *Filter)
{
	OPENFILENAME ofn = {0};
	char *fpath = calloc(MAX_PATH, sizeof(char));

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = fpath;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER|OFN_DONTADDTORECENT|OFN_ENABLESIZING|OFN_HIDEREADONLY;
        ofn.lpstrFilter = Filter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nFilterIndex = 0;
        ofn.lpstrFileTitle = NULL;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = NULL;
	
	GetOpenFileName(&ofn);
	//TODO: Error checking?

	//Get just the filename
	if(nameOffset != NULL){
		*nameOffset = ofn.nFileOffset;
	}
	
	return fpath;
}