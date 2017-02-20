/*

  Blood Pressure Monitor | Version 1.1 | January 9, 2017

  Blood pressure is taken automatically every minute and data is sent to a web server. Every operation lasts about 40-50 seconds until the device turns off.
  After a minute, the device turns on again and takes another blood pressure test.

  Details:
  Board - Adafruit Feather M0 WiFi
  Device - Omron HEM-6121 Automatic Wrist Blood Pressure Monitor

  Functions:
  WiFi Enabled
  Display via web browser

  Changes:
  Device makes a GET request before proceeding to activating the blood pressure monitor. If it receives a go signal, it will turn
  the bp monitor on and proceed to the old process of taking blood pressure every minute.
  The POST request is expecting a response from the server whether to turn the device off or to continue running.

*/


#include <Wire.h>
#include <Adafruit_WINC1500.h>
#include "network.h"

/*Set to true when in test mode*/
bool TESTMODE = false;

int devID = 101;
int RELAY = A1;
int main_indicator = 11;
int bpm_indicator = 12;
int EEPROM_ADDR = 0x54;
int nth_entry = 1;
int nth_entry_ADDR = 0x0C;
int systolic = 0;
int diastolic = 0;
int hr = 0;
int actv_startTime, actv_dur, inactv_startTime;
int inactv_dur = 0;
int actv_max = 45000;
int inactv_max = 60000;
int stat = 0;
int main_stat = 0;
String post_add = "incoming/";
int onFlag = 0;
int offFlag = 0;


unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long gettingInterval = 1L * 1000L; // delay between updates, in milliseconds


void setup()
{

#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif

  pinMode(bpm_indicator, OUTPUT);
  pinMode(main_indicator, OUTPUT);
  digitalWrite(bpm_indicator, LOW);
  digitalWrite(main_indicator, LOW);
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  delay(2000);

  if (TESTMODE) {
    Serial.println("Test mode on");
    actv_max = 5000;
    inactv_max = 5000;
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop()
{
  while (client.available()) {
    char c = client.read();
    //Serial.write(c);
    if (String(c) == "$") {
      Serial.println("");
      Serial.write(c);
      Serial.println("");
      if (offFlag == 0) {
        main_stat = 0;
        onFlag = 0;
        digitalWrite(main_indicator, main_stat);
        if (stat) {
          power_PUSH();
        }
      }
      offFlag++;
    } else if (String(c) == "%") {
      Serial.println("");
      Serial.write(c);
      Serial.println("");
      if (onFlag  == 0) {
        main_stat = 1;
        offFlag = 0;
        digitalWrite(main_indicator, main_stat);
        actv_startTime = millis();
      }
      onFlag++;
    }
  }


  if (stat) {
    actv_dur = (millis() - actv_startTime);
    if (actv_dur > actv_max && main_stat) {
      if (TESTMODE) {
        /* Creating dummy data. For testing purposes where the blood pressure monitor is not connected */
        diastolic = rand() % 56 + 55;
        systolic = rand() % 91 + 90;
        Serial.println("Dummy data created");
      } else {
        getMeasurements(EEPROM_ADDR);
      }
      httpRequestPOST(String(devID), String(systolic), String(diastolic), post_add);
      delay(500);
      power_PUSH();
      inactv_startTime = millis();
    }
  } else {
    inactv_dur = (millis() - inactv_startTime);
    if (inactv_dur > inactv_max && main_stat) {
      power_PUSH();
      actv_startTime = millis();
    }
  }

  //  Serial.print("\t");
  //  Serial.print(actv_dur);
  //  Serial.print("\t");
  //  Serial.println(inactv_dur);

  if ((millis() - lastConnectionTime > gettingInterval) && !main_stat) {
    Serial.println("stat: " + String(stat) + "\n");
    httpRequestGET("/powerRequest/" + String(devID), 3000);
    lastConnectionTime = millis();
  }
}

void power_PUSH() {
  digitalWrite(RELAY, HIGH);
  delay(250);
  digitalWrite(RELAY, LOW);
  delay(250);
  stat = !stat;
  digitalWrite(bpm_indicator, stat);
}


void getMeasurements(int e_addr) {

  int addr;

  Wire.beginTransmission(e_addr);
  Wire.write(nth_entry_ADDR);
  Wire.endTransmission();
  Wire.requestFrom(e_addr, 1);
  nth_entry = Wire.read();
  Serial.println("last entry: " + String(nth_entry));

  if (nth_entry < 25) {
    addr = (10 * nth_entry) + 14;
  } else {
    addr = (10 * (nth_entry - 25)) + 8;
    e_addr += 1;
  }

  Wire.beginTransmission(e_addr);
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(e_addr, 3);
  Serial.println("Reading...");

  int av = Wire.available();

  systolic = Wire.read();
  systolic += 25;
  diastolic = Wire.read();
  hr = Wire.read();

  Serial.print("Systolic Pressure: ");
  Serial.println(systolic);
  Serial.print("Diastolic Pressure: ");
  Serial.println(diastolic);
  Serial.print("Pulse Rate: ");
  Serial.println(hr);
}



