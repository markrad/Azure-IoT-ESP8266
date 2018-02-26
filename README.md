# Using an ESP8266 and a Third Party MQTT Stack to Communicate with an Azure IoT Hub
This is an example of how one might use a third party MQTT stack instead of the [Azure-IoT-C-SDK](https://github.com/Azure/azure-iot-sdk-c) to communicate with an Azure IoT Hub. 
This is a project that can be built and run from the Arduino IDE along with an ESP8266 flashed with suitable firmware. 
You will need to use the Arduino Library Manager to install the following libraries:
- NTPClient [https://github.com/arduino-libraries/NTPClient](https://github.com/arduino-libraries/NTPClient)
- MQTT [https://github.com/256dpi/arduino-mqtt](https://github.com/256dpi/arduino-mqtt)
- WiFiManager [https://github.com/tzapu/WiFiManager](https://github.com/tzapu/WiFiManager)

GitHub URLs provided only for reference.


The first time the application is executed it will set itself up as an access point and start a webserver. Connect to the access point and enter the device's IP address into your browser to fill in the SSID, WiFi password and the Azure IoT device connection string. Thereafter this information will be used each time the device is booted relieving one from the requirement of setting it up every time.

