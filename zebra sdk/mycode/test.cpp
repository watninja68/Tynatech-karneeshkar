#include <iostream>
#include "BasicRFID.h"

void printTagData(TAG_DATA* pTagData) {
    char tagBuffer[1024] = { 0 };
    char* pTagReportData = tagBuffer;
    int index = 0;

    for (index = 0; index < pTagData->tagIDLength; index++) {
        if (index > 0 && index % 2 == 0) {
            *pTagReportData++ = '-';
        }
        sprintf(pTagReportData, "%02X", pTagData->pTagID[index]);
        while (*pTagReportData) pTagReportData++;
    }

    std::cout << tagBuffer << std::endl;
}

int main() {
    RFID_HANDLE32 readerHandle;
    CONNECTION_INFO connectionInfo;
    connectionInfo.version = RFID_API3_5_1;

    // Connect to the reader
    if (RFID_API_SUCCESS == RFID_Connect(&readerHandle, TEXT("192.168.1.100"), 0, 0, &connectionInfo)) {
        std::cout << "Connected to the reader." << std::endl;

        // Start continuous inventory scanning
        if (RFID_API_SUCCESS == RFID_PerformInventory(readerHandle, NULL, NULL, NULL, NULL)) {
            std::cout << "Continuous inventory started." << std::endl;

            TAG_DATA* pTagData = RFID_AllocateTag(readerHandle);

            // Loop to get tag data
            while (RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData)) {
                printTagData(pTagData);
            }

            RFID_DeallocateTag(readerHandle, pTagData);

            // Stop continuous inventory scanning
            RFID_StopInventory(readerHandle);
            std::cout << "Continuous inventory stopped." << std::endl;
        } else {
            std::cout << "Failed to start continuous inventory." << std::endl;
        }

        // Disconnect from the reader
        RFID_Disconnect(readerHandle);
        std::cout << "Disconnected from the reader." << std::endl;
    } else {
        std::cout << "Failed to connect to the reader." << std::endl;
    }

    return 0;
}