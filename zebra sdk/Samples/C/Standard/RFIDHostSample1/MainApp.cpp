#include "stdafx.h"
#include <Commdlg.h>

TCHAR* memory_banks[] = {
	L"None",
	L"Reserved",
	L"EPC",
	L"TID",
	L"USER"
};

TCHAR* tag_states[] = {
	L"New",
	L"Gone",
	L"Back",
	L"None"
};

CRFIDAppData g_appData;
HANDLE stopTestingEventHandle;
HANDLE readerEventAwaitingThreadHandle;
HWND	g_hTagList;
HWND 	g_hMainDialog, g_hConnectionDlg;
HMENU hmenu;
DWORD WINAPI readerEventAwaitingThread(LPVOID pvarg);

TCHAR			g_ClientCertFilePath[MAX_PATH];
TCHAR			g_ClientKeyFilePath[MAX_PATH];
TCHAR			g_RootCertFilePath[MAX_PATH];
TCHAR			g_KeyPassword[MAX_PATH];
BYTE			g_ClientCertContent[MAX_CERT_KEY_FILE_SIZE];
int				g_ClientCertSize;
BYTE			g_ClientKeyContent[MAX_CERT_KEY_FILE_SIZE];
int				g_ClientKeySize;
BYTE			g_RootCertContent[MAX_CERT_KEY_FILE_SIZE];
int				g_RootCertSize;
BYTE			g_Password[MAX_PATH];

CONNECTION_INFO g_ConnectionInfo;
SEC_CONNECTION_INFO g_secConnParams;
OPENFILENAME ofn;

bool StopReading();
bool StartReading();
void printTagData(TAG_DATA *pTagData);
void UpdateMainDialog(void);

void UpdateReadState();
void UpdateReadState(UINT16 readState)
{
	TCHAR szBuffer[MAX_PATH] = {0,};
	g_appData.m_OperationState = readState;
	SetWindowText(GetDlgItem(g_hMainDialog, IDC_START_READ), 
		g_appData.m_OperationState == IDLE? TEXT("Start Read") :
		TEXT("Stop Read"));
#ifdef WINCE
    // Update read time static text, in case of Mobile device
	if (g_appData.m_OperationState == IDLE)
	{
		g_appData.m_stopTickCount = GetTickCount();
		// Update the Read Time in case of Mobile App
		_stprintf(szBuffer, TEXT("%lu sec"), ((g_appData.m_stopTickCount - g_appData.m_startTickCount) / 1000));
		SendDlgItemMessage(g_hMainDialog, IDC_STATIC_READ_TIME, WM_SETTEXT, 0, (LPARAM)szBuffer);
	}
#endif
}


int GetFilePath(wchar_t *szPath)
{
	wchar_t szFile[MAX_PATH] = {0,};
	// open a file name
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.pem\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	GetOpenFileName( &ofn );
	if(_tcslen(szFile) > 0)
		wcscpy(szPath, szFile);
	return 0;
}

int GetFileContent(char* filePath, BYTE *pBuffer, int *pLen)
{
  FILE * pFile;
  size_t result;

  pFile = fopen ( filePath , "r" );
  if (pFile==NULL) return 0;

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  *pLen = ftell (pFile);
#ifdef WINCE
  fseek (pFile, 0, SEEK_SET);
#else
  rewind (pFile);
#endif

  // copy the file into the buffer:
  result = fread (pBuffer, 1, *pLen, pFile);
  *pLen = result;

  /* the whole file is now loaded in the memory buffer. */

  // terminate
  fclose (pFile);
  return *pLen;
}

