
#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>

#include "rfidapi.h"


bool ConnectToReader(TCHAR* pHostName);
bool StartContinuousInventory();
bool StopContinuousInventory();
bool DisconnectReader();
bool ReadTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 length, UINT32 offset, 
				   UINT32 password, UINT8* pData, UINT32 *pDataLength);
bool WriteTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
	     		   UINT32 password, UINT8* pData, UINT32 dataLength);

bool LockTag(UINT8* pTagID, UINT32 tagIDLength, LOCK_DATA_FIELD dataField, LOCK_PRIVILEGE privilege,
	     		   UINT32 password);

bool KillTag(UINT8* pTagID, UINT32 tagIDLength, UINT32 password);

bool GetLastErrorInfo(LPERROR_INFO lpErrorInfo);
TCHAR* getDisconnectionEventString(DISCONNECTION_EVENT_TYPE disconnectionType);