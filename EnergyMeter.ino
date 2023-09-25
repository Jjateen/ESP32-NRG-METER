#include "EmonLib.h"
#include <EEPROM.h>
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
 
EnergyMonitor emon;
#define vCalibration 83.3
const int sensorIn = 34; // Pin where the OUT pin from the sensor is connected on the ESP
int mVperAmp = 185;      // Use 100 for 20A Module and 66 for 30A Module
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

float getCurrent() {
  Voltage = getVPP();
  VRMS = (Voltage / 2.0) * 0.707; // Root 2 is 0.707
  AmpsRMS = ((VRMS * 1000) / mVperAmp);
//  if (AmpsRMS<0.16)
//  {
//    AmpsRMS=0;
//  }
  Serial.print(AmpsRMS);
  Serial.print(" Amps RMS  ---  ");
  Watt = (AmpsRMS * 240 / 1.2);
  // Note: 1.2 is my own empirically established calibration factor
  // as the voltage measured at D34 depends on the length of the OUT-to-D34 wire
  // 240 is the main AC power voltage – this parameter changes locally
  Serial.print(Watt);
  Serial.println(" Watts");

//  delay(1000); // Delay for 1 second between readings
  return AmpsRMS;
}

// ** Function to get VPP **
float getVPP() {
  float result;
  int readValue; // Value read from the sensor
  int maxValue = 0; // Store max value here
  int minValue = 4096; // Store min value here (ESP32 ADC resolution)

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) // Sample for 1 second
  {
    readValue = analogRead(sensorIn);
    // See if you have a new maxValue
    if (readValue > maxValue) {
      /* Record the maximum sensor value */
      maxValue = readValue;
    }
    if (readValue < minValue) {
      /* Record the minimum sensor value */
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 3.3) / 4096.0; // ESP32 ADC resolution 4096

  return result;
}


BlynkTimer timer;
char auth[] = "JBYXT3D71PrvkQ5KwpMWb6Syadmha7RO";
char ssid[] = "jjj";
char pass[] = "12345678";
 
float kWh = 0;
unsigned long lastmillis = millis();
 
void myTimerEvent()
{
  float irms = getCurrent();
  emon.calcVI(20, 2000);
  float power = (AmpsRMS * 240 / 1.2);
  kWh = kWh + power * (millis() - lastmillis) / 3600000000.0;
  
  lastmillis = millis();
 
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, irms);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, kWh);
}
 
void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  emon.voltage(35, vCalibration, 0);  // Voltage: input pin, calibration, phase_shift
//  emon.current(34, currCalibration);    // Current: input pin, calibration.
  timer.setInterval(5000L, myTimerEvent);
  Serial.print("IoT Energy");
  Serial.println("Meter");
  delay(3000);
}
 
void loop()
{

  Serial.print(emon.Vrms); Serial.print("     "); Serial.print(getVPP()); Serial.print("     "); Serial.println(getCurrent());
  Blynk.run();
  timer.run();
}
