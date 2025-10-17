/*
This program is written for an ESP 32 which can be installed with ease into any electricity meter to measure the number of flashes made and calculate the usage over time.
The data is stored in flashes.log on the local IP and can then be further accessed.
From the file stored on the file, which includes the length of each impulse it is very easy to track any garbage readings since all of the flashes from the meter are more or less the same time duration with very small margin of error.
You'll notice that the loop is empty and that everything is done through the ReadADC ticker. this was just a convenient choice for CPU usage optimization.
*/


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <time.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>

//insert WiFi and password below
const char* ssid = "";
const char* password = "";

long LastFlash = millis();
int Pin = 4; 
int A;
int lastA;
int StartTime;
int OnTime;
int ThisTime;
int LastTime;
int Watt;
int State;
AsyncWebServer server(80);
Ticker ReadADC;



void CheckADC()
{
  A = analogRead(A0);
  
  if (A > 150)
  {
    if ( State == 0)
    {
      State = 1;
      ThisTime = millis() - LastTime;
      OnTime = millis() - StartTime;
      Serial.print(millis());
      Serial.print(" ");
      Serial.print(OnTime);
      Serial.print(" ");
      Serial.print(ThisTime);
      Serial.print(" ");
      Serial.print(A);
      Watt = 3600000 / (ThisTime);
      Serial.print(" Watt:");
      Serial.println(Watt);
      LastTime = StartTime;
      StartTime = 0;

    File file = LittleFS.open("/flashes.log", "a");
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }

    file.print(millis());
    file.print("\r\n");
    file.close();

    }
  }
  else
  {
    if (State == 1)
    {
        StartTime = millis();
        State = 0;
    }
  }
}


void setup() {
  Serial.begin(19200);
  Serial.println("Flash meter");
  State = 1;
  WiFi.begin(ssid, password); 
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { 
    delay(100);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.println("Connected.");
  

  
///////////////////////////////////////////////////////////////////////////////////////////////////////
  configTime(3600 * 2, 0, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  struct tm tmstruct;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct.tm_year) + 1900, (tmstruct.tm_mon) + 1, tmstruct.tm_mday, tmstruct.tm_hour, tmstruct.tm_min, tmstruct.tm_sec);
////////////////////////////////////////////////////////////////////////////////////////////////////////
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });
  server.on("/flashes.log", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/flashes.log", "text/html"); });
  server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request)
            { LittleFS.remove("/flashes.log");
              request->send(200, "Done", "Cleared. <a href=/flashes.log>Continue</a>"); });
/////////////////////////////////////////////////////////////////////////////////////////////////////////

  
  server.begin();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    LittleFS.end();
    LittleFS.format();
  }

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed again");
  }
  ReadADC.attach_ms(5, CheckADC);
}

void loop() {
  delay(1);

}