void ProcessSecurityParams(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	static bool fSecParamsInitialized = false;
	// Process and add security parameters, if present
	//if (!fSecParamsInitialized && g_secConnParams.secureMode)
	if (g_secConnParams.secureMode)
	{
		char tempStr[MAX_PATH];
		if (wcslen(g_appData.m_szClientCertFilePath) > 0)
		{
			wcstombs(tempStr, g_appData.m_szClientCertFilePath, MAX_PATH);
			if (GetFileContent(tempStr, g_ClientCertContent, &g_ClientCertSize) > 0)
			{
				g_secConnParams.clientCertBuff = g_ClientCertContent;
				g_secConnParams.sizeCertBuff = g_ClientCertSize;
			}
		}
		if (wcslen(g_appData.m_szClientKeyFilePath) > 0)
		{
			wcstombs(tempStr, g_appData.m_szClientKeyFilePath, MAX_PATH);
			if (GetFileContent(tempStr, g_ClientKeyContent, &g_ClientKeySize) > 0)
			{
				g_secConnParams.clientKeyBuff = g_ClientKeyContent;
				g_secConnParams.sizeKeyBuff = g_ClientKeySize;
			}
		}
		if (wcslen(g_appData.m_szKeyPassword) > 0)
		{
			wcstombs((char *)g_Password, g_appData.m_szKeyPassword, MAX_PATH);
			if(strlen((char *)g_Password) > 0)
			{
				g_secConnParams.phraseBuff = g_Password;
				g_secConnParams.sizePhraseBuff = strlen((char *)g_Password);
			}
		}
		if (wcslen(g_appData.m_szRootCertFilePath) > 0)
		{
			wcstombs(tempStr, g_appData.m_szRootCertFilePath, MAX_PATH);
			if (GetFileContent(tempStr, g_RootCertContent, &g_RootCertSize) > 0)
			{
				g_secConnParams.rootCertBuff = g_RootCertContent;
				g_secConnParams.sizeRootCertBuff = g_RootCertSize;
			}
		}
		if (IsDlgButtonChecked(hDlg, IDC_VALIDATE_PEER_CERTIFICATE))
		{
			g_secConnParams.validatePeerCert = true;
		}
		else
		{
			g_secConnParams.validatePeerCert = false;
		}
		fSecParamsInitialized = true;
	}
}
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	hmenu = (HMENU)wParam; 
	UINT32 enableMenu;
	HICON hIcon;
	BOOLEAN powerState = false;

	switch(Message)
	{
	case WM_INITMENUPOPUP:
		{
			if(g_appData.m_ConnectionStatus == TRUE)
				enableMenu = MF_ENABLED;
			else
				enableMenu = MF_GRAYED;

			EnableMenuItem(hmenu, ID_CAPABILITIES, enableMenu);
			EnableMenuItem(hmenu, ID_CONFIGURATION_ANTENNA, enableMenu);
			EnableMenuItem(hmenu, ID_CONFIGURATION_RFMODE, enableMenu);

			EnableMenuItem(hmenu, ID_CONFIGURATION_SINGULATION, enableMenu);

			if(g_appData.m_ReaderCaps.radioPowerControlSupported)
			{
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_RADIOPOWERON, enableMenu);

				BOOLEAN powerState = false;
				MENUITEMINFOW menuItemInfo;
				TCHAR str[] = L"Power Off Radio";

				RFID_GetRadioPowerState(g_appData.m_RfidReaderHandle, &powerState);
				menuItemInfo.cbSize = sizeof(MENUITEMINFO);
				menuItemInfo.fMask = MIIM_TYPE;
				menuItemInfo.fType = MFT_STRING;
				menuItemInfo.fState = MFS_DEFAULT;
				menuItemInfo.cch = wcslen(str);
				menuItemInfo.dwTypeData = str;
				if(powerState)
				{
					menuItemInfo.dwTypeData = L"Power Off Radio";
					SetMenuItemInfo(hmenu, ID_READERMANAGEMENT_RADIOPOWERON,false, &menuItemInfo); 
				}
				else
				{
					menuItemInfo.dwTypeData = L"Power On Radio";
					SetMenuItemInfo(hmenu, ID_READERMANAGEMENT_RADIOPOWERON,false, &menuItemInfo); 
				}
			}
			else
			{
				DeleteMenu(hmenu, ID_READERMANAGEMENT_RADIOPOWERON, MF_BYCOMMAND);

			}

			/* 
			if(g_appData.m_ReaderType == MC)
			{
				DeleteMenu(hmenu, ID_READERMANAGEMENT_ANTENNAMODE, MF_BYCOMMAND);
				DeleteMenu(hmenu, ID_READERMANAGEMENT_REBOOT, MF_BYCOMMAND);
				DeleteMenu(hmenu, ID_READERMANAGEMENT_READPOINT, MF_BYCOMMAND);
			}
			*/


			if(g_appData.m_ReaderCaps.numGPIs || g_appData.m_ReaderCaps.numGPOs)
			{
				EnableMenuItem(hmenu, ID_CONFIGURATION_GPIO, enableMenu);
			}
			else
			{
				DeleteMenu(hmenu, ID_CONFIGURATION_GPIO, MF_BYCOMMAND);
			}

			EnableMenuItem(hmenu, ID_READER_RESETFACTORYDEFAULTS, enableMenu);
			EnableMenuItem(hmenu, ID_TAG_STORAGESETTINGS, enableMenu);
			EnableMenuItem(hmenu, ID_FILTER_PRE, enableMenu);
			EnableMenuItem(hmenu, ID_FILTER_POST, enableMenu);
			EnableMenuItem(hmenu, ID_FILTER_ACCESS, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_READ, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_WRITE, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_LOCK, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_KILL, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_BLOCKWRITE, enableMenu);
			EnableMenuItem(hmenu, ID_ACCESS_BLOCK_ERASE, enableMenu);
			EnableMenuItem(hmenu, ID_TAG_TRIGGERS, enableMenu);
			EnableMenuItem(hmenu, ID_OPERATIONS_ANTENNAINFO, enableMenu);
			if(g_appData.m_LoggedIn)
			{
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_ANTENNAMODE, MF_ENABLED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_READPOINT, MF_ENABLED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_REBOOT, MF_ENABLED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_SOFTWAREUPDATE, MF_ENABLED);
			}
			else
			{
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_ANTENNAMODE, MF_GRAYED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_READPOINT, MF_GRAYED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_REBOOT, MF_GRAYED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_SOFTWAREUPDATE, MF_GRAYED);
			}

			if(g_appData.m_ReaderType == MC)
			{
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_ANTENNAMODE, MF_GRAYED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_READPOINT, MF_GRAYED);
				EnableMenuItem(hmenu, ID_READERMANAGEMENT_REBOOT, MF_GRAYED);

			}
			EnableMenuItem(hmenu, IDC_START_READ, enableMenu);
			EnableMenuItem(hmenu, IDC_CB_MEMORY_BANK, enableMenu);

		}
		break;
	case WM_CONTEXTMENU:
		if (ListView_GetSelectedCount(GetDlgItem(g_hMainDialog, IDC_TAG_REPORT)) > 0)
			DoContextMenu(hWnd, wParam, lParam);
		break;
	case WM_CLOSE:
		{
			// Invoke Application Clean-up
			Cleanup();
			PostQuitMessage(0);
			EndDialog(hWnd,0); // kill dialog
		}
		break;
	case WM_INITDIALOG:
		{
			g_hTagList = GetDlgItem(hWnd, IDC_TAG_REPORT);
			g_hMainDialog = hWnd;

			Createbutton(hWnd);
			Createmenu(hWnd);

			// Load Memory Bank Drop List 
			for(int nIndex = 0; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
			{
				SendMessage(GetDlgItem(hWnd, IDC_CB_MEMORY_BANK), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);
			}

			// Set the default to NONE 
			SendMessage(GetDlgItem(hWnd, IDC_CB_MEMORY_BANK), CB_SETCURSEL, 0, 0);

			// Connect to reader if it runs on Windows Mobile
#ifdef WINCE
			ConnectToReader(g_appData.m_szHostName, _ttoi(g_appData.m_szPort));
#endif

			// Update main dialog
			UpdateMainDialog();

			// Create List View 
			CreateListView();	
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam)) // what we press on?
			{
			case ID_CONFIG_CONNECTION:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_CONNECT), hWnd, ConnectDlg);
				break;

			case ID_CAPABILITIES:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_CAPABILITIES), hWnd, CapabilitiesDlg );
				break;

			case ID_READER_RESETFACTORYDEFAULTS:
				g_appData.m_RfidStatus = RFID_ResetConfigToFactoryDefaults(g_appData.m_RfidReaderHandle);
				if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
				{
					// From API3 version 5.1.XX and higher, RFID_ResetConfigToFactoryDefaults would
					// reset TagStorageSettings to API defaults.
					g_appData.m_TagStorageSettings.enableAccessReports = false;
					g_appData.m_TagStorageSettings.maxMemoryBankByteCount = 64;
					g_appData.m_TagStorageSettings.maxTagCount = 4096;
					g_appData.m_TagStorageSettings.maxTagIDByteCount = 12;
					g_appData.m_TagStorageSettings.tagFields = (UINT16)(ANTENNA_ID				|
						FIRST_SEEN_TIME_STAMP	|
						LAST_SEEN_TIME_STAMP	|
						PEAK_RSSI				|
						TAG_SEEN_COUNT			|
						PC						|
						XPC						|
						CRC);

				}
				PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
				break;

			case ID_READER_EXIT:
			case IDC_APPLY:
				EndDialog(hWnd, LOWORD(wParam));
				SendMessage (hWnd, WM_CLOSE, 0, 0);				
				break;

			case ID_TAG_STORAGESETTINGS:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_TAG_STORAGE ), hWnd, TagStorageDlg);
				break;

			case ID_TAG_TRIGGERS:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_TRIGGER_INFO ), hWnd, TriggerInfoDlg);
				break;

			case ID_OPERATIONS_ANTENNAINFO:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_ANTENNA_INFO ), hWnd, AntennaInfoDlg);
				break;

			case ID_READERMANAGEMENT_LOGIN:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_LOGIN ), hWnd, RMLoginDlg);
				break;

			case ID_READERMANAGEMENT_ANTENNAMODE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_ANTENNA_MODE ), hWnd, RMAntennaModeDlg);
				break;

			case ID_READERMANAGEMENT_READPOINT:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_READ_POINT ), hWnd, ReadPointDlg);
				break;

			case ID_READERMANAGEMENT_REBOOT:
				g_appData.m_RfidStatus = RFID_Restart(g_appData.m_RfidRMHandle);
				PostRFIDStatus(g_appData.m_RfidStatus, RM_INTERFACE);
				break;
			case ID_READERMANAGEMENT_SOFTWAREUPDATE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_SWUPDATE ), hWnd, RMSoftwareUpdateDlg);
				break;

			case ID_HELP_ABOUT:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlg);
				break;

			case ID_ACCESS_READ:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_READ), hWnd, AccessReadDlg);
				break;

			case ID_ACCESS_LOCK:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_LOCK), hWnd, AccessLockDlg);
				break;

			case ID_ACCESS_KILL	:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_KILL), hWnd, AccessKillDlg);
				break;

			case ID_ACCESS_WRITE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_WRITE), hWnd, AccessWriteDlg);
				break;

			case ID_ACCESS_BLOCKWRITE:
				g_appData.m_BlockWrite = true;
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_WRITE), hWnd, AccessWriteDlg);
				g_appData.m_BlockWrite = false;
				break;
			case ID_TAG_LOCATE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_LOCATE), hWnd, LocateTagDlg);
				break;

			case ID_ACCESS_BLOCK_ERASE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_ERASE), hWnd, AccessBlockEraseDlg);
				break;

			case ID_CONFIGURATION_ANTENNA://config menu starts here
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_ANTENNA_CONFIG), hWnd, AntennaConfigDlg);
				break;

			case ID_CONFIGURATION_RFMODE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_RFMODE), hWnd, RFmodeConfigDlg);
				break;

			case ID_CONFIGURATION_GPIO:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_GPIO), hWnd, GPIOConfigDlg);
				break;

			case ID_CONFIGURATION_SINGULATION:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_SINGULATION), hWnd, SingulationConfigDlg);
				break;

			case ID_READERMANAGEMENT_RADIOPOWERON://config menu starts here

				RFID_GetRadioPowerState(g_appData.m_RfidReaderHandle, &powerState);
				if(powerState)
				{
					g_appData.m_RfidStatus = RFID_SetRadioPowerState(g_appData.m_RfidReaderHandle, false);
				}
				else
				{
					g_appData.m_RfidStatus = RFID_SetRadioPowerState(g_appData.m_RfidReaderHandle, true);

				}
				break;

			case ID_FILTER_PRE:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_PRE_FILTER), hWnd, PreFilterDlg);
				break;

			case ID_FILTER_POST:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hWnd, PostFilterDlg);
				break;

			case ID_FILTER_ACCESS:
				DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hWnd, AccessFilterDlg);
				break;

			case IDC_START_READ:

				if (g_appData.m_OperationState == RUNNING)
				{
					if (StopReading())
					{
						UpdateReadState(IDLE);
					}
				}
				else
				{
					if (StartReading())
					{
						UpdateReadState(RUNNING);
					}
				}
				break;	

			case IDC_CLEAR_LIST:
				ListView_DeleteAllItems(GetDlgItem(hWnd,IDC_TAG_REPORT));
				SendDlgItemMessage(hWnd, IDC_CLEAR_LIST, BM_SETCHECK, BST_UNCHECKED, 0);
				g_appData.m_CummulativeTagCount = 0;
				g_appData.m_UniqueTagCount = 0;
				// Set the read time to 0 sec if it is mobile app
				SendDlgItemMessage(g_hMainDialog, IDC_STATIC_READ_TIME, WM_SETTEXT, 0, (LPARAM)TEXT("0 Sec"));	
				UpdateTagCount();
				break;

			case IDC_CHECK_AUTONOMOUS:
				if ( HIWORD( wParam ) == BN_CLICKED )
				{
					if(IsDlgButtonChecked(hWnd, IDC_CHECK_AUTONOMOUS))
					{
						g_appData.m_TriggerInfo.lpTagEventReportInfo = &g_appData.m_TagEventReportInfo;
					}
					else
					{
						g_appData.m_TriggerInfo.lpTagEventReportInfo = NULL;
					}
				}
				break;
			case IDOK:

				EndDialog(hWnd, LOWORD(wParam));
				SendMessage (hWnd, WM_CLOSE, 0, 0);				

			}
		}
		break;
	default:
		{
			return FALSE;
		}
	}

	return TRUE;
}


void UpdateMainDialog(void)
{
	UINT32 enableMenu;
	UINT32 dlgItem;
	UINT16 nIndex;
	LRESULT result;
	BOOLEAN enable;
	GPI_PORT_STATE state;
	RFID_STATUS rfidStatus;

	if(g_appData.m_ConnectionStatus == TRUE)
	{

		SetWindowText(GetDlgItem(g_hMainDialog, IDC_START_READ), TEXT("Start Read"));
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_START_READ), TRUE);

		EnableWindow(GetDlgItem(g_hMainDialog, IDC_CB_MEMORY_BANK), TRUE);
		EnableWindow(GetDlgItem(g_hMainDialog, MEM_BANK_TEXT), TRUE);
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_CLEAR_LIST), TRUE);
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_CHECK_AUTONOMOUS), TRUE);

		dlgItem = IDC_PIC1;
		for(nIndex = 1; nIndex <=(g_appData.m_ReaderCaps.numGPIs); nIndex++, dlgItem++)
		{ 			
			rfidStatus = RFID_GetGPIState(g_appData.m_RfidReaderHandle, nIndex, &enable, &state);

			if (RFID_API_SUCCESS == rfidStatus)
				UpdateGPI(nIndex, enable, state);

			//ShowWindow(GetDlgItem(g_hMainDialog, GPI_STATES_TEXT), SW_SHOW);

		}
		dlgItem = IDC_STATIC_GPI1;
		for(nIndex = 1; nIndex <=g_appData.m_ReaderCaps.numGPIs; nIndex++, dlgItem++)
			ShowWindow(GetDlgItem(g_hMainDialog, dlgItem),SW_SHOW);

		SendMessage(GetDlgItem(g_hMainDialog, IDC_PIC7),STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)g_appData.m_BitmapGreen); 
		if(g_appData.m_LoggedIn)
		{
			enableMenu = MF_ENABLED;
		}
		else
		{
			enableMenu = MF_GRAYED;
		}
		//EnableMenuItem(hmenu, ID_MGMT_ANTENNAMODE, enableMenu);
		//EnableMenuItem(hmenu, ID_READERMANAGEMENT_REBOOT, enableMenu);
		//EnableMenuItem(hmenu, ID_READERMANAGEMENT_SOFTWAREUPDATE, enableMenu);
	}
	else
	{  
		SetWindowText(GetDlgItem(g_hMainDialog, IDC_START_READ), TEXT("Start Read"));
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_START_READ), FALSE);
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_CB_MEMORY_BANK), FALSE);
		EnableWindow(GetDlgItem(g_hMainDialog, MEM_BANK_TEXT), FALSE);
		EnableWindow(GetDlgItem(g_hMainDialog, IDC_CHECK_AUTONOMOUS), FALSE);

		dlgItem = IDC_PIC1;
		for(nIndex = 1; nIndex <= g_appData.m_ReaderCaps.numGPIs; nIndex++, dlgItem++)
		{ 				
			SendMessage(GetDlgItem(g_hMainDialog, dlgItem), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_appData.m_BitmapPlain); 
			ShowWindow(GetDlgItem(g_hMainDialog, dlgItem), SW_SHOW);
		}
		result = SendMessage(GetDlgItem(g_hMainDialog, IDC_PIC7), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_appData.m_BitmapRed); 
	}
}

