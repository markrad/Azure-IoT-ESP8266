# Using an ESP8266 and a Third Party MQTT Stack to Communicate with an Azure IoT Hub
This is an example of how one might use a third party MQTT stack instead of the [Azure-IoT-C-SDK](https://github.com/Azure/azure-iot-sdk-c) to communicate with an Azure IoT Hub. 
This is a project that can be built and run from the Arduino IDE along with an ESP8266 flashed with suitable firmware. 
You will need to use the Arduino Library Manager to install the following libraries:
- NTPClient [https://github.com/arduino-libraries/NTPClient](https://github.com/arduino-libraries/NTPClient)
- MQTT [https://github.com/256dpi/arduino-mqtt](https://github.com/256dpi/arduino-mqtt)

GitHub URLs provided only for reference.

**Important** In the MQTT library, you will need to modify the definition _#define MQTT\_BUFFER\_SIZE 128_ in MQTTClient.h to read _#define MQTT\_BUFFER\_SIZE 256_ or you will not have a large enough buffer to connect to the Azure IoT hub. 

Finally you will need to add the Wi-Fi SSID and password and the IoT hub connection string to the top of the .ino file.

