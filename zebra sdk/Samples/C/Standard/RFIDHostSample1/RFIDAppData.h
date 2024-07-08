#pragma once
#include "rfidapi.h"

#define ID_UPDATE_TIMER			1
//
class CRFIDAppData
{
public:
	RFID_HANDLE32 m_RfidReaderHandle;  /// handle of the reader connected
	BOOLEAN m_ConnectionStatus;      
	TCHAR m_szHostName[MAX_PATH], m_szPort[MAX_PATH];
	RFID_STATUS m_RfidStatus;
	READER_CAPS m_ReaderCaps ; 
	TAG_STORAGE_SETTINGS m_TagStorageSettings;

	// Two PreFilters supported in Sample App
	UINT16 m_PreFilter1AntennaID;
	PRE_FILTER m_PreFilter1;
	UINT32 m_StateAwareAction1;
	BOOLEAN m_PreFilter1Set;
	UINT16 m_PreFilter2AntennaID;
	PRE_FILTER m_PreFilter2; 
	UINT32 m_StateAwareAction2;
	BOOLEAN m_PreFilter2Set;

	POST_FILTER m_PostFilter;  
	BOOLEAN m_PostFilterSet;

	ACCESS_FILTER m_AccessFilter;
	BOOLEAN m_AccessFilterSet;

	UINT16		m_AntennaList[8];
	ANTENNA_INFO m_AntennaInfo;

	TRIGGER_INFO m_TriggerInfo;
	TAG_EVENT_REPORT_INFO m_TagEventReportInfo;
	REPORT_TRIGGERS m_ReportTriggers;
	SYSTEMTIME m_SystemTime;

	RFID_HANDLE32 m_RfidRMHandle;

	BOOLEAN m_LoggedIn;
	LOGIN_INFO m_LoginInfo;
	READER_TYPE m_ReaderType;

	UINT32 m_UniqueTagCount;
	UINT32 m_CummulativeTagCount;
	HBITMAP		m_BitmapPlain;
	HBITMAP		m_BitmapDisabled;
	HBITMAP		m_BitmapRed;
	HBITMAP		m_BitmapGreen;
	HBITMAP		m_BitmapUnknown;

	UINT16 m_OperationState;
	BOOLEAN m_BlockWrite;
	BOOLEAN m_AccessSequenceRunning;

	UINT64 m_startTickCount;
	UINT64 m_stopTickCount;
	HWND   m_hLocateDialog;
	UINT64 m_locatedTagSeenTickCount;

	TCHAR m_szClientCertFilePath[MAX_PATH];
	TCHAR m_szClientKeyFilePath[MAX_PATH];
	TCHAR m_szRootCertFilePath[MAX_PATH];
	TCHAR m_szKeyPassword[MAX_PATH];
	bool m_bSecureConnection;
	bool m_bValidatePeer;

