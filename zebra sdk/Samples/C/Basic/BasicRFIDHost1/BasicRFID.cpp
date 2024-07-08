#include "BasicRFID.h"
#include "resource.h"


void  LOG_MSG(TCHAR *msg);
void printTagData(TAG_DATA *pTagData);

extern HWND g_hDlg;
RFID_HANDLE32 readerHandle;
bool isConnected = false;
DWORD readThread = NULL;


HANDLE stopTestingEventHandle;
HANDLE readerEventAwaitingThreadHandle;
DWORD WINAPI readerEventAwaitingThread(LPVOID pvarg);

bool ConnectToReader(TCHAR* pHostName)
{
	bool retVal = false;
	DWORD dwThreadID;
	CONNECTION_INFO connectionInfo;
	connectionInfo.version = RFID_API3_5_1;
	if(isConnected == false && RFID_API_SUCCESS == RFID_Connect(&readerHandle, pHostName, 0, 0,&connectionInfo))
	{		
		isConnected = true;
		stopTestingEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		readerEventAwaitingThreadHandle = CreateThread(NULL, 0, readerEventAwaitingThread, NULL, 0, &dwThreadID);
		retVal = true;
	}
	return retVal;
}

bool DisconnectReader()
{
	bool retVal = false;
	if(isConnected == true && RFID_API_SUCCESS == RFID_Disconnect(readerHandle))
	{
		StopContinuousInventory();
		SetEvent(stopTestingEventHandle);
		WaitForSingleObject(readerEventAwaitingThreadHandle, INFINITE);
		CloseHandle(readerEventAwaitingThreadHandle);
		readerEventAwaitingThreadHandle = NULL;
		retVal = true;
		isConnected = false;
	}
	return retVal;
}

bool StartContinuousInventory()
{
	bool retVal = false;

	if(isConnected)
	{
		if(RFID_API_SUCCESS == RFID_PerformInventory(readerHandle, NULL, NULL, NULL, NULL))
		{
			retVal = true;
		}
	}
	return retVal;
}


bool StopContinuousInventory()
{
	bool retVal = false;

	if(isConnected)
	{
		if(RFID_API_SUCCESS == RFID_StopInventory(readerHandle))
		{
			retVal = true;
		}
	}
	return retVal;
}



