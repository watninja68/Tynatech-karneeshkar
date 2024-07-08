#include "stdafx.h"
#include "BasicRFID.h"
#include "BasicRFIDHost1.h"
#include <stdio.h>
#include <commctrl.h>

#include <string>
#include <set>
#include <queue>
std::set<std::string> uniqueTags;
struct RFIDTag {
	std::string id; // unique tag ID
	// other tag data (e.g., timestamp, etc.)
};
// Global Variables:
HINSTANCE g_hInst;								// current instance

// About Dialog Handler
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
// Inventory Dialog Handler
INT_PTR CALLBACK	Inventory(HWND, UINT, WPARAM, LPARAM);
// Reader Connection input Dialog Handler
INT_PTR CALLBACK	ConnectionInput(HWND, UINT, WPARAM, LPARAM);
// List Box Message Handler (Tag Inventory)
INT_PTR CALLBACK	LB_MsgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Read Tag Dialog Handler
INT_PTR CALLBACK	ReadTag(HWND, UINT, WPARAM, LPARAM);

// Write Tag Dialog Handler
INT_PTR CALLBACK	WriteTag(HWND, UINT, WPARAM, LPARAM);

// Lock Tag Dialog Handler
INT_PTR CALLBACK	LockTag(HWND, UINT, WPARAM, LPARAM);

// Kill Tag Dialog Handler
INT_PTR CALLBACK    KillTag(HWND, UINT, WPARAM, LPARAM);



// Prints the status message in the status bar
void PostStatus(TCHAR *status);

// Convert Hex string to byte array
void ConvertHexStringToBytePtr(TCHAR *pHexString, BYTE *pData, int *dataLength);

HWND g_hDlg;
WNDPROC g_MainWndProc;

#define IDLE		0
#define RUNNING		1

int g_hCurrentState = IDLE;
bool g_bIsConnected = FALSE;

// Handle to the list box font
HFONT g_hFont;

// Holds the reader IP 
TCHAR szReaderIP[MAX_PATH] = {0,};

// Selected TAG ID details
TCHAR szTagID[MAX_PATH] = {0,};

// API Error Info
ERROR_INFO errorInfo;

// Entry point to the SampleApp
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	g_hInst = hInstance;
	// Inventory Dialog is invoked here...
	DialogBox(NULL, MAKEINTRESOURCE(IDD_INVENTORY_DIALOG), NULL, Inventory);

}

// Message handler for Inventory Dialog box.
INT_PTR CALLBACK Inventory(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	DWORD dwThreadId;
	HMENU hPopupMenu;
	WORD wmId;
	WORD wmEvent;
	
	TCHAR szBuffer[MAX_PATH] ={0,};
	
	static HANDLE g_hThread;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Assign the Dialog handle 
			g_hDlg = hDlg;

			// Initialize IP COM Control
			INITCOMMONCONTROLSEX stInitCtrls;
			stInitCtrls.dwICC = ICC_INTERNET_CLASSES;
			stInitCtrls.dwSize = sizeof(stInitCtrls);
			InitCommonControlsEx(&stInitCtrls);

			// Creates Courier New Font
			g_hFont=CreateFont (14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, 
								OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
								DEFAULT_PITCH | FF_SWISS, L"Courier New");
			// Set Inventory List box font to Courier New
			SendDlgItemMessage(hDlg, IDC_INVENTORY_LIST, WM_SETFONT, (WPARAM)g_hFont, TRUE);
			
			// Sub-classing - send the keyboard messages to List box handler
#ifdef _WIN64
			g_MainWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_INVENTORY_LIST), GWLP_WNDPROC, (LONG_PTR)LB_MsgHandler);
#else
			g_MainWndProc = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, IDC_INVENTORY_LIST), GWL_WNDPROC, (LONG)LB_MsgHandler);
#endif
			PostStatus(TEXT("Ready.."));
		}
		return (INT_PTR)TRUE;
	case WM_CLOSE:
		DeleteObject(g_hFont);
		DisconnectReader();
		EndDialog(hDlg, LOWORD(wParam));
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			// Connect to the reader
		case IDM_CONNECT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_IP_DIALOG), hDlg, ConnectionInput);
			break;
			// Disconnect from the reader
		case IDM_DISCONNECT:
			if (!g_bIsConnected)
			{
				PostStatus(TEXT("No connection established.."));
				return 0;
			}
			else if (DisconnectReader())
			{
				g_bIsConnected = false;
				SendMessage(GetDlgItem(hDlg, IDC_BUTTON1), WM_SETTEXT, 0, (LPARAM)TEXT("Start Read"));
				_stprintf(szBuffer, TEXT("Disconnect from %s success"), szReaderIP);
				PostStatus (szBuffer);

			}
			else
			{
				_stprintf(szBuffer, TEXT("Disconnect from %s failed"), szReaderIP);
				PostStatus (szBuffer);
			}
			g_hCurrentState = IDLE;
			EnableMenuItem(GetMenu(g_hDlg), IDM_CONNECT, MF_ENABLED); 
			break;
			// About Box invoked
		case IDM_ABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
			// Exit menu button clicked
		case IDM_EXIT:
			DisconnectReader();
			EndDialog(hDlg, LOWORD(wParam));
			break;
			// Start/Stop Read Button invoked
		case IDC_BUTTON1:
			if (g_hCurrentState == RUNNING)
			{
				if (StopContinuousInventory())
				{
					SendMessage(GetDlgItem(hDlg, IDC_BUTTON1), WM_SETTEXT, 0, (LPARAM)TEXT("Start Read"));
					g_hCurrentState = IDLE;
				}
			}
			else
			{
				if (StartContinuousInventory())
				{
					SendMessage(GetDlgItem(hDlg, IDC_BUTTON1), WM_SETTEXT, 0, (LPARAM)TEXT("Stop Read"));
					g_hCurrentState = RUNNING;
				}
			}
		default:
			return DefWindowProc(hDlg, message, wParam, lParam);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	VERSION_INFO VersionInfo;
	switch (message)
	{
	case WM_INITDIALOG:
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
	}
	return (INT_PTR)FALSE;
}

// List Box Message Handler for handling right mouse click button
INT_PTR CALLBACK LB_MsgHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hmenu;            // menu template          
	HMENU hmenuTrackPopup;  // shortcut menu   
	POINT pt;
	RECT rc;
	WORD wmId;
	WORD wmEvent;

	int  selectedIndex;
	bool showAccessOperationDlg = true;

	switch (message)
	{
	case WM_RBUTTONDOWN:
		//  Load the menu template containing the shortcut menu from the 
		//  application's resources. 
		hmenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU1)); 
		if (hmenu == NULL) 
			return 0; 

		// Get the first shortcut menu in the menu template. This is the 
		// menu that TrackPopupMenu displays. 
		hmenuTrackPopup = GetSubMenu(hmenu, 0); 

		// TrackPopup uses screen coordinates, so convert the 
		// coordinates of the mouse click to screen coordinates. 
		GetCursorPos (&pt);

		// Draw and track the shortcut menu.  
		SetForegroundWindow (hDlg);

		TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
		pt.x, pt.y, 0, hDlg, NULL); 

		SetForegroundWindow (hDlg);
		// Destroy the menu. 
		DestroyMenu(hmenuTrackPopup);
		DestroyMenu(hmenu); 

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Initialize Tag ID Details
		memset (szTagID, NULL, MAX_PATH);

		// get currently selected tag index from the list box
		selectedIndex = SendDlgItemMessage(g_hDlg, IDC_INVENTORY_LIST, LB_GETCURSEL, 0, 0);

		if (selectedIndex == LB_ERR)
		{
			showAccessOperationDlg = false;
		}
		else
		{
			SendDlgItemMessage(g_hDlg, IDC_INVENTORY_LIST, LB_GETTEXT, (WPARAM) selectedIndex, (LPARAM)(LPCTSTR)szTagID);
		}

		// Parse the menu selections:
		switch (wmId)
		{
			// Clear Tags Sub-Menu clicked
		case ID_CLEAR_TAGS:
				// remove all items from a list box
				SendDlgItemMessage(g_hDlg, IDC_INVENTORY_LIST, LB_RESETCONTENT, 0, 0);
			break;
			// Read operation on Selected tag
		case ID_TAG_READ:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_READ_TAG_DIALOG), NULL, ReadTag);
			break;
			// Write operation on selected tag
		case ID_TAG_WRITE:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_WRITE_TAG_DIALOG), NULL, WriteTag);
			break;
			// Lock operation on selected tag
		case ID_TAG_LOCK:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_LOCK_TAG_DIALOG), NULL, LockTag);
			break;
			// Kill operation on selected tag
		case ID_TAG_KILL:
			//DialogBox(g_hInst, MAKEINTRESOURCE(IDD_LOCK_TAG_DIALOG), NULL, LockTag);
			 DialogBox(g_hInst, MAKEINTRESOURCE(IDD_KILL_TAG_DIALOG), NULL, KillTag);
			break;
			// None

		default:
			break;
		}
	default:
		break;
	}
	return CallWindowProc(g_MainWndProc, hDlg, message, wParam, lParam);
}

// This function is used to report the events
void LOG_MSG(TCHAR *msg)
{
	TCHAR szBuffer[MAX_PATH] ={0,};
	_stprintf(szBuffer, TEXT("%s"), msg);
	SendDlgItemMessage(g_hDlg, IDC_EVENT_NOTIFY, WM_SETTEXT, 0, (LPARAM)szBuffer);
}

// Print the status message on the status bar
void PostStatus(TCHAR *status)
{
	SendDlgItemMessage(g_hDlg, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)status);	
}


// Format Tag and print on screen
void printTagData(TAG_DATA* pTagData)
{
	char  tagBuffer[1024] = { 0, };
	char* pTagReportData = tagBuffer;
	int   index = 0;
	TCHAR resultBuffer[MAX_PATH];

	for (index = 0; index < pTagData->tagIDLength; index++)
	{
		if (0 < index && index % 2 == 0)
		{
			*pTagReportData++ = '-';
		}
		sprintf(pTagReportData, "%02X", pTagData->pTagID[index]);
		while (*pTagReportData) pTagReportData++;
	}
	_stprintf(resultBuffer, TEXT("%S"), tagBuffer);

	// Check if the tag is unique
	std::wstring wideStr(resultBuffer); // Construct a std::wstring from TCHAR*
	std::string tag(wideStr.begin(), wideStr.end());	if (uniqueTags.find(tag) == uniqueTags.end()) {
		uniqueTags.insert(tag); // Add the tag to the set of unique tags

		FILE* file = _tfopen(TEXT("tags.txt"), TEXT("a+"));
		if (file != NULL)
		{
			_fputts(resultBuffer, file);
			_fputts(TEXT("\n"), file);
			fclose(file);

			// Send the tag over UART
			HANDLE hSerial = CreateFile(TEXT("COM4"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hSerial != INVALID_HANDLE_VALUE)
			{
				DCB dcbSerialParams = { 0 };
				COMMTIMEOUTS timeouts = { 0 };

				dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
				GetCommState(hSerial, &dcbSerialParams);
				dcbSerialParams.BaudRate = CBR_115200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;
				SetCommState(hSerial, &dcbSerialParams);

				timeouts.ReadIntervalTimeout = 50;
				timeouts.ReadTotalTimeoutConstant = 50;
				timeouts.ReadTotalTimeoutMultiplier = 10;
				timeouts.WriteTotalTimeoutConstant = 50;
				timeouts.WriteTotalTimeoutMultiplier = 10;
				SetCommTimeouts(hSerial, &timeouts);

				DWORD bytesWritten;
				WriteFile(hSerial, tagBuffer, strlen(tagBuffer), &bytesWritten, NULL);
				CloseHandle(hSerial);
			}
			else
			{
				PostStatus(TEXT("Failed to open COM port."));
			}
		}
		else
		{
			PostStatus(TEXT("Failed to open tags.txt file for writing."));
		}
	}

	SendDlgItemMessage(g_hDlg, IDC_INVENTORY_LIST, LB_ADDSTRING, 0, (LPARAM)resultBuffer);
	SendDlgItemMessage(g_hDlg, IDC_INVENTORY_LIST, WM_VSCROLL, (WPARAM)SB_BOTTOM, 0);
}// Reader Connection Input capture dialog
INT_PTR CALLBACK	ConnectionInput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szIPAddress[MAX_PATH] = {0,};
	TCHAR szBuffer[MAX_PATH] = {0,};
	DWORD dwIPAddr = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_PORT), WM_SETTEXT, 0, (LPARAM)TEXT("5084"));
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == ID_CONNECT)
		{
			// Gets the IP address from the IP Control
			SendMessage(GetDlgItem(hDlg, IDC_IPADDRESS), (UINT)IPM_GETADDRESS, 0, (LPARAM)&dwIPAddr);
		
			_stprintf(szIPAddress, TEXT("%d.%d.%d.%d"), 
								FIRST_IPADDRESS(dwIPAddr),
								SECOND_IPADDRESS(dwIPAddr),
								THIRD_IPADDRESS(dwIPAddr),
								FOURTH_IPADDRESS(dwIPAddr) );

			// copy the IP address to the global variable
			_tcscpy(szReaderIP, szIPAddress);
											
			if (ConnectToReader(szReaderIP))
			{
				g_bIsConnected = true;
				_stprintf(szBuffer, TEXT("Connnection to %s success"), szReaderIP);
				PostStatus (szBuffer);
				EndDialog(hDlg, LOWORD(wParam));
				EnableMenuItem(GetMenu(g_hDlg), IDM_CONNECT, MF_GRAYED); 
			}
			else
			{
				_stprintf(szBuffer, TEXT("Connection to %s failed"), szReaderIP);
				PostStatus (szBuffer);
			}
			
		}
		else if (LOWORD(wParam) == ID_CANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		
	}
	return (INT_PTR)FALSE;
}

// Converts Hex string to Byte Ptr
void ConvertHexStringToBytePtr(TCHAR *pHexString, BYTE *pData, int *dataLength)
{
	TCHAR *pHexStringData = pHexString;
	int length = 0;
	while (*pHexStringData)
	{
		_stscanf(pHexStringData, TEXT("%02x"), pData);
		pHexStringData += 2;

		// in case of Tag ID, ignore this character
		if (*pHexStringData == '-')
			pHexStringData++;

		pData++; 
		length++;
	}
	*dataLength = length;
}

// Read Tag Dialog Handler
INT_PTR CALLBACK	ReadTag(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};
	DWORD dwIPAddr = 0;

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;

	TCHAR szPassword[MAX_PATH] = {0,};
	TCHAR szLength[MAX_PATH] = {0,};
	TCHAR szOffSet[MAX_PATH] = {0,};
	UINT32 passWord = 0;
	int   memoryBank = 1;
	TAG_DATA *lpTagData = NULL;
	
	UINT8    memoryBankData[MAX_PATH] = {0,};
	UINT32   memoryBankDataLength = 0;

	TCHAR szDataRead[MAX_PATH] = {0,};

	switch (message)
	{
	case WM_INITDIALOG:
		PostStatus(TEXT("Read Tag in Progress"));
		// Add Memory Bank to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("RESERVED"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("EPC"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("TID"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("USER"));

		// Set the index (1) to select EPC as default
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_SETCURSEL, 3, 0);

		// initialize Password, offset and length to 0
		SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_OFFSET), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_LENGTH), WM_SETTEXT, 0, (LPARAM)TEXT("0"));

		// Set Tag ID 
		SendMessage(GetDlgItem(hDlg, IDC_TAG_ID), WM_SETTEXT, 0, (LPARAM)szTagID);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_READ)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TAG_ID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);
			if (_tcslen(szBuffer) > 0)
			{	
				// convert Tag ID Hex string to byte array
				ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);
			
				// Get Password
				SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
				_stscanf(szPassword, TEXT("%8x"), &passWord);

				// Get Selected memory bank index
				memoryBank = SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_GETCURSEL, 0, 0);
			
				// Get Length 
				SendMessage(GetDlgItem(hDlg, IDC_LENGTH), WM_GETTEXT, MAX_PATH, (LPARAM)szLength);

				// Get OffSet
				SendMessage(GetDlgItem(hDlg, IDC_OFFSET), WM_GETTEXT, MAX_PATH, (LPARAM)szOffSet);

				// ReadTag
				if (ReadTag(tagID, tagIDLength, (MEMORY_BANK)memoryBank, _ttoi(szLength), _ttoi(szOffSet), passWord, memoryBankData, &memoryBankDataLength))
				{
					int memoryBankDataIndex = 0;
					int index = 0;
					while (true && memoryBankDataLength > 0)
					{	
						_stprintf(&szDataRead[index], TEXT("%02X"), memoryBankData[memoryBankDataIndex++]);
						index+=2;
						
						if ((memoryBankDataIndex % 2) == 0)
							_stprintf(&szDataRead[index++], TEXT(" "));

						if (memoryBankDataIndex == memoryBankDataLength)
							break;
					}

					SendMessage(GetDlgItem(hDlg, IDC_DATA_READ), WM_SETTEXT, 0, (LPARAM)szDataRead);
					PostStatus(TEXT("Read Tag Success..."));
				}
				else
				{
					GetLastErrorInfo(&errorInfo);
					if (_tcslen(errorInfo.vendorMessage) > 0)
						PostStatus(errorInfo.vendorMessage);
				}
				
			}
		}
		break;
	case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			PostStatus(TEXT("Ready.."));
			return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

// Write Tag Dialog Handler
INT_PTR CALLBACK	WriteTag(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};
	DWORD dwIPAddr = 0;

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;

	TCHAR szPassword[MAX_PATH] = {0,};
	TCHAR szLength[MAX_PATH] = {0,};
	TCHAR szOffSet[MAX_PATH] = {0,};
	UINT32 passWord = 0;
	int   memoryBank = 1;
	TAG_DATA *lpTagData = NULL;
	
	UINT8    memoryBankData[MAX_PATH] = {0,};
	UINT32   memoryBankDataLength = 0;

	TCHAR szDataToWrite[MAX_PATH] = {0,};

	switch (message)
	{
	case WM_INITDIALOG:
		PostStatus(TEXT("Write Tag in Progress"));
		// Add Memory Bank to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("RESERVED"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("EPC"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("TID"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_ADDSTRING, 0, (LPARAM)TEXT("USER"));

		// Set the index (1) to select EPC as default
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_SETCURSEL, 3, 0);

		// initialize Password, offset and length to 0
		SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_OFFSET), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_LENGTH), WM_SETTEXT, 0, (LPARAM)TEXT("0"));

		// Set Tag ID 
		SendMessage(GetDlgItem(hDlg, IDC_TAG_ID), WM_SETTEXT, 0, (LPARAM)szTagID);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_WRITE)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TAG_ID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);
			if (_tcslen(szBuffer) > 0)
			{	
				// convert Tag ID Hex string to byte array
				ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

				// Get Password
				SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
				_stscanf(szPassword, TEXT("%8x"), &passWord);

				// Get Selected memory bank index
				memoryBank = SendMessage(GetDlgItem(hDlg, IDC_COMBO_MEM_BANK), CB_GETCURSEL, 0, 0);

				// Get Length 
				SendMessage(GetDlgItem(hDlg, IDC_LENGTH), WM_GETTEXT, MAX_PATH, (LPARAM)szLength);

				// Get OffSet
				SendMessage(GetDlgItem(hDlg, IDC_OFFSET), WM_GETTEXT, MAX_PATH, (LPARAM)szOffSet);

				// Get Data to Write
				SendMessage(GetDlgItem(hDlg, IDC_DATA_WRITE), WM_GETTEXT, MAX_PATH, (LPARAM)szDataToWrite);
				if (_tcslen(szDataToWrite) > 0)
				{
					ConvertHexStringToBytePtr(szDataToWrite, memoryBankData, (int*)&memoryBankDataLength);
					// WriteTag
					if (WriteTag(tagID, tagIDLength, (MEMORY_BANK)memoryBank, _ttoi(szOffSet), passWord, memoryBankData, memoryBankDataLength))
					{
						PostStatus(TEXT("Write Tag Success..."));
					}
					else
					{
						GetLastErrorInfo(&errorInfo);
						if (_tcslen(errorInfo.vendorMessage) > 0)
							PostStatus(errorInfo.vendorMessage);
					}
				}
			}
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		PostStatus(TEXT("Ready.."));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK KillTag(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	  UNREFERENCED_PARAMETER(lParam);
	  TCHAR szBuffer[MAX_PATH] = {0,};

	  BYTE  tagID[MAX_PATH/2] = {0,};
	  int   tagIDLength = 0;

	  TCHAR szPassword[MAX_PATH] = {0,};
	  UINT32 passWord = 0;

	  switch (message)
	  {
	    case WM_INITDIALOG:
		{
		  PostStatus(TEXT("Kill Tag in Progress"));
		  // initialize password
		  SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		  // Set Tag ID 
		  SendMessage(GetDlgItem(hDlg, IDC_TAG_ID), WM_SETTEXT, 0, (LPARAM)szTagID);
		  return (INT_PTR)TRUE;
		}
	    
	    case WM_COMMAND:
		if (LOWORD(wParam) == IDC_KILL)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TAG_ID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);
			if (_tcslen(szBuffer) > 0)
			{			  
				// convert Tag ID Hex string to byte array
				ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);
			
				// Get Password
				SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
				_stscanf(szPassword, TEXT("%8x"), &passWord);

				if (KillTag(tagID, tagIDLength,passWord))
				{
					PostStatus(TEXT("Kill Tag Success..."));
				}
				else
				{
					GetLastErrorInfo(&errorInfo);
					if (_tcslen(errorInfo.vendorMessage) > 0)
						PostStatus(errorInfo.vendorMessage);
				}
			}
		}
		break;
		case WM_CLOSE:
		{
			EndDialog(hDlg, LOWORD(wParam));
			PostStatus(TEXT("Ready.."));
			return (INT_PTR)TRUE;
		}
		break;
	  }
	  return (INT_PTR)FALSE;

}



// Lock Tag Dialog Handler
INT_PTR CALLBACK	LockTag(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};
	DWORD dwIPAddr = 0;

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;

	TCHAR szPassword[MAX_PATH] = {0,};
	UINT32 passWord = 0;

	int privilege = 0;
	int dataField = 0;


	switch (message)
	{
	case WM_INITDIALOG:
		PostStatus(TEXT("Lock Tag in Progress"));

		// Add Lock Data field to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_ADDSTRING, 0, (LPARAM)TEXT("Kill Password"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_ADDSTRING, 0, (LPARAM)TEXT("Access Password"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_ADDSTRING, 0, (LPARAM)TEXT("EPC Memory"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_ADDSTRING, 0, (LPARAM)TEXT("TID Memory"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_ADDSTRING, 0, (LPARAM)TEXT("User Memory"));

		// Add Privilege Data fields to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Read-Write"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Permanent Lock"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Permanent unlock"));
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Unlock"));

		// Set the index to 0
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_SETCURSEL, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_SETCURSEL, 0, 0);

		// initialize password
		SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		
		// Set Tag ID 
		SendMessage(GetDlgItem(hDlg, IDC_TAG_ID), WM_SETTEXT, 0, (LPARAM)szTagID);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LOCK)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TAG_ID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);
			if (_tcslen(szBuffer) > 0)
			{	
				// convert Tag ID Hex string to byte array
				ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);
			
				// Get Password
				SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
				_stscanf(szPassword, TEXT("%8x"), &passWord);

				// Get data field
				dataField = SendMessage(GetDlgItem(hDlg, IDC_COMBO_DATA_FIELD), CB_GETCURSEL, 0, 0);

				// Get Lock privilege
				privilege = SendMessage(GetDlgItem(hDlg, IDC_COMBO_PRIVILEGE), CB_GETCURSEL, 0, 0);
				privilege +=1 ;
				

				// Lock Tag
				if (LockTag(tagID, tagIDLength, (LOCK_DATA_FIELD)dataField, (LOCK_PRIVILEGE)privilege, passWord))
				{
					PostStatus(TEXT("Lock Tag Success..."));
				}
				else
				{
					GetLastErrorInfo(&errorInfo);
					if (_tcslen(errorInfo.vendorMessage) > 0)
						PostStatus(errorInfo.vendorMessage);
				}		
			}
		}
		break;
	case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			PostStatus(TEXT("Ready.."));
			return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}