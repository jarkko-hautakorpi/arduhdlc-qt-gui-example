![Screenshot](https://github.com/jarkko-hautakorpi/arduhdlc-qt-gui-example/blob/master/Screenshot.png)

# Qt GUI to Arduino Arduhdlc Example Project

Example project with Qt GUI Application controlling servo on Arduino UNO, over HDLC protocol.
HDLC protocol is used to wrap all communication into [HDLC frames](https://en.wikipedia.org/wiki/High-Level_Data_Link_Control#Structure). On both ends, on the Qt GUI and on the Arduino, valid HDLC frames are passed to command router or dispatcher.

## Command router/dispatcher

Application defined "protocol" or frame structure is, that the first byte is command byte.
Dispatcher then extracts the first byte of the frame, which in this program example is used to hold the command, and calls the right command, passing the frame to it as a reference. Command function then extracts the rest of the data from the frame, if it requires it.

## Qt HDLC Singleton Class

Arduhdlc library can not be conveniently used in Qt, so it is converted to a singleton class. All function pointers are replaced with Qt's signals and slots. Any actions can then easily be connected to HDLC Class by connecting/disconnecting signals.
User must implement the same receive and valid frame handler functions, than on the Arduino side when using [Arduhdlc library](https://github.com/jarkko-hautakorpi/Arduhdlc).


### Links to related resources

[Simple Servo Control tutorial](http://playground.arduino.cc/Learning/SingleServoExample)

[HDLC Protocol in Wikipedia](https://en.wikipedia.org/wiki/High-Level_Data_Link_Control#Structure)

[Open Source HDLC (OSHDLC)](https://github.com/dipman/OSHDLC)

[Piconomic FW Library, with HDLC encapsulation layer](http://piconomic.co.za/fwlib/group___h_d_l_c.html)

[Dust SmartMeshSDK C Library](https://github.com/dustcloud/sm_clib)

[C++ Qt 03 - Intro to GUI programming](https://youtu.be/GxlB34Cn0zw)

[Qt & Arduino - Making an RGB LED Controller - Intro](https://youtu.be/s3QuNvYce1o)
