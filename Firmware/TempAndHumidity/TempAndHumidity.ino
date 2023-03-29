#include <WiFi.h>
#include <HTTPClient.h>
#include  <Wire.h>
#include <Adafruit_AHTX0.h>

const char* ssid     = "Your_ssid";
const char* password = "Your_Password";


Adafruit_AHTX0 aht;
sensors_event_t aht20_humidity, aht20_temp;
const char* serverUrl = "http://34.217.96.154/sensordata";

// Pin definitions
const int relayPin = 13; // Relay pin number



boolean status = false;

void setup() {
  Serial.begin(115200);
  
  //Setting up relaypin to output humidifier (ON/OFF) status
  pinMode(relayPin, OUTPUT); 

  //Setting up wifi connection
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

 // Start AHT20 (temp + humidity) Sensor
  if (! aht.begin()) {
    Serial.println("AHT20 sensor not found :(");
    while (1) delay(10);
  }
 
  Serial.println("AHT20 started successfully"); 
    
}

//Declaring sensor data variable:
String postData;
String postVariable = "SensorData=";


void loop(){

  // Get AHT20 Data
  aht.getEvent(&aht20_humidity, &aht20_temp);
  
  // Read temperature and humidity from AHT20
  float local_temperature = aht20_temp.temperature;
  float local_humidity = aht20_humidity.relative_humidity;
  
  // AHT20 data
    Serial.print(F("Temperature: \t\t\t")); Serial.print(aht20_temp.temperature, 2); Serial.println(" C");
    Serial.print(F("Humidity: \t\t\t"));  Serial.print(aht20_humidity.relative_humidity); Serial.println(" %rh");
    
    String sensorData =" Humidity-" + (String)local_humidity + ";Temperature-" + (String)local_temperature;
    postData = postVariable + sensorData;

    Serial.print("postData: ");
    Serial.print(postData);

//To send sensor data
//Sending data via wifi connection:
  if (WiFi.status() == WL_CONNECTED) {
   // Create an HTTPClient object
   HTTPClient http;

   // Specify the URL and content type of the server
   http.begin(serverUrl);
   http.addHeader("Content-Type", "text/plain");
   //serializeJson(doc, postData);

   // Make the HTTP POST request
   int httpResponseCode = http.POST(postData);
   Serial.print("response code : ");
   Serial.println(httpResponseCode);
   String resp = http.getString();
   Serial.print("resp: ");
   Serial.println(resp);
   

   // Check for a successful request
   if (httpResponseCode == HTTP_CODE_OK) {
     Serial.println("Data sent successfully!");
   } else {
     
     Serial.print("Error code: ");
     Serial.println(httpResponseCode);
   }

       
   // Close the connection
   http.end();
   }

   if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://34.217.96.154/devicestatus");
    int httpCode = http.GET();

    if(httpCode == HTTP_CODE_OK){
      String payload = http.getString();
      Serial.println(payload);
      if(payload == "ON"){
        digitalWrite(relayPin, HIGH); // Turn on device connected to relay
        Serial.println("Humidifier ON");
      }
      else{
        digitalWrite(relayPin, LOW);
        Serial.println("Humidifier OFF");  
      }
    }
     else{
      Serial.println("HTTP ERRROR CODE");
      Serial.println("httpCode");
    }
    http.end();
  }
  
  delay(5000); // check every 5 seconds
// delay(10000); 

}
