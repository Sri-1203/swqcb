#include <WiFi.h>
#include <WiFiClient.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyC1TqDa-subE34cSXfmKt_FS46RjvitvHM"
#define DATABASE_URL "https://water-monitoring-bracelet-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define WIFI_SSID "wifi-name"
#define WIFI_PWD "wifi-password"

const long updateInterval = 10 * 1000;
WiFiClient client;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int ldrData = 0;


#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point
#define r1 10000    
#define THERMISTORPIN 35
#define TURBIDITY_PIN 32 

float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float vs=3.3;
double Beta = 3950.0;
double To = 298.15; 
float averageVoltage = 0;
float tds = 0;
unsigned long lastUpdate = 0;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println();
  Serial.print("started");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp( &config, &auth, "", "")) {
      Serial.println("SIgnup OK");
      signupOK = true;
    }
  else{
    Serial.println("ERROR OCCUREDDDD!!!!");
  }
    //Serial.println("%s\n", config.signer.signupError.message.c_str());

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin( &config, &auth);
  Firebase.reconnectWiFi(true);

}

void loop(){
    float Vout;
  float T,rt,Tc,Tf;
  Vout = analogRead(THERMISTORPIN);
  Vout = Vout * vs /4095;
  rt=r1*Vout /(vs-Vout);
  T = 1/(1/To + log(rt/r1)/Beta);    // Temperature in Kelvin
  Tc = T - 273.15;                   // Celsius
  Tf = Tc * 9 / 5 + 32;              // Fahrenheit
  Serial.println(Tc);
  float ntu;
  int sensorValue = analogRead(TURBIDITY_PIN); 
  float voltage =(sensorValue * (3.3 / 4096));
  ntu = (-1120.4*voltage*voltage)+5742.3*voltage-4353.8; 
  if (ntu<0){
    ntu=0;
  }
  Serial.print("Turbidity Value: ");
  Serial.println(ntu);

  tds = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
  averageVoltage = tds * (float)VREF / 4096.0;
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
  float compensationCoefficient = 1.0+0.02*(Tc-25.0);
      //temperature compensation
  float compensationVoltage=averageVoltage/compensationCoefficient;
      //convert voltage value to tds value
  tds=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
  float wqi=15.48+(0.1648*Tc)-(0.0018*tds);
  int y = wqi;
  Serial.print("TDS Value:");
      Serial.print(tds);
      Serial.println("ppm");

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setFloat( & fbdo, "Sensor/tds", tds)) {
      Serial.println();
      Serial.print("TDS Value:");
      Serial.print(tds);
      Serial.println("ppm");
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat( & fbdo, "Sensor/ntu", ntu)) {
      Serial.println();
      Serial.print("Turbidity Value: ");
      Serial.print(ntu);
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat( & fbdo, "Sensor/Tc", Tc)) {
      Serial.println();
      Serial.print("Turbidity Value: ");
      Serial.print(Tc);
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat( & fbdo, "Sensor/wqi", y)) {
      Serial.println();
      Serial.print("Water Value: ");
      Serial.print(y);
    } else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }


  delay(1000);
}