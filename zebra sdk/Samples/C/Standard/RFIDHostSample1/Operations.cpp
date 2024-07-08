#include "stdafx.h"

#define START_TRIGGER_TAB		0
#define STOP_TRIGGER_TAB		1
#define REPORT_TRIGGER_TAB		2
#define TAB_A					0
#define TAB_B					1



static DLGTEMPLATE* pDlgTemplateStartTrigger = NULL;
static DLGTEMPLATE* pDlgTemplateStopTrigger = NULL;
static DLGTEMPLATE* pDlgTemplateReportTrigger = NULL;
static HWND hDlgStartTrigger, hDlgStopTrigger, hDlgReportTrigger;
static DLGTEMPLATE* pDlgTemplateTabPageA = NULL;
static DLGTEMPLATE* pDlgTemplateTabPageB = NULL;
static HWND hDlgTabPageA, hDlgTabPageB;
static bool postFilterCurrentlyDisplayed = false;

static LPWSTR Lock_privilege[] = 
{
	L"None",
	L"Read Write",
	L"Perma Lock",
	L"Perma Unlock",
	L"Unlock"
};
static LPWSTR start_triggerTypes[] = 
{
	L"Immediate",
	L"Periodic",
	L"GPI",
	L"Handheld trigger"
};
#define START_TRIGGER_TYPES		4

static LPWSTR stop_triggerTypes[] = 
{
	L"Immediate",
	L"Duration",
	L"GPI with timeout",
	L"Tag Observation",
	L"N attempts",
	L"Handheld trigger with timeout"
};
#define STOP_TRIGGER_TYPES		6

static LPWSTR match_patterns[] =
{
	L"A_AND_B",
	L"NOTA_AND_B",
	L"NOTA_AND_NOTB",
	L"A_AND_NOTB"
};
#define MATCH_PATTERN_TYPES		4

static LPWSTR filter_action[] = 
{
	L"Default",
	L"State Aware",
	L"State Unaware"
};
#define FILTER_ACTION_TYPES 3

static LPWSTR state_aware_action_target[] = 
{
	L"SL",
	L"S0",
	L"S1",
	L"S2",
	L"S3"
};
#define STATE_AWARE_ACTION_TARGET_TYPES 5

static LPWSTR state_aware_action_action[] = 
{
	L"Inv A Not Inv B",
	L"Asrt SL Not Dsrt SL",
	L"Inv A",
	L"Asrt SL",
	L"Not Inv B",
	L"Not Dsrt SL",
	L"Inv A2BB2A Not Inv B",
	L"Neg SL Not Asrt SL",
	L"Inv B Not Inv A",
	L"Dsrt SL Not Asrt SL",
	L"Inv B",
	L"Dsrt SL",
	L"Not Inv A",
	L"Not Asrt SL",
	L"Not Inv A2BB2A",
	L"Not Neg SL"
};
#define STATE_AWARE_ACTION_ACTION_TYPES 16

static LPWSTR state_unaware_action_action[] = 
{
	L"Select Not Unselect",
	L"Select",
	L"Not Unselect",
	L"Unselect",
	L"Unselect Not Select",
	L"Not Select"
};
#define STATE_UNAWARE_ACTION_ACTION_TYPES 6

static LPWSTR report_triggerTypes[] =
{
	L"Never",
	L"Immediate",
	L"Moderated"
};

HWND	hAccessFilterDlg;

void ShowSelectionStartTrigger(HWND hDlg)
{
	TCHAR szConversion[MAX_PATH];
	UINT16 nIndex;
	FILETIME FileTime, LocalFileTime;
	SYSTEMTIME LocalTime;
	
	switch(g_appData.m_TriggerInfo.startTrigger.type)
	{
	case START_TRIGGER_TYPE_IMMEDIATE:
		ShowWindow(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_GPIPORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTIMETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATE),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTTIME),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PERIODTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_PERIOD),SW_HIDE);

		break;

	case START_TRIGGER_TYPE_PERIODIC:
		SetDlgItemInt(hDlg, IDC_TB_PERIOD, g_appData.m_TriggerInfo.startTrigger.value.periodic.periodMilliseconds, false);
		SystemTimeToFileTime(&g_appData.m_SystemTime,&FileTime);
		FileTimeToLocalFileTime(&FileTime, &LocalFileTime);
		FileTimeToSystemTime(&LocalFileTime, &LocalTime);
		g_appData.m_TriggerInfo.startTrigger.value.periodic.startTime = &(g_appData.m_SystemTime);
		SendMessage(GetDlgItem(hDlg, IDC_STARTTIME), DTM_SETSYSTEMTIME, 0, (LPARAM)&LocalTime);
		SendMessage(GetDlgItem(hDlg, IDC_STARTDATE), DTM_SETSYSTEMTIME, 0, (LPARAM)&LocalTime);

		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATETEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTIMETEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATE),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTTIME),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_PERIODTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_PERIOD),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_GPIPORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);

		break;

	case START_TRIGGER_TYPE_HANDHELD:
		
		CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, HANDHELD_TRIGGER_RELEASED == g_appData.m_TriggerInfo.startTrigger.value.handheld.handheldEvent);
		CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, HANDHELD_TRIGGER_PRESSED == g_appData.m_TriggerInfo.startTrigger.value.handheld.handheldEvent);

		SetDlgItemText(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, TEXT("Trigger Released"));
		SetDlgItemText(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, TEXT("Trigger Pressed"));
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_GPIPORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTIMETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATE),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTTIME),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PERIODTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_PERIOD),SW_HIDE);
		break;

	case START_TRIGGER_TYPE_GPI:

		SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT), CB_RESETCONTENT, 0, 0);			
		for(nIndex = 0; nIndex < g_appData.m_ReaderCaps.numGPIs; nIndex++)
		{
			wsprintf(szConversion, (LPCTSTR)TEXT("%u"), nIndex+1);
			SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		
		// Lets initialize the value to 1
		if(g_appData.m_TriggerInfo.startTrigger.value.gpi.portNumber == 0)
				g_appData.m_TriggerInfo.startTrigger.value.gpi.portNumber = 1;

		SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT), CB_SETCURSEL, g_appData.m_TriggerInfo.startTrigger.value.gpi.portNumber-1, 0);

		CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, false == g_appData.m_TriggerInfo.startTrigger.value.gpi.gpiEvent);
		CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, true == g_appData.m_TriggerInfo.startTrigger.value.gpi.gpiEvent);

		SetDlgItemText(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, TEXT("High to Low"));
		SetDlgItemText(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, TEXT("Low to High"));
		ShowWindow(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_GPI_PORT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_GPIPORTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTIMETEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTDATE),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STARTTIME),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PERIODTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_PERIOD),SW_HIDE);
		break;
	}
}

INT_PTR CALLBACK TagStartTriggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16 nIndex;
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 0; nIndex < START_TRIGGER_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_TYPE), CB_ADDSTRING, 0,(LPARAM) start_triggerTypes[nIndex]);
		}

		SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_TYPE), CB_SETCURSEL, g_appData.m_TriggerInfo.startTrigger.type, 0);
		ShowSelectionStartTrigger(hDlg);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			g_appData.m_TriggerInfo.startTrigger.type = (START_TRIGGER_TYPE)SendMessage(GetDlgItem(hDlg, IDC_CB_START_TRIGGER_TYPE), CB_GETCURSEL, 0, 0);
			if(LOWORD(wParam) == IDC_CB_START_TRIGGER_TYPE)
			{
				ShowSelectionStartTrigger(hDlg);
			}
		}
		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			if (LOWORD(wParam) == IDC_CHECK_GPI_HIGH_TO_LOW)
			{
				CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, false);
			}
			else if (LOWORD(wParam) == IDC_CHECK_GPI_LOW_TO_HIGH)
			{
				CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, false);
			}
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				

		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;

}