void PathStripFileName(TCHAR *Path) 
{
    size_t Len = _tcslen(Path);
    if (Len==0) {return;};
    size_t Idx = Len-1;
    while (TRUE) {
        TCHAR Chr = Path[Idx];
        if (Chr==TEXT('\\')||Chr==TEXT('/')) {
            if (Idx==0||Path[Idx-1]==':') {Idx++;};
            break;
        } else if (Chr==TEXT(':')) {
            Idx++; break;
        } else {
            if (Idx==0) {break;} else {Idx--;};
        };
    };
    Path[Idx] = TEXT('\0');
};

INT_PTR CALLBACK ConnectDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int status = TRUE;
	HWND hmainDlg;
	g_hConnectionDlg = hDlg;
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);
		SetDlgItemText(hDlg, IDC_TB_HOSTNAME, g_appData.m_szHostName);
		SetDlgItemText(hDlg,  IDC_TB_PORT, g_appData.m_szPort);

		SetDlgItemText(hDlg, IDC_CERTIFICATE_FILE_EDITBOX, g_appData.m_szClientCertFilePath);
		SetDlgItemText(hDlg, IDC_PRIVATE_KEY_FILE_EDITBOX, g_appData.m_szClientKeyFilePath);
		SetDlgItemText(hDlg, IDC_KEY_PASS_PHARSE_EDITBOX, g_appData.m_szKeyPassword);
		SetDlgItemText(hDlg, IDC_ROOT_CERT_FILE_EDITBOX, g_appData.m_szRootCertFilePath);

		CheckDlgButton(hDlg, IDC_SECURE_MODE_CHECKBOX, g_appData.m_bSecureConnection);
		CheckDlgButton(hDlg, IDC_VALIDATE_PEER_CERTIFICATE, g_appData.m_bValidatePeer);

		if(g_appData.m_ConnectionStatus == TRUE)
		{
			SetWindowText ( GetDlgItem(hDlg, IDC_CONNECT), L"Disconnect");
			EnableWindow(GetDlgItem(hDlg,IDC_TB_HOSTNAME), false);
			EnableWindow(GetDlgItem(hDlg,IDC_TB_PORT), false);
			EnableWindow(GetDlgItem(hDlg,IDC_SECURE_MODE_CHECKBOX), false);
		}
		else if (g_appData.m_bSecureConnection) 
		{
			EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), true);
			EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_EDITBOX), true);
			EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_BROWSE_BUTTON), true);
			EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_EDITBOX), true);
			EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON), true);
			EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_EDITBOX), true);
			EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_BROWSE_BUTTON), true);
			EnableWindow(GetDlgItem(hDlg,IDC_KEY_PASS_PHARSE_EDITBOX), true);
			EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), true);
			return (INT_PTR)TRUE;
		}
		EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), false);
		EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_EDITBOX), false);
		EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_BROWSE_BUTTON), false);
		EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_EDITBOX), false);
		EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON), false);
		EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_EDITBOX), false);
		EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_BROWSE_BUTTON), false);
		EnableWindow(GetDlgItem(hDlg,IDC_KEY_PASS_PHARSE_EDITBOX), false);
		EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), false);

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_CONNECT)
		{
			if(!(g_appData.m_ConnectionStatus))
			{
				GetDlgItemText(hDlg, IDC_TB_HOSTNAME, g_appData.m_szHostName, 50);
				GetDlgItemText(hDlg, IDC_TB_PORT, g_appData.m_szPort, 50);

				// Get the secure parameters
				GetDlgItemText(hDlg, IDC_CERTIFICATE_FILE_EDITBOX, g_appData.m_szClientCertFilePath, MAX_PATH);
				GetDlgItemText(hDlg, IDC_PRIVATE_KEY_FILE_EDITBOX, g_appData.m_szClientKeyFilePath, MAX_PATH);
				GetDlgItemText(hDlg, IDC_KEY_PASS_PHARSE_EDITBOX, g_appData.m_szKeyPassword, MAX_PATH);
				GetDlgItemText(hDlg, IDC_ROOT_CERT_FILE_EDITBOX, g_appData.m_szRootCertFilePath, MAX_PATH);
				
				if(IsDlgButtonChecked(hDlg, IDC_SECURE_MODE_CHECKBOX)) 
					g_appData.m_bSecureConnection = true; 
				else 
					g_appData.m_bSecureConnection = false;

				if(IsDlgButtonChecked(hDlg, IDC_VALIDATE_PEER_CERTIFICATE)) 
					g_appData.m_bValidatePeer = true; 
				else 
					g_appData.m_bValidatePeer = false;
				
				ProcessSecurityParams(hDlg, wParam, lParam);

				if ( ConnectToReader(g_appData.m_szHostName, _ttoi(g_appData.m_szPort)) )
					EndDialog(hDlg, LOWORD(wParam));				
			}
			else
			{   
				// Stop Inventory if it is already running...
				if (RUNNING == g_appData.m_OperationState)
				{
					// Stop Read
					PostMessage(g_hMainDialog, WM_COMMAND, IDC_START_READ, 0); 
				}

				// Disconnect now
				if(DisconnectFromReader())
				{	
					SetWindowText ( GetDlgItem(hDlg, IDC_CONNECT), L"Connect");

					EnableWindow(GetDlgItem(hDlg,IDC_TB_HOSTNAME), true);
					EnableWindow(GetDlgItem(hDlg,IDC_TB_PORT), true);
					EnableWindow(GetDlgItem(hDlg,IDC_SECURE_MODE_CHECKBOX), true);

					if (g_appData.m_bSecureConnection) 
					{
						EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), true);
						EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_EDITBOX), true);
						EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_BROWSE_BUTTON), true);
						EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_EDITBOX), true);
						EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON), true);
						EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_EDITBOX), true);
						EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_BROWSE_BUTTON), true);
						EnableWindow(GetDlgItem(hDlg,IDC_KEY_PASS_PHARSE_EDITBOX), true);
						EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), true);
					}

				}
			}			

			
			UpdateMainDialog();
			if(!g_appData.m_ConnectionStatus) {
				// Update Status Bar with error
				TCHAR szErrorString[MAX_PATH] = {0,};
				_tcscpy(szErrorString, RFID_GetErrorDescription(g_appData.m_RfidStatus));

				SendDlgItemMessage(g_hMainDialog, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szErrorString);
			}
			else
			{
				//Post Status to the status bar
				PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
			}
		}
		else if(LOWORD(wParam) == IDC_CERTIFICATE_FILE_BROWSE_BUTTON)
		{
			GetFilePath(g_appData.m_szClientCertFilePath);			
			SetDlgItemText(hDlg, IDC_CERTIFICATE_FILE_EDITBOX, g_appData.m_szClientCertFilePath);
		}
		else if(LOWORD(wParam) == IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON)
		{
			GetFilePath(g_appData.m_szClientKeyFilePath);
			SetDlgItemText(hDlg, IDC_PRIVATE_KEY_FILE_EDITBOX, g_appData.m_szClientKeyFilePath);
		}
		else if(LOWORD(wParam) == IDC_ROOT_CERT_FILE_BROWSE_BUTTON)
		{
			GetFilePath(g_appData.m_szRootCertFilePath);
			SetDlgItemText(hDlg, IDC_ROOT_CERT_FILE_EDITBOX, g_appData.m_szRootCertFilePath);
		}
		else if(LOWORD(wParam) == IDOK)
		{
			hmainDlg = GetParent(hDlg);
			EndDialog(hDlg, LOWORD(wParam));
		}
		else if(LOWORD(wParam) == IDC_SECURE_MODE_CHECKBOX)
		{
			//if (SendDlgItemMessage(hDlg,IDC_SECURE_MODE_CHECKBOX,BM_GETCHECK,0,0) == BST_CHECKED)
			if(IsDlgButtonChecked(hDlg, IDC_SECURE_MODE_CHECKBOX))
			{
				// change the port no. first
				SetDlgItemText(hDlg,  IDC_TB_PORT, TEXT("5085"));
				// Enable the other controls
				EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), true);
				EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_EDITBOX), true);
				EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_BROWSE_BUTTON), true);
				EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_EDITBOX), true);
				EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON), true);
				EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_EDITBOX), true);
				EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_BROWSE_BUTTON), true);
				EnableWindow(GetDlgItem(hDlg,IDC_KEY_PASS_PHARSE_EDITBOX), true);

				// set the certificate path as project path
				HMODULE hModule = GetModuleHandleW(NULL);
				WCHAR path[MAX_PATH];
				GetModuleFileNameW(hModule, path, MAX_PATH);
				PathStripFileName(path);
				wcscpy(g_appData.m_szClientCertFilePath, path);
				wcscat(g_appData.m_szClientCertFilePath, L"\\client_crt.pem");
				SetDlgItemText(hDlg, IDC_CERTIFICATE_FILE_EDITBOX, g_appData.m_szClientCertFilePath);
				
				wcscpy(g_appData.m_szClientKeyFilePath, path);
				wcscat(g_appData.m_szClientKeyFilePath, L"\\client_key.pem");
				SetDlgItemText(hDlg, IDC_PRIVATE_KEY_FILE_EDITBOX, g_appData.m_szClientKeyFilePath);

				SetDlgItemText(hDlg, IDC_KEY_PASS_PHARSE_EDITBOX, TEXT("abcd12345"));

				wcscpy(g_appData.m_szRootCertFilePath, path);
				wcscat(g_appData.m_szRootCertFilePath, L"\\cacert.pem");
				SetDlgItemText(hDlg, IDC_ROOT_CERT_FILE_EDITBOX, g_appData.m_szRootCertFilePath);

				g_secConnParams.secureMode = true;
				g_ConnectionInfo.lpSecConInfo = &g_secConnParams;
			}
			else
			{
				// change the port no. back to 5084
				SetDlgItemText(hDlg,  IDC_TB_PORT, TEXT("5084"));
				// Enable the other controls
				EnableWindow(GetDlgItem(hDlg,IDC_VALIDATE_PEER_CERTIFICATE), false);
				EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_EDITBOX), false);
				EnableWindow(GetDlgItem(hDlg,IDC_CERTIFICATE_FILE_BROWSE_BUTTON), false);
				EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_EDITBOX), false);
				EnableWindow(GetDlgItem(hDlg,IDC_PRIVATE_KEY_FILE_BROWSE_BUTTON), false);
				EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_EDITBOX), false);
				EnableWindow(GetDlgItem(hDlg,IDC_ROOT_CERT_FILE_BROWSE_BUTTON), false);
				EnableWindow(GetDlgItem(hDlg,IDC_KEY_PASS_PHARSE_EDITBOX), false);

				g_secConnParams.secureMode = false;
				g_ConnectionInfo.lpSecConInfo = NULL;
			}			
		}
		return (INT_PTR)TRUE;
		
	case WM_CLOSE:

		hmainDlg = GetParent(hDlg);
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	VERSION_INFO VersionInfo;
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		RFID_GetDllVersionInfo(&VersionInfo);
		SetWindowText(hDlg, L"About");
		SetDlgItemText(hDlg, IDC_TB_DLL_VERSION, VersionInfo.dllVersion);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

// Get Communication Standard based on index
TCHAR* GetCommunicationStandard(int index)
{	
	switch(index)
	{
	case 1: return(L"US_FCC_PART_15");
	case 2: return(L"ETSI_302_208");
	case 3: return(L"ETSI_300_220");
	case 4: return(L"AUSTRALIA_LIPD_1W");
	case 5: return(L"AUSTRALIA_LIPD_4W");
	case 6: return(L"JAPAN_ARIB_STD_T89");
	case 7: return(L"HONGKONG_OFTA_1049");
	case 8: return(L"TAIWAN_DGT_LP0002");
	case 9: return(L"KOREA_MIC_ARTICLE_5_2");
	default: return(L"UNSPECIFIED");
	}
}


// Message handler for Capabilities box.
INT_PTR CALLBACK CapabilitiesDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hList=NULL;  // List View identifier
	LVCOLUMN lvColumn;
	LVITEM LvItem;
	TCHAR sztext[MAX_PATH] = {0};
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		hList=GetDlgItem(hDlg,IDC_CAPABILITIES_LIST); // get the ID of the ListView				 
		lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		// Insert Value Column
		lvColumn.cx = 150;
		lvColumn.pszText = L"Value";
		SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&lvColumn); 

		// Insert Capability Column
		lvColumn.cx = 190;
		lvColumn.pszText = L"Capability";
		SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&lvColumn); 

		memset(&LvItem,0,sizeof(LvItem)); // Reset Item Struct

		//  Setting properties Of Items:

		LvItem.mask=LVIF_TEXT;   // Text Style
		LvItem.cchTextMax = 256; // Max size of test

		//  Attributes being added 
		// lets add a new Items:

		LvItem.iItem=0;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Reader ID";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		LvItem.pszText=g_appData.m_ReaderCaps.readerID.value;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=1;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Firmware Version";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		LvItem.pszText=g_appData.m_ReaderCaps.firmWareVersion;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=2;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Model Name";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		LvItem.pszText=g_appData.m_ReaderCaps.modelName;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=3;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Number of Antennas";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.numAntennas);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=4;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Number of GPI";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.numGPIs);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=5;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Number of GPO";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.numGPOs);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=6;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Max Ops in Access Sequence";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.maxNumOperationsInAccessSequence);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=7;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Max No. of Pre-Filters";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.maxNumPreFilters);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=8;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Country Code";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		wsprintf(sztext, (LPCTSTR)TEXT("%u"), g_appData.m_ReaderCaps.countryCode);
		LvItem.pszText=sztext;   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=9;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Communication Standard";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		LvItem.pszText=GetCommunicationStandard(g_appData.m_ReaderCaps.communicationStandard);   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=10;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"UTC Clock";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.utcClockSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=11;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Block Erase";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.blockEraseSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=12;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Block Write";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.blockWriteSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=13;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Block Permalock";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.blockPermalockSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=14;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Recommission";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.recommissionSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=15;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Write UMI";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.writeUMISupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=16;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"State-aware Singulation";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.stateAwareSingulationSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=17;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"Tag Event Reporting Supported";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.tagEventReportingSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		LvItem.iItem=18;            // choose item  
		LvItem.iSubItem=0;         // Put in first coluom
		LvItem.pszText=L"RSSI Filter Supported";   // Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&LvItem); // Send to the Listview
		LvItem.iSubItem=1;         // Put in second coluom
		if(g_appData.m_ReaderCaps.rssiFilterSupported)
			LvItem.pszText=L"Yes";   //  Text to display (can be from a char variable) (Items)
		else
			LvItem.pszText=L"No";   //  Text to display (can be from a char variable) (Items)
		SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

		return (INT_PTR)TRUE;

	case WM_CLOSE:
	case WM_COMMAND:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI readerEventAwaitingThread(LPVOID pvarg)
{

	HANDLE hEvents[MAX_EVENTS];
	DWORD dwStatus;
	RFID_STATUS rfidStatus = RFID_API_SUCCESS;
	TCHAR szEventData[MAX_PATH] = {0,};
	TCHAR szAlarmSource[50] = {0,};
	TCHAR szAlarmLevel[50] = {0,};

	CRFIDAppData * globalAppData = (CRFIDAppData *)pvarg;
	HANDLE gpiEventHandle;
	HANDLE tagReadEventHandle;
	HANDLE bufferFullWarningEventHandle;
	HANDLE bufferFullEventHandle;
	HANDLE antennaEventHandle;
	HANDLE readerDisconnectedEventHandle;
	HANDLE inventoryStartEventHandle;
	HANDLE accessStartEventHandle;
	HANDLE inventoryStopEventHandle;
	HANDLE accessStopEventHandle;
	HANDLE readerExceptionEventHandle;
	HANDLE temperatureAlarmEventHandle;

	gpiEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	tagReadEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	bufferFullWarningEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	bufferFullEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	antennaEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	inventoryStartEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	inventoryStopEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	accessStartEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	accessStopEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	readerDisconnectedEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	readerExceptionEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	temperatureAlarmEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);

	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, GPI_EVENT, gpiEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, TAG_READ_EVENT, tagReadEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, BUFFER_FULL_WARNING_EVENT, bufferFullWarningEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, BUFFER_FULL_EVENT, bufferFullEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, ANTENNA_EVENT, antennaEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, INVENTORY_START_EVENT, inventoryStartEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, INVENTORY_STOP_EVENT, inventoryStopEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, ACCESS_START_EVENT, accessStartEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, ACCESS_STOP_EVENT, accessStopEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, DISCONNECTION_EVENT, readerDisconnectedEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, READER_EXCEPTION_EVENT, readerExceptionEventHandle);
	rfidStatus = RFID_RegisterEventNotification(g_appData.m_RfidReaderHandle, TEMPERATURE_ALARM_EVENT, temperatureAlarmEventHandle);

	hEvents[0] = gpiEventHandle;
	hEvents[1] = tagReadEventHandle;
	hEvents[2] = bufferFullWarningEventHandle;
	hEvents[3] = bufferFullEventHandle;
	hEvents[4] = antennaEventHandle;
	hEvents[5] = inventoryStartEventHandle;
	hEvents[6] = inventoryStopEventHandle;
	hEvents[7] = accessStartEventHandle;
	hEvents[8] = accessStopEventHandle;
	hEvents[9] = readerDisconnectedEventHandle;
	hEvents[9] = readerDisconnectedEventHandle;
	hEvents[10] = readerExceptionEventHandle;
	hEvents[11] = temperatureAlarmEventHandle;
	hEvents[MAX_EVENTS-1] = stopTestingEventHandle;

	BOOL threadRunning = TRUE;
	TAG_DATA* pTagData = RFID_AllocateTag(g_appData.m_RfidReaderHandle);

	while(threadRunning)
	{
		dwStatus = WaitForMultipleObjects(MAX_EVENTS, hEvents, FALSE, INFINITE);
		switch(dwStatus)
		{
		case WAIT_OBJECT_0://gpiEventHandle
			{
				GPI_EVENT_DATA gpiEventData;
				while(RFID_API_SUCCESS == RFID_GetEventData(g_appData.m_RfidReaderHandle, GPI_EVENT, (STRUCT_HANDLE)&gpiEventData))
				{
					UpdateGPI(gpiEventData.port, true, gpiEventData.eventInfo);
					PostNotification(GPI_EVENT, TEXT(""));
				}
				ResetEvent(gpiEventHandle);
			}
			break;
     
		case WAIT_OBJECT_0+2://bufferFullWarningEventHandle
			PostNotification(BUFFER_FULL_WARNING_EVENT, TEXT(""));
			while(RFID_API_SUCCESS == RFID_GetReadTag(g_appData.m_RfidReaderHandle, pTagData))
			{
				printTagData(pTagData);
			}
			ResetEvent(bufferFullWarningEventHandle);
			break;

		case WAIT_OBJECT_0+3://bufferFullEventHandle
			PostNotification(BUFFER_FULL_EVENT, TEXT(""));
			while(RFID_API_SUCCESS == RFID_GetReadTag(g_appData.m_RfidReaderHandle, pTagData))
			{
				printTagData(pTagData);
			}
			ResetEvent(bufferFullEventHandle);
			break;

		case WAIT_OBJECT_0+1://tagReadEventHandle	
			while(RFID_API_SUCCESS == RFID_GetReadTag(g_appData.m_RfidReaderHandle, pTagData))
			{
				if (pTagData->lpLocation)
				{
					g_appData.m_locatedTagSeenTickCount = GetTickCount();
					SendMessage(GetDlgItem(g_appData.m_hLocateDialog, IDC_PROGRESS_BAR), PBM_SETPOS, pTagData->lpLocation->relativeDistance, 0);
				}
				else
				{
					printTagData(pTagData);
				}
			}
			ResetEvent(tagReadEventHandle);
			break;

		case WAIT_OBJECT_0+4://antennaEventHandle
			{
				ANTENNA_EVENT_DATA antennaEventData;
				_tcscpy(szEventData, TEXT(""));
				if(RFID_API_SUCCESS == RFID_GetEventData(g_appData.m_RfidReaderHandle, ANTENNA_EVENT, (STRUCT_HANDLE)&antennaEventData))
				{
					_stprintf(szEventData, TEXT("Antenna %u %s"), antennaEventData.id, antennaEventData.eventInfo ? TEXT("connected") : TEXT("disconnected"));
				}
			}
			PostNotification(ANTENNA_EVENT, szEventData);
			break;

		case WAIT_OBJECT_0+5://inventoryStartEventHandle
			PostNotification(INVENTORY_START_EVENT, TEXT(""));
			UpdateReadState(RUNNING);
			break;

		case WAIT_OBJECT_0+6://inventoryStopEventHandle
			PostNotification(INVENTORY_STOP_EVENT, TEXT(""));
			if (globalAppData->m_TriggerInfo.startTrigger.type == START_TRIGGER_TYPE_IMMEDIATE)
			{
			UpdateReadState(IDLE);
			}
			while(RFID_API_SUCCESS == RFID_GetReadTag(g_appData.m_RfidReaderHandle, pTagData))
			{
				printTagData(pTagData);
			}
			break;

		case WAIT_OBJECT_0+7://accessStartEventHandle
			PostNotification(ACCESS_START_EVENT, TEXT(""));
			UpdateReadState(RUNNING);
			break;

		case WAIT_OBJECT_0+8://accessStopEventHandle
			PostNotification(ACCESS_STOP_EVENT, TEXT(""));
			UpdateReadState(IDLE);
			break;

		case WAIT_OBJECT_0+9://readerDisconnectedEventHandle
			{
				DISCONNECTION_EVENT_DATA readerDisconnectionEventData;
				_tcscpy(szEventData, TEXT(""));
				if(RFID_API_SUCCESS == RFID_GetEventData(g_appData.m_RfidReaderHandle, DISCONNECTION_EVENT, 
					(STRUCT_HANDLE)&readerDisconnectionEventData))
				{
					
					_stprintf(szEventData, TEXT("%s"), 
						READER_INITIATED_DISCONNECTION == readerDisconnectionEventData.eventInfo ? TEXT("Reader initiated disconnection" : 
						CONNECTION_LOST == readerDisconnectionEventData.eventInfo ? TEXT("Connection lost") : TEXT("")));
				}
			}
			PostNotification(DISCONNECTION_EVENT, szEventData);
			SendMessage(g_hConnectionDlg,WM_COMMAND,(WPARAM)IDC_CONNECT,NULL);
			g_appData.m_ConnectionStatus = false;
			UpdateMainDialog();

			threadRunning = false;
			break;

		case WAIT_OBJECT_0+10://readerExceptionEventHandle
			{
				READER_EXCEPTION_EVENT_DATA readerExceptionEventData;
				_tcscpy(szEventData, TEXT(""));
				if(RFID_API_SUCCESS == RFID_GetEventData(g_appData.m_RfidReaderHandle, READER_EXCEPTION_EVENT, 
					(STRUCT_HANDLE)&readerExceptionEventData))
				{
					_stprintf(szEventData, TEXT("%s"), readerExceptionEventData.exceptionInfo);
				}
			}
			PostNotification(READER_EXCEPTION_EVENT, szEventData);
			break;
		case WAIT_OBJECT_0+11: // temperatureAlarmEventHandle
			{
				TEMPERATURE_ALARM_DATA temperatureAlarmEventData;
				_tcscpy(szEventData, TEXT(""));
				if(RFID_API_SUCCESS == RFID_GetEventData(g_appData.m_RfidReaderHandle, TEMPERATURE_ALARM_EVENT, (STRUCT_HANDLE)&temperatureAlarmEventData))
				{
					if (temperatureAlarmEventData.sourceName == PA)
						_tcscpy(szAlarmSource, TEXT("PA"));
					else if(temperatureAlarmEventData.sourceName == AMBIENT)
						_tcscpy(szAlarmSource, TEXT("Ambient"));

					if (temperatureAlarmEventData.alarmLevel == HIGH)
						_tcscpy(szAlarmLevel, TEXT("High"));
					else if (temperatureAlarmEventData.alarmLevel == LOW)
						_tcscpy(szAlarmLevel, TEXT("Low"));	
					else if (temperatureAlarmEventData.alarmLevel == CRITICAL)
						_tcscpy(szAlarmLevel, TEXT("Critical"));

					_stprintf(szEventData, TEXT("%s %d %s"), szAlarmSource, temperatureAlarmEventData.currentTemperature, szAlarmLevel );
				}
			}
			PostNotification(TEMPERATURE_ALARM_EVENT, szEventData);
			break;
		case WAIT_OBJECT_0+12://stopTestingEventHandle
			threadRunning = false;
			break;
		default:
			break;
		}
	}

	CloseHandle(gpiEventHandle);
	CloseHandle(tagReadEventHandle);
	CloseHandle(bufferFullEventHandle);
	CloseHandle(bufferFullWarningEventHandle);
	CloseHandle(antennaEventHandle);
	CloseHandle(inventoryStartEventHandle);
	CloseHandle(inventoryStopEventHandle);
	CloseHandle(accessStartEventHandle);
	CloseHandle(accessStopEventHandle);
	CloseHandle(readerDisconnectedEventHandle);
	CloseHandle(readerExceptionEventHandle);
	CloseHandle(stopTestingEventHandle);

	RFID_DeallocateTag(g_appData.m_RfidReaderHandle, pTagData);

	return 0;
}

// Format Tag and print on screen
void printTagData(TAG_DATA *pTagData)
{
	char  tagBuffer[MAX_PATH] = {0,};

	char* pTagReportData = tagBuffer;
	UINT32  index = 0;
	TCHAR	resultBuffer[MAX_PATH];
	LVITEM lvItem;  // ListView Item struct
	UINT8 memoryBankId;
	UINT16 nIndex = 0;
	UINT8 tagState = 0;
	BOOLEAN uniqueTag = false;

	if(g_appData.m_OperationState == IDLE)
		return;

	// Check the tag is already exists, 
	// if exists update, else add new tag in the list view
	if (FindItem(pTagData, &uniqueTag))
		return;

	// Check for Access Operation
	// If Access, take only the OpCode Success tag only for printing
	if (pTagData->opCode != ACCESS_OPERATION_NONE &&
		pTagData->opStatus != ACCESS_SUCCESS)
		return;

	for(index = 0; index < pTagData->tagIDLength; index++)
	{
		sprintf(pTagReportData,"%02X", pTagData->pTagID[index]);
		while(*pTagReportData) pTagReportData++;

	}
	_stprintf(resultBuffer, TEXT("%S"), tagBuffer);

	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = ListView_GetItemCount(g_hTagList);

	// Tag ID
	lvItem.iSubItem = COLUMN_EPCID;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_InsertItem(g_hTagList, &lvItem);

	// Tag State (for Autonomous Mode)
	lvItem.iSubItem = COLUMN_STATE;
	tagState = (UINT8)pTagData->tagEvent;
	lvItem.pszText = (LPWSTR)tag_states[tagState - 1];
	ListView_SetItem(g_hTagList, &lvItem);

	// Antenna ID
	_stprintf(resultBuffer, TEXT("%d"), pTagData->antennaID);
	lvItem.iSubItem = COLUMN_ANTENNA_ID;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_SetItem(g_hTagList, &lvItem);


	// Tag Seen Count
	_stprintf(resultBuffer, TEXT("%d"), pTagData->tagSeenCount);
	lvItem.iSubItem = COLUMN_SEEN_COUNT;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_SetItem(g_hTagList, &lvItem);

	// Peak RSSI
	_stprintf(resultBuffer,TEXT("%d"), pTagData->peakRSSI);
	lvItem.iSubItem = COLUMN_RSSI;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_SetItem(g_hTagList, &lvItem);

	//Phase information
	_stprintf(resultBuffer,TEXT("%3.2f"), (180.0/32767)*pTagData->phaseInfo);
	lvItem.iSubItem = COLUMN_PHASE;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_SetItem(g_hTagList, &lvItem);

	// PC Bits
	_stprintf(resultBuffer, TEXT("%02X"), pTagData->PC);
	lvItem.iSubItem = COLUMN_PC_BITS;
	lvItem.pszText = (LPWSTR)resultBuffer;
	SendMessage(g_hTagList,LVM_SETITEM,0,(LPARAM)&lvItem); // Enter text to SubItems

	// Memory Bank Data
	if(pTagData->memoryBankDataLength)
	{
		pTagReportData = tagBuffer;
		for(index = 0; index < pTagData->memoryBankDataLength; index++)
		{
			sprintf(pTagReportData,"%02X", pTagData->pMemoryBankData[index]);
			while(*pTagReportData) pTagReportData++;

		}
		_stprintf(resultBuffer, TEXT("%S"), tagBuffer);

		lvItem.iSubItem = COLUMN_MEM_BANK_DATA;
		lvItem.pszText = (LPWSTR)resultBuffer;
		ListView_SetItem(g_hTagList, &lvItem);
	}

	// Memory Bank
	if (pTagData->opCode != ACCESS_OPERATION_NONE)
	{
		_stprintf(resultBuffer, TEXT("%d"), pTagData->memoryBank);
		memoryBankId = _ttoi(resultBuffer);
		lvItem.iSubItem = COLUMN_MEM_BANK;
		lvItem.pszText = (LPWSTR)memory_banks[memoryBankId+1];
		ListView_SetItem(g_hTagList, &lvItem);

	}

	// Off Set
	_stprintf(resultBuffer, TEXT("%02X"), pTagData->memoryBankDataByteOffset);
	lvItem.iSubItem = COLUMN_OFFSET;
	lvItem.pszText = (LPWSTR)resultBuffer;
	ListView_SetItem(g_hTagList, &lvItem);

	// Scroll to bottom of List
	SendMessage(g_hTagList, WM_VSCROLL, (WPARAM)SB_BOTTOM, 0);

	// Update Tag Count
	if (!uniqueTag)
		g_appData.m_UniqueTagCount++;

	g_appData.m_CummulativeTagCount++;

	// Update tag count in the main page
	UpdateTagCount();

}

// Start Reading of tags
bool StartReading()
{
	bool retVal = false;
	UINT8  memoryBankSelected = 0;
	READ_ACCESS_PARAMS readAccessParams;
	RFID_STATUS rfidStatus = RFID_API_SUCCESS;
	OP_CODE_PARAMS opCodeParams; 
	UINT32 opCodeIndex = 0;
	g_appData.m_startTickCount = 0;
	TCHAR szBuffer[MAX_PATH] = {0,};

	if(g_appData.m_ConnectionStatus)
	{
		memoryBankSelected = (UINT8)SendDlgItemMessage(g_hMainDialog, IDC_CB_MEMORY_BANK, (UINT) CB_GETCURSEL, 0, 0);  

		// If Valid Memory Bank selected, perform Access Sequence to the selected Memory Bank
		if (memoryBankSelected > 0)
		{
			RFID_InitializeAccessSequence(g_appData.m_RfidReaderHandle);

			readAccessParams.byteCount = 0;
			readAccessParams.byteOffset = 0;
			readAccessParams.accessPassword = 0;
			opCodeParams.opCode = ACCESS_OPERATION_READ;
			opCodeParams.opParams = &readAccessParams;
			readAccessParams.memoryBank = (MEMORY_BANK) (memoryBankSelected - 1);

			// Before add new opeation access sequence, delete all...
			if (RFID_API_SUCCESS != RFID_DeleteOperationFromAccessSequence (g_appData.m_RfidReaderHandle, 0))
			{
				_stprintf(szBuffer, TEXT("RFID_DeleteOperationFromAccessSequence API failed")); 
			}
			rfidStatus = RFID_AddOperationToAccessSequence(g_appData.m_RfidReaderHandle, &opCodeParams, &opCodeIndex);
			if (RFID_API_SUCCESS == rfidStatus)
			{
				// Before inventory purge all the tags
				if (RFID_API_SUCCESS != RFID_PurgeTags (g_appData.m_RfidReaderHandle, NULL))
				{
					_stprintf(szBuffer, TEXT("RFID_PurgeTags API failed")); 
				}
				// Perform Access Sequence
				rfidStatus = RFID_PerformAccessSequence(g_appData.m_RfidReaderHandle, 
					g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
					g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL,
					&g_appData.m_TriggerInfo, NULL);

				if (RFID_API_SUCCESS == rfidStatus)
				{
					g_appData.m_startTickCount = GetTickCount();
					retVal = true;
					UpdateReadState(RUNNING);
					g_appData.m_AccessSequenceRunning = true;
				}
				else
				{
					UpdateReadState(IDLE);
				}
			}		
			PostRFIDStatus(rfidStatus, GENERIC_INTERFACE);
		}
		// Inventory Operation
		else
		{
			// Before inventory purge all the tags
			if (RFID_API_SUCCESS != RFID_PurgeTags (g_appData.m_RfidReaderHandle, NULL))
			{
				_stprintf(szBuffer, TEXT("RFID_PurgeTags API failed")); 
			}
			// Perform Access Sequence
			rfidStatus = RFID_PerformInventory(g_appData.m_RfidReaderHandle,
				g_appData.m_PostFilterSet ? &g_appData.m_PostFilter : NULL,
				g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL,
				&g_appData.m_TriggerInfo, NULL);

			if (RFID_API_SUCCESS == rfidStatus)
			{
				g_appData.m_startTickCount = GetTickCount();
				retVal = true;
				UpdateReadState(RUNNING);
			}
			else
			{
				UpdateReadState(IDLE);
			}
			PostRFIDStatus(rfidStatus, GENERIC_INTERFACE);
		}
	}
	return retVal;
}

// Stop Reading of tags
bool StopReading()
{
	bool retVal = false;
	TCHAR szBuffer[MAX_PATH] = {0,};

	RFID_STATUS rfidStatus = RFID_API_SUCCESS;
	g_appData.m_stopTickCount = 0;

	if(g_appData.m_ConnectionStatus)
	{
		// If memory bank selected
		if (g_appData.m_AccessSequenceRunning)
		{
			// Stop Access Sequence
			if(rfidStatus == RFID_StopAccessSequence(g_appData.m_RfidReaderHandle))
			{				
				// Delete All Operations from the Access Sequence
				RFID_DeleteOperationFromAccessSequence(g_appData.m_RfidReaderHandle, 0);
				RFID_DeinitializeAccessSequence(g_appData.m_RfidReaderHandle);
				g_appData.m_AccessSequenceRunning = false;
				retVal = true;
				UpdateReadState(IDLE);
				g_appData.m_stopTickCount = GetTickCount();
			}

		}
		else
		{
			if(rfidStatus == RFID_StopInventory(g_appData.m_RfidReaderHandle))
			{
				retVal = true;
				UpdateReadState(IDLE);
				g_appData.m_stopTickCount = GetTickCount();
			}
		}
		// Update the Read Time in case of Mobile App
		_stprintf(szBuffer, TEXT("%lu sec"), ((g_appData.m_stopTickCount - g_appData.m_startTickCount) / 1000));
		SendDlgItemMessage(g_hMainDialog, IDC_STATIC_READ_TIME, WM_SETTEXT, 0, (LPARAM)szBuffer);	

		PostRFIDStatus(rfidStatus, GENERIC_INTERFACE);
	}
	return retVal;
}
void Createmenu(HWND hWnd)
{
#ifdef WINCE
	SHMENUBARINFO mbi;
	HWND				g_hWndMenuBar;		// menu bar handle

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize     = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDR_MENU1;
	mbi.hInstRes   = g_hInst;
	mbi.dwFlags = SHCMBF_HMENU;
	// Create Left Menu
	if (!SHCreateMenuBar(&mbi)) 
	{
		g_hWndMenuBar = NULL;
	}
	else
	{
		g_hWndMenuBar = mbi.hwndMB;
	}


#endif
}
void Createbutton(HWND hWnd)
{
#ifdef WINCE
	// Create a Done button and size it.  
	// Create a Done button and size it.  
	SHINITDLGINFO shidi;
	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags =  SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU | SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN ;
	shidi.hDlg = hWnd;
	SHInitDialog(&shidi);
#endif
}
// Print the status message on the status bar
void PostRFIDStatus(RFID_STATUS statusCode, RFID_INTERFACE interfaceType)
{
	ERROR_INFO errorInfo;
	RFID_STATUS rfidStatus;

	TCHAR szBuffer[MAX_PATH * 2] = {0,};
	TCHAR *pStatusDescription;
	pStatusDescription = RFID_GetErrorDescription(statusCode);

	_tcscpy(szBuffer, pStatusDescription);

	if (RFID_API_SUCCESS != statusCode)
	{
		if (interfaceType == RM_INTERFACE)
			rfidStatus = RFID_GetLastErrorInfo(g_appData.m_RfidReaderHandle, &errorInfo);
		else if (interfaceType == GENERIC_INTERFACE)
			rfidStatus = RFID_GetLastErrorInfo(g_appData.m_RfidReaderHandle, &errorInfo);

		if (RFID_API_SUCCESS == rfidStatus) 
			_stprintf(szBuffer, TEXT("%s [%s]"), szBuffer, errorInfo.vendorMessage);
#ifdef WINCE
		MessageBox(g_hMainDialog, szBuffer, TEXT("API Result"), MB_OK);
#endif
	}
	// Update Status Bar
	SendDlgItemMessage(g_hMainDialog, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szBuffer);	
}
// Print the status message on the status bar
void PostStatus(TCHAR *pStatusDescription)
{
	TCHAR szBuffer[MAX_PATH] = {0,};
	_tcscpy(szBuffer, pStatusDescription);
	SendDlgItemMessage(g_hMainDialog, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szBuffer);	
}

// Do Context Menu
BOOL DoContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND  hwndListView = (HWND)wParam;
	HMENU hMenuLoad,
		hMenu;

	if(hwndListView != GetDlgItem(g_hMainDialog, IDC_TAG_REPORT))
		return FALSE;

	hMenuLoad = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU2));
	hMenu = GetSubMenu(hMenuLoad, 0);
#ifdef WINCE
	TrackPopupMenu(   hMenu,
		TPM_LEFTALIGN,
		LOWORD(lParam),
		HIWORD(lParam),
		0,
		hWnd,
		NULL);
#else
	TrackPopupMenu(   hMenu,
		TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		LOWORD(lParam),
		HIWORD(lParam),
		0,
		hWnd,
		NULL);
#endif
	DestroyMenu(hMenuLoad);

	return TRUE;
}


// Finds the tag in the ListView, if exists, update the item else insert the item
bool FindItem(TAG_DATA *pTagData,  BOOLEAN* pUniqueTag)
{
	bool	bResult = false;
	char	tagBuffer[MAX_PATH] = {0,};
	char*	pTagReportData = tagBuffer;
	UINT32  index = 0;
	TCHAR	resultBuffer[MAX_PATH] = {0,};
	int		foundLocation = -1;
	int		tagSeenCount = 0;
	UINT16	nIndex = 0;
	UINT8   tagState = 0;

	// List View Item
	LVITEM	lvItem;  

	for(index = 0; index < pTagData->tagIDLength; index++)
	{
		sprintf(pTagReportData,"%02X", pTagData->pTagID[index]);
		while(*pTagReportData) pTagReportData++;

	}
	_stprintf(resultBuffer, TEXT("%S"), tagBuffer);
	foundLocation = GetItemLocation(pTagData, pUniqueTag);

	// if the tag is found, update the relevant fields in the list view item
	if (foundLocation >= 0)
	{
		lvItem.iItem = foundLocation;

		// Update Tag Seen Count
		ListView_GetItemText(g_hTagList, foundLocation, COLUMN_SEEN_COUNT, resultBuffer, MAX_PATH);
		tagSeenCount = _ttoi(resultBuffer);
		tagSeenCount += pTagData->tagSeenCount;
		_stprintf(resultBuffer, TEXT("%d"), tagSeenCount);
		lvItem.iSubItem = COLUMN_SEEN_COUNT;
		ListView_SetItemText(g_hTagList, foundLocation, COLUMN_SEEN_COUNT, resultBuffer);

		// Antenna ID
		_stprintf(resultBuffer, TEXT("%d"), pTagData->antennaID);
		ListView_SetItemText(g_hTagList, foundLocation, COLUMN_ANTENNA_ID, resultBuffer);

		// Peak RSSI
		_stprintf(resultBuffer,TEXT("%d"), pTagData->peakRSSI);
		ListView_SetItemText(g_hTagList, foundLocation, COLUMN_RSSI, resultBuffer);

		// Phase Information
		_stprintf(resultBuffer,TEXT("%3.2f"), (180.0/32767)*pTagData->phaseInfo);
		ListView_SetItemText(g_hTagList, foundLocation, COLUMN_PHASE, resultBuffer);

		// Tag State (for Autonomous Mode)
		tagState = (UINT8)pTagData->tagEvent;
		ListView_SetItemText(g_hTagList, foundLocation, COLUMN_STATE, tag_states[tagState-1]);

		bResult = true;

		g_appData.m_CummulativeTagCount++;

		// Update tag count in the main page
		UpdateTagCount();

	}

	return bResult;

}

// Converts Hex string to Byte Ptr
void ConvertHexStringToBytePtr(TCHAR *pHexString, BYTE *pData, int *dataLength)
{
	TCHAR *pHexStringData = pHexString;
	*dataLength = 0;
	int length = 0;
	while (*pHexStringData)
	{
		_stscanf(pHexStringData, TEXT("%02x"), pData);
		pHexStringData += 2;
		pData++; 
		length++;
	}
	*dataLength = length;
}

// Converts Byte Ptr to Hex string
void ConvertBytePtrToHexString(UINT8 *pTagID, UINT32 tagIDLength, TCHAR *pHexString)
{
	char  tagBuffer[MAX_PATH] = {0,};
	char* pTagReportData = tagBuffer;
	short index = 0;

	for(index = 0; index < tagIDLength; index++)
	{
		sprintf(pTagReportData,"%02X", pTagID[index]);
		while(*pTagReportData) pTagReportData++;
	}
	_stprintf(pHexString, TEXT("%S"), tagBuffer);
}

// Get Tag ID from Selected Row
bool GetSelectedTagID(TCHAR *pTagID)
{
	int foundLocation = -1;
	bool bRetVal = false;

	// Get the Tag ID
	foundLocation = (int)SendMessage(g_hTagList, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
	if (foundLocation >=0)
	{
		ListView_GetItemText(g_hTagList, foundLocation, 0, pTagID, MAX_PATH);
		bRetVal = true;
	}
	return bRetVal;
}

// Read Tag
bool ReadTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 length, UINT32 offset, 
			 UINT32 password, UINT8* pData, UINT32 *pDataLength)
{
	bool retVal = false;

	TAG_DATA* pTagData = RFID_AllocateTag(g_appData.m_RfidReaderHandle);

	READ_ACCESS_PARAMS params;
	params.accessPassword = password;
	params.byteCount = length;
	params.memoryBank = mb;
	params.byteOffset = offset;

	g_appData.m_RfidStatus = RFID_Read(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params,
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, pTagData, NULL);
	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		memcpy(pData, pTagData->pMemoryBankData, pTagData->memoryBankDataLength);
		*pDataLength = pTagData->memoryBankDataLength;
		retVal = true;
	}
	RFID_DeallocateTag(g_appData.m_RfidReaderHandle, pTagData);

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Write Tag
bool WriteTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
			  UINT32 password, UINT8* pData, UINT32 dataLength)
{
	bool retVal = false;

	WRITE_ACCESS_PARAMS params;
	params.accessPassword = password;
	params.writeDataLength = dataLength;
	params.memoryBank = mb;
	params.byteOffset = offset;
	params.pWriteData = pData;

	if (g_appData.m_BlockWrite)
		g_appData.m_RfidStatus = RFID_BlockWrite(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params,
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, NULL);
	else
		g_appData.m_RfidStatus = RFID_Write(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params, 
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, NULL);

	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Lock Tag
bool LockTag(UINT8* pTagID, UINT32 tagIDLength, LOCK_DATA_FIELD dataField, LOCK_PRIVILEGE privilege,
			 UINT32 password)
{
	bool retVal = false;

	LOCK_ACCESS_PARAMS params;
	params.accessPassword = password;
	params.privilege[dataField] = privilege;

	g_appData.m_RfidStatus = RFID_Lock(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params,
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, NULL);
	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Kill Tag
bool KillTag(UINT8* pTagID, UINT32 tagIDLength, UINT32 password)
{
	bool retVal = false;

	KILL_ACCESS_PARAMS params;
	params.killPassword = password;

	g_appData.m_RfidStatus = RFID_Kill(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params, 
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, NULL);
	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Block Erase Tag
bool BlockEraseTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
				   UINT32 password, UINT32 dataLength)
{
	bool retVal = false;
	BLOCK_ERASE_ACCESS_PARAMS params;
	params.accessPassword = password;
	params.byteCount = dataLength;
	params.memoryBank = mb;
	params.byteOffset = offset;

	g_appData.m_RfidStatus = RFID_BlockErase(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &params, 
		g_appData.m_AccessFilterSet ? &g_appData.m_AccessFilter : NULL, 
		g_appData.m_AntennaInfo.length ? &g_appData.m_AntennaInfo : NULL, NULL);

	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Start Tag Locationing 
bool StartTagLocationing(UINT8* pTagID, UINT32 tagIDLength)
{
	bool retVal = false;
	ANTENNA_INFO antennaInfo;
	OPERATION_QUALIFIER opQualifier[1] = {LOCATE_TAG};

	UINT16 antennaList[1] = {1};
	antennaInfo.pAntennaList = antennaList;
	antennaInfo.length = 1;
	antennaInfo.pAntennaOpList = opQualifier;

	g_appData.m_RfidStatus = RFID_PerformTagLocationing(g_appData.m_RfidReaderHandle, pTagID, tagIDLength, &antennaInfo, NULL, NULL);
	

	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}
	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

	return retVal;
}

// Stop Tag Locationing 
bool StopTagLocationing()
{
	bool retVal = false;
	g_appData.m_RfidStatus = RFID_StopTagLocationing(g_appData.m_RfidReaderHandle);

	if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
	{
		retVal = true;
	}

	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
	return retVal;
}

// Create List View which displays tag report in the main page
void CreateListView()
{
	// Columns for List View
	LVCOLUMN LvCol; 
	// Row Item for List View
	LVITEM LvItem;  
	LVBKIMAGE plvbki={0};

	HWND hList=NULL;  

	// Initialize COM controls
	InitCommonControls();

	// Get ID of List View
	hList=GetDlgItem(g_hMainDialog,IDC_TAG_REPORT); // get the ID of the ListView				 
	SendMessage(hList,LVM_SETTEXTBKCOLOR, 0,(LPARAM)CLR_NONE);
	ListView_SetExtendedListViewStyle(hList, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	//	SendMessage(hWnd,WM_SETICON,(WPARAM) 1,(LPARAM) LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON1)));

	// Create the Columns
	memset(&LvItem, NULL, sizeof(LvItem)); 
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of test

	// EPC ID
	memset(&LvCol,0,sizeof(LvCol)); // Reset Coluom
	LvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; // Type of mask
	LvCol.cx=0x9A;                                
	LvCol.pszText=L"EPCID";                     
	LvCol.fmt = LVCFMT_LEFT;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_EPCID, (LPARAM)&LvCol); 

	// State of the tag (Autonomous Mode)
	LvCol.cx=0x30;                                // width between each coloum
	LvCol.fmt = LVCFMT_CENTER;
	LvCol.pszText=L"State";                     // First Header
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_STATE, (LPARAM)&LvCol); // Insert/Show the coloum

	// Antenna ID
	LvCol.cx=0x45;
	LvCol.pszText=L"AntennaID";  
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_ANTENNA_ID, (LPARAM)&LvCol); 


	// Seen Count
	LvCol.cx=0x45;
	LvCol.pszText=L"Seen Count";  
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_SEEN_COUNT, (LPARAM)&LvCol); 

	// RSSI
	LvCol.cx=0x30;
	LvCol.pszText=L"RSSI"; 
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_RSSI, (LPARAM)&LvCol); 

	// Phase
	LvCol.cx=0x30;
	LvCol.pszText=L"Phase"; 
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_PHASE, (LPARAM)&LvCol); 

	// PC Bits
	LvCol.cx=0x30;
	LvCol.pszText=L"PC bits"; 
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_PC_BITS, (LPARAM)&LvCol); 

	// Memory Bank Data
	LvCol.cx=0x100;
	LvCol.pszText=L"Memory Bank Data";                     
	LvCol.fmt = LVCFMT_LEFT;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_MEM_BANK_DATA, (LPARAM)&LvCol); 

	// Memory Bank
	LvCol.cx=0x40;
	LvCol.pszText=L"MB";
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_MEM_BANK, (LPARAM)&LvCol); 

	// Off set
	LvCol.cx=0x30;
	LvCol.pszText=L"Offset"; 
	LvCol.fmt = LVCFMT_CENTER;
	SendMessage(hList, LVM_INSERTCOLUMN, COLUMN_OFFSET, (LPARAM)&LvCol);

	// Set the Status Bar to "Ready"
	SendDlgItemMessage(g_hMainDialog, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)TEXT("Ready"));
}


