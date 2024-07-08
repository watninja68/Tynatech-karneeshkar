#pragma once

#define IDLE					0
#define RUNNING					1
#define MEM_BANKS_SUPPORTED		(4 + 1) // +1 Added for None 
#define NUMBER_OF_GPIO			6
#define MAX_EVENTS				13
#define MAX_NUM_ANTENNA			8 // As supported by Sample App


enum RFID_INTERFACE
{
	GENERIC_INTERFACE = 0,
	RM_INTERFACE,
};

/* Main Page List View Column ID */
#define COLUMN_EPCID  			0
#define COLUMN_STATE 			1
#define COLUMN_ANTENNA_ID 		2
#define COLUMN_SEEN_COUNT		3
#define COLUMN_RSSI				4
#define COLUMN_PHASE			5
#define COLUMN_PC_BITS			6
#define COLUMN_MEM_BANK_DATA	7
#define COLUMN_MEM_BANK			8
#define COLUMN_OFFSET			9

#define MAX_CERT_KEY_FILE_SIZE		10240


extern HINSTANCE			g_hInst;			// current instance
extern HWND					g_hWndMenuBar;		// menu bar handle

extern INT_PTR CALLBACK DialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK ConnectDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK CapabilitiesDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AntennaConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RFmodeConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK GPIOConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK SingulationConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RadioPowerConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK TagStorageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK TriggerInfoDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AntennaInfoDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RMLoginDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RMAntennaModeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK ReadPointDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RMSoftwareUpdateDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK RMRadioPowerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK PreFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK PostFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessReadDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessWriteDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessLockDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessKillDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AccessBlockEraseDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK LocateTagDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK TagStartTriggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK TagStopTriggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


extern void Createmenu(HWND hWnd);
extern void Createbutton(HWND hWnd);
extern void PostRFIDStatus(RFID_STATUS statusCode, RFID_INTERFACE interfaceType);
extern BOOL DoContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam);
extern bool FindItem(TAG_DATA *pTagData, BOOLEAN* pUniqueTag);
extern bool GetSelectedTagID(TCHAR *pTagID);
extern bool ConnectToReader(TCHAR* pszHostName, UINT16 portNumber);
extern bool DisconnectFromReader();

extern bool ReadTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 length, UINT32 offset, 
					UINT32 password, UINT8* pData, UINT32 *pDataLength);

extern bool WriteTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
					 UINT32 password, UINT8* pData, UINT32 dataLength);

extern bool LockTag(UINT8* pTagID, UINT32 tagIDLength, LOCK_DATA_FIELD dataField, LOCK_PRIVILEGE privilege,
					UINT32 password);
extern bool KillTag(UINT8* pTagID, UINT32 tagIDLength, UINT32 password);

extern bool BlockEraseTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
						  UINT32 password, UINT32 dataLength);

extern bool StartTagLocationing(UINT8* pTagID, UINT32 tagIDLength);
extern bool StopTagLocationing();

extern void ConvertBytePtrToHexString(UINT8*pTagID, UINT32 tagIDLength, TCHAR *pHexString);
extern void ConvertHexStringToBytePtr(TCHAR *pHexString, BYTE *pData, int *dataLength);

void CreateListView();
int GetItemLocation(TAG_DATA *pTagData,  BOOLEAN* pUniqueTag);
void UpdateTagCount();
void PostNotification(RFID_EVENT_TYPE eventId, TCHAR* pEventData);
void Cleanup();
void UpdateGPI(UINT16 portNumber, BOOLEAN enable, BOOLEAN state);

extern TCHAR* memory_banks[];