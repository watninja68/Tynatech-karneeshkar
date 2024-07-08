#include "stdafx.h"

// Global Variables:
HINSTANCE	g_hInst;

// Application Entry Point
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	INITCOMMONCONTROLSEX InitCtrls;

	// Initializes the Common controls
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES|ICC_DATE_CLASSES;
	InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	BOOL bRet = InitCommonControlsEx(&InitCtrls);
	g_hInst = hInstance; 

	// Load the bitmap from resource
	g_appData.m_BitmapPlain = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PLAIN)); 
	g_appData.m_BitmapGreen = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_GREEN_BUTTON)); 
	g_appData.m_BitmapDisabled = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DISABLED)); 
	g_appData.m_BitmapRed = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_RED_BUTTON)); 
	g_appData.m_BitmapUnknown = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_UNKNOWN)); 

	// Load the main Dialog Proc
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINMENU), NULL, (DLGPROC)DialogProc,0);

	return 0;
}
