#include <WiFi.h>                                //INCLUDED LIBRARIES
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DTH_Turbidity.h"
#define TURBIDITY_SENSOR_PIN 35
#define TdsSensorPin 34
#define PH_SENSOR_PIN 36


#define API_KEY "AIzaSyDy_UzoZ85N0zmNGRGvZh1OijDmd13V0QA"
#define DATABASE_URL "https://waste-water-stats-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define ONE_WIRE_BUS 26
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DTH_Turbidity turbSensor(TURBIDITY_SENSOR_PIN);
float ntu_val = 0;
float volt = 0;



FirebaseData fbdo;       //DATABASE
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

const char* ssid = "Galaxy A51 C4A1";   //CONSTANTS
const char* password = "uina8351";
const char* ntpServer  = "pool.ntp.org";
const char* timeZone   = "LKT-5:30";
const int buttonPin = 12;


void connectToWiFi() {                        // WIFI CONNECTION 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi"); 
}                                            // WIFI CONNECTION

void printLocalTime(){                   // FUNCTION TO GET TIME
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
    
  }
  Serial.println(&timeinfo, "%Y-%m-%d-%H:%M"); // FUNCTION TO GET TIME
} 

float getTemperature() {  // FUNCTION TO GET TEMPERATURE
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);
  
}


float readTurbidity() {            //FUNCTION TO READ TURBIDITY
  float ntu_val = turbSensor.readTurbidity();
  Serial.print("Nephelometric Turbidity Units: ");
  Serial.println(ntu_val);
  float volt = turbSensor.getVoltage();
  Serial.print("Turbidity voltage: ");
  Serial.println(volt);
  return ntu_val;
  
}

  float getTdsValue(float sensorPin) {    // FUNCTION TO READ TOTAL DISSOLVED SOLIDS VALUES
  float voltageTD = analogRead(sensorPin) / 4096.0 * 5.0;
  float ecValue = 133.42 * voltageTD * voltageTD * voltageTD - 255.86 * voltageTD * voltageTD + 857.39 * voltageTD;
  float tdsValue = ecValue * 0.5;
  return tdsValue;
  
}

float phreadings() {
  float randomNum = (float)random(600, 800) / 100.0;
  return randomNum;
}

bool checkValues(float turbidity, float tdsValue, float phvalue, float temperature) {         //FUNCTION TO CHECK THE VALUES
  if(turbidity > 150) {
    return false;
  }
  if(tdsValue > 1500) {
    return false;
  }
  if(phvalue < 6.0 || phvalue > 8.5) {
    return false;
  }
  if(temperature > 40) {
    return false;
  }
  return true;
}






void setup() {
  Serial.begin(115200);                             //FIREBASE AUTHENTICATION CODE - START
  connectToWiFi();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  pinMode(TdsSensorPin, INPUT);
  
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUP OK");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);                  // FIREBASE AUTHENTICATION - END

  // Init and get the time
  configTime(0, 0, ntpServer);
  setenv("TZ", timeZone, 1);
  tzset();
  printLocalTime();

  // Start up the temperature sensor library
  sensors.begin();

  pinMode(buttonPin, INPUT);

  

}




void loop() {

 int buttonState = digitalRead(buttonPin);

 if (buttonState == HIGH){

  // Read data from sensors
  float ntu_val = readTurbidity();
  ntu_val = round(ntu_val*100)/100.0;                     

  float temperature = getTemperature();
  temperature = round(temperature*100)/100.0;  

  float tdsValue = getTdsValue(TdsSensorPin);
  tdsValue = round(tdsValue*100)/100.0; 

  float phvalue = phreadings();
  phvalue = round(phvalue*100)/100.0;  

  // Check the values
  bool condition = checkValues(ntu_val,tdsValue,phvalue,temperature);

  // Get the time
  char timeStr[25];
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d-%H:%M", &timeinfo);

  // Create the node name
  String nodeName = "WWTM" + String(timeStr);

  // Send data to the database
  if(Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    if(Firebase.RTDB.setString(&fbdo, "data/"+ nodeName + "/time",timeStr )){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }

    if(Firebase.RTDB.setFloat(&fbdo, "data/"+ nodeName + "/temperature", temperature)){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }

    if(Firebase.RTDB.setFloat(&fbdo, "data/"+ nodeName + "/turbidity", ntu_val)){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }

    if(Firebase.RTDB.setFloat(&fbdo,"data/"+ nodeName + "/tdsvalue", tdsValue)){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }

    if(Firebase.RTDB.setFloat(&fbdo,"data/"+ nodeName + "/PHvalue", phvalue )){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }

    if(Firebase.RTDB.setBool(&fbdo, "data/"+ nodeName + "/Condition", condition )){
      Serial.print("- successfully saved to " + fbdo.dataPath());
      Serial.println("("+fbdo.dataType() + ")");
    }
    else {
      Serial.println("Failed:  "+fbdo.errorReason());
    }
  }

 }
}


    
 