// Get Location of List View Item if the tag is already exists
int GetItemLocation(TAG_DATA *pTagData,  BOOLEAN* pUniqueTag)
{
	bool	bResult = false;
	char	tagBuffer[MAX_PATH] = {0,};
	char*	pTagReportData = tagBuffer;

	UINT32  index = 0;
	int     startLocation = -1;
	int		foundLocation = -1;

	TCHAR   szFindKey[MAX_PATH] = {0,};
	TCHAR	szBuffer[MAX_PATH] = {0,};
	TCHAR	szFindString[MAX_PATH] = {0,};
	TCHAR	szMatchString[MAX_PATH] = {0,};
	int		memoryBankId = 0;

	// List View Item
	LVITEM	lvItem;  
	LVFINDINFO findInfo;

	// Get the Tag ID
	for(index = 0; index < pTagData->tagIDLength; index++)
	{
		sprintf(pTagReportData,"%02X", pTagData->pTagID[index]);
		while(*pTagReportData) pTagReportData++;

	}
	_stprintf(szBuffer, TEXT("%S"), tagBuffer);
	_tcscpy(szFindKey, szBuffer);
	_tcscpy(szFindString, szFindKey);

	// Get the Bank
	if (pTagData->opCode != ACCESS_OPERATION_NONE)
	{
		_stprintf(szBuffer, TEXT("%d"), pTagData->memoryBank);
		memoryBankId = _ttoi(szBuffer);

		_tcscat(szFindString, memory_banks[memoryBankId+1]);
	}


	// Find Info
	//lvItem.iSubItem = COLUMN_EPCID;
	//findInfo.lParam = (LPARAM)&lvItem;
	findInfo.flags = LVFI_STRING;
	findInfo.psz = szFindKey;
	foundLocation = ListView_FindItem(g_hTagList, -1,  &findInfo);

	while (foundLocation >= 0)
	{
		*pUniqueTag = true;
		// Get Tag ID
		ListView_GetItemText(g_hTagList, foundLocation, COLUMN_EPCID, szBuffer, MAX_PATH);
		_tcscpy(szMatchString, szBuffer);

		// Get Memory Bank
		ListView_GetItemText(g_hTagList, foundLocation, COLUMN_MEM_BANK, szBuffer, MAX_PATH);
		_tcscat(szMatchString, szBuffer);

		// if matches
		if (_tcscmp(szFindString, szMatchString) == 0)
		{
			break;
		}
		else
		{
			startLocation = foundLocation;
			foundLocation = ListView_FindItem(g_hTagList, startLocation,  &findInfo);
		}
	}
	return foundLocation;
}

void UpdateTagCount()
{
	TCHAR szBuffer[MAX_PATH] = {0,};
	_stprintf(szBuffer, TEXT("(%lu/%lu)"), g_appData.m_UniqueTagCount, g_appData.m_CummulativeTagCount);

	SetWindowText(GetDlgItem(g_hMainDialog, IDC_STATIC_TAGS_COUNT), szBuffer);

}

bool ConnectToReader(TCHAR* pszHostName, UINT16 portNumber)
{
	bool bRetVal = false;
	DWORD dwThreadID;
	TCHAR szBuffer[MAX_PATH] = {0,};
	g_ConnectionInfo.version = RFID_API3_5_5;

	g_appData.m_RfidStatus = RFID_Connect(&g_appData.m_RfidReaderHandle, g_appData.m_szHostName,_ttoi(g_appData.m_szPort), 0, &g_ConnectionInfo);
	//>	g_appData.m_RfidReaderHandle = g_appData.m_RfidReaderHandle;

	if(g_appData.m_RfidStatus == RFID_API_SUCCESS) 
	{
		g_appData.m_ConnectionStatus = true;

		g_appData.m_OperationState = IDLE;

		// Start the Thread for receiving the Events from the Reader
		stopTestingEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		readerEventAwaitingThreadHandle = CreateThread(NULL, 0, readerEventAwaitingThread, (LPVOID)&g_appData, 0, &dwThreadID);

		// Update the Application Title
#ifdef WINCE
		_stprintf(szBuffer, TEXT("%s"), g_appData.m_szHostName); 
#else
		_stprintf(szBuffer, TEXT("Connected to %s"), g_appData.m_szHostName); 
#endif
		SendMessage(g_hMainDialog, WM_SETTEXT, 0, (LPARAM)szBuffer);

		// Call Get Reader Caps and update global data
		RFID_GetReaderCaps(g_appData.m_RfidReaderHandle, &g_appData.m_ReaderCaps);
		bRetVal = true;
	}
	else
		g_appData.m_ConnectionStatus = false;

	return bRetVal;
}

bool DisconnectFromReader()
{
	bool bRetVal = false;
	TCHAR szBuffer[MAX_PATH] = {0,};

	if((g_appData.m_RfidStatus = RFID_Disconnect(g_appData.m_RfidReaderHandle))== RFID_API_SUCCESS)
	{	
		g_appData.m_ConnectionStatus = false;
		g_appData.m_LoggedIn = false;
		SetEvent(stopTestingEventHandle);
		WaitForSingleObject(readerEventAwaitingThreadHandle, INFINITE);
		CloseHandle(readerEventAwaitingThreadHandle);
		readerEventAwaitingThreadHandle = NULL;
		bRetVal = true;
		// Update the Application Title
		LoadString(g_hInst, IDS_APP_TITLE, szBuffer, MAX_PATH);
		SendMessage(g_hMainDialog, WM_SETTEXT, 0, (LPARAM)szBuffer);
	}
	return bRetVal;
}

void PostNotification(RFID_EVENT_TYPE eventId, TCHAR* pEventData)
{
	TCHAR szBuffer[MAX_PATH] = {0,};

	switch (eventId)
	{
	case GPI_EVENT:
		_tcscpy(szBuffer, TEXT("GPI Event"));
		break;
	case TAG_DATA_EVENT:
		_tcscpy(szBuffer, TEXT("TAG Data Event"));
		break;
	case BUFFER_FULL_WARNING_EVENT:
		_tcscpy(szBuffer, TEXT("Buffer Full Warning Event"));
		break;
	case ANTENNA_EVENT:
		_tcscpy(szBuffer, TEXT("Antenna Event"));
		break;
	case INVENTORY_START_EVENT:
		_tcscpy(szBuffer, TEXT("Inventory Start Event"));
		break;
	case INVENTORY_STOP_EVENT:
		_tcscpy(szBuffer, TEXT("Inventory Stop Event"));
		break;
	case ACCESS_START_EVENT:
		_tcscpy(szBuffer, TEXT("Access Start Event"));
		break;
	case ACCESS_STOP_EVENT:
		_tcscpy(szBuffer, TEXT("Access Stop Event"));
		break;
	case DISCONNECTION_EVENT:
		_tcscpy(szBuffer, TEXT("Disconnection Event"));
		break;
	case BUFFER_FULL_EVENT:
		_tcscpy(szBuffer, TEXT("Buffer Full Event"));
		break;
	case READER_EXCEPTION_EVENT:
		_tcscpy(szBuffer, TEXT("Reader Exception Event"));
		break;
	case TEMPERATURE_ALARM_EVENT:
		_tcscpy(szBuffer, TEXT("Temperature Alarm Event"));
		break;
	default:
		return;

	}
	// Update Last Event in the Main Dialog
	SendDlgItemMessage(g_hMainDialog, IDC_LAST_EVENT, WM_SETTEXT, 0, (LPARAM)szBuffer);	

	// Update Last Event Data in the Main Dialog
	SendDlgItemMessage(g_hMainDialog, IDC_LAST_EVENT_DATA, WM_SETTEXT, 0, (LPARAM)pEventData);	

}

void Cleanup()
{
	// if RM Logged In
	if (g_appData.m_LoggedIn)
	{
		RFID_Logout(g_appData.m_RfidRMHandle);
		g_appData.m_LoggedIn = false;
	}

	// if Reader is connected
	if (g_appData.m_ConnectionStatus)
		DisconnectFromReader();
}

void UpdateGPI(UINT16 portNumber, BOOLEAN enable, BOOLEAN state)
{
	UINT32 dlgItem;
	dlgItem = IDC_PIC1;

	dlgItem = dlgItem + (portNumber - 1);

	if(state == GPI_PORT_STATE_HIGH)
		SendMessage(GetDlgItem(g_hMainDialog, dlgItem), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_appData.m_BitmapGreen); 
	else if(state == GPI_PORT_STATE_LOW)
		SendMessage(GetDlgItem(g_hMainDialog, dlgItem), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_appData.m_BitmapRed); 
	else if(state == GPI_PORT_STATE_UNKNOWN)
		SendMessage(GetDlgItem(g_hMainDialog, dlgItem), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_appData.m_BitmapUnknown); 

	ShowWindow(GetDlgItem(g_hMainDialog, dlgItem), SW_SHOW);
}