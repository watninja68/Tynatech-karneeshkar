/* Heltec Automation LoRaWAN communication example
 *
 * Function:
 * 1. Upload node data to the server using the standard LoRaWAN protocol.
 *  
 * Description:
 * 1. Communicate using LoRaWAN protocol.
 * 
 * HelTec AutoMation, Chengdu, China
 * 成都惠利特自动化科技有限公司
 * www.heltec.org
 *
 * */
 bool temp =  false;

#include "LoRaWan_APP.h"
#define UART_BUFFER_SIZE 32  // Maximum size of the UART buffer
#define RXD2 32
#define TXD2 33

static uint8_t uartBuffer[UART_BUFFER_SIZE];
static uint8_t uartBufferIndex = 0;
/* OTAA para*/
uint8_t devEui[] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x53, 0xC8 , 0x33 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x74, 0xD6, 0x6E, 0x63, 0x45, 0x82, 0x48, 0x27, 0xFE, 0xC5, 0xB7, 0x70, 0xBA, 0x2B, 0x50, 0x45 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

DeviceClass_t  loraWanClass = CLASS_A;

uint32_t appTxDutyCycle = 15000;

bool overTheAirActivation = true;

bool loraWanAdr = true;

bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;
void stringToBytes(const String& str, uint8_t* bytes, size_t maxBytes) {
    size_t len = min(str.length() / 2, maxBytes);
    for (size_t i = 0; i < len; i++) {
        bytes[i] = strtoul(str.substring(i * 2, i * 2 + 2).c_str(), NULL, 16);
    }
}
/* Prepares the payload of the frame */
static void prepareTxFrame(uint8_t port) {
    if (uartBufferIndex > 0) {
        // Copy the UART data to the appData buffer
        memcpy(appData, uartBuffer, min(static_cast<size_t>(uartBufferIndex), static_cast<size_t>(LORAWAN_APP_DATA_MAX_SIZE)));
        appDataSize = static_cast<uint8_t>(min(uartBufferIndex, static_cast<uint8_t>(LORAWAN_APP_DATA_MAX_SIZE)));

        // Reset the UART buffer index
        uartBufferIndex = 0;
    } else {
        // No UART data available, send a default payload
        appDataSize = 4;
        appData[0] = 0x00;
        appData[1] = 0x01;
        appData[2] = 0x02;
        appData[3] = 0x03;
    }
}

//if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ 


void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  // Initialize Serial2 with the correct baud rate and pins

  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
}

void loop(){


    Serial2.println(1);
    while (Serial2.available() && uartBufferIndex < UART_BUFFER_SIZE) {
        uartBuffer[uartBufferIndex] = Serial2.read();
        Serial.print("Received byte: ");
        Serial.println(uartBuffer[uartBufferIndex], HEX);
        uartBufferIndex++;
        uartBuffer[uartBufferIndex] = '\0';
temp = true;
    }
      Serial2.println(0);

    if(temp){
    prepareTxFrame(appPort);

  // Copy UART data to LoRaWAN appData buffer
    memcpy(appData, uartBuffer, uartBufferIndex);
    appDataSize = uartBufferIndex;

  // Reset UART buffer index
    uartBufferIndex = 0;

  // Send LoRaWAN frame to gateway
    LoRaWAN.send();
    temp = false;}


  switch( deviceState )
  {
    case DEVICE_STATE_INIT:
    {
#if(LORAWAN_DEVEUI_AUTO)
      LoRaWAN.generateDeveuiByChipID();
#endif
      LoRaWAN.init(loraWanClass,loraWanRegion);
      //both set join DR and DR when ADR off 
      LoRaWAN.setDefaultDR(3);
      break;
    }
    case DEVICE_STATE_JOIN:
    {
      LoRaWAN.join();
      break;
    }
    case DEVICE_STATE_SEND:
    {
      prepareTxFrame( appPort );
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    }
    case DEVICE_STATE_CYCLE:
    {
      // Schedule next packet transmission
      txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
      LoRaWAN.cycle(txDutyCycleTime);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    }
    case DEVICE_STATE_SLEEP:
    {
      LoRaWAN.sleep(loraWanClass);
      break;
    }
    default:
    {
      deviceState = DEVICE_STATE_INIT;
      break;
    }
  }
}