/* 
 Datalogger para Robotic Arm v2
 By: Mauricio Cano Bedoya
 For: e-NABLE Medellín
 Completed ~February 2020
 */

// Libraries inclusion 
#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>


// Variables
File myFile;
const int chipSelect = 10; // For Data Logger Shield 
float value = 0;
int count = 0;
String time ;
String values; 
tmElements_t tm;
int Compare = 0;

// Setup
void setup()
{
  Serial.begin(38400);
  while (!Serial) ;
  delay(200);
  Serial.println("Data logger Robotic Arm v2");
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) 
  {
    Serial.println("SD Card initialization failed!");
    return;  
  }
  Serial.println("SD Card OK.");
 // ReadText();
}

void loop()
{
  value = analogRead(A0);
  int valor = (value*500)/1023;
  values = valor;
  values = values + "," + Now();
  WriteValues(values);
  WriteText(String(analogRead(A0))+","+ Now());
  if ( value >600)
  {
    count++;
    time = Now()+ " Numero de accionamientos: "+ count;
    Serial.println(time);
    //WriteText(time);
  }

  if (Compare != tm.Minute)
  {
    Serial.println(Compare);
    Compare = tm.Minute;
  }
  else
  {
    Serial.println("SDSD");
    WriteText(String(analogRead(A0))+","+ Now());
  }
}

// Functions
void ReadText(){
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void WriteText(String txt){
  myFile = SD.open("registry.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(txt);
    myFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening registry.txt");
  }
}

void WriteValues(String txt){
  myFile = SD.open("plot.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(txt);
    myFile.close();
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening plot.txt");
  }
}

String Now(){
  String time = "";
  if (RTC.read(tm)) {
    time += tm.Hour;
    time += ":";
    time += tm.Minute;
    time += ":";
    time += tm.Second;
    time += ",";
    time += tm.Day;
    time += "/";
    time += tm.Month;
    time += "/";
    time += tmYearToCalendar(tm.Year);
  } 
  else {
    time = "NO";
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } 
    else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }
  return time;
}
