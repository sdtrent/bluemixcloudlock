/*  IBM IoT Design Challeng Entry 12-8-14
    based on "arudinoyun" by Kyle Brown
    https://hub.jazz.net/git/brownkyl/arduinoyun
*/

/*
 MQTT IOT Example
 - continuously obtains values from the Virtuabotix DHT11 temperature/humidity sensor
 - formats the results as a JSON string for the IBM IOT example
 - connects to an MQTT server (either local or at the IBM IOT Cloud)
 - and publishes the JSON String to the topic named quickstart:MY_MAC_ADDRESS
 */

/* Shane Trent - modified to disable DHT11 and use an LM35 temperature sensor
 - want to open a solienod lock on command from IBM IoT service
 - PWM lock to hold open for (90 seconds)
 - monitor vibration(sound) to ensure lock opened and closed
 - can test lock by verifying a toggle when user leaves space 
 - monitoring vibration to determine mimium PWM for pull-in and hold
 - report these values to cloud for maintanance
 - report lock jam and report if jam was cleared with X of Ymax cycles
 - log access to database
*/

#include <SPI.h>
#include <Bridge.h>
#include <YunClient.h>
#include <PubSubClient.h>
#include <dht11.h>
#include <Adafruit_NeoPixel.h>

#define MODE_GREEN 1
#define MODE_RED 2
#define MODE_OFF 0
#define PIXEL_PIN 6
#define NUMPIXELS 1

#define Vs   A0  // +5V for LM35   temperature C in millivolts 100c = 1000 mV
#define LM35 A1  // Vout for LM35
#define GND  A2  // GND for LM35

// Update this to either the MAC address found on the sticker on your ethernet shield (newer shields)
// or a different random hexadecimal value (change at least the last four bytes)
byte mac[]    = {0x90, 0xA2, 0xDA, 0xF5, 0x11, 0x3F };
char macstr[] = "90a2daf5113f";

char servername[]="mgslnp.messaging.internetofthings.ibmcloud.com";
String clientName = String("d:mgslnp:ArduinoYun:") + macstr;
String topicName = String("iot-2/evt/status/fmt/json");
char username[]="use-token-auth";
char password[]="AiwBlo(0i5DFWVLgN6";

dht11 DHT11;
float tempF = 10.0;
float tempC = 0.0;
float humidity = 50.0;
int signal_mode =MODE_OFF;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
unsigned long time = 0;
YunClient ethClient;

void callback(char* topic, byte* payload, unsigned int length);

char localserver[] = "192.168.123.3";
//PubSubClient client(localserver, 1883, callback, ethClient);
PubSubClient client(servername, 1883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  char message_buff[100];
  int i;
 // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  Serial.println("Payload: " + msgString);
 // Check to see if you have received a command you recognize.
  if (msgString.equals("{\"lightmode\": \"OFF\"}")) {
      signal_mode = MODE_OFF;
      digitalWrite(13, LOW);    //turn off LED
      Serial.println("Signal mode is off");
  } else if (msgString.equals("{\"lightmode\": \"RED\"}")) {
      signal_mode = MODE_RED;
      digitalWrite(13, HIGH);     //turn ON LED
      Serial.println("Signal mode is red");
  } else if (msgString.equals("{\"lightmode\": \"GREEN\"}")) {
      signal_mode = MODE_GREEN;
      Serial.println("Signal mode is green");
  }
}

void setup()
{

  Serial.begin(9600);
  //while (!Serial);
  Serial.println("attaching DHT11");
  DHT11.attach(3);
  
  pinMode(Vs, OUTPUT);
  digitalWrite(Vs, HIGH);
  
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
analogReference(INTERNAL);    // WANT 1.1V ref
  
  pinMode(13,OUTPUT);
  //digitalWrite(13, LOW);
  Serial.println("About to start bridge");
  Bridge.begin();
  Serial.println("Bridge started");
  //digitalWrite(13, HIGH);
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.println("exiting setup()");

}

void loop()
{
  char clientStr[33];
  clientName.toCharArray(clientStr,33);
  char topicStr[26];
  topicName.toCharArray(topicStr,26);
  //getData();
  
  tempC=analogRead(A1);
  tempC = tempC/5.9;  // convert counts to degrees C
//  Serial.print("Temperature is: ");
//  Serial.println(int(tempC));
    
  if (!client.connected()) {
    Serial.print("Trying to connect to: ");
    Serial.println(clientStr);
    client.connect(clientStr,username,password);
    client.subscribe("iot-2/cmd/+/fmt/json");
  }
  if (client.connected() ) {
    String json = buildJson();
    char jsonStr[200];
    json.toCharArray(jsonStr,200);
    boolean pubresult = client.publish(topicStr,jsonStr);
    Serial.print("attempt to send ");
    Serial.println(jsonStr);
    Serial.print("to ");
    Serial.println(topicStr);
    if (pubresult)
      Serial.println("successfully sent");
    else
      Serial.println("unsuccessfully sent");
  }
  client.loop();
  changeLED();
  delay(1000);
}

String buildJson() {
  String data = "{";
  data+="\n";
  data+= "\"d\": {";
  data+="\n";
  data+="\"myName\": \"Arduino DHT11\",";
  data+="\n";
  data+="\"temperature (F)\": ";
  data+=(int)tempF;
  data+= ",";
  data+="\n";
  data+="\"temperature\": ";
  data+=(int)tempC;
  data+= ",";
  data+="\n";
  data+="\"humidity\": ";
  data+=(int)humidity;
  data+="\n";
  data+="}";
  data+="\n";
  data+="}";
  return data;
}

void getData() {
  int chk = DHT11.read();
  switch (chk)
  {
  case 0: 
    Serial.println("Read OK"); 
    humidity = (float)DHT11.humidity;
    tempF = DHT11.fahrenheit();
    tempC = DHT11.temperature;
    break;
  case -1: 
    Serial.println("Checksum error"); 
    break;
  case -2: 
    Serial.println("Time out error"); 
    break;
  default: 
    Serial.println("Unknown error"); 
    break;
  }
}

void changeLED() {
  if (signal_mode == MODE_OFF) {
    pixels.setPixelColor(0, pixels.Color(0,0,0));
  }
  else if (signal_mode == MODE_RED) {
    pixels.setPixelColor(0, pixels.Color(255,0,0));
  }
  else if (signal_mode == MODE_GREEN) {
    pixels.setPixelColor(0, pixels.Color(0,255,0));
  } 
    pixels.show(); // This sends the updated pixel color to the hardware.
}
