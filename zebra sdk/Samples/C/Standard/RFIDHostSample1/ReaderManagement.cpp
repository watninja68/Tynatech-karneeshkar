#include "stdafx.h"

#define READERS_SUPPORTED			3
#define ANTENNA_MODES_SUPPORTED		2
#define UPDATE_REGIONS				2
#define UPDATE_FIRMWARE				0
#define UPDATE_OEM					1

static LPWSTR supported_readers[] =
{
	L"XR",
	L"FX",
	L"MC"
};

static LPWSTR antennaModes[] =
{
	L"bistatic",
	L"monostatic",
};
static LPWSTR flash_regions[] =
{
	L"Firmware",
	L"OEM",
};
static	LPWSTR ReadpointState[] = 
{
	L"Disable",
	L"Enable"
};
INT_PTR CALLBACK RMLoginDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16				nIndex,length;
	TCHAR				*pinput;
	TCHAR				szinput[MAX_PATH];

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);
		InitCommonControls();
		for(nIndex = 0; nIndex < READERS_SUPPORTED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_READER_TYPE), CB_ADDSTRING, 0,(LPARAM) supported_readers[nIndex]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_READER_TYPE), CB_SETCURSEL, g_appData.m_ReaderType, 0);

		SetWindowText(GetDlgItem(hDlg, IDC_TB_HOSTNAME),g_appData.m_szHostName); 
		if(g_appData.m_LoggedIn)
		{  
			SetWindowText(GetDlgItem(hDlg, IDC_LOGIN), L"Logout");
			SetWindowText(GetDlgItem(hDlg, IDC_TB_USERNAME),g_appData.m_LoginInfo.userName); 
			SetWindowText(GetDlgItem(hDlg, IDC_TB_PASSWORD),g_appData.m_LoginInfo.password); 
			SendMessage(GetDlgItem(hDlg, IDC_CB_READER_TYPE), CB_SETCURSEL, g_appData.m_ReaderType, 0);
			SetWindowText(GetDlgItem(hDlg, IDC_TB_HOSTNAME),g_appData.m_LoginInfo.hostName); 
		}
		else
		{
			SetWindowText(GetDlgItem(hDlg, IDC_LOGIN), L"Login");

		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			g_appData.m_ReaderType = (READER_TYPE)SendMessage(GetDlgItem(hDlg, IDC_CB_READER_TYPE), CB_GETCURSEL, 0, 0);
			if(g_appData.m_ReaderType == MC)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_TB_USERNAME), false);
				EnableWindow(GetDlgItem(hDlg, IDC_LOGINNAMETEXT), false);
				EnableWindow(GetDlgItem(hDlg, IDC_TB_PASSWORD), false);
				EnableWindow(GetDlgItem(hDlg, IDC_PASSWORDTEXT), false);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_TB_USERNAME), true);
				EnableWindow(GetDlgItem(hDlg, IDC_LOGINNAMETEXT), true);
				EnableWindow(GetDlgItem(hDlg, IDC_TB_PASSWORD), true);
				EnableWindow(GetDlgItem(hDlg, IDC_PASSWORDTEXT), true);

			}
		}
		else if (LOWORD(wParam) == IDC_LOGIN )
		{
			if(!(g_appData.m_LoggedIn))
			{
				g_appData.m_LoginInfo.version = RFID_API3_5_1;
				g_appData.m_LoginInfo.forceLogin = true;
				length = GetWindowText(GetDlgItem(hDlg, IDC_TB_HOSTNAME),szinput, MAX_PATH);

				for(nIndex = 0,pinput = szinput; nIndex <length; nIndex++, pinput++)
					g_appData. m_LoginInfo.hostName[nIndex] = *pinput; 
				g_appData.m_LoginInfo.hostName[nIndex] = L'\0';
				length = GetWindowText(GetDlgItem(hDlg, IDC_TB_PASSWORD),szinput, MAX_PATH);

				for(nIndex = 0,pinput = szinput; nIndex <length; nIndex++, pinput++)
					g_appData. m_LoginInfo.password[nIndex] = *pinput; 
				g_appData.m_LoginInfo.password[nIndex] = L'\0';
			
				length = GetWindowText(GetDlgItem(hDlg, IDC_TB_USERNAME),szinput, MAX_PATH);

				for(nIndex = 0,pinput=szinput; nIndex <length; nIndex++, pinput++)
					g_appData.m_LoginInfo.userName[nIndex] = *pinput; 
				g_appData.m_LoginInfo.userName[nIndex] = L'\0';

				g_appData.m_ReaderType = (READER_TYPE)SendMessage(GetDlgItem(hDlg, IDC_CB_READER_TYPE), CB_GETCURSEL, 0, 0);

				if((g_appData.m_RfidStatus = RFID_Login(&(g_appData.m_RfidRMHandle),&(g_appData.m_LoginInfo),g_appData.m_ReaderType,false,NULL))== RFID_API_SUCCESS)
				{
					g_appData.m_LoggedIn = true;
					EndDialog(hDlg, message);
				}

				//Post Status to the status bar
				PostRFIDStatus(g_appData.m_RfidStatus, RM_INTERFACE);
			}
			else
			{   
				if((g_appData.m_RfidStatus = RFID_Logout(g_appData.m_RfidRMHandle))== RFID_API_SUCCESS)
				{
					g_appData.m_LoggedIn = false;
					EndDialog(hDlg, message);

				}
			}
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}

INT_PTR CALLBACK ReadPointDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR					szConversion[MAX_PATH];
	UINT16					antennaID, nIndex;
	BOOLEAN					antannaStatus;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);
		antennaID = 1;

		for(nIndex = 1; nIndex <= g_appData.m_ReaderCaps.numAntennas; nIndex++)
		{
			wsprintf(szConversion, _T("%d"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_ID), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		for(nIndex = 0; nIndex <= 1; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_STATE), CB_ADDSTRING, 0,(LPARAM) ReadpointState[nIndex]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_ID), CB_SETCURSEL, 0, 0);

		g_appData.m_RfidStatus = RFID_GetReadPointStatus(g_appData.m_RfidRMHandle, antennaID, &antannaStatus);
		if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_STATE), CB_SETCURSEL, antannaStatus, 0);
		else
		{	
			//MessageBox(RFID_GetErrorDescription(g_appData.m_RfidStatus),L"Error",MB_OK);
		}
		return (INT_PTR)TRUE;


	case WM_COMMAND:

		if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_CB_ANTENNA_ID)
		{			
			antennaID = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_ID), CB_GETCURSEL, 0, 0);
			g_appData.m_RfidStatus = RFID_GetReadPointStatus(g_appData.m_RfidRMHandle, antennaID+1, &antannaStatus);
			if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
				SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_STATE), CB_SETCURSEL, antannaStatus, 0);

		}

		if (LOWORD(wParam) == IDC_APPLY )
		{			
			antennaID = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_ID), CB_GETCURSEL, 0, 0);
			antannaStatus = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_STATE), CB_GETCURSEL, 0, 0);

			g_appData.m_RfidStatus = RFID_EnableReadPoint(g_appData.m_RfidRMHandle, antennaID+1, antannaStatus);
			EndDialog(hDlg, LOWORD(wParam));

		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}

		return (INT_PTR)TRUE;


	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}


INT_PTR CALLBACK RMAntennaModeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ANTENNA_MODE amAntennaMode;
	UINT16 nIndex;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 0; nIndex < ANTENNA_MODES_SUPPORTED; nIndex++)
		{
			//	wsprintf(szConversion, (LPCTSTR)TEXT("%d"), memory_banks[nIndex]);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_MODE), CB_ADDSTRING, 0,(LPARAM) antennaModes[nIndex]);
		}

		g_appData.m_RfidStatus = RFID_GetAntennaMode(g_appData.m_RfidRMHandle, &amAntennaMode);
		if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_MODE), CB_SETCURSEL, amAntennaMode, 0);
		else
		{	
			//MessageBox(RFID_GetErrorDescription(g_appData.m_RfidStatus),L"Error",MB_OK);
		}
		return (INT_PTR)TRUE;


	case WM_COMMAND:

		/*if (HIWORD(wParam) == CBN_SELCHANGE )
		{			
		amAntennaMode = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0);
		amAntennaMode = (ANTENNA_MODE)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_MODE), CB_GETCURSEL, 0, 0);

		g_appData.m_RfidStatus = RFID_SetAntennaMode(g_appData.m_RfidRMHandle, amAntennaMode);
		}*/

		if (LOWORD(wParam) == IDC_APPLY )
		{			
			//amAntennaMode = (ANTENNA_MODE)m_mode.GetCurSel();  
			amAntennaMode = (ANTENNA_MODE)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNA_MODE), CB_GETCURSEL, 0, 0);

			g_appData.m_RfidStatus = RFID_SetAntennaMode(g_appData.m_RfidRMHandle, amAntennaMode);
			EndDialog(hDlg, LOWORD(wParam));

		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}

		return (INT_PTR)TRUE;


	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}
#ifdef WINCE
BOOL DibFileOpenDlg (HWND hwnd, int FileType, TCHAR *szFileName)
{
	OPENFILENAME ofn ;
	WCHAR sFileName[MAX_PATH] = {0};
	ZeroMemory( &ofn , sizeof( ofn));
	static TCHAR szFilter1[] = TEXT ("Firmware Files (*.a79)\0*.a79\0")  \
                               TEXT ("All Files (*.*)\0*.*\0\0") ;
	static TCHAR szFilter2[] = TEXT ("dmp Files (*.dmp)\0*.dmp\0") \
                               TEXT ("All Files (*.*)\0*.*\0\0") ;
	if(UPDATE_FIRMWARE==FileType)
		ofn.lpstrFilter       = szFilter1 ;
	else if(UPDATE_OEM==FileType)
		ofn.lpstrFilter       = szFilter2;
	ofn.lStructSize = sizeof (OPENFILENAME);
	ofn.hwndOwner         = hwnd ;//
	ofn.lpstrFile         = sFileName ;//
	ofn.lpstrFileTitle    =0 ;//
	ofn.Flags             = OFN_EXPLORER; //
	ofn.nMaxFile = MAX_PATH;
	ofn.hwndOwner = hwnd ; 
	ofn.nFilterIndex =1;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir=NULL;

	BOOL bRet = GetOpenFileName( &ofn );
	wcscpy ( szFileName, ofn.lpstrFile);
	return bRet;
}
#endif
INT_PTR CALLBACK RMSoftwareUpdateDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR						szConversion[MAX_PATH] = {0};
	UPDATE_STATUS				UpdateStatus;
	FTPSERVER_INFO				ftpServerInfo;
	UINT16						nIndex, UpdateType;
	UPDATE_STATUS updateStatus;
	TCHAR *pszErrorBuffer = NULL;
	RFID_STATUS rfidStatus = RFID_API_SUCCESS;

	BOOL bRet;
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwICC = ICC_LISTVIEW_CLASSES|ICC_DATE_CLASSES|ICC_PROGRESS_CLASS;
	InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	bRet = InitCommonControlsEx(&InitCtrls);

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		if(g_appData.m_ReaderType == MC) 
		{
			ShowWindow(GetDlgItem(hDlg, IDC_TB_USERNAME), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_USERTEXT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_TB_PASSWORD), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_PASSWORDTEXT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), SW_SHOW); 
			ShowWindow(GetDlgItem(hDlg, IDC_UPDATETEXT), SW_SHOW);

			for(nIndex = 0; nIndex < UPDATE_REGIONS; nIndex++)
			{
				SendMessage(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), CB_ADDSTRING, 0,(LPARAM) flash_regions[nIndex]);
			}
			SendMessage(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), CB_SETCURSEL, 0, 0);
		}
		else
		{  
			ShowWindow(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_UPDATETEXT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_TB_USERNAME), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_USERTEXT), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_TB_PASSWORD), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_PASSWORDTEXT), SW_SHOW);
		}

		SendMessage(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100)); 
		SendMessage(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), PBM_SETSTEP, (WPARAM) 1, 0); 

		SetWindowText(GetDlgItem(hDlg, IDC_TB_UPDATE_STATUS), L"");
		return (INT_PTR)TRUE;

	case WM_TIMER:
		{
			g_appData.m_RfidStatus = RFID_GetUpdateStatus(g_appData.m_RfidRMHandle, &updateStatus);

			if(RFID_API_SUCCESS == g_appData.m_RfidStatus)
			{
				TCHAR updateMsg[MAX_PATH];
				_stprintf(updateMsg, TEXT("%s: %d %"), updateStatus.updateInfo, updateStatus.percentage);
				SetDlgItemText(hDlg, IDC_TB_UPDATE_STATUS, updateMsg);
				SendMessage(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), PBM_SETPOS, updateStatus.percentage, 0);
			}
			else if(RFID_RM_NO_UPDATION_IN_PROGRESS == g_appData.m_RfidStatus)
			{
				KillTimer(hDlg, ID_UPDATE_TIMER);
				SendMessage(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), PBM_SETPOS, 100, 0);
				SetWindowText(GetDlgItem(hDlg, IDC_TB_UPDATE_STATUS), L"Update Complete");
				rfidStatus = RFID_Logout(g_appData.m_RfidRMHandle);
				g_appData.m_LoggedIn = false;
				break;
			}
			else
			{
				pszErrorBuffer = RFID_GetErrorDescription(rfidStatus);
				SetDlgItemText(hDlg, IDC_TB_UPDATE_STATUS, pszErrorBuffer);
			}
			if(RFID_API_SUCCESS != g_appData.m_RfidStatus)
			{
				ERROR_INFO errorInfo;
				RFID_GetLastErrorInfo(g_appData.m_RfidRMHandle, &errorInfo);
				PostRFIDStatus(g_appData.m_RfidStatus, RM_INTERFACE);
			}
		}
		break;
	case WM_COMMAND:

		if(LOWORD(wParam) == IDC_STARTUPDATE)
		{
			if(g_appData.m_ReaderType !=MC)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_USERNAME), szConversion, MAX_PATH);
				wsprintf(ftpServerInfo.userName,szConversion);

				GetWindowText(GetDlgItem(hDlg, IDC_TB_PASSWORD),szConversion, MAX_PATH);
				wsprintf(ftpServerInfo.password,szConversion);

				GetWindowText(GetDlgItem(hDlg, IDC_TB_LOCATION), szConversion, MAX_PATH);
				wsprintf(ftpServerInfo.hostName,szConversion);

				g_appData.m_RfidStatus = RFID_UpdateSoftware(g_appData.m_RfidRMHandle, &ftpServerInfo);

			}
			else
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_LOCATION), szConversion, MAX_PATH);
				UpdateType = SendMessage(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), CB_GETCURSEL, 0, 0);

				if(UpdateType == UPDATE_OEM) 	 
					g_appData.m_RfidStatus = RFID_UpdateRadioConfig(g_appData.m_RfidRMHandle, szConversion);
				else
					g_appData.m_RfidStatus = RFID_UpdateRadioFirmware(g_appData.m_RfidRMHandle, szConversion);
			}
			if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
			{
				SetWindowText(GetDlgItem(hDlg, IDC_TB_UPDATE_STATUS), L"Update Started");
				EnableWindow(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), true);
				EnableWindow(GetDlgItem(hDlg, IDC_TB_UPDATE_STATUS),true);
				ShowWindow(GetDlgItem(hDlg, IDC_UPDATEPROGRESS), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_TB_UPDATE_STATUS),SW_SHOW);
				SetTimer(hDlg, ID_UPDATE_TIMER, 1000, NULL);
			}
			else
			{
				PostRFIDStatus(g_appData.m_RfidStatus, RM_INTERFACE);
			}
		}
#ifdef WINCE
		else if(LOWORD(wParam) == IDC_BUTTON_BROWSE)
		{
			TCHAR szFileName[MAX_PATH];
			UpdateType = SendMessage(GetDlgItem(hDlg, IDC_CB_UPDATE_TYPE), CB_GETCURSEL, 0, 0);
			if(DibFileOpenDlg(hDlg, UpdateType, szFileName))
				SetDlgItemText(hDlg, IDC_TB_LOCATION, szFileName);
		}
#endif
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		KillTimer(hDlg, ID_UPDATE_TIMER);
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}