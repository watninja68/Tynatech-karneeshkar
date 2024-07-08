/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL3C_R9xvtF"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "cqssBdnBRKv-j_v5lfrOm32skrzpFAxl"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <Arduino.h>

// Define the combined RFID tag and TID data structure
struct RFIDTag {
    uint32_t killPassword;
    uint32_t accessPassword;
    uint64_t tid; // Combined TID
    uint16_t pc;
    uint8_t epc[12]; // 96-bit EPC Code
    int amm;
};

RFIDTag tag;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Tynatech Jio";
char pass[] = "Tynatech@w62";

BlynkTimer timer;

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  char tem[100];
  int temps = (int) tag.accessPassword;
  sprintf(tem, "%d", temps);
  int acessKey = 122344;
  // You can send any value at any time.
  // Please don't send more than 10 values per second.
  Blynk.virtualWrite(V2, acessKey);
  delay(100);

  Blynk.virtualWrite(V4, tag.amm);

}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Initialize the combined RFID tag and TID data
  tag.killPassword = 0x12345678; // Example Kill Password value
  tag.accessPassword = 0x87654551; // Example Access Password value
  tag.tid = (uint64_t)0b1110000000001011 << 48 | (uint64_t)0b0000000000000010 << 32 | 12345678;
  tag.pc = 0xABCD; // Example PC value
  tag.amm = 1221;
  // Example EPC value (all zeros for demonstration)
  memset(tag.epc, 0, sizeof(tag.epc));

  // Print the contents of each memory bank
  Serial.println("Memory Organization:");
  Serial.print("Kill Password: 0x");
  Serial.println(tag.killPassword, HEX);
  Serial.print("Access Password: 0x");
  Serial.println(tag.accessPassword, HEX);
  Serial.print("TID: 0x");
  Serial.println(tag.tid, HEX);
  Serial.print("PC (Protocol Control): 0x");
  Serial.println(tag.pc, HEX);
  Serial.println("EPC (Electronic Product Code):");
  Serial.print("  ");
  for (int i = 0; i < sizeof(tag.epc); i++) {
    Serial.print(tag.epc[i], HEX);
    if ((i + 1) % 2 == 0) {
      Serial.print(" ");
    }
  }
  Serial.println();

  // Start the timer for sending data to Blynk
  timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
