//Include the library
#include <WiFi.h>
#include <HTTPClient.h>
#include <MQUnifiedsensor.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP-32") // Wemos ESP-32 or other board, whatever have ESP32 core.

//https://www.amazon.com/HiLetgo-ESP-WROOM-32-Development-Microcontroller-Integrated/dp/B0718T232Z (Although Amazon shows ESP-WROOM-32 ESP32 ESP-32S, the board is the ESP-WROOM-32D)
#define         Pin                     (36) //check the esp32-wroom-32d.jpg image on ESP32 folder 

/***********************Software Related Macros************************************/
#define         Type                    ("MQ-2") //MQ2 or other MQ Sensor, if change this verify your a and b values.
#define         Voltage_Resolution      (3.3) // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         ADC_Bit_Resolution      (12) // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define         RatioMQ2CleanAir        (9.83) //RS / R0 = 9.83 ppm
/*****************************Globals***********************************************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/*****************************Globals***********************************************/

const char* ssid     = "Your_ssid";
const char* password = "Your_Password";

    
  const char* serverUrl = "http://34.217.96.154/sensordata";

  String postData;
  String postVariable = "SensorData=";
  WiFiClient client;

// Your threshold value
  int sensorThres = 500;

//buzzer:
int buzzer = 32;
int smokeA0 = 39;

void setup()
{

  //Init the serial port communication - to debug the library
  Serial.begin(115200); //Init serial port
  delay(10);

  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
   delay(1000);
   Serial.println("Connecting to WiFi...");
 }
  pinMode(buzzer, OUTPUT);
  pinMode(smokeA0, INPUT);

  //Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(987.99); MQ2.setB(-2.162); // Configure the equation to to calculate H2 concentration
  
/*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ2.init(); 
 
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ2.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
   // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recomend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  //MQ2.serialDebug(true); uncomment if you want to print the table on the serial port

}

void loop()
{
  MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ2.serialDebug(); // Will print the table on the serial port
  MQ2.readSensor(); 
  int analogSensor = analogRead(smokeA0);
  Serial.print("Pin A0: ");
  Serial.print(analogSensor); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.println(" PPM");
  // Checks if it has reached the threshold value
  if (analogSensor > sensorThres)
  {
    //Serial.print("came here H: ");  
    digitalWrite(buzzer,HIGH);
  }
  else
  {
    //Serial.print("came here L: ");
    digitalWrite(buzzer,LOW);
  }
  Serial.println("** Values from MQ-2 ****");
  Serial.println("|  CO  |  Alcohol  |  H2  |  LPG  |  Propane  |");  

  MQ2.setA(36974); MQ2.setB(-3.109); // Configure the equation to calculate CO concentration value
  float CO = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(3616.1); MQ2.setB(-2.675); //Configure the equation to calculate Alcohol concentration value
  float Alcohol = MQ2.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(987.99); MQ2.setB(-2.162); // Configure the equation to calculate CO2 concentration value
  float H2 = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(574.25); MQ2.setB(-2.222); // Configure the equation to calculate Toluen concentration value
  float LPG = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ2.setA(658.71); MQ2.setB(-2.168); // Configure the equation to calculate Toluen concentration value
  float Propane = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup


  Serial.print("|   "); Serial.print(CO); 
  Serial.print("   |   "); Serial.print(Alcohol);
  Serial.print("   |   "); Serial.print(H2); 
  Serial.print("   |   "); Serial.print(LPG); 
  Serial.print("   |   "); Serial.print(Propane);
  Serial.println("   |"); 

  String gasData = " SmokeData-" + (String)analogSensor + ";CO-" + (String)CO + ";Alcohol-" + (String)Alcohol + ";H2-" + (String)H2 + ";LPG-" + (String)LPG + ";Propane-" + (String)Propane;

  postData = postVariable + gasData;

  if (WiFi.status() == WL_CONNECTED) {
  // Create an HTTPClient object
  HTTPClient http;
 
  // Specify the URL and content type of the server
  http.begin(serverUrl);
  http.addHeader("Content-Type", "text/plain");
 
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
  
  Serial.println(postData);
  
  http.end();
   }

  delay(5000); //Sampling frequency
}
