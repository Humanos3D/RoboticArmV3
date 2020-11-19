/*
 Wireless Robotic Arm - ESP8266 EMG Sender (Transceiver)
 By: Mauricio Cano Bedoya
 For: Humanos3D/e-NABLE Medellín
 Completed ~August 2020
 */

 // Inclusion of libraries and definitions
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "EMGFilters.h"
#define SensorInputPin 2 
#define CALIBRATE 0
EMGFilters myFilter;

// Parameters
int baseline = 1000;                                          // Any signal value below this value will be a zero
SAMPLE_FREQUENCY sampleRate = SAMPLE_FREQ_500HZ;              // Try `SAMPLE_FREQ_500HZ` or `SAMPLE_FREQ_1000HZ`.
const int SIGNAL_THRESHOLD = 3.0;                             // Change this parameter depending on how much strength
                                                              // is going to be applied to activate the arm
unsigned long long interval = 1000000ul / sampleRate;
typedef struct struct_message{                                // Message structure to send data
    float ToSend;
} struct_message;
struct_message myData;

NOTCH_FREQUENCY humFreq = NOTCH_FREQ_60HZ;                    // In places with 60Hz power line: "NOTCH_FREQ_60HZ"
                                                              // In places with 50Hz power line: "NOTCH_FREQ_50HZ"
uint8_t broadcastAddress[] = {0xCC,0x50,0xE3,0x96,0x1D,0xF4}; // Type the MAC Adress of the device you're 
                                                              // going to couple with, Ex: CC-50-E3-96-1D-F4

// Functions

// Preliminary Setup
void setup() 
{
    myFilter.init(sampleRate, humFreq, true, true, true);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    if(esp_now_init()!= 0)
    {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    //esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE,1,NULL,0);
}

// Confirmation of sending to the ESP32, use it while calibrating 

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.println("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery failed");
  }
}
// Infinite Loop
void loop() {
    unsigned long long timeStamp = micros();
    int data = analogRead(SensorInputPin);

    // Filters and signal conditioning
    int dataAfterFilter = myFilter.update(data);
    int envelope = sq(dataAfterFilter);
    envelope = envelope * (3.3 / 1023.0);

    if (CALIBRATE)
    {
        // Remember to also enable the OnDataSent function above
        Serial.print("Squared Data: ");
        //Serial.println(envelope);
    }
    else
    {
        if (envelope < baseline) {                 // Any value below the "baseline" value will be treated as zero
            dataAfterFilter = 0;
            envelope = 0;
        }
        envelope = envelope/1000;
        Serial.println(envelope);
        if (envelope > SIGNAL_THRESHOLD)          // When a contraction is above the threshold, a confirmation will
                                                  // be sent to the ESP32
        {
          myData.ToSend = envelope;
          Serial.println("Sent");
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        }
    }
    unsigned long long timeElapsed = micros() - timeStamp;  // Time elapsed measurement
    
}
