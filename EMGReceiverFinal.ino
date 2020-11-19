/*
 Wireless Robotic Arm - ESP32 (Actuator)
 By: Mauricio Cano Bedoya
 For: Humanos3D/e-NABLE Medellín
 Completed ~August 2020
 */

 // Inclusion of libraries and definitions
#include <esp_now.h>
#include <WiFi.h>
#include <Servo.h>
#define LED_BUILTIN 2
#define ServoPin 14
#define High_s 1
#define Low_s 0
Servo servo; 

// Parameters
typedef struct struct_message{                                  // Message structure to recieve data
    float ToSend;
} struct_message;
struct_message myData;
uint8_t broadcastAddress[] = {0xCC,0x50,0xE3,0x96,0x1D,0xF4};   // Type the MAC Adress of the device you're 
                                                                // going to couple with, Ex: CC-50-E3-96-1D-F4
float state = 0;
int count = 0;
int pos = 1;
unsigned long time_now = 0;
unsigned long time_high = 0;
unsigned long time_lapse = 0;
const int SIGNAL_THRESHOLD = 3;                                 // Change this parameter depending on how much strength
                                                                // is going to be applied to activate the arm
const int WINDOW_LOW = 100;                                     // Signal window lower limit
const int WINDOW_UP = 400;                                      // Signal window upper limit
const int OPEN_HAND = 165;                                      // If using another servo: servo.write(0)
const int CLOSED_HAND = 20;                                     // If using another servo: servo.write(180)
const int GRIP_DELAY = 300;                                     // Delay while the hand is in miliseconds
const int TIMEOUT = 1000;                                       // Time after wich the servos will stop trying to turn
// Functions

// Signal reception from ESP8266
void OnDataRecv( const uint8_t * mac_addr, const uint8_t * incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  state = myData.ToSend;
  Serial.println("Received");
}

// Preliminary Setup
void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(LED_BUILTIN, OUTPUT);
  servo.attach(ServoPin);
  if(esp_now_init()!= ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }else Serial.println("ESP-NOW Ok");

  esp_now_register_recv_cb(OnDataRecv);
}

// Infinite Loop
void loop() {
  time_now = millis();
  if (state >= SIGNAL_THRESHOLD)                               // Confirmation of a recieved signal with a value
                                                               // above the established threshold
  {
    time_high = time_now;
    state = Low_s;
    count = High_s;
  }
  time_lapse = time_now - time_high;                          // Signal window from the last peak of contraction
  //Serial.println (time_lapse);
  if ((time_lapse > WINDOW_LOW && time_lapse < WINDOW_UP) && count == High_s)
  {
    Serial.println("Move motor:");
    if (pos)                                                            // "pos = High_s" the hand will open
    {
      Serial.println("Open Hand");
      digitalWrite(LED_BUILTIN,HIGH);                                   // Confirmation light
      servo.attach(ServoPin);
      servo.write(OPEN_HAND); 
      delay(GRIP_DELAY);
      pos = Low_s;
    }
    else                                                                // "pos = High_s" the hand will close
    {
      Serial.println("Close Hand");
      digitalWrite(LED_BUILTIN,HIGH);                                   // Confirmation light
      servo.attach(ServoPin);
      servo.write(CLOSED_HAND); 
      delay(GRIP_DELAY);
      pos = High_s;
    }
    count = Low_s;                                                      // Deactivation of the servo
    delay(TIMEOUT);
    digitalWrite(LED_BUILTIN,LOW);
    servo.detach();
  }
}
