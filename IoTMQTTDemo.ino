#include <MQTTClient.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


#include "sha256.h"
#include "ConnectionStringHelper.h"

/* ----------------------------------------------------------------------------- *
 * This Sample Code is provided for  the purpose of illustration only and is not * 
 * intended  to be used in a production  environment.  THIS SAMPLE  CODE AND ANY * 
 * RELATED INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER * 
 * EXPRESSED OR IMPLIED, INCLUDING  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF * 
 * MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.                      * 
 * ----------------------------------------------------------------------------- */

/*
 * 
 * Demonstrates connecting an ESP8266 to an Azure IoT hub using a third party MQTT stack.
 * See: https://azure.microsoft.com/en-us/documentation/articles/iot-hub-mqtt-support/#using-the-mqtt-protocol-directly
 * 
 */

// Next five variables need to have your appropriate values added
const String SSID = "<SSID>";
const String PASSWORD = "<Password>";
// Connection string in the form HostName=myhost.azure-devices.net;DeviceId=mydevice;SharedAccessKey=sharedaccesskey
// This can be acquired from the Azure Portal or the Device Explorer
const String CONNECTIONSTRING = "<Connection String>";
const uint INTERVAL = 20;             // SAS token validity period in minutes
const int REFRESHPERCENTAGE = 80;     // Percentage of SAS token validity used when reauthentication is attempted
const int MISSEDTOOMANY = 10;         // Reconnect to MQTT when consecutively missed this many messages 
const int BLINKER = 0;				  // GPIO of LED to indicate activity
const int BLINKTIME = 700;
const int BLINKOFF = HIGH;
const int BLINKON = LOW;

WiFiUDP ntpUDP;
WiFiClientSecure net;
NTPClient timeClient(ntpUDP);
MQTTClient client;
ConnectionStringHelper *csHelper;
String pubTopic;
String subTopic;
int32_t refreshTime = 0;

//
// Included for debug print of binary arrays
void printBinary(uint8_t *data, size_t dataLength)
{
  for (int i = 0; i < dataLength; i++)
  {
    Serial.print("0123456789abcdef"[data[i]>>4]);
    Serial.print("0123456789abcdef"[data[i]&0xf]);
  }
  
  Serial.println();
}

void softwareReboot()
{
  Serial.println("Rebooting...");
  wdt_enable(WDTO_250MS);
  while(true);
}

void setupWiFi(const String ssid, const String password)
{
  Serial.print("Connecting to SSID ");
  Serial.println(ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  while ( WiFi.status() != WL_CONNECTED ) 
  {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println();
  Serial.print("IP Address = ");
  Serial.println(WiFi.localIP());
}

void setupMQTT(uint interval)
{
  long now = timeClient.getEpochTime();
  long epochTime = now + (interval * 60);
  String mqttUserName = csHelper->getKeywordValue("hostname") + "/" + csHelper->getKeywordValue("deviceid");
  String mqttClientId = csHelper->getKeywordValue("deviceid");
  String mqttServer = csHelper->getKeywordValue("hostname");
  String mqttPassword = "";

  
  csHelper->generatePassword(mqttServer, mqttClientId, csHelper->getKeywordValue("SharedAccessKey"), epochTime, mqttPassword);
  refreshTime = now + (interval * 60 * REFRESHPERCENTAGE / 100);
  Serial.print("Time is now ");
  Serial.println(now, DEC);
  Serial.print("SAS token expires at ");
  Serial.println(epochTime, DEC);
  Serial.print("Will refresh at ");
  Serial.println(refreshTime, DEC);
  
//  Serial.print("MQTT_Server=");
//  Serial.println(mqttServer);
//  Serial.print("MQTT_ClientId=");
//  Serial.println(mqttClientId);
//  Serial.print("MQTT_UserName=");
//  Serial.println(mqttUserName);
//  Serial.print("MQTT_Password=");
//  Serial.println(mqttPassword);

  uint8_t ret;
  client.begin(mqttServer.c_str(), 8883, net);

  int counter = 20;
  
  // If we can't connect in 20 attempts then reboot
  while (!client.connect(mqttClientId.c_str(), mqttUserName.c_str(), mqttPassword.c_str()))
  {
    Serial.print(".");
    delay(1000);    

    if (0 >= --counter)
      softwareReboot();
  }

  Serial.println("MQTT connected");
  
  pubTopic = "devices/" + mqttClientId + "/messages/events/";
  subTopic = "devices/" + mqttClientId + "/messages/devicebound/#";

  if (!client.subscribe(subTopic.c_str()))
    Serial.println("Subscribe failed");
}

void setup() 
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Start Test");

  pinMode(BLINKER, OUTPUT);
  digitalWrite(BLINKER, BLINKOFF);
  csHelper = new ConnectionStringHelper(CONNECTIONSTRING);

  setupWiFi(SSID, PASSWORD);

  timeClient.begin();

  // Cannot continue until we have a clock update
  while (!timeClient.update())
  {
    Serial.println("Initial clock update failed");
    delay(250);
  }
  delay(1000);
  
  setupMQTT(INTERVAL);
}

void loop() {

  static uint32_t counter = 0;
  static uint32_t lastMsg = 0;
  static uint32_t missedMessageCount = 0;
  static uint32_t ledOffAt = 0;
  uint32_t currMillis;
  
  while (!timeClient.update())
  {
    Serial.println("NTP update failed");
    delay(250);
  }
  
  client.loop();

  // Refresh the SAS token if necessary
  if (timeClient.getEpochTime() > refreshTime)
  {
    Serial.println("Reconnecting to refresh SAS token");

    client.disconnect();
    setupMQTT(INTERVAL);
    missedMessageCount = 0;
  }

  currMillis = millis();

  if (ledOffAt && currMillis >= ledOffAt)
  {
    digitalWrite(BLINKER, BLINKOFF);
    ledOffAt = 0;
  }
  
  // Check for wrap (around every 50 days)
  if (currMillis < lastMsg)
    lastMsg = 0;

  // Send a message about every three seconds
  if (currMillis - lastMsg > 3000)
  {
    lastMsg = currMillis;
    String msg = "{ \"counter\": " + String(counter++) + ", \"epoch\": " + String(timeClient.getEpochTime()) + ", \"millis\": " + String(currMillis) + " }";
    
    if (!client.publish(pubTopic.c_str(), msg.c_str()))
    {
      Serial.println("Publish failed");

      if (++missedMessageCount > MISSEDTOOMANY)
      {
        missedMessageCount = 0;
        client.disconnect();
        setupMQTT(INTERVAL);
      }
    }
    else
    {
      missedMessageCount = 0;
      digitalWrite(BLINKER, BLINKON);
      ledOffAt = currMillis + BLINKTIME;
    }
  }
}

void messageReceived(String topic, String payload, char *bytes, unsigned int length)
{
  Serial.println("Message received");
  Serial.println("\tTopic: " + topic + "\n\tPayload: " + payload);
}