void ShowSelectionStopTrigger(HWND hDlg)
{
	TCHAR szConversion[MAX_PATH];
	UINT16 nIndex;

	switch(g_appData.m_TriggerInfo.stopTrigger.type)
	{
	case STOP_TRIGGER_TYPE_IMMEDIATE:

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_HIDE);

		break;

	case STOP_TRIGGER_TYPE_DURATION:

		SetDlgItemInt(hDlg, IDC_TB_DURATION, g_appData.m_TriggerInfo.stopTrigger.value.duration, false);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_SHOW);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_SHOW);//duration

		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_HIDE);

		break;

	case STOP_TRIGGER_TYPE_HANDHELD_WITH_TIMEOUT:

		CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, HANDHELD_TRIGGER_RELEASED == g_appData.m_TriggerInfo.stopTrigger.value.handheld.handheldEvent);
		CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, HANDHELD_TRIGGER_PRESSED == g_appData.m_TriggerInfo.stopTrigger.value.handheld.handheldEvent);

		SetDlgItemInt(hDlg, IDC_TB_TIMEOUT, g_appData.m_TriggerInfo.stopTrigger.value.handheld.timeoutMilliseconds, false);

		SetDlgItemText(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, TEXT("Trigger Released"));
		SetDlgItemText(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, TEXT("Trigger Pressed"));
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_HIDE);
		break;


	case STOP_TRIGGER_TYPE_GPI_WITH_TIMEOUT:

		SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT), CB_RESETCONTENT, 0, 0);			

		for(nIndex = 0; nIndex < g_appData.m_ReaderCaps.numGPIs; nIndex++)
		{
			wsprintf(szConversion, (LPCTSTR)TEXT("%u"), nIndex + 1);
			SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}

		// Lets initialize the value to 1
		if(g_appData.m_TriggerInfo.stopTrigger.value.gpi.portNumber == 0)
				g_appData.m_TriggerInfo.stopTrigger.value.gpi.portNumber = 1;

		SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT), CB_SETCURSEL, g_appData.m_TriggerInfo.stopTrigger.value.gpi.portNumber-1, 0);

		CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, false == g_appData.m_TriggerInfo.stopTrigger.value.gpi.gpiEvent);
		CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, true == g_appData.m_TriggerInfo.stopTrigger.value.gpi.gpiEvent);

		SetDlgItemInt(hDlg, IDC_TB_TIMEOUT, g_appData.m_TriggerInfo.stopTrigger.value.gpi.timeoutMilliseconds, false);

		SetDlgItemText(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, TEXT("High to Low"));
		SetDlgItemText(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, TEXT("Low to High"));
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_HIDE);


		break;

	case STOP_TRIGGER_TYPE_TAG_OBSERVATION_WITH_TIMEOUT:
		SetDlgItemInt(hDlg, IDC_TB_TAGOBSERVATION, g_appData.m_TriggerInfo.stopTrigger.value.tagObservation.n, false);
		SetDlgItemInt(hDlg, IDC_TB_TIMEOUT, g_appData.m_TriggerInfo.stopTrigger.value.tagObservation.timeoutMilliseconds, false);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_HIDE);

		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_HIDE);

		break;

	case STOP_TRIGGER_TYPE_N_ATTEMPTS_WITH_TIMEOUT:

		SetDlgItemInt(hDlg, IDC_TB_NATTEMPTS, g_appData.m_TriggerInfo.stopTrigger.value.numAttempts.n, false);
		SetDlgItemInt(hDlg, IDC_TB_TIMEOUT, g_appData.m_TriggerInfo.stopTrigger.value.numAttempts.timeoutMilliseconds, false);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_NATTEMPTS),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TIMEOUT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_NATTEMPTSTEXT),SW_SHOW);
		ShowWindow(GetDlgItem(hDlg, IDC_TIMEOUTTEXT),SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDC_TB_DURATION),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_GPI_PORT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TB_TAGOBSERVATION),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_DURATIONTEXT),SW_HIDE);//duration
		ShowWindow(GetDlgItem(hDlg, IDC_PORTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_EVENTTEXT),SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_TAGOBSERVATIONTEXT),SW_HIDE);

		break;
	}


}


INT_PTR CALLBACK TagStopTriggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16 nIndex;
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);
		for(nIndex = 0; nIndex < STOP_TRIGGER_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_TYPE), CB_ADDSTRING, 0,(LPARAM) stop_triggerTypes[nIndex]);
		}

		SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_TYPE), CB_SETCURSEL, g_appData.m_TriggerInfo.stopTrigger.type, 0);
		ShowSelectionStopTrigger(hDlg);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE)
		{
			g_appData.m_TriggerInfo.stopTrigger.type = (STOP_TRIGGER_TYPE)SendMessage(GetDlgItem(hDlg, IDC_CB_STOP_TRIGGER_TYPE), CB_GETCURSEL, 0, 0);
			if(LOWORD(wParam) == IDC_CB_STOP_TRIGGER_TYPE)
			{
				ShowSelectionStopTrigger(hDlg);
			}
		}
		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			if (LOWORD(wParam) == IDC_CHECK_GPI_HIGH_TO_LOW)
			{
				CheckDlgButton(hDlg, IDC_CHECK_GPI_LOW_TO_HIGH, false);
			}
			else if (LOWORD(wParam) == IDC_CHECK_GPI_LOW_TO_HIGH)
			{
				CheckDlgButton(hDlg, IDC_CHECK_GPI_HIGH_TO_LOW, false);
			}
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				

		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;

}

INT_PTR CALLBACK TagReportTriggerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16 nIndex;
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 0; nIndex <= MODERATED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_NEW_TAG_EVENT), CB_ADDSTRING, 0,(LPARAM) report_triggerTypes[nIndex]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_NEW_TAG_EVENT), CB_SETCURSEL, g_appData.m_TagEventReportInfo.reportNewTagEvent, 0);
		SetDlgItemInt(hDlg, IDC_TB_NEW_TAG_MODERATED, g_appData.m_TagEventReportInfo.newTagEventModeratedTimeoutMilliseconds, false);

		for(nIndex = 0; nIndex <= MODERATED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_TAG_INVISIBLE_EVENT), CB_ADDSTRING, 0,(LPARAM) report_triggerTypes[nIndex]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_TAG_INVISIBLE_EVENT), CB_SETCURSEL, g_appData.m_TagEventReportInfo.reportTagInvisibleEvent, 0);
		SetDlgItemInt(hDlg, IDC_TB_INVISIBLE_TAG_MODERATED, g_appData.m_TagEventReportInfo.tagInvisibleEventModeratedTimeoutMilliseconds, false);

		for(nIndex = 0; nIndex <= MODERATED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_TAG_BACKTO_VISIBILITY_EVENT), CB_ADDSTRING, 0,(LPARAM) report_triggerTypes[nIndex]);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_TAG_BACKTO_VISIBILITY_EVENT), CB_SETCURSEL, g_appData.m_TagEventReportInfo.reportTagBackToVisibilityEvent, 0);
		SetDlgItemInt(hDlg, IDC_TB_BACK_TAG_MODERATED, g_appData.m_TagEventReportInfo.tagBackToVisibilityModeratedTimeoutMilliseconds, false);


		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				

		}

		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;

}

DLGTEMPLATE * WINAPI DoLockDlgRes(LPCWSTR lpszResName) 
{ 
	DWORD dwError;
	HRSRC hrsrc = FindResource(g_hInst, lpszResName, RT_DIALOG);
	dwError = GetLastError();
	HGLOBAL hglb = LoadResource(g_hInst, hrsrc); 
	return (DLGTEMPLATE *) LockResource(hglb); 
} 

void displayChildDialogs(HWND hDlg, UINT16 lResult, RECT tabRect)
{
	if (lResult == START_TRIGGER_TAB )
	{
		ShowWindow(hDlgStopTrigger, SW_HIDE);
		ShowWindow(hDlgReportTrigger, SW_HIDE);
		ShowWindow(hDlgStartTrigger, SW_SHOW);
	}
	else if (lResult == STOP_TRIGGER_TAB )
	{
		ShowWindow(hDlgStartTrigger, SW_HIDE);
		ShowWindow(hDlgReportTrigger, SW_HIDE);
		ShowWindow(hDlgStopTrigger, SW_SHOW);

	}
	else if (lResult == REPORT_TRIGGER_TAB )
	{
		ShowWindow(hDlgStartTrigger, SW_HIDE);
		ShowWindow(hDlgStopTrigger, SW_HIDE);
		ShowWindow(hDlgReportTrigger, SW_SHOW);
	}
}

INT_PTR CALLBACK TriggerInfoDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT16 tabsel;
	TCITEM item = {0};
	UINT16 lResult;
	int nStartSelection, nStopSelection;
	TCHAR szConversion[MAX_PATH];
	DWORD dwError;
	DWORD dwDlgBase; 
	int cxMargin; 
	int cyMargin; 
	RECT rcTab = {0}; 
	int index; 
	HWND hwndTab;
	RECT tabRect = {0};
	FILETIME FileTime, LocalFileTime;
	SYSTEMTIME LocalTime;	
	int textLength;
	int value;
    
	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		dwDlgBase = GetDialogBaseUnits(); 
		cxMargin = LOWORD(dwDlgBase) / 4; 
		cyMargin = HIWORD(dwDlgBase) / 8; 

		hwndTab = GetDlgItem(hDlg, IDC_TRIGGERTAB);
		// Add a tab for each of the three child dialog boxes. 
		item.mask = TCIF_TEXT; 
		item.iImage = -1; 
		item.pszText = L"Start Trigger"; 
		item.cchTextMax = wcslen(L"Start Trigger") + 1;
		item.lParam = 0;
		index = TabCtrl_InsertItem(hwndTab, 0, &item); 
		dwError = GetLastError();
		item.pszText =L"Stop Trigger"; 
		item.cchTextMax = wcslen(L"Stop Trigger") + 1;
		item.lParam = 0;
		index = TabCtrl_InsertItem(hwndTab, 1, &item); 
		item.pszText =L"Report Trigger"; 
		item.cchTextMax = wcslen(L"Report Trigger") + 1;
		item.lParam = 0;
		index = TabCtrl_InsertItem(hwndTab, 2, &item); 

		GetWindowRect(hwndTab, &tabRect);
		// Lock the resources for the three child dialog boxes. 
		pDlgTemplateStartTrigger  = DoLockDlgRes(MAKEINTRESOURCE(IDD_TRIGGER_STARTTRIGGER )); 
		pDlgTemplateStopTrigger  = DoLockDlgRes(MAKEINTRESOURCE(IDD_TRIGGER_STOPTRIGGER)); 
		pDlgTemplateReportTrigger  = DoLockDlgRes(MAKEINTRESOURCE(IDD_TRIGGER_TAG_EVENT_REPORT)); 

		tabsel = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_TRIGGERTAB));

		hDlgStartTrigger = CreateDialogIndirect(g_hInst, pDlgTemplateStartTrigger, hDlg,TagStartTriggerDlg );
		hDlgStopTrigger = CreateDialogIndirect(g_hInst, pDlgTemplateStopTrigger, hDlg,TagStopTriggerDlg);
		hDlgReportTrigger = CreateDialogIndirect(g_hInst, pDlgTemplateReportTrigger, hDlg,TagReportTriggerDlg );
		SetDlgItemInt(hDlg,IDC_TB_TAG_REPORT_TRIGGER , g_appData.m_TriggerInfo.tagReportTrigger, false);		
		SetDlgItemInt(hDlg,IDC_EDIT_PERIODIC_REPORT_TRIGGER , g_appData.m_TriggerInfo.lpReportTriggers->periodicReportDuration, false);
		
#ifdef WINCE
		{
			int displacement = -25;
			int resize_x = 5;
			int resize_y = 42;
			GetWindowRect(GetDlgItem(hDlg,IDC_TRIGGERTAB), &tabRect);
			SetWindowPos(hDlgStartTrigger, HWND_TOPMOST, tabRect.left + 2, tabRect.top + displacement, 
				tabRect.right - tabRect.left - resize_x, tabRect.bottom - tabRect.top - resize_y, SWP_SHOWWINDOW);
			SetWindowPos(hDlgStopTrigger, HWND_TOPMOST, tabRect.left + 2, tabRect.top + displacement, 
				tabRect.right - tabRect.left - resize_x, tabRect.bottom - tabRect.top - resize_y, SWP_SHOWWINDOW);
			SetWindowPos(hDlgReportTrigger, HWND_TOPMOST, tabRect.left + 2, tabRect.top + displacement, 
				tabRect.right - tabRect.left - resize_x, tabRect.bottom - tabRect.top - resize_y, SWP_SHOWWINDOW);
		}
#endif

		displayChildDialogs(hDlg, tabsel, tabRect);
		return (INT_PTR)TRUE;

	case WM_NOTIFY:

		lResult = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_TRIGGERTAB));
		if(tabsel != lResult)
		{
			tabsel = lResult;
			displayChildDialogs(hDlg, tabsel, tabRect);
		}
		break;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			lResult = SendMessage(GetDlgItem(hDlg, IDC_TRIGGERTAB), TCM_GETCURSEL, 0, 0);
			displayChildDialogs(hDlg, lResult, tabRect);
		}
		if (LOWORD(wParam) == IDC_APPLY )
		{
			nStartSelection = SendMessage(GetDlgItem(hDlgStartTrigger, IDC_CB_START_TRIGGER_TYPE), CB_GETCURSEL, 0, 0);
			nStopSelection = SendMessage(GetDlgItem(hDlgStopTrigger, IDC_CB_STOP_TRIGGER_TYPE), CB_GETCURSEL, 0, 0);
			switch(nStopSelection)
			{
			case STOP_TRIGGER_TYPE_DURATION:

				g_appData.m_TriggerInfo.stopTrigger.type = STOP_TRIGGER_TYPE_DURATION ;
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_DURATION), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.duration = _ttoi(szConversion);
				break;

			case STOP_TRIGGER_TYPE_HANDHELD_WITH_TIMEOUT:

				g_appData.m_TriggerInfo.stopTrigger.type = STOP_TRIGGER_TYPE_HANDHELD_WITH_TIMEOUT;
				g_appData.m_TriggerInfo.stopTrigger.value.handheld.handheldEvent = IsDlgButtonChecked(hDlgStopTrigger, 
					IDC_CHECK_GPI_HIGH_TO_LOW)?HANDHELD_TRIGGER_RELEASED:HANDHELD_TRIGGER_PRESSED;
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_TIMEOUT), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.handheld.timeoutMilliseconds = _ttoi(szConversion);
				break;

			case STOP_TRIGGER_TYPE_GPI_WITH_TIMEOUT:

				g_appData.m_TriggerInfo.stopTrigger.type = STOP_TRIGGER_TYPE_GPI_WITH_TIMEOUT;
				g_appData.m_TriggerInfo.stopTrigger.value.gpi.portNumber = SendMessage(GetDlgItem(hDlgStopTrigger, IDC_CB_STOP_TRIGGER_GPI_PORT), CB_GETCURSEL, 0, 0) + 1;
				g_appData.m_TriggerInfo.stopTrigger.value.gpi.gpiEvent = IsDlgButtonChecked(hDlgStopTrigger, IDC_CHECK_GPI_HIGH_TO_LOW)?false:true;
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_TIMEOUT), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.gpi.timeoutMilliseconds = _ttoi(szConversion);
				break;

			case STOP_TRIGGER_TYPE_TAG_OBSERVATION_WITH_TIMEOUT:

				g_appData.m_TriggerInfo.stopTrigger.type= STOP_TRIGGER_TYPE_TAG_OBSERVATION_WITH_TIMEOUT;
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_TAGOBSERVATION), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.tagObservation.n = (UINT16)_ttoi(szConversion);
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_TIMEOUT), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.tagObservation.timeoutMilliseconds = _ttoi(szConversion);
				break;

			case STOP_TRIGGER_TYPE_N_ATTEMPTS_WITH_TIMEOUT:

				g_appData.m_TriggerInfo.stopTrigger.type =STOP_TRIGGER_TYPE_N_ATTEMPTS_WITH_TIMEOUT;
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_NATTEMPTS), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.numAttempts.n =(UINT16)_ttoi(szConversion);
				GetWindowText(GetDlgItem(hDlgStopTrigger, IDC_TB_TIMEOUT), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.stopTrigger.value.numAttempts.timeoutMilliseconds = _ttoi(szConversion);

				break;
			default:
				g_appData.m_TriggerInfo.stopTrigger.type = STOP_TRIGGER_TYPE_IMMEDIATE ;
				break;
			}
			switch(nStartSelection)
			{
			case START_TRIGGER_TYPE_PERIODIC:
				g_appData.m_TriggerInfo.startTrigger.type = START_TRIGGER_TYPE_PERIODIC;
				GetWindowText(GetDlgItem(hDlgStartTrigger, IDC_TB_PERIOD), szConversion, MAX_PATH);
				g_appData.m_TriggerInfo.startTrigger.value.periodic.periodMilliseconds = _ttoi(szConversion);
				SendMessage(GetDlgItem(hDlgStartTrigger, IDC_STARTDATE), DTM_GETSYSTEMTIME, 0, (LPARAM)&LocalTime);
				SendMessage(GetDlgItem(hDlgStartTrigger, IDC_STARTTIME), DTM_GETSYSTEMTIME, 0, (LPARAM)&LocalTime);
				SystemTimeToFileTime(&LocalTime,&LocalFileTime);
				LocalFileTimeToFileTime(&LocalFileTime,&FileTime);
				FileTimeToSystemTime(&FileTime,&g_appData.m_SystemTime);
		    	g_appData.m_TriggerInfo.startTrigger.value.periodic.startTime = &g_appData.m_SystemTime;
				break;

			case START_TRIGGER_TYPE_HANDHELD:

				g_appData.m_TriggerInfo.startTrigger.type = START_TRIGGER_TYPE_HANDHELD ;
				g_appData.m_TriggerInfo.startTrigger.value.handheld.handheldEvent = IsDlgButtonChecked(hDlgStartTrigger, IDC_CHECK_GPI_HIGH_TO_LOW)?HANDHELD_TRIGGER_RELEASED:HANDHELD_TRIGGER_PRESSED;
				break;

			case START_TRIGGER_TYPE_GPI:
				g_appData.m_TriggerInfo.startTrigger.type = START_TRIGGER_TYPE_GPI ;
				g_appData.m_TriggerInfo.startTrigger.value.gpi.portNumber = SendMessage(GetDlgItem(hDlgStartTrigger, IDC_CB_START_TRIGGER_GPI_PORT), CB_GETCURSEL, 0, 0) + 1;
				g_appData.m_TriggerInfo.startTrigger.value.gpi.gpiEvent = IsDlgButtonChecked(hDlgStartTrigger, IDC_CHECK_GPI_HIGH_TO_LOW)?false:true;
				break;

			default:
				g_appData.m_TriggerInfo.startTrigger.type  = START_TRIGGER_TYPE_IMMEDIATE;
				break;
			}
			
			textLength = GetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_REPORT_TRIGGER),szConversion, MAX_PATH);
			value = _ttoi(szConversion);
			if(value == 0 && !(textLength == 1 && *szConversion == _T('0'))) value = 1;  // ReportTrigger default value
			g_appData.m_TriggerInfo.tagReportTrigger = (textLength == 0 || value < 0) ? 1: value;


			g_appData.m_TagEventReportInfo.reportNewTagEvent = (TAG_EVENT_REPORT_TRIGGER)SendMessage(GetDlgItem(hDlgReportTrigger, IDC_CB_NEW_TAG_EVENT), CB_GETCURSEL, 0, 0);
			g_appData.m_TagEventReportInfo.reportTagInvisibleEvent = (TAG_EVENT_REPORT_TRIGGER)SendMessage(GetDlgItem(hDlgReportTrigger, IDC_CB_TAG_INVISIBLE_EVENT), CB_GETCURSEL, 0, 0);
			g_appData.m_TagEventReportInfo.reportTagBackToVisibilityEvent = (TAG_EVENT_REPORT_TRIGGER)SendMessage(GetDlgItem(hDlgReportTrigger, IDC_CB_TAG_BACKTO_VISIBILITY_EVENT), CB_GETCURSEL, 0, 0);
			GetWindowText(GetDlgItem(hDlgReportTrigger, IDC_TB_NEW_TAG_MODERATED), szConversion, MAX_PATH);
			g_appData.m_TagEventReportInfo.newTagEventModeratedTimeoutMilliseconds = _ttoi(szConversion);
			GetWindowText(GetDlgItem(hDlgReportTrigger, IDC_TB_INVISIBLE_TAG_MODERATED), szConversion, MAX_PATH);
			g_appData.m_TagEventReportInfo.tagInvisibleEventModeratedTimeoutMilliseconds = _ttoi(szConversion);
			GetWindowText(GetDlgItem(hDlgReportTrigger, IDC_TB_BACK_TAG_MODERATED), szConversion, MAX_PATH);
			g_appData.m_TagEventReportInfo.tagBackToVisibilityModeratedTimeoutMilliseconds = _ttoi(szConversion);
			
			textLength = GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PERIODIC_REPORT_TRIGGER),szConversion, MAX_PATH);
			value = _ttoi(szConversion);
			g_appData.m_TriggerInfo.lpReportTriggers->periodicReportDuration = (textLength == 0 || value < 0) ? 0: value;

			EndDialog(hDlg, LOWORD(wParam));
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				

		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}



INT_PTR CALLBACK AntennaInfoDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	UINT16 nIndex,nSelectedCount=0;
	UINT32 dlgItem = IDC_CHECK_ANTENNA1;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		g_appData.m_AntennaInfo.pAntennaList = g_appData.m_AntennaList;

		for(nIndex = 0; nIndex < g_appData.m_ReaderCaps.numAntennas; nIndex++, dlgItem++)
		{  	  
			ShowWindow(GetDlgItem(hDlg, dlgItem),SW_SHOW);
		}
		dlgItem = IDC_CHECK_ANTENNA1;
		for(nIndex = 0; nIndex < g_appData.m_AntennaInfo.length; nIndex++)
		{  
			int dlgItemToBeChecked = dlgItem+g_appData.m_AntennaList[nIndex]-1;
			CheckDlgButton(hDlg,  dlgItemToBeChecked,TRUE);
		}

		if(g_appData.m_AntennaInfo.length == 0)
		{
			dlgItem = IDC_CHECK_ANTENNA1;
			for(nIndex = 0; nIndex < (g_appData.m_ReaderCaps.numAntennas); nIndex++, dlgItem++)
			{
				CheckDlgButton(hDlg, dlgItem,true);
			}
		}

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			if (LOWORD(wParam) == IDC_CHECK_SELECT_ALL )
			{

				BOOLEAN newState;
				newState = IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_ALL);

				dlgItem = IDC_CHECK_ANTENNA1;
				for(nIndex = 0; nIndex < (g_appData.m_ReaderCaps.numAntennas); nIndex++, dlgItem++)
				{
					CheckDlgButton(hDlg, dlgItem,newState);
				}
			}
		}
		if (LOWORD(wParam) == IDC_APPLY )
		{
			dlgItem = IDC_CHECK_ANTENNA1;

			for(nIndex = 0, nSelectedCount = 0; nIndex < g_appData.m_ReaderCaps.numAntennas; nIndex++, dlgItem++)
			{
				if(IsDlgButtonChecked(hDlg, dlgItem))
				{
					g_appData.m_AntennaList[nSelectedCount++] = nIndex+1;
				}
			}
			g_appData.m_AntennaInfo.length = nSelectedCount;

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if(LOWORD(wParam) == IDOK)
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
void LoadFilterActionValues(HWND hDlg, int tabSel)
{
	UINT16 nIndex = 0;
	LPPRE_FILTER lpPreFilter = TAB_A == tabSel ? &g_appData.m_PreFilter1 : &g_appData.m_PreFilter2;
	UINT32 stateAwareAction = TAB_A == tabSel ? g_appData.m_StateAwareAction1 : g_appData.m_StateAwareAction2;

	SendMessage(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), CB_RESETCONTENT, 0, 0);			
	SendMessage(GetDlgItem(hDlg, IDC_CB_TARGET_A + tabSel), CB_RESETCONTENT, 0, 0);					
	lpPreFilter->filterAction = (FILTER_ACTION)SendMessage(GetDlgItem(hDlg, IDC_CB_FILTER_ACTION_A + tabSel), CB_GETCURSEL, 0, 0);

	if(lpPreFilter->filterAction == FILTER_ACTION_DEFAULT)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CB_TARGET_A + tabSel), FALSE);
	}
	else if(lpPreFilter->filterAction == FILTER_ACTION_STATE_AWARE)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_CB_TARGET_A + tabSel), TRUE);

		// Populate State Aware Filter Action - Action Combo-Box
		for(nIndex = 0; nIndex < STATE_AWARE_ACTION_ACTION_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), CB_ADDSTRING, 0,
				(LPARAM) state_aware_action_action[nIndex]);
		}
		// Populate State Aware Filter Action - Target Combo-Box
		for(nIndex = 0; nIndex < STATE_AWARE_ACTION_TARGET_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_TARGET_A + tabSel), CB_ADDSTRING, 0,
				(LPARAM) state_aware_action_target[nIndex]);
		}
		SendDlgItemMessage(hDlg, IDC_CB_ACTION_A + tabSel,(UINT) CB_SETCURSEL, 
			(WPARAM)stateAwareAction,(LPARAM) 0 );  
		SendDlgItemMessage(hDlg, IDC_CB_TARGET_A + tabSel,(UINT) CB_SETCURSEL,
			(WPARAM)lpPreFilter->filterActionParams.stateAwareParams.target,(LPARAM) 0 );  
	}
	else if(lpPreFilter->filterAction == FILTER_ACTION_STATE_UNAWARE)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_CB_TARGET_A + tabSel), FALSE);

		// Populate State Aware Filter Action - Action Combo-Box
		for(nIndex = 0; nIndex < STATE_UNAWARE_ACTION_ACTION_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_ACTION_A + tabSel), CB_ADDSTRING, 0,
				(LPARAM) state_unaware_action_action[nIndex]);
		}
		SendDlgItemMessage(hDlg, IDC_CB_ACTION_A + tabSel,(UINT) CB_SETCURSEL,
			(WPARAM)lpPreFilter->filterActionParams.stateUnawareAction,(LPARAM) 0 );  
	}
}


void LoadValuesToPreFilterDlg(HWND hDlg, int tabSel)
{
	TCHAR szValue[MAX_PATH];

	LPPRE_FILTER lpPreFilter = TAB_A == tabSel ? &g_appData.m_PreFilter1 : &g_appData.m_PreFilter2;
	UINT16 antennaID = TAB_A == tabSel ? g_appData.m_PreFilter1AntennaID : g_appData.m_PreFilter2AntennaID;

	SendDlgItemMessage(hDlg, IDC_CB_MEMORYBANK_A + tabSel,(UINT) CB_SETCURSEL,(WPARAM)lpPreFilter->memoryBank - 1,(LPARAM) 0 );  
	SendDlgItemMessage(hDlg, IDC_CB_ANTENNAID_A + tabSel, (UINT) CB_SETCURSEL,(WPARAM)antennaID,(LPARAM) 0 );  
	SendDlgItemMessage(hDlg, IDC_CB_FILTER_ACTION_A + tabSel,(UINT) CB_SETCURSEL,(WPARAM)lpPreFilter->filterAction,(LPARAM) 0 );  

	LoadFilterActionValues(hDlg, tabSel);

	_stprintf(szValue, TEXT("%d"),lpPreFilter->bitOffset);	
	SetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET_A + tabSel), szValue);

	if(lpPreFilter->pTagPattern)
	{
		ConvertBytePtrToHexString(lpPreFilter->pTagPattern, lpPreFilter->tagPatternBitCount/8, szValue);
	}
	else
	{	
		_stprintf(szValue, TEXT(""));
	}
	SetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_PATTERN_A + tabSel), szValue);
}

INT_PTR CALLBACK PreFilterTabPageCommonDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam,  int tabSel)
{
	TCHAR szConversion[MAX_PATH];
	UINT16 nIndex;
	bool useFilter = false;
	LPPRE_FILTER lpPreFilter = TAB_A == tabSel ? &g_appData.m_PreFilter1 : &g_appData.m_PreFilter2;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		useFilter =  TAB_A == tabSel ? g_appData.m_PreFilter1Set : g_appData.m_PreFilter2Set;
		CheckDlgButton(hDlg, IDC_CHECK_USE_PREFILTER_A + tabSel, useFilter);

		// Populate Memory Banks Combo-Box
		for(nIndex = 2; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK_A + tabSel), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);
		}
		// Populate Antenna IDs Combo-box
		for(nIndex = 0; nIndex < g_appData.m_ReaderCaps.numAntennas + 1; nIndex++)
		{
			_stprintf(szConversion, TEXT("%d"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID_A + tabSel), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		// Populate Filter Action Combo-Box
		for(nIndex = 0; nIndex < FILTER_ACTION_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_FILTER_ACTION_A + tabSel), CB_ADDSTRING, 0,(LPARAM) filter_action[nIndex]);
		}
		LoadValuesToPreFilterDlg(hDlg, tabSel);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{	
			if(LOWORD(wParam) == IDC_CB_FILTER_ACTION_A + tabSel)
			{
				LoadFilterActionValues(hDlg, tabSel);
			}
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				
		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}


INT_PTR CALLBACK PreFilterTabPageADlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return PreFilterTabPageCommonDlg(hDlg, message, wParam, lParam,  TAB_A);
}

INT_PTR CALLBACK PreFilterTabPageBDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return PreFilterTabPageCommonDlg(hDlg, message, wParam, lParam, TAB_B);
}


void processPreFilterDlgMsg(HWND hDlg, int tabSel)
{
	TCITEM item = {0};
	TCHAR szConversion[MAX_PATH];
	RECT rcTab = {0}; 
	RECT tabRect = {0};
	TCHAR* pInput;
	bool useFilter = false;
	int byteCount = 0;

	HWND hDlgTagPattern = TAB_A == tabSel ? hDlgTabPageA : hDlgTabPageB;

	LPPRE_FILTER lpPreFilter = TAB_A == tabSel ? &g_appData.m_PreFilter1 : &g_appData.m_PreFilter2;
	UINT32 *pStateAwareAction = TAB_A == tabSel ? &g_appData.m_StateAwareAction1 : &g_appData.m_StateAwareAction2;

	// Fetch Offset
	GetWindowText(GetDlgItem(hDlgTagPattern, IDC_TB_OFFSET_A + tabSel), szConversion, MAX_PATH);
	pInput = szConversion;
	lpPreFilter->bitOffset=(UINT16)_ttoi(pInput);

	// Fetch memory bank
	lpPreFilter->memoryBank = (MEMORY_BANK)(SendMessage(GetDlgItem(hDlgTagPattern, IDC_CB_MEMORYBANK_A + tabSel), CB_GETCURSEL, 0, 0) + 1);

	// Fetch Memory Tag Pattern
	GetWindowText(GetDlgItem(hDlgTagPattern, IDC_TB_TAG_PATTERN_A + tabSel), szConversion, MAX_PATH);
	if(_tcslen(szConversion) != 0)
	{
		if(lpPreFilter->pTagPattern == NULL)
			lpPreFilter->pTagPattern = new UINT8[MAX_PATH/2];

		ConvertHexStringToBytePtr(szConversion, lpPreFilter->pTagPattern, &byteCount);
		lpPreFilter->tagPatternBitCount = byteCount * 8;
	}
	else
	{
		lpPreFilter->tagPatternBitCount = 0;
	}

	// Fetch Antenna ID		
	UINT16 antennaID = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID_A + tabSel), CB_GETCURSEL, 0, 0);
	if(TAB_A == tabSel)
		g_appData.m_PreFilter1AntennaID = antennaID;
	else 
		g_appData.m_PreFilter2AntennaID = antennaID;

	// Fetch Filter Action
	if(lpPreFilter->filterAction == FILTER_ACTION_STATE_AWARE)
	{
		*pStateAwareAction = SendMessage(GetDlgItem(hDlgTagPattern, IDC_CB_ACTION_A + tabSel), CB_GETCURSEL, 0, 0);
		lpPreFilter->filterActionParams.stateAwareParams.stateAwareAction = 
			(STATE_AWARE_ACTION)(*pStateAwareAction/2);
		lpPreFilter->filterActionParams.stateAwareParams.target = 
			(TARGET)SendMessage(GetDlgItem(hDlgTagPattern, IDC_CB_TARGET_A + tabSel), CB_GETCURSEL, 0, 0);
	}
	else if(lpPreFilter->filterAction == FILTER_ACTION_STATE_UNAWARE)
	{
		lpPreFilter->filterActionParams.stateUnawareAction = 
			(STATE_UNAWARE_ACTION)(SendMessage(GetDlgItem(hDlgTagPattern, IDC_CB_ACTION_A + tabSel), CB_GETCURSEL, 0, 0));
	}

	if(TAB_A == tabSel)
		g_appData.m_PreFilter1Set = IsDlgButtonChecked(hDlgTagPattern, IDC_CHECK_USE_PREFILTER_A);
	else		
		g_appData.m_PreFilter2Set = IsDlgButtonChecked(hDlgTagPattern, IDC_CHECK_USE_PREFILTER_B);
}

void displayFilterTabsChildDialogs(HWND hDlg, int tabSel, RECT tabRect)
{	
	ShowWindow(hDlgTabPageA, TAB_A == tabSel ? SW_SHOW : SW_HIDE);
	ShowWindow(hDlgTabPageB, TAB_B == tabSel ? SW_SHOW : SW_HIDE);
}

INT_PTR CALLBACK PreFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT16 tabsel;
	TCITEM item = {0};
	UINT16 lResult;
	DWORD dwError;
	RECT rcTab = {0}; 
	RECT tabRect = {0};

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		SetWindowText(hDlg, TEXT("Pre Filter"));

		// Add a tab for each of the three child dialog boxes. 
		item.mask = TCIF_TEXT; 
		item.iImage = -1; 
		item.pszText = L"Prefilter 1"; 
		item.cchTextMax = wcslen(L"Prefilter 1") + 1;
		item.lParam = 0;
		TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_PRE_FILTER_TABS), 0, &item); 
		dwError = GetLastError();
		item.pszText =L"Prefilter 2"; 
		item.cchTextMax = wcslen(L"Prefilter 2") + 1;
		item.lParam = 0;
		TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_PRE_FILTER_TABS), 1, &item); 
		GetWindowRect(hDlg, &tabRect);

		// Lock the resources for the three child dialog boxes. 
		pDlgTemplateTabPageA  = DoLockDlgRes(MAKEINTRESOURCE(IDD_PRE_FILTER_TAB_A)); 
		pDlgTemplateTabPageB  = DoLockDlgRes(MAKEINTRESOURCE(IDD_PRE_FILTER_TAB_B)); 

		tabsel = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_PRE_FILTER_TABS));

		hDlgTabPageA = CreateDialogIndirect(g_hInst, pDlgTemplateTabPageA, hDlg,PreFilterTabPageADlg );
		hDlgTabPageB = CreateDialogIndirect(g_hInst, pDlgTemplateTabPageB, hDlg,PreFilterTabPageBDlg);

#ifdef WINCE
		{
			int displacement = -25;
			int resize_x = 5;
			int resize_y = 42;
			GetWindowRect(GetDlgItem(hDlg,IDC_PRE_FILTER_TABS), &tabRect);
			SetWindowPos(hDlgTabPageA, HWND_TOPMOST, tabRect.left + 2, tabRect.top + displacement, 
				tabRect.right - tabRect.left - resize_x, tabRect.bottom - tabRect.top - resize_y, SWP_SHOWWINDOW);
			SetWindowPos(hDlgTabPageB, HWND_TOPMOST, tabRect.left + 2, tabRect.top + displacement,
				tabRect.right - tabRect.left - resize_x, tabRect.bottom - tabRect.top - resize_y, SWP_SHOWWINDOW);
		}
#endif

		displayFilterTabsChildDialogs(hDlg, tabsel, tabRect);
		return (INT_PTR)TRUE;

	case WM_NOTIFY:
		lResult = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_PRE_FILTER_TABS));
		if(tabsel != lResult)
		{
			tabsel = lResult;
			displayFilterTabsChildDialogs(hDlg, tabsel, tabRect);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_APPLY )
		{
			UINT32 filterIndex = 0;

			processPreFilterDlgMsg(hDlg, TAB_A); // Fetch values of Prefilter1
			processPreFilterDlgMsg(hDlg, TAB_B); // Fetch values of Prefilter2

			RFID_DeletePreFilter(g_appData.m_RfidReaderHandle, 0, 0);
			g_appData.m_RfidStatus = RFID_API_SUCCESS;
			if(g_appData.m_PreFilter1Set)
			{
				g_appData.m_RfidStatus = RFID_AddPreFilter(g_appData.m_RfidReaderHandle, g_appData.m_PreFilter1AntennaID, 
					&g_appData.m_PreFilter1, &filterIndex);
			}
			if(g_appData.m_RfidStatus  == RFID_API_SUCCESS && g_appData.m_PreFilter2Set)
			{
				g_appData.m_RfidStatus = RFID_AddPreFilter(g_appData.m_RfidReaderHandle, g_appData.m_PreFilter2AntennaID, 
					&g_appData.m_PreFilter2, &filterIndex);
			}

			//Post Status to the status bar
			PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);			
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);			
		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

void LoadValuesToTagPatternDlg(HWND hDlg, bool postFilter, int tabSel)
{
	TCHAR szValue[MAX_PATH];

	LPTAG_PATTERN lpTagPattern = postFilter ? 
		(TAB_A == tabSel ? g_appData.m_PostFilter.lpTagPatternA : g_appData.m_PostFilter.lpTagPatternB): 
		(TAB_A == tabSel ? g_appData.m_AccessFilter.lpTagPatternA : g_appData.m_AccessFilter.lpTagPatternB);

	SendDlgItemMessage(hDlg, IDC_CB_MEMORYBANK_A + tabSel,(UINT) CB_SETCURSEL,(WPARAM) lpTagPattern->memoryBank,(LPARAM) 0 );  
	_stprintf(szValue, TEXT("%d"),lpTagPattern->bitOffset);	
	SetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET_A + tabSel), szValue);
	if(lpTagPattern->pTagMask)
	{
		ConvertBytePtrToHexString(lpTagPattern->pTagMask, lpTagPattern->tagMaskBitCount/8, szValue);
	}
	else
	{	
		_stprintf(szValue, TEXT(""));
	}
	SetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_MASK_A + tabSel), szValue);
	if(lpTagPattern->pTagPattern)
	{
		ConvertBytePtrToHexString(lpTagPattern->pTagPattern, lpTagPattern->tagPatternBitCount/8, szValue);
	}
	else
	{	
		_stprintf(szValue, TEXT(""));
	}
	SetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_PATTERN_A + tabSel), szValue);
}

INT_PTR CALLBACK PostAccessFilterTabPageCommonDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, bool postFilter,  
												  int tabSel)
{
	UINT16 nIndex;
	LPTAG_PATTERN lpTagPattern;

	switch (message)
	{
	case WM_INITDIALOG:

		lpTagPattern = postFilter ? 
			(TAB_A == tabSel ? g_appData.m_PostFilter.lpTagPatternA : g_appData.m_PostFilter.lpTagPatternB): 
			(TAB_A == tabSel ? g_appData.m_AccessFilter.lpTagPatternA : g_appData.m_AccessFilter.lpTagPatternB);

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 1; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK_A + tabSel), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);
		}
		LoadValuesToTagPatternDlg(hDlg, postFilter, tabSel);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);				

		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}


INT_PTR CALLBACK PostAccessFilterTabPageADlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return PostAccessFilterTabPageCommonDlg(hDlg, message, wParam, lParam, postFilterCurrentlyDisplayed,  TAB_A);
}

INT_PTR CALLBACK PostAccessFilterTabPageBDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return PostAccessFilterTabPageCommonDlg(hDlg, message, wParam, lParam, postFilterCurrentlyDisplayed, TAB_B);
}

void processPostAccessFilterDlgMsg(HWND hDlg, bool postFilter, int tabSel)
{
	TCITEM item = {0};
	TCHAR szConversion[MAX_PATH];
	RECT rcTab = {0}; 
	RECT tabRect = {0};
	TCHAR* pInput;
	bool useFilter = false;
	int byteCount = 0;

	HWND hDlgTagPattern = TAB_A == tabSel ? hDlgTabPageA : hDlgTabPageB;

	MATCH_PATTERN currentMatchPattern;

	LPTAG_PATTERN lpTagPattern = postFilter ? 
		(TAB_A == tabSel ? g_appData.m_PostFilter.lpTagPatternA : g_appData.m_PostFilter.lpTagPatternB): 
		(TAB_A == tabSel ? g_appData.m_AccessFilter.lpTagPatternA : g_appData.m_AccessFilter.lpTagPatternB);

	// Fetch Offset
	GetWindowText(GetDlgItem(hDlgTagPattern, IDC_TB_OFFSET_A + tabSel), szConversion, MAX_PATH);
	pInput = szConversion;
	lpTagPattern->bitOffset=(UINT16)_ttoi(pInput);

	// Fetch memory bank
	lpTagPattern->memoryBank = (MEMORY_BANK)SendMessage(GetDlgItem(hDlgTagPattern, IDC_CB_MEMORYBANK_A + tabSel), CB_GETCURSEL, 0, 0);

	// Fetch Memory Tag Pattern
	GetWindowText(GetDlgItem(hDlgTagPattern, IDC_TB_TAG_PATTERN_A + tabSel), szConversion, MAX_PATH);
	if(_tcslen(szConversion) != 0)
	{
		if(lpTagPattern->pTagPattern == NULL)
			lpTagPattern->pTagPattern = new UINT8[MAX_PATH/2];

		ConvertHexStringToBytePtr(szConversion, lpTagPattern->pTagPattern, &byteCount);
		lpTagPattern->tagPatternBitCount = byteCount * 8;
	}
	else
	{
		lpTagPattern->tagPatternBitCount = 0;
	}

	// Fetch Memory Bit Mask
	GetWindowText(GetDlgItem(hDlgTagPattern, IDC_TB_TAG_MASK_A + tabSel),szConversion, MAX_PATH);
	if(lpTagPattern->pTagMask == NULL)
		lpTagPattern->pTagMask = new UINT8[MAX_PATH/2];
	ConvertHexStringToBytePtr(szConversion, lpTagPattern->pTagMask, &byteCount);
	lpTagPattern->tagMaskBitCount = byteCount * 8;

	useFilter = IsDlgButtonChecked(hDlg, IDC_CHECK_USE_FILTER);
	currentMatchPattern = (MATCH_PATTERN)SendMessage(GetDlgItem(hDlg, IDC_CB_MATCHPATTERN), CB_GETCURSEL, 0, 0);
	if(postFilter)
	{
		g_appData.m_PostFilter.matchPattern = currentMatchPattern;
		g_appData.m_PostFilterSet = useFilter;
	}
	else
	{
		g_appData.m_AccessFilter.matchPattern = currentMatchPattern;
		g_appData.m_AccessFilterSet = useFilter;
	}
}

INT_PTR CALLBACK PostAccessFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, bool postFilter)
{
	static UINT16 tabsel;
	TCITEM item = {0};
	DWORD dwError;
	RECT rcTab = {0}; 
	RECT tabRect = {0};
	UINT16 lResult;
	MATCH_PATTERN currentMatchPattern;
	bool useFilter = false;

	BOOL bRet;
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwICC = ICC_TAB_CLASSES;
	InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	bRet = InitCommonControlsEx(&InitCtrls);


	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		SetWindowText(hDlg, postFilter ? TEXT("Post Filter") : TEXT("Access Filter"));

		if(postFilter)
		{
			hAccessFilterDlg = hDlg;
			if(g_appData.m_PostFilter.lpTagPatternA == NULL)
			{
				g_appData.m_PostFilter.lpTagPatternA = new TAG_PATTERN;
				memset(g_appData.m_PostFilter.lpTagPatternA, 0, sizeof(TAG_PATTERN));
			}

			if(g_appData.m_PostFilter.lpTagPatternB == NULL)
			{
				g_appData.m_PostFilter.lpTagPatternB = new TAG_PATTERN;
				memset(g_appData.m_PostFilter.lpTagPatternB, 0, sizeof(TAG_PATTERN));
			}
		}
		else
		{	
			if(g_appData.m_AccessFilter.lpTagPatternA == NULL)
			{
				g_appData.m_AccessFilter.lpTagPatternA = new TAG_PATTERN;
				memset(g_appData.m_AccessFilter.lpTagPatternA, 0, sizeof(TAG_PATTERN));
			}

			if(g_appData.m_AccessFilter.lpTagPatternB == NULL)
			{
				g_appData.m_AccessFilter.lpTagPatternB = new TAG_PATTERN;
				memset(g_appData.m_AccessFilter.lpTagPatternB, 0, sizeof(TAG_PATTERN));
			}
		}
		useFilter = postFilter ? g_appData.m_PostFilterSet : g_appData.m_AccessFilterSet;
		CheckDlgButton(hDlg, IDC_CHECK_USE_FILTER, useFilter);

		for(int nIndex = 0; nIndex < MATCH_PATTERN_TYPES; nIndex++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CB_MATCHPATTERN), CB_ADDSTRING, 0,(LPARAM) match_patterns[nIndex]);
		}	
		currentMatchPattern = postFilter ? g_appData.m_PostFilter.matchPattern : g_appData.m_AccessFilter.matchPattern;

		SendMessage(GetDlgItem(hDlg, IDC_CB_MATCHPATTERN), CB_SETCURSEL, currentMatchPattern, 0);

		// Add a tab for each of the three child dialog boxes. 
		item.mask = TCIF_TEXT; 
		item.iImage = -1; 
		item.pszText = L"Tag Patttern A"; 
		item.cchTextMax = wcslen(L"Tag Patttern A") + 1;
		item.lParam = 0;
		TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_POST_ACCESS_FILTER_TABS), 0, &item); 
		dwError = GetLastError();
		item.pszText =L"Tag Pattern B"; 
		item.cchTextMax = wcslen(L"Tag Pattern B") + 1;
		item.lParam = 0;
		TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_POST_ACCESS_FILTER_TABS), 1, &item); 

		dwError = GetLastError();
		// Lock the resources for the three child dialog boxes. 
		pDlgTemplateTabPageA  = DoLockDlgRes(MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER_TAB_A )); 
		pDlgTemplateTabPageB  = DoLockDlgRes(MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER_TAB_B)); 

		tabsel = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_POST_ACCESS_FILTER_TABS));

		hDlgTabPageA = CreateDialogIndirect(g_hInst, pDlgTemplateTabPageA, hDlg,PostAccessFilterTabPageADlg );

		hDlgTabPageB = CreateDialogIndirect(g_hInst, pDlgTemplateTabPageB, hDlg,PostAccessFilterTabPageBDlg);
#ifdef WINCE
		{
			int displacement = -23;
			int resize = 40;
			GetWindowRect(GetDlgItem(hDlg,IDC_POST_ACCESS_FILTER_TABS), &tabRect);
			SetWindowPos(hDlgTabPageA, HWND_TOPMOST, tabRect.left + 5, tabRect.top + displacement, 
				tabRect.right - tabRect.left -  resize, tabRect.bottom - tabRect.top - resize, SWP_SHOWWINDOW);
			SetWindowPos(hDlgTabPageB, HWND_TOPMOST, tabRect.left + 5, tabRect.top + displacement,
				tabRect.right - tabRect.left -  resize, tabRect.bottom - tabRect.top - resize, SWP_SHOWWINDOW);
		}
#endif
		displayFilterTabsChildDialogs(hDlg, tabsel, tabRect);
		return (INT_PTR)TRUE;

	case WM_NOTIFY:
		lResult = TabCtrl_GetCurSel(GetDlgItem(hDlg,IDC_POST_ACCESS_FILTER_TABS));
		if(tabsel != lResult)
		{
			tabsel = lResult;
			displayFilterTabsChildDialogs(hDlg, tabsel, tabRect);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_APPLY )
		{
			processPostAccessFilterDlgMsg(hDlg, postFilter, TAB_A); // Fetch values of TagPatternA
			processPostAccessFilterDlgMsg(hDlg, postFilter, TAB_B); // Fetch values of TagPatternB
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);			
		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			SendMessage (hDlg, WM_CLOSE, 0, 0);			
		}
		return (INT_PTR)TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PostFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	postFilterCurrentlyDisplayed = true;
	return PostAccessFilterDlg(hDlg, message,wParam, lParam, postFilterCurrentlyDisplayed);
}


INT_PTR CALLBACK AccessFilterDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	postFilterCurrentlyDisplayed = false;
	return PostAccessFilterDlg(hDlg, message,wParam, lParam, postFilterCurrentlyDisplayed);
}

// Locate Tag Dialog Proc
INT_PTR CALLBACK LocateTagDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;

	TAG_DATA *lpTagData = NULL;
	INITCOMMONCONTROLSEX InitCtrls;


	BOOLEAN bRet;
	switch (message)
	{
	case WM_INITDIALOG:
		InitCtrls.dwICC = ICC_PROGRESS_CLASS;
		InitCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
		bRet = InitCommonControlsEx(&InitCtrls);
		SetTimer(hDlg, ID_UPDATE_TIMER, 1000, NULL);

		g_appData.m_hLocateDialog = hDlg;
		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		ShowWindow(hDlg, SW_SHOW);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LOCATE)
		{
			// Get the text of Locate Button
			GetWindowText(GetDlgItem(hDlg, IDC_LOCATE), szBuffer, MAX_PATH);

			if (_tcscmp(szBuffer, TEXT("&Start")) == 0)
			{

				// Get the Tag ID
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID),szBuffer, MAX_PATH);

				// convert Tag ID Hex string to byte array
				ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

				//SetCursor(LoadCursor(NULL, IDC_WAIT));
				StartTagLocationing(tagID, tagIDLength);

			//	SetCursor(LoadCursor(NULL, IDC_ARROW));

				SetWindowText(GetDlgItem(hDlg, IDC_LOCATE), TEXT("Sto&p"));
			}
			else if (_tcscmp(szBuffer, TEXT("Sto&p")) == 0)
			{
				StopTagLocationing();
				SendMessage(GetDlgItem(g_appData.m_hLocateDialog, IDC_PROGRESS_BAR), PBM_SETPOS, 0, 0);
				SetWindowText(GetDlgItem(hDlg, IDC_LOCATE), TEXT("&Start"));
			}

		}
		break;

	case WM_TIMER:
		if(GetTickCount() - g_appData.m_locatedTagSeenTickCount > 1000)
			SendMessage(GetDlgItem(g_appData.m_hLocateDialog, IDC_PROGRESS_BAR), PBM_SETPOS, 0, 0);
		break;

	case WM_CLOSE:
		KillTimer(hDlg, ID_UPDATE_TIMER);
		if (g_appData.m_OperationState == RUNNING)
			StopTagLocationing();
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

// Read Tag Dialog Proc
INT_PTR CALLBACK AccessReadDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;
	int   foundLocation = -1;

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
		//PostRFIDStatus(TEXT("Read Tag in Progress"));

		// Add Memory Bank to the Drop Down List
		for(int nIndex = 1; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
			SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);

		// Set the index (1) to select EPC as default
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_SETCURSEL, 3, 0);

		// initialize Password, offset and length to 0
		SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_TB_OFFSET), WM_SETTEXT, 0, (LPARAM)TEXT("0"));
		SendMessage(GetDlgItem(hDlg, IDC_TB_LENGTH), WM_SETTEXT, 0, (LPARAM)TEXT("0"));

		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_READ)
		{
			// Get the Tag ID
			GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID),szBuffer, MAX_PATH);

			// convert Tag ID Hex string to byte array
			ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

			// Get Password
			SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
			_stscanf(szPassword, TEXT("%8x"), &passWord);

			// Get Selected memory bank index
			memoryBank = SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_GETCURSEL, 0, 0);

			// Get Length 
			GetWindowText(GetDlgItem(hDlg, IDC_TB_LENGTH), szLength, MAX_PATH);

			// Get OffSet
			GetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET), szOffSet, MAX_PATH);

			
			SetCursor(LoadCursor(NULL, IDC_WAIT));
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

				SendMessage(GetDlgItem(hDlg, IDC_TB_DATA_READ), WM_SETTEXT, 0, (LPARAM)szDataRead);

#ifdef WINCE
				if (tagIDLength > 0)
					MessageBox(NULL, TEXT("Succeeded"), TEXT("Read Tag"), MB_OK);
#endif

			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));

		}
		else if (LOWORD(wParam) == IDC_TB_TAGID)
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID), szBuffer, MAX_PATH);
				if (_tcslen(szBuffer) > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), FALSE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), TRUE);
			}	
		}
		else if (LOWORD(wParam) == IDC_ACCESSFILTER)
		{
			DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hDlg, AccessFilterDlg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

// Write Tag Dialog Proc
INT_PTR CALLBACK AccessWriteDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};

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
		//PostRFIDStatus(TEXT("Write Tag in Progress"));

		if (g_appData.m_BlockWrite)
			SetWindowText(hDlg, TEXT("Block Write Tag"));
		// Add Memory Bank to the Drop Down List
		for(int nIndex = 1; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
			SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORY_BANK), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);

		// Set the index (1) to select EPC as default
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORY_BANK), CB_SETCURSEL, 3, 0);

		// initialize Password, offset and length to 0
		SetWindowText(GetDlgItem(hDlg, IDC_TB_PASSWORD), TEXT("0"));
		SetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET), TEXT("0"));
		SetWindowText(GetDlgItem(hDlg, IDC_TB_LENGTH), TEXT("0"));

		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_WRITE)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TB_TAGID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);

			// convert Tag ID Hex string to byte array
			ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

			// Get Password
			SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
			_stscanf(szPassword, TEXT("%8x"), &passWord);

			// Get Selected memory bank index
			memoryBank = SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORY_BANK), CB_GETCURSEL, 0, 0);

			// Get Length 
			GetWindowText(GetDlgItem(hDlg, IDC_TB_LENGTH), szLength, MAX_PATH);

			// Get OffSet
			GetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET), szOffSet, MAX_PATH);

			// Get Data to Write
			GetWindowText(GetDlgItem(hDlg, IDC_TB_DATA), szDataToWrite, MAX_PATH);

			if (_tcslen(szDataToWrite) > 0)
			{
				ConvertHexStringToBytePtr(szDataToWrite, memoryBankData, (int*)&memoryBankDataLength);
				SetCursor(LoadCursor(NULL, IDC_WAIT));
				// WriteTag
				if (WriteTag(tagID, tagIDLength, (MEMORY_BANK)memoryBank, _ttoi(szOffSet), passWord, memoryBankData, memoryBankDataLength))
				{
#ifdef WINCE
					if (tagIDLength > 0)
					{
						if (g_appData.m_BlockWrite)
							MessageBox(NULL, TEXT("Succeeded"), TEXT("Block Write Tag"), MB_OK);
						else
							MessageBox(NULL, TEXT("Succeeded"), TEXT("Write Tag"),  MB_OK);
					}
#endif
				}

				SetCursor(LoadCursor(NULL, IDC_ARROW));

			}
		}
		else if (LOWORD(wParam) == IDC_TB_TAGID)
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID), szBuffer, MAX_PATH);
				if (_tcslen(szBuffer) > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), FALSE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), TRUE);
			}	
		}
		else if (LOWORD(wParam) == IDC_ACCESSFILTER)
		{
			DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hDlg, AccessFilterDlg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;

}

// Lock Tag Dialog Proc
INT_PTR CALLBACK AccessLockDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};

	BYTE  tagID[MAX_PATH/2] = {0,};
	int   tagIDLength = 0;

	TCHAR szPassword[MAX_PATH] = {0,};
	UINT32 passWord = 0;

	int privilege = 0;
	int dataField = 0;


	switch (message)
	{
	case WM_INITDIALOG:

		// Add Lock Data field to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0, (LPARAM)TEXT("Kill Password"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0, (LPARAM)TEXT("Access Password"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0, (LPARAM)TEXT("EPC Memory"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0, (LPARAM)TEXT("TID Memory"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0, (LPARAM)TEXT("User Memory"));

		// Add Privilege Data fields to the Drop Down List
		SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Read-Write"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Permanent Lock"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Permanent unlock"));
		SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_ADDSTRING, 0, (LPARAM)TEXT("Unlock"));

		// Set the index to 0
		SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_SETCURSEL, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_SETCURSEL, 0, 0);

		// initialize password
		SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));

		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LOCK)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TB_TAGID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);

			// convert Tag ID Hex string to byte array
			ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

			// Get Password
			SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
			_stscanf(szPassword, TEXT("%8x"), &passWord);

			// Get data field
			dataField = SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_GETCURSEL, 0, 0);

			// Get Lock privilege
			privilege = SendMessage(GetDlgItem(hDlg, IDC_CB_PRIVILEGE), CB_GETCURSEL, 0, 0);
			privilege +=1 ;

			SetCursor(LoadCursor(NULL, IDC_WAIT));
			// Lock Tag
			if(LockTag(tagID, tagIDLength, (LOCK_DATA_FIELD)dataField, (LOCK_PRIVILEGE)privilege, passWord))
			{
#ifdef WINCE
				if (tagIDLength > 0)
					MessageBox(NULL, TEXT("Succeeded"), TEXT("Lock Tag"), MB_OK);
#endif
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else if (LOWORD(wParam) == IDC_TB_TAGID)
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID), szBuffer, MAX_PATH);
				if (_tcslen(szBuffer) > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), FALSE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), TRUE);
			}	
		}
		else if (LOWORD(wParam) == IDC_ACCESSFILTER)
		{
			DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hDlg, AccessFilterDlg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;

}

// Kill Tag Dialog Proc
INT_PTR CALLBACK AccessKillDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
		// initialize password
		SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_SETTEXT, 0, (LPARAM)TEXT("0"));

		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_KILL)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TB_TAGID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);
			// convert Tag ID Hex string to byte array
			ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

			// Get Password
			SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
			_stscanf(szPassword, TEXT("%8x"), &passWord);

			SetCursor(LoadCursor(NULL, IDC_WAIT));
			// Kill Tag
			if (KillTag(tagID, tagIDLength, passWord))
			{
#ifdef WINCE
				if (tagIDLength > 0)
					MessageBox(NULL, TEXT("Succeeded"), TEXT("Kill Tag"), MB_OK);
#endif
			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else if (LOWORD(wParam) == IDC_TB_TAGID)
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID), szBuffer, MAX_PATH);
				if (_tcslen(szBuffer) > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), FALSE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), TRUE);
			}	
		}
		else if (LOWORD(wParam) == IDC_ACCESSFILTER)
		{
			DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hDlg, AccessFilterDlg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AccessBlockEraseDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	TCHAR szBuffer[MAX_PATH] = {0,};

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
		//PostRFIDStatus(TEXT("Write Tag in Progress"));

		// Add Memory Bank to the Drop Down List
		for(int nIndex = 1; nIndex < MEM_BANKS_SUPPORTED; nIndex++)
			SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_ADDSTRING, 0,(LPARAM) memory_banks[nIndex]);

		// Set the index (1) to select EPC as default
		SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_SETCURSEL, 3, 0);

		// initialize Password, offset and length to 0
		SetWindowText(GetDlgItem(hDlg, IDC_TB_PASSWORD), TEXT("0"));
		SetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET), TEXT("0"));
		SetWindowText(GetDlgItem(hDlg, IDC_TB_LENGTH), TEXT("0"));

		// Set Tag ID 
		GetSelectedTagID(szBuffer);
		SendMessage(GetDlgItem(hDlg, IDC_TB_TAGID), WM_SETTEXT, 0, (LPARAM)szBuffer);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ERASE)
		{
			// Get the Tag ID
			SendDlgItemMessage(hDlg, IDC_TB_TAGID, WM_GETTEXT, MAX_PATH, (LPARAM)(LPCTSTR)szBuffer);

			// convert Tag ID Hex string to byte array
			ConvertHexStringToBytePtr(szBuffer, tagID, &tagIDLength);

			// Get Password
			SendMessage(GetDlgItem(hDlg, IDC_TB_PASSWORD), WM_GETTEXT, MAX_PATH, (LPARAM)szPassword);
			_stscanf(szPassword, TEXT("%8x"), &passWord);

			// Get Selected memory bank index
			memoryBank = SendMessage(GetDlgItem(hDlg, IDC_CB_MEMORYBANK), CB_GETCURSEL, 0, 0);

			// Get Length 
			GetWindowText(GetDlgItem(hDlg, IDC_TB_LENGTH), szLength, MAX_PATH);
			memoryBankDataLength = _ttoi(szLength);

			// Get OffSet
			GetWindowText(GetDlgItem(hDlg, IDC_TB_OFFSET), szOffSet, MAX_PATH);
			
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			// WriteTag
			if (BlockEraseTag(tagID, tagIDLength, (MEMORY_BANK)memoryBank, _ttoi(szOffSet), passWord, memoryBankDataLength))
			{
#ifdef WINCE
				if (tagIDLength > 0)
					MessageBox(NULL,  TEXT("Succeeded"), TEXT("Block Erase Tag"), MB_OK);
#endif
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else if (LOWORD(wParam) == IDC_TB_TAGID)
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAGID), szBuffer, MAX_PATH);
				if (_tcslen(szBuffer) > 0)
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), FALSE);
				else
					EnableWindow(GetDlgItem(hDlg, IDC_ACCESSFILTER), TRUE);
			}	
		}
		else if (LOWORD(wParam) == IDC_ACCESSFILTER)
		{
			DialogBox(g_hInst, (LPCTSTR)MAKEINTRESOURCE(IDD_POST_ACCESS_FILTER), hDlg, AccessFilterDlg);
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
}