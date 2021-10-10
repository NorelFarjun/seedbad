# seedbad
germination without the need for supervision

The program is designed for Arduino-compliant ESP32 boards, using Arduino IDE.
The program uses FreeRTOS to control multiple processes running parallelly in the background
The pins used, values and intervals are customized for my personal use.
You may need to adjust them for your own use.

The program has several processes:
1) A process that ensures a continuous connection to a given WIFI network, and reconnects in case the connection is disconnected.
2) A process that ensures ideal temperature and humidity maintenance (tailored to my needs) by controlling a heater, fan and water pump.
3) A process that ensures irrigation at regular intervals.
 
In addition, the program uses listeners to receive HTTP calls and execute commands as required.

The program depends on several other libraries:

AsyncTCP : https://github.com/me-no-dev/AsyncTCP

ESP Async WebServer: https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/ESPAsyncWebServer.h

ClosedCube HDC1080: https://github.com/closedcube/ClosedCube_HDC1080_Arduino


