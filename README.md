This project combines a Node-RED application on Bluemix using the IBM IoT Service with an Arduino Yun to allow the actuation of a solenoid controlled lock.

The incoming e-mail node in Node-RED reads e-mail coming to the account bluemixclouldlock@gmail.com (change to your
desired account). A switch node searched the incoming e-mail for an Access Code (change the Access Code and optionally
add more Access Codes).

If the Access Code is found in the e-mail, the system sends a message to the Arduino Yun to turn ON the desired output. The output is signal to the Arduino is in the ON state for a time set by the delay node, currently 3 seconds. You can control the duration of the output ON time by modifying the delay node.

Both the Node-RED and Arduino code are based on previously available IoT code examples. The examples were minimally
modified to provide the functionality desired for this project.

Import the script into Node-RED and install the .ino file on an Arduino Yun that has been provisioned on the
WiFi network. The Arduino program uses pin13 to allow the status of the output to be displayed on the
on board red LED.

Opening the serial port monitor in the Arduino IDE will allow you to see the status of the communications
between the Arduino Yun and the IBM Bluemix IoT Service.

The Arduino code also configures Analog inputs A0, A1 and A3 to read temperatures from an LM35 temperature sensor
and push the temperature to the IBM IoT Service.



