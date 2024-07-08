#include "stdafx.h"


#define SELECT_ANTENNA					0
#define SELECT_RFMODE					1

static	LPWSTR rf_parameters[] = 
{	
	L"Mode Identifier",
	L"M",
	L"DR",
	L"Forward Link Modulation",
	L"PIE",
	L"Min Tari",
	L"Max Tari",
	L"Step Tari",
	L"EPC HAG T&C conformance",
	L"Spectral Mask Indicator",
	L"BDR"
};
static	LPWSTR session[] = 
{
	L"S0",
	L"S1",
	L"S2",
	L"S3"
};
static	LPWSTR invState[] = 
{
	L"A",
	L"B",
	L"AB_FLIP"
};
static	LPWSTR SLflag[] = 
{
	L"ASSERTED",
	L"DEASSERTED",
	L"SL_ALL"
};

HWND hRFmodeParamList;

static BOOL UpdateRFModeTable(HWND, UINT8);
//static void UpdateRFModeTable(HWND, UINT8, RF_MODE_TABLE_ENTRY* pTableEntry);
static void InsertItem(TCHAR* szConversion, UINT16 itemNumber);

INT_PTR CALLBACK RFmodeConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16					nIndex, nAntenna; 
	UINT32					nTableEntry = 0,nRFModeIndex = 0,nTariValue;
	LVCOLUMN				lvColumn;
	LVITEM					lvItem;
	HWND					hList;
	TCHAR					szConversion[MAX_PATH];
	RF_MODE_TABLE_ENTRY*	pTableEntry;

	switch (message)
	{
	case WM_INITDIALOG:  

		Createbutton(hDlg);
		Createmenu(hDlg);

		hList=GetDlgItem(hDlg,IDC_RFMODE_TABLE_LIST); // get the ID of the ListView				 
		hRFmodeParamList = hList;

		lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 125;
		lvColumn.pszText =L"Parameter";
		SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&lvColumn); // Insert/Show the coloum
		lvColumn.pszText =L"Value";
		SendMessage(hList,LVM_INSERTCOLUMN,1,(LPARAM)&lvColumn); // Insert/Show the coloum

		for(nIndex = 0; nIndex <11; nIndex++)
		{
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = nIndex;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPWSTR)rf_parameters[nIndex];
			SendMessage(hList,LVM_INSERTITEM,nIndex,(LPARAM)&lvItem); // Insert/Show the coloum
		}

		for(nIndex = 1; nIndex <= (g_appData.m_ReaderCaps.numAntennas); nIndex++)
		{
			wsprintf(szConversion, _T("%d"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_SETCURSEL, 0,0);

		for(nIndex = 0; nIndex <(g_appData.m_ReaderCaps.rfModes.pUHFTables->numEntries); nIndex++)
		{
			wsprintf(szConversion, _T("%d"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_RFMODE_TABLE_INDEX), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		UpdateRFModeTable(hDlg, SELECT_ANTENNA);

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			if(LOWORD(wParam) == IDC_CB_ANTENNAID)
				//UpdateRFModeTable(hDlg, SELECT_ANTENNA, pTableEntry);
				UpdateRFModeTable(hDlg, SELECT_ANTENNA);

			else if(LOWORD(wParam) == IDC_CB_RFMODE_TABLE_INDEX)
				//UpdateRFModeTable(hDlg, SELECT_RFMODE, pTableEntry);
				UpdateRFModeTable(hDlg, SELECT_RFMODE);

		}
		else if (LOWORD(wParam) == IDC_APPLY )
		{
			nAntenna = (UINT16)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0)+1;
			nTableEntry = (UINT16)SendMessage(GetDlgItem(hDlg, IDC_CB_RFMODE_TABLE_INDEX), CB_GETCURSEL, 0, 0);
			nRFModeIndex = nTableEntry; 
			GetWindowText(GetDlgItem(hDlg, IDC_TB_TARIVALUE), szConversion, MAX_PATH);
			nTariValue = _ttol(szConversion);
			g_appData.m_RfidStatus = RFID_SetRFMode(g_appData.m_RfidReaderHandle, nAntenna,nRFModeIndex,nTariValue);
			PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
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


INT_PTR CALLBACK GPIOConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16				nIndex, dlgItem;
	BOOLEAN				bGPOState[6];
	GPI_PORT_STATE		bGPIState[6];
	BOOLEAN				enableGPI = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 0, dlgItem = IDC_CHECK_GPO1; nIndex < (g_appData.m_ReaderCaps.numGPOs); nIndex++, dlgItem++)
		{   
			ShowWindow(GetDlgItem(hDlg, dlgItem),SW_SHOW);
			g_appData.m_RfidStatus = RFID_GetGPOState(g_appData.m_RfidReaderHandle, nIndex+1, &bGPOState[nIndex]);
			PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

			if(bGPOState[nIndex])
				CheckDlgButton(hDlg, dlgItem, TRUE);
			else
				CheckDlgButton(hDlg, dlgItem, FALSE);
		}

		for(nIndex = 0, dlgItem = IDC_CHECK_GPI1; nIndex<(g_appData.m_ReaderCaps.numGPIs); nIndex++, dlgItem++)
		{
			ShowWindow(GetDlgItem(hDlg, dlgItem),SW_SHOW);
			g_appData.m_RfidStatus = RFID_GetGPIState(g_appData.m_RfidReaderHandle, nIndex+1,&enableGPI, &bGPIState[nIndex]);
			PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

			if(enableGPI)
			    CheckDlgButton(hDlg, dlgItem, TRUE);
			else 
				CheckDlgButton(hDlg, dlgItem, FALSE);
			
		
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_APPLYGPO )
		{
			for(nIndex = 0, dlgItem = IDC_CHECK_GPO1; nIndex <(g_appData.m_ReaderCaps.numGPOs); nIndex++, dlgItem++)
			{
				bGPOState[nIndex] = IsDlgButtonChecked(hDlg,dlgItem) ;
				g_appData.m_RfidStatus = RFID_SetGPOState(g_appData.m_RfidReaderHandle, nIndex+1, bGPOState[nIndex]);
				PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if(LOWORD(wParam) == IDC_APPLYGPI )
		{
			for(nIndex = 0, dlgItem = IDC_CHECK_GPI1; nIndex<(g_appData.m_ReaderCaps.numGPIs); nIndex++, dlgItem++)
			{
				g_appData.m_RfidStatus = RFID_EnableGPIPort(g_appData.m_RfidReaderHandle, nIndex+1,(BOOLEAN)IsDlgButtonChecked(hDlg,dlgItem));
				PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
			}
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

void updateSigulationSettings(HWND hDlg)
{
	SINGULATION_CONTROL SingulationControl;
	UINT16				nAntenna;
	TCHAR				szConversion[MAX_PATH];

	nAntenna = (UINT16)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0);
	nAntenna+= 1;

	g_appData.m_RfidStatus = RFID_GetSingulationControl(g_appData.m_RfidReaderHandle, nAntenna, &SingulationControl);
	PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

	SendMessage(GetDlgItem(hDlg, IDC_CB_SESSION), CB_SETCURSEL, SingulationControl.session, 0);

	wsprintf(szConversion, _T("%d"), SingulationControl.tagPopulation);
	SetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_POPULATION), szConversion);

	wsprintf(szConversion, _T("%d"), SingulationControl.tagTransitTimeMilliseconds);
	SetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_TRANSIT_TIME) , szConversion);

	//  Depending on whether State Aware Singulation is supported or not, the fields
	// 	Inventoried State and SL flag get unlocked 
	if(SingulationControl.stateAwareSingulationAction.perform)
	{  
		CheckDlgButton(hDlg, IDC_STATE_AWARE_SINGULATION, true);
		EnableWindow(GetDlgItem(hDlg, IDC_INVENTORY_STATE), true);
		EnableWindow(GetDlgItem(hDlg, IDC_SL), true);

		SendMessage(GetDlgItem(hDlg, IDC_INVENTORY_STATE), CB_SETCURSEL, (WPARAM)SingulationControl.stateAwareSingulationAction.inventoryState, 0);
		SendMessage(GetDlgItem(hDlg, IDC_SL), CB_SETCURSEL, (WPARAM)SingulationControl.stateAwareSingulationAction.slFlag, 0);

	}

	else
	{
		CheckDlgButton(hDlg, IDC_STATE_AWARE_SINGULATION, false);
		EnableWindow(GetDlgItem(hDlg, IDC_INVENTORY_STATE), false);
		EnableWindow(GetDlgItem(hDlg, IDC_SL), false);
	}
}

INT_PTR CALLBACK SingulationConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR				population_str[MAX_PATH],transit_str[MAX_PATH];
	TCHAR				szConversion[MAX_PATH];
	SINGULATION_CONTROL SingulationControl;
	UINT16				nAntenna, nIndex;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 1; nIndex <= g_appData.m_ReaderCaps.numAntennas; nIndex++)
		{ 
			wsprintf(szConversion, _T("%d"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_SETCURSEL, 0,0);

		for(nIndex = 0; nIndex <= SESSION_S3; nIndex++)
		{ 
			wsprintf(szConversion, _T("%s"), session[nIndex]);
			SendMessage(GetDlgItem(hDlg, IDC_CB_SESSION), CB_ADDSTRING, 0,(LPARAM) szConversion);
		}

		if(g_appData.m_ReaderCaps.stateAwareSingulationSupported)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_INVENTORY_STATE), true);
			EnableWindow(GetDlgItem(hDlg, IDC_SL), true);

			for(nIndex = 0; nIndex <=INVENTORY_STATE_AB_FLIP; nIndex++)
			{ 
				wsprintf(szConversion, _T("%s"), invState[nIndex]);
				SendMessage(GetDlgItem(hDlg, IDC_INVENTORY_STATE), CB_ADDSTRING, 0,(LPARAM) szConversion);
			}
			for(nIndex = 0; nIndex <=SL_ALL; nIndex++)
			{ 
				wsprintf(szConversion, _T("%s"), SLflag[nIndex]);
				SendMessage(GetDlgItem(hDlg, IDC_SL), CB_ADDSTRING, 0,(LPARAM) szConversion);
			}
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_INVENTORY_STATE), false);
			EnableWindow(GetDlgItem(hDlg, IDC_SL), false);
		}
		updateSigulationSettings(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			if(LOWORD(wParam) == IDC_CB_ANTENNAID)
			{
				updateSigulationSettings(hDlg);
			}
		}
		else if(HIWORD(wParam) == BN_CLICKED) // what we press on?
		{
			if(LOWORD(wParam) == IDC_STATE_AWARE_SINGULATION)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_INVENTORY_STATE), IsDlgButtonChecked(hDlg, IDC_STATE_AWARE_SINGULATION));
				EnableWindow(GetDlgItem(hDlg, IDC_SL), IsDlgButtonChecked(hDlg, IDC_STATE_AWARE_SINGULATION));
			}
			if (LOWORD(wParam) == IDC_APPLY )
			{
				nAntenna = (ANTENNA_MODE)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0);
				nAntenna+=1;
				SingulationControl.session = (SESSION)SendMessage(GetDlgItem(hDlg, IDC_CB_SESSION), CB_GETCURSEL, 0, 0);
				if(IsDlgButtonChecked(hDlg, IDC_STATE_AWARE_SINGULATION))
				{
					SingulationControl.stateAwareSingulationAction.perform = true;
					SingulationControl.stateAwareSingulationAction.inventoryState = (INVENTORY_STATE)SendMessage(GetDlgItem(hDlg, IDC_INVENTORY_STATE), CB_GETCURSEL, 0, 0);
					SingulationControl.stateAwareSingulationAction.slFlag = (SL_FLAG)SendMessage(GetDlgItem(hDlg, IDC_SL), CB_GETCURSEL, 0, 0);
				}
				else
					SingulationControl.stateAwareSingulationAction.perform = false;
				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_POPULATION), population_str, MAX_PATH);
				SingulationControl.tagPopulation=_ttoi(population_str);

				GetWindowText(GetDlgItem(hDlg, IDC_TB_TAG_TRANSIT_TIME), transit_str, MAX_PATH);
				SingulationControl.tagTransitTimeMilliseconds=_ttoi(transit_str);

				g_appData.m_RfidStatus = RFID_SetSingulationControl(g_appData.m_RfidReaderHandle, nAntenna, &SingulationControl);
				PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

				EndDialog(hDlg, message);
				return (INT_PTR)TRUE;


			}

		}
		else if(LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, message);
			return (INT_PTR)TRUE;

		}

		return (INT_PTR)TRUE;

	case WM_CLOSE:

		EndDialog(hDlg, message);
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;

}

INT_PTR CALLBACK TagStorageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);
		SetWindowText(hDlg,L"Tag Storage Settings");

		if (RFID_API_SUCCESS == g_appData.m_RfidStatus)
		{
			// From API3 version 5.1.XX and higher, use RFID_GetTagStorageSettings to get API defaults.
			SetDlgItemInt(hDlg, IDC_TB_MAX_TAG_COUNT, g_appData.m_TagStorageSettings.maxTagCount, FALSE);
			SetDlgItemInt(hDlg, IDC_TB_MAX_TAG_LENGTH, g_appData.m_TagStorageSettings.maxTagIDByteCount, FALSE);
			SetDlgItemInt(hDlg, IDC_TB_MAX_SIZE_MB, g_appData.m_TagStorageSettings.maxMemoryBankByteCount, FALSE);
		}

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_APPLY )
		{
			g_appData.m_TagStorageSettings.maxTagCount = GetDlgItemInt(hDlg, IDC_TB_MAX_TAG_COUNT, NULL, FALSE);
			g_appData.m_TagStorageSettings.maxTagIDByteCount = GetDlgItemInt(hDlg, IDC_TB_MAX_TAG_LENGTH, NULL, FALSE);
			g_appData.m_TagStorageSettings.maxMemoryBankByteCount = GetDlgItemInt(hDlg, IDC_TB_MAX_SIZE_MB, NULL, FALSE);
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_PHASEINFO))
				g_appData.m_TagStorageSettings.tagFields |= PHASE_INFO;
			else
				g_appData.m_TagStorageSettings.tagFields &= ~PHASE_INFO;
			g_appData.m_RfidStatus = RFID_SetTagStorageSettings(g_appData.m_RfidReaderHandle, &g_appData.m_TagStorageSettings);
			//Post Status to the status bar
			PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
			if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
			{
				EndDialog(hDlg, LOWORD(wParam));
			}
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
//void UpdateRFModeTable(HWND hDlg, UINT8 selction, RF_MODE_TABLE_ENTRY* pTableEntry)
BOOL UpdateRFModeTable(HWND hDlg, UINT8 selction)
{  
	TCHAR	szConversion[MAX_PATH];
	UINT16	nAntenna;
	UINT32	nTableEntry,nRFModeIndex,nTariValue;
	RF_MODE_TABLE_ENTRY* pTableEntry;	

	nAntenna = (UINT16)SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0)+1;

	nTableEntry=0;
	pTableEntry = g_appData.m_ReaderCaps.rfModes.pUHFTables->pTablesEntries;
	//pTableEntry = &(g_appData.m_ReaderCaps.rfModes.pUHFTables->pTablesEntries[nIndex]);

	if(selction == SELECT_ANTENNA)
	{
		g_appData.m_RfidStatus = RFID_GetRFMode(g_appData.m_RfidReaderHandle, nAntenna,&nRFModeIndex,&nTariValue);
		PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);
		if(g_appData.m_RfidStatus != RFID_API_SUCCESS)
			return(false);
		wsprintf(szConversion, _T("%d"), nTariValue);
		SetWindowText(GetDlgItem(hDlg, IDC_TB_TARIVALUE) , szConversion);
	}
	else if(selction == SELECT_RFMODE)
	{
		nRFModeIndex = (UINT32)SendMessage(GetDlgItem(hDlg, IDC_CB_RFMODE_TABLE_INDEX), CB_GETCURSEL, 0, 0);
	}
	if(g_appData.m_ReaderCaps.rfModes.pUHFTables->numEntries != 1)
	{
		nTableEntry = nRFModeIndex;
		pTableEntry = (g_appData.m_ReaderCaps.rfModes.pUHFTables->pTablesEntries)+nTableEntry;
		
	}
	else
	{
		nTableEntry = 0;
	}

	SendMessage(GetDlgItem(hDlg, IDC_CB_RFMODE_TABLE_INDEX), CB_SETCURSEL, nTableEntry,0);
	wsprintf(szConversion, _T("%d"), pTableEntry->modeIdentifer);
	InsertItem(szConversion, 0);

	switch(pTableEntry->modulation)
	{
	case MV_FM0: 

		wsprintf(szConversion, L"MV_FM0");
		break;

	case MV_2: 

		wsprintf(szConversion, L"MV_2");
		break;

	case MV_4: 

		wsprintf(szConversion, L"MV_4");
		break;

	case MV_8: 

		wsprintf(szConversion, L"MV_8");
		break;

	default: break;
	}

	InsertItem(szConversion, 1);

	if(pTableEntry->divideRatio)
		wsprintf(szConversion, L"DR_64_3");
	else
		wsprintf(szConversion, L"DR_8");

	InsertItem(szConversion, 2);

	switch(pTableEntry->forwardLinkModulationType)
	{
	case FORWARD_LINK_MODULATION_PR_ASK:
		wsprintf(szConversion, L"PR_ASK");
		break;

	case FORWARD_LINK_MODULATION_SSB_ASK:

		wsprintf(szConversion, L"SSB_ASK");
		break;

	case FORWARD_LINK_MODULATION_DSB_ASK:

		wsprintf(szConversion, L"DSB_ASK");
		break;

	default:
		break;
	}
	InsertItem(szConversion, 3);

	wsprintf(szConversion, _T("%d"), pTableEntry->pieValue);
	InsertItem(szConversion, 4);

	wsprintf(szConversion, _T("%d"), pTableEntry->minTariValue);
	InsertItem(szConversion, 5);

	wsprintf(szConversion, _T("%d"), pTableEntry->maxTariValue);
	InsertItem(szConversion, 6);

	wsprintf(szConversion, _T("%d"), pTableEntry->stepTariValue);
	InsertItem(szConversion, 7);

	if(pTableEntry->epcHAGTCConformance)
		wsprintf(szConversion, L"yes");
	else
		wsprintf(szConversion, L"no");
	InsertItem(szConversion, 8);

	switch(pTableEntry->spectralMaskIndicator)
	{

	case SMI_UNKNOWN :  

		wsprintf(szConversion, L"Unknown");
		break;

	case SMI_SI :  

		wsprintf(szConversion, L"SI");
		break;

	case SMI_MI :

		wsprintf(szConversion, L"MI");
		break;

	case SMI_DI :

		wsprintf(szConversion, L"DI");
		break;
	}
	InsertItem(szConversion, 9);
	wsprintf(szConversion, _T("%d"), pTableEntry->bdrValue);
	InsertItem(szConversion, 10);
}

void updateAntennaConfig(HWND hDlg )
{
	UINT16			nAntenna, nReceiveSensitivityIndex, nTransmitPowerIndex,nHopTableIndex,nTransmitFrequencyIndex;
	FREQ_HOP_TABLE* pTable;
	UINT16			nIndex;
	UINT32*			pList1;
	UINT16*			pList2;
	UINT32			number = 0;
	TCHAR			szFrequencyList[MAX_PATH * 4] = {0};

	nAntenna = GetDlgItemInt(hDlg, IDC_CB_ANTENNAID, NULL, FALSE); 
	if(nAntenna)
	{
		if(g_appData.m_ReaderCaps.hoppingEnabled)
		{
			g_appData.m_RfidStatus = RFID_GetAntennaConfig(g_appData.m_RfidReaderHandle, nAntenna, &nReceiveSensitivityIndex, &nTransmitPowerIndex, &nHopTableIndex);
			if(g_appData.m_RfidStatus == RFID_API_SUCCESS)
			{
				pTable = g_appData.m_ReaderCaps.freqHopInfo.pFreqTables;
				pTable = pTable + (nHopTableIndex-1);
				SendMessage(GetDlgItem(hDlg, IDC_CB_HOP_TABLE), CB_SETCURSEL, nHopTableIndex-1, 0);
				for(nIndex = 0,pList1 =  pTable->pFreqList; nIndex < pTable->numFreq; nIndex++, pList1++)
				{   
					number+= wsprintf(&szFrequencyList[number], (LPCTSTR)TEXT("%u, "), *pList1);
				}
				if (pTable->numFreq > 0)
					szFrequencyList[number - 2] = TEXT('\0');

				SetDlgItemText(hDlg,IDC_TB_HOP_FREQUENCIES, szFrequencyList);
			}
		}
		else
		{
			g_appData.m_RfidStatus  = RFID_GetAntennaConfig(g_appData.m_RfidReaderHandle, nAntenna, &nReceiveSensitivityIndex, &nTransmitPowerIndex, &nTransmitFrequencyIndex);
			SendMessage(GetDlgItem(hDlg, IDC_TRM_FREQUENCY), CB_SETCURSEL, nTransmitFrequencyIndex, 0);
		}
		PostRFIDStatus(g_appData.m_RfidStatus, GENERIC_INTERFACE);

		// Update Receive Sensitivity Index
		SendMessage(GetDlgItem(hDlg, IDC_CB_RX_SENSITIVITY), CB_SETCURSEL, nReceiveSensitivityIndex, 0);

		// Update Transmit Power Index
		SendMessage(GetDlgItem(hDlg, IDC_CB_TX_POWER), CB_SETCURSEL, nTransmitPowerIndex, 0);

	}

}

INT_PTR CALLBACK AntennaConfigDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT32*					pList1;
	UINT16*					pList2;
	FREQ_HOP_TABLE*			pTable;
	HWND					hList=NULL;  // List View identifier
	UINT16					nIndex;
	TCHAR					szFrequencyList[MAX_PATH * 4] = {0};
	LVCOLUMN				lvColumn;
	UINT32					number = 0;
	static UINT16			nAntenna, nReceiveSensitivityIndex, nTransmitPowerIndex,nHopTableIndex,nTransmitFrequencyIndex;

	switch (message)
	{
	case WM_INITDIALOG:

		Createbutton(hDlg);
		Createmenu(hDlg);

		for(nIndex = 1; nIndex <= g_appData.m_ReaderCaps.numAntennas; nIndex++)
		{   
			wsprintf(szFrequencyList, (LPCTSTR)TEXT("%u"), nIndex);
			SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_ADDSTRING, 0,(LPARAM) szFrequencyList);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_SETCURSEL, 0, 0);

		for(nIndex = 0, pList1=(g_appData.m_ReaderCaps.receiveSensitivtyTable.pReceiveSensitivityValueList); 
			nIndex <(g_appData.m_ReaderCaps.receiveSensitivtyTable.numValues); nIndex++, pList1++)
		{ 
			wsprintf(szFrequencyList, (LPCTSTR)TEXT("%u"), *pList1);
			SendMessage(GetDlgItem(hDlg, IDC_CB_RX_SENSITIVITY), CB_ADDSTRING, 0,(LPARAM) szFrequencyList);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_RX_SENSITIVITY), CB_SETCURSEL, 0, 0);

		for(nIndex = 0,pList2=(g_appData.m_ReaderCaps.transmitPowerLevelTable.pPowerValueList); nIndex <(g_appData.m_ReaderCaps.transmitPowerLevelTable.numValues);pList2++,nIndex++)
		{
			wsprintf(szFrequencyList, (LPCTSTR)TEXT("%u"), *pList2);
			SendMessage(GetDlgItem(hDlg, IDC_CB_TX_POWER), CB_ADDSTRING, 0,(LPARAM) szFrequencyList);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CB_TX_POWER), CB_SETCURSEL, 0, 0);

		if(g_appData.m_ReaderCaps.hoppingEnabled)
		{   
			ShowWindow(GetDlgItem(hDlg, IDC_CB_HOP_TABLE),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_TB_HOP_FREQUENCIES),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_HOPINDEX_TEXT),SW_SHOW);

			for(nIndex=1,pTable =(g_appData.m_ReaderCaps.freqHopInfo.pFreqTables) ; nIndex <= (g_appData.m_ReaderCaps.freqHopInfo.numTables); nIndex++, pTable++)
			{
				wsprintf(szFrequencyList, (LPCTSTR)TEXT("%d"), g_appData.m_ReaderCaps.freqHopInfo.pFreqTables->hopTableID);
				SendMessage(GetDlgItem(hDlg, IDC_CB_HOP_TABLE), CB_ADDSTRING, 0,(LPARAM) szFrequencyList);
			}
			SendMessage(GetDlgItem(hDlg, IDC_CB_HOP_TABLE), CB_SETCURSEL, 0, 0);

			lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
			lvColumn.fmt = LVCFMT_LEFT;
			lvColumn.cx = 162;
			lvColumn.pszText =L"Frequencies";
			hList=GetDlgItem(hDlg,IDC_TB_HOP_FREQUENCIES); 				 
			SendMessage(hList,LVM_INSERTCOLUMN,0,(LPARAM)&lvColumn); 

		}
		else
		{ 
			ShowWindow(GetDlgItem(hDlg, IDC_TRM_FREQUENCY),SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_TRM_FREQUENCY_TEXT),SW_SHOW);

			for(nIndex = 0, pList1 =(g_appData.m_ReaderCaps.fixedFreqInfo.pFreqList); nIndex < (g_appData.m_ReaderCaps.fixedFreqInfo.numFreq); nIndex++, pList1++)
			{
				wsprintf(szFrequencyList, (LPCTSTR)TEXT("%u"), *pList1);
				SendMessage(GetDlgItem(hDlg, IDC_TRM_FREQUENCY), CB_ADDSTRING, 0,(LPARAM) szFrequencyList);
			}
			SendMessage(GetDlgItem(hDlg, IDC_TRM_FREQUENCY), CB_SETCURSEL, 0, 0);
		}

		updateAntennaConfig(hDlg);

		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if(HIWORD(wParam) == CBN_SELCHANGE) // what we press on?
		{
			if(LOWORD(wParam) == IDC_CB_ANTENNAID)
			{
				updateAntennaConfig(hDlg);
			}

			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDC_APPLY)
		{			
			nAntenna = SendMessage(GetDlgItem(hDlg, IDC_CB_ANTENNAID), CB_GETCURSEL, 0, 0);
			nAntenna += 1;

			nTransmitPowerIndex = SendMessage(GetDlgItem(hDlg, IDC_CB_TX_POWER), CB_GETCURSEL, 0, 0);
			nReceiveSensitivityIndex = SendMessage(GetDlgItem(hDlg, IDC_CB_RX_SENSITIVITY), CB_GETCURSEL, 0, 0);

			if(g_appData.m_ReaderCaps.hoppingEnabled)
			{
				nHopTableIndex = GetDlgItemInt(hDlg, IDC_CB_HOP_TABLE, NULL, FALSE); 

				g_appData.m_RfidStatus  = RFID_SetAntennaConfig(g_appData.m_RfidReaderHandle, nAntenna, nReceiveSensitivityIndex, nTransmitPowerIndex, nHopTableIndex);
			}
			else
			{
				nTransmitFrequencyIndex = SendMessage(GetDlgItem(hDlg, IDC_TRM_FREQUENCY), CB_GETCURSEL, 0, 0);
				g_appData.m_RfidStatus  = RFID_SetAntennaConfig(g_appData.m_RfidReaderHandle, nAntenna, nReceiveSensitivityIndex, nTransmitPowerIndex, nTransmitFrequencyIndex+1);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;

		}
		if(LOWORD(wParam) == IDOK)
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


void InsertItem(TCHAR* szConversion, UINT16 itemNumber)
{
	LVITEM lvItem = {0};

	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = itemNumber;
	lvItem.iSubItem = 1;
	lvItem.pszText = (LPWSTR)szConversion;
	SendMessage(hRFmodeParamList,LVM_SETITEM,0,(LPARAM)&lvItem); // Enter text to SubItems
}