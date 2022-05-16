
// This code is for the First Set and Second Set of Solar Panels giving voltage, current and power output.
// This is a combination of Electric_Monitor3_v2 and Electric_Monitor4 with code for two ADS1115s and one ESP32.
// Each ADS1115 needs identification and specific call variables to read all the analog inputs needed.
// Version tracking with Git and GitHub 4/25/22


#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads0;  /* Use this for the 16-bit version */
Adafruit_ADS1115 ads1;
//Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>

const char* ssid     = "Tim's Network"; // ESP32 and ESP8266 uses 2.4GHZ wifi only
const char* password = "carlsagan"; 

//MQTT Setup Start
#include <PubSubClient.h>
#define mqtt_server "192.168.7.238"
WiFiClient espClient;
PubSubClient client(espClient);

#define mqttVoltsIn "Volts/Vin"
#define mqttVoltsSol "Volts/Vsol"
#define mqttVoltsSol2 "Volts/Vsol2"
#define mqttCurrentOut "Current/current"
#define mqttCurrent2Out "Current/current2"
#define mqttCurrent3Out "Current/current3"
#define mqttPower "Power/power"
#define mqttPower2 "Power/power2"
#define mqttPower3 "Power/power3"
#define mqttPower4 "Power/power4"
#define mqttRelay "Solar/Relay"
#define mqttRelay2 "Solar/Relay2"

unsigned long R1 = 6800;  //Top Resistor for Battery Voltage Divider
unsigned long R2 = 1000;    //Bottom Resistor for Battery Voltage Divider
unsigned long R3 = 120000;  // Top Resistor for Solar Voltage Divider
unsigned long R4 = 3300;   // Bottom Resistor for Solar Voltage Divider
unsigned long R5 = 15000;  //Top Resistor for Solar2 Voltage Divider
unsigned long R6 = 560;    //Bottom Resistor for Solar2 Voltage Divider

float Vin;
float Vsol;
float Vsol2;
float current;
float current2;
float current3;
float power;
float power2;
float power3;
float power4;
bool manual1;
bool manual2;

unsigned long millisNow = 0;  // for delay purposes
unsigned int sendDelay = 10000;  // delay before sending info via MQTT

const int Relay = 18;
const int Relay2 = 19;


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  String messageTemp2;
  for (int i = 0; i < length; i++){
  Serial.print((char)message[i]);
  messageTemp += (char)message[i];
}
Serial.println();

if(String(topic)== mqttRelay){
  Serial.print("Changing Solar Relay to ");
  if(messageTemp == "on"){
    digitalWrite(Relay, HIGH);
    Serial.print("On");
    manual1 = 1;
  }
  else if(messageTemp == "off"){
    digitalWrite(Relay, LOW);
    Serial.print("Off");
    manual1 = 0;
  }
}
Serial.println();

 Serial.print("Message2 arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message2: ");
  for (int i = 0; i < length; i++){
  Serial.print((char)message[i]);
  messageTemp2 += (char)message[i];
}

Serial.println();
if(String(topic)== mqttRelay2){
  Serial.print("Changing Solar Relay2 to ");
  if(messageTemp2 == "on"){
    digitalWrite(Relay2, HIGH);
    Serial.print("On");
    manual2 = 1;
  }
  else if(messageTemp2 == "off"){
    digitalWrite(Relay2, LOW);
    Serial.print("Off");
    manual2 = 0;
  }
}
Serial.println();
}

void setup()
{
  pinMode(Relay, OUTPUT);
  pinMode(Relay2, OUTPUT);
  Serial.begin(9600);
  Serial.println("Hello!");

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");


 // begin Wifi connect
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //end Wifi connect

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  digitalWrite(Relay, HIGH);
  digitalWrite(Relay2, HIGH);
  manual1 = 1;
  manual2 = 1;
  

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  /*if (!ads_one.begin(0x48)) {
    Serial.println("Failed to initialize ADS_1.");   //initializing first ADS1115 (Default Gain for now)
    while (1);
  }

  if (!ads_two.begin(0x49)) {
    Serial.println("Failed to initialize ADS_2.");   //initializing second ADS1115 (Default Gain for now)
    while (1);
  }
 */

   ads0.begin(0x48);      // ADDR --> GND
   ads1.begin(0x49);      // ADDR --> VCC
  
}
  void topicsSubscribe(){
  client.subscribe("Solar/Relay");
  client.subscribe("Solar/Relay2");
}

void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!client.connected()) {
    if (counter==5){
      ESP.restart();
    }
    counter+=1;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
   
    if (client.connect("electricMonitor2")) {
      Serial.println("connected");
      topicsSubscribe();
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  topicsSubscribe();
}

void loop() {
  if (!client.connected()){
    reconnect();
  }

  if (millis() > millisNow + sendDelay) {
    
  int16_t adc0, adc1, adc2, adc3, adc4, adc5, adc6, adc7;
  float volts0, volts1, volts2, volts3, volts4, volts5, volts6, volts7;

  adc0 = ads0.readADC_SingleEnded(0);
  adc1 = ads0.readADC_SingleEnded(1);
  adc2 = ads0.readADC_SingleEnded(2);
  adc3 = ads0.readADC_SingleEnded(3);
  adc4 = ads1.readADC_SingleEnded(0);
  adc5 = ads1.readADC_SingleEnded(1);
  adc6 = ads1.readADC_SingleEnded(2);
  adc7 = ads1.readADC_SingleEnded(3);

  volts0 = ads0.computeVolts(adc0);
  volts1 = ads0.computeVolts(adc1);
  volts2 = ads0.computeVolts(adc2);
  volts3 = ads0.computeVolts(adc3);
  volts4 = ads1.computeVolts(adc4);
  volts5 = ads1.computeVolts(adc5);
  volts6 = ads1.computeVolts(adc6);
  volts7 = ads1.computeVolts(adc7);

  
  volts1 = volts1 + 0.005;
  volts1 = volts1 * 100.00;
  int voltsA = (int)volts1;
  float voltsA1 = ((float)voltsA)/100.00;
  volts1 = voltsA1;   // method to bring volts1 to two digits behind the decimal, allows for more accurate current calculations

  volts5 = volts5 + 0.005;                       // voltage for calculation of PV2 current
  volts5 = volts5 * 100.00;
  int voltsB = (int)volts5;
  float voltsB1 = ((float)voltsB)/100.00;
  volts5 = voltsB1;   // method to bring volts5 to two digits behind the decimal, allows for more accurate current calculations

   Vin = (volts0)* (R1+R2)/R2;                    // voltage divider calculations for battery
   Vsol = (volts4)* (R3+R4)/R4;                   // voltage divider calculations for solar input
   Vsol2 = (volts3)* (R5+R6)/R6;                   // voltage divider calculations for Solar2 Setup input
   
   current = (1.675-(volts2)) * 10.0/0.12;        // current to/from battery calculation (from voltage change) current sensor
   current2 = (1.675-(volts1)) * 10.0/0.12;       // current from solar input calculation (from voltage change) current2 sensor
   current3 = (1.675-(volts5)) * 10.0/0.12;        // current from Solar2 Setup
   
   if (current2 > 0.10 && current2 < 0.65) {
    current2 = 0.00;
   }                                              // parameters to correct low voltage for low current(0) reading
   if (current > 0.10 && current < 0.65) {
    current = 0.00;
   }
   
   if (current2 > -.85 && current2 <= 0.10){
    current2 = 0.00;
   }
   
   
   if (current3 > 0.10 && current3 < 0.65) {
    current3 = 0.00;
   }                                              // parameters to correct low voltage for low current(0) reading


   
   power = Vin * current;                // Power going to or leaving from the Battery (will not = Total Solar Power, if inverter is on and power going to Tesla's Battery, etc.)
   power2 = Vsol * current2;             // PV1 Solar Power
   power3 = Vsol2 * current3;            // PV2 Solar Power
   power4 = power2 + power3;             // Total Solar Power
   //power = abs(power);

   if ((Relay2 == LOW) && (Vin > 26.30)) {                       // This is second tier automation for voltage control to the battery
    digitalWrite(Relay, LOW); }                                  // PV1 will shut-down if battery voltage remains above 26.3 when PV2 is off
   else {                                                        // PV1 will go back ON first when battery voltage drops below 25.30
    if ((Relay2 == LOW) && (Vin < 25.30) && (manual1 == 1)) {    // manual1 needs to be ON for Relay to go back on, manual1 as a means of control 
     digitalWrite(Relay, HIGH);                                  // for PV1 input
    }
   }

   if (Vin > 26.29){                                          // This is first tier automation for voltage control to the battery
    digitalWrite(Relay2, LOW);                                // PV2 will shut-down if battery voltage goes to 26.30 or higher
    Serial.print("Relay2 is now Off"); }                      // PV2 will go back ON when battery voltage drops below 24.85
   else {                                                     // PV2 will not go back ON until PV1 is ON from a higher voltage (25.29 V) trigger
    if ((Vin < 24.85) && (manual2 == 1)) {                    // manual2 needs to be ON for Relay2 to go back on, manual2 as a means of control
    digitalWrite(Relay2, HIGH);                               // for PV2 input
   }
   }

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");
  Serial.print("AIN4: "); Serial.print(adc4); Serial.print("  "); Serial.print(volts4); Serial.println("V");
  Serial.print("AIN5: "); Serial.print(adc5); Serial.print("  "); Serial.print(volts5); Serial.println("V");
  Serial.print("AIN6: "); Serial.print(adc6); Serial.print("  "); Serial.print(volts6); Serial.println("V");
  Serial.print("AIN7: "); Serial.print(adc7); Serial.print("  "); Serial.print(volts7); Serial.println("V");
  
  Serial.print("Battery Voltage = "); Serial.println(Vin);
  Serial.print("PV1 Voltage = "); Serial.println(Vsol);
  Serial.print("PV2 Voltage = "); Serial.println(Vsol2);
  Serial.print("Battery Current = "); Serial.println(current);
  Serial.print("PV1 Current = "); Serial.println(current2);
  Serial.print("PV2 Current = "); Serial.println(current3);
  Serial.print("Battery Power = "); Serial.println(power);
  Serial.print("PV1 Power = "); Serial.println(power2);
  Serial.print("PV2 Power = "); Serial.println(power3);
  Serial.print("Total PV Power = "); Serial.println(power4);


  client.publish(mqttVoltsIn, String(Vin).c_str(),true);
  client.publish(mqttVoltsSol, String(Vsol).c_str(),true);
  client.publish(mqttVoltsSol2, String(Vsol2).c_str(),true);
  client.publish(mqttCurrentOut, String(current).c_str(),true);
  client.publish(mqttCurrent2Out, String(current2).c_str(),true);
  client.publish(mqttCurrent3Out, String(current3).c_str(),true);
  client.publish(mqttPower, String(power).c_str(),true);
  client.publish(mqttPower2, String(power2).c_str(),true);
  client.publish(mqttPower3, String(power3).c_str(),true);
  client.publish(mqttPower4, String(power4).c_str(),true);
  millisNow = millis();

}
  
  client.loop();
}