	// constructor
	CRFIDAppData()
	{
		m_hLocateDialog = NULL;
		_tcscpy(m_szPort, TEXT("5084"));
#ifdef WINCE
		_tcscpy(m_szHostName, TEXT("127.0.0.1"));
#else
		_tcscpy(m_szHostName, TEXT(""));
#endif
		m_ReaderType = FX;

		m_RfidReaderHandle = NULL;
		m_ConnectionStatus = false;    

		m_RfidStatus = RFID_API_SUCCESS;
		memset(&m_ReaderCaps, 0, sizeof(READER_CAPS)); 

		// Two PreFilters supported in Sample App
		m_PreFilter1AntennaID = 0;
		memset(&m_PreFilter1, 0, sizeof(PRE_FILTER)); 
		m_PreFilter1Set = false;
		m_PreFilter1.memoryBank = MEMORY_BANK_EPC;

		m_PreFilter2AntennaID = 0;
		memset(&m_PreFilter2, 0, sizeof(PRE_FILTER)); 
		m_PreFilter2Set = false;
		m_PreFilter2.memoryBank = MEMORY_BANK_EPC;

		m_StateAwareAction1 = m_StateAwareAction2 = 0;
		m_PostFilterSet = false; 
		memset(&m_PostFilter, 0, sizeof(POST_FILTER)); 

		m_AccessFilterSet = false;
		memset(&m_AccessFilter, 0, sizeof(ACCESS_FILTER));

		m_AntennaInfo.length = 0;
		m_AntennaInfo.pAntennaList = m_AntennaList;
		memset(m_AntennaList, 0, sizeof(m_AntennaList)/sizeof(UINT16));

		m_TriggerInfo.startTrigger.type = START_TRIGGER_TYPE_IMMEDIATE;
		m_TriggerInfo.stopTrigger.type = STOP_TRIGGER_TYPE_IMMEDIATE;
		m_TriggerInfo.tagReportTrigger = 1;
		m_ReportTriggers.periodicReportDuration = 0;
		m_TriggerInfo.lpReportTriggers = &m_ReportTriggers;
		memset(&m_TagEventReportInfo, 0, sizeof(TAG_EVENT_REPORT_INFO));
		m_TagEventReportInfo.newTagEventModeratedTimeoutMilliseconds = 500;
		m_TagEventReportInfo.tagBackToVisibilityModeratedTimeoutMilliseconds = 500;
		m_TagEventReportInfo.tagInvisibleEventModeratedTimeoutMilliseconds = 500;
		m_TagEventReportInfo.reportNewTagEvent = MODERATED;
		m_TagEventReportInfo.reportTagBackToVisibilityEvent = MODERATED;
		m_TagEventReportInfo.reportTagInvisibleEvent = MODERATED;
		memset(&m_SystemTime, 0, sizeof(SYSTEMTIME));

		m_RfidRMHandle = NULL;

		m_LoggedIn = false;
		memset(&m_LoginInfo, 0, sizeof(LOGIN_INFO)); 

		m_UniqueTagCount = 0;
		m_CummulativeTagCount = 0;
		m_OperationState = IDLE;
		m_BlockWrite = false;
		m_AccessSequenceRunning = false;
		m_startTickCount = 0;
		m_stopTickCount = 0;

		m_TagStorageSettings.enableAccessReports = false;
		m_TagStorageSettings.maxMemoryBankByteCount = 64;
		m_TagStorageSettings.maxTagCount = 4096;
		m_TagStorageSettings.maxTagIDByteCount = 12;
		m_TagStorageSettings.tagFields = (UINT16)(ANTENNA_ID				|
		FIRST_SEEN_TIME_STAMP	|
		LAST_SEEN_TIME_STAMP	|
		PEAK_RSSI				|
		TAG_SEEN_COUNT			|
		PC						|
		XPC						|
		CRC);
		
		m_bSecureConnection = m_bValidatePeer = false;
		_tcscpy(m_szClientCertFilePath, _T(""));
		_tcscpy(m_szClientKeyFilePath, _T(""));
		_tcscpy(m_szKeyPassword, _T(""));
		_tcscpy(m_szRootCertFilePath, _T(""));		 
	}

	// destructor
	~CRFIDAppData()
	{
		if(m_PreFilter1.pTagPattern)
			delete m_PreFilter1.pTagPattern;
		if(m_PreFilter2.pTagPattern)
			delete m_PreFilter2.pTagPattern;

		if(m_AccessFilter.lpTagPatternA)
		{
			if(m_AccessFilter.lpTagPatternA->pTagMask)
				delete m_AccessFilter.lpTagPatternA->pTagMask;

			if(m_AccessFilter.lpTagPatternA->pTagPattern)
				delete m_AccessFilter.lpTagPatternA->pTagPattern;

			if(m_AccessFilter.lpTagPatternA)
				delete m_AccessFilter.lpTagPatternA;
		}

		if(m_AccessFilter.lpTagPatternB)
		{
			if(m_AccessFilter.lpTagPatternB->pTagMask)
				delete m_AccessFilter.lpTagPatternB->pTagMask;
			if(m_AccessFilter.lpTagPatternB->pTagPattern)
				delete m_AccessFilter.lpTagPatternB->pTagPattern;
			if(m_AccessFilter.lpTagPatternB)
				delete m_AccessFilter.lpTagPatternB;
		}


		if(m_PostFilter.lpTagPatternA)
		{
			if(m_PostFilter.lpTagPatternA->pTagMask)
				delete m_PostFilter.lpTagPatternA->pTagMask;

			if(m_PostFilter.lpTagPatternA->pTagPattern)
				delete m_PostFilter.lpTagPatternA->pTagPattern;

			if(m_PostFilter.lpTagPatternA)
				delete m_PostFilter.lpTagPatternA;
		}

		if(m_PostFilter.lpTagPatternB)
		{
			if(m_PostFilter.lpTagPatternB->pTagMask)
				delete m_PostFilter.lpTagPatternB->pTagMask;
			if(m_PostFilter.lpTagPatternB->pTagPattern)
				delete m_PostFilter.lpTagPatternB->pTagPattern;
			if(m_PostFilter.lpTagPatternB)
				delete m_PostFilter.lpTagPatternB;
		}
	}

};

extern CRFIDAppData g_appData;