DWORD WINAPI readerEventAwaitingThread(LPVOID pvarg)
{
	#define MAX_EVENTS 12

	HANDLE hEvents[MAX_EVENTS];
	DWORD dwStatus;
	RFID_STATUS rfidStatus = RFID_API_SUCCESS;

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

	gpiEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	tagReadEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	bufferFullWarningEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	bufferFullEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	antennaEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	inventoryStartEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	inventoryStopEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	accessStartEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	accessStopEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	readerExceptionEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	readerDisconnectedEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    
	RFID_SetTraceLevel(readerHandle,TRACE_LEVEL_ALL);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, GPI_EVENT, gpiEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, TAG_READ_EVENT, tagReadEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, BUFFER_FULL_WARNING_EVENT, bufferFullWarningEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, BUFFER_FULL_EVENT, bufferFullEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, ANTENNA_EVENT, antennaEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, INVENTORY_START_EVENT, inventoryStartEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, INVENTORY_STOP_EVENT, inventoryStopEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, ACCESS_START_EVENT, accessStartEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, ACCESS_STOP_EVENT, accessStopEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, READER_EXCEPTION_EVENT,readerExceptionEventHandle);
	rfidStatus = RFID_RegisterEventNotification(readerHandle, DISCONNECTION_EVENT, readerDisconnectedEventHandle);

	hEvents[0] = gpiEventHandle;
	hEvents[1] = tagReadEventHandle;
	hEvents[2] = bufferFullWarningEventHandle;
	hEvents[3] = bufferFullEventHandle;
	hEvents[4] = antennaEventHandle;
	hEvents[5] = readerDisconnectedEventHandle;
	hEvents[6] = inventoryStartEventHandle;
	hEvents[7] = inventoryStopEventHandle;
	hEvents[8] = accessStartEventHandle;
	hEvents[9] = accessStopEventHandle;
	hEvents[10] =  readerExceptionEventHandle;
	hEvents[MAX_EVENTS-1] = stopTestingEventHandle;
	BOOL testRunning = TRUE;
	TCHAR szTemp[MAX_PATH] = {0,};
	TAG_DATA* pTagData = RFID_AllocateTag(readerHandle);

	while(testRunning)
	{
		dwStatus = WaitForMultipleObjects(MAX_EVENTS, hEvents, FALSE, INFINITE);
		switch(dwStatus)
		{
		case WAIT_OBJECT_0://gpiEventHandle
			{
				GPI_EVENT_DATA gpiEventData;
				BOOL pass;
				if(RFID_API_SUCCESS == RFID_GetEventData(readerHandle, GPI_EVENT, 
					(STRUCT_HANDLE)&gpiEventData))
				{
					pass = TRUE;
				}
				else 
					pass = FALSE;

				ResetEvent(gpiEventHandle);
				LOG_MSG(TEXT("SIGNALLED: GPI_EVENT"));
			}
			break;
		case WAIT_OBJECT_0+1://tagReadEventHandle
			//LOG_MSG(TEXT("SIGNALLED: TAG_READ_EVENT"));
			
			while(RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData))
			{
				printTagData(pTagData);
			}
			ResetEvent(tagReadEventHandle);
			break;
		case WAIT_OBJECT_0+2://bufferFullWarningEventHandle
			LOG_MSG(TEXT("SIGNALLED: BUFFER_FULL_WARNING_EVENT"));
			while(RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData))
			{
				printTagData(pTagData);
			}
			ResetEvent(bufferFullWarningEventHandle);
			break;
		case WAIT_OBJECT_0+3://bufferFullEventHandle
			LOG_MSG(TEXT("SIGNALLED: BUFFER_FULL_EVENT"));
			while(RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData))
			{
				printTagData(pTagData);
			}
			ResetEvent(bufferFullEventHandle);
		   break;
		case WAIT_OBJECT_0+4://antennaEventHandle
			{
				ANTENNA_EVENT_DATA antennaEventData;
				BOOL pass;
				if(RFID_API_SUCCESS == RFID_GetEventData(readerHandle, ANTENNA_EVENT, 
					(STRUCT_HANDLE)&antennaEventData))
				{
					pass = TRUE;
				}
				else 
					pass = FALSE;

				LOG_MSG(TEXT("SIGNALLED: ANTENNA_EVENT"));
			}
			break;
		case WAIT_OBJECT_0+5://readerDisconnectedEventHandle
			DISCONNECTION_EVENT_DATA disconnectEventData;
			RFID_GetEventData(readerHandle,DISCONNECTION_EVENT,(STRUCT_HANDLE)&disconnectEventData);
			_tcscpy(szTemp,TEXT("SIGNALLED:  "));
			_tcscat(szTemp,getDisconnectionEventString(disconnectEventData.eventInfo));
			LOG_MSG(szTemp);
			PostMessage(g_hDlg,WM_COMMAND,(WPARAM)IDM_DISCONNECT,NULL);
			break;
		case WAIT_OBJECT_0+6://inventoryStartEventHandle
			LOG_MSG(TEXT("SIGNALLED: INVENTORY_START_EVENT"));
			break;

		case WAIT_OBJECT_0+7://inventoryStopEventHandle
			LOG_MSG(TEXT("SIGNALLED: INVENTORY_STOP_EVENT"));
			while(RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData))
			{
				printTagData(pTagData);
			}
			break;

		case WAIT_OBJECT_0+8://accessStartEventHandle
			LOG_MSG(TEXT("SIGNALLED: ACCESS_START_EVENT"));
			break;

		case WAIT_OBJECT_0+9://inventoryStartEventHandle
			LOG_MSG(TEXT("SIGNALLED: ACCESS_STOP_EVENT"));
			break;
        case WAIT_OBJECT_0+10://ReaderExceptionEventHandle

			READER_EXCEPTION_EVENT_DATA readerExceptionEventData;
			RFID_GetEventData(readerHandle,READER_EXCEPTION_EVENT,(STRUCT_HANDLE)&readerExceptionEventData);
			_tcscpy(szTemp,TEXT("SIGNALLED: READER_EXCEPTION_EVENT  "));
			_tcscat(szTemp,readerExceptionEventData.exceptionInfo);
			LOG_MSG(szTemp);
			break;
		case WAIT_OBJECT_0+11://stopTestingEventHandle
			testRunning = false;
			break;
		}
	}

	CloseHandle(gpiEventHandle);
	CloseHandle(tagReadEventHandle);
	CloseHandle(bufferFullWarningEventHandle);
	CloseHandle(bufferFullEventHandle);
	CloseHandle(antennaEventHandle);
	CloseHandle(inventoryStartEventHandle);
	CloseHandle(inventoryStopEventHandle);
	CloseHandle(accessStartEventHandle);
	CloseHandle(accessStopEventHandle);
	CloseHandle(readerDisconnectedEventHandle);
	CloseHandle(readerExceptionEventHandle);
	CloseHandle(stopTestingEventHandle);

	
	RFID_DeallocateTag(readerHandle, pTagData);
	return 0;
}

bool ReadTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 length, UINT32 offset, 
			 UINT32 password, UINT8* pData, UINT32 *pDataLength)
{
	bool retVal = false;

	if(isConnected)
	{
		TAG_DATA* pTagData = RFID_AllocateTag(readerHandle);

		READ_ACCESS_PARAMS params;
		params.accessPassword = password;
		params.byteCount = length;
		params.memoryBank = mb;
		params.byteOffset = offset;

		if(RFID_API_SUCCESS == RFID_Read(readerHandle, pTagID, tagIDLength, &params, NULL, NULL, pTagData, NULL))
		{
			memcpy(pData, pTagData->pMemoryBankData, pTagData->memoryBankDataLength);
			*pDataLength = pTagData->memoryBankDataLength;
			retVal = true;
		}
		RFID_DeallocateTag(readerHandle, pTagData);
	}
	return retVal;
}

bool WriteTag(UINT8* pTagID, UINT32 tagIDLength, MEMORY_BANK mb, UINT32 offset, 
			  UINT32 password, UINT8* pData, UINT32 dataLength)
{
	bool retVal = false;

	if(isConnected)
	{
		WRITE_ACCESS_PARAMS params;
		params.accessPassword = password;
		params.writeDataLength = dataLength;
		params.memoryBank = mb;
		params.byteOffset = offset;
		params.pWriteData = pData;
		
		if(RFID_API_SUCCESS == RFID_Write(readerHandle, pTagID, tagIDLength, &params, NULL, NULL, NULL))
		{
			retVal = true;
		}
	}
	return retVal;
}

bool LockTag(UINT8* pTagID, UINT32 tagIDLength, LOCK_DATA_FIELD dataField, LOCK_PRIVILEGE privilege,
	     		   UINT32 password)
{
	bool retVal = false;

	if(isConnected)
	{
		LOCK_ACCESS_PARAMS params;
		params.accessPassword = password;
		params.privilege[dataField] = privilege;
		
		if(RFID_API_SUCCESS == RFID_Lock(readerHandle, pTagID, tagIDLength, &params, NULL, NULL, NULL))
		{
			retVal = true;
		}
	}
	return retVal;
}

// Kill Tag
bool KillTag(UINT8* pTagID, UINT32 tagIDLength, UINT32 password)
{
	bool retVal = false;

	if(isConnected)
	{
		KILL_ACCESS_PARAMS params;
	    params.killPassword = password;
				
	    if(RFID_API_SUCCESS == RFID_Kill(readerHandle, pTagID, tagIDLength, &params, 
								 NULL,NULL, NULL))
	    {
		    retVal = true;
	     }
	}
    return retVal;
}



bool GetLastErrorInfo(LPERROR_INFO lpErrorInfo)
{
	bool retVal = false;

	if(isConnected)
	{
		if(RFID_API_SUCCESS == RFID_GetLastErrorInfo(readerHandle, lpErrorInfo))
		{
			retVal = true;
		}
	}
	return retVal;
}


TCHAR* getDisconnectionEventString(DISCONNECTION_EVENT_TYPE disconnectionType)
{
	switch(disconnectionType)
	{
	case READER_INITIATED_DISCONNECTION:
		return TEXT("READER_INITIATED_DISCONNECTION");
	case READER_EXCEPTION:
		return TEXT("READER_EXCEPTION");
	case CONNECTION_LOST:
		return TEXT("CONNECTION_LOST");
	default: return TEXT("");
	}

}