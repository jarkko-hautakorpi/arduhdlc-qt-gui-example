#include <Servo.h>
#include <avr/boot.h>
#include "Arduhdlc.h"
#include "defined_commands.h"

/* How long (bytes) is the longest HDLC frame? */
#define MAX_HDLC_FRAME_LENGTH 32
#define SERVO_PIN 14
#define LED1_PIN 15
#define VERSION_NUMBER 0.1


/* Variable for LED toggle */
boolean toggle = false;

/* Function to send out byte/char */
void send_character(uint8_t data);
/* Function to handle a valid HDLC frame */
void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength);

/* Command execute functions for each command */
void command_set_servo_position(const uint8_t *framebuffer, uint16_t framelength);
void command_toggle_led(const uint8_t *framebuffer, uint16_t framelength);
void command_read_signature(const uint8_t *framebuffer, uint16_t framelength);
void command_echo_data(const uint8_t *framebuffer, uint16_t framelength);

/* Initialize Servo library */
Servo servo1;

/* Initialize Arduhdlc library with three parameters.
1. Character send function, to send out HDLC frame one byte at a time.
2. HDLC frame handler function for received frame.
3. Length of the longest frame used, to allocate buffer in memory */
Arduhdlc hdlc(&send_character, &hdlc_command_router, MAX_HDLC_FRAME_LENGTH);

/* Function to send out one 8bit character */
void send_character(uint8_t data) {
    Serial.print((char)data);
}

/*
* Command router/dispatcher
* Calls the right command, passing data to it.
* @TODO Make it better
* https://doanduyhai.wordpress.com/2012/08/04/design-pattern-the-asynchronous-dispatcher/
*/
void hdlc_command_router(const uint8_t *framebuffer, uint16_t framelength) {
    enum serial_commands command = static_cast<serial_commands>(framebuffer[0]);
    switch(command)
    {
        case COMMAND_ERROR: command_error(); break;
        case COMMAND_SET_SERVO_POSITION:     command_set_servo_position(framebuffer, framelength); break;
        case COMMAND_TOGGLE_LED:             command_toggle_led(framebuffer, framelength); break;
        case COMMAND_READ_SIGNATURE:         command_read_signature(framebuffer, framelength); break;
        case COMMAND_ECHO_DATA:              command_echo_data(framebuffer, framelength); break;
        default:
        command_default();
        break;
    }
}

void command_set_servo_position(const uint8_t *framebuffer, uint16_t framelength) {
    static int servo_position = 0;
    char num[5];
    servo_position = framebuffer[1];
    servo1.write(servo_position);
}

void command_toggle_led(const uint8_t *framebuffer, uint16_t framelengthd) {
    toggle = !toggle;
    digitalWrite(LED1_PIN, toggle);
}

void command_read_signature(const uint8_t *framebuffer, uint16_t framelengthd) {
    char signature[4];
    signature[0] = RESPONSE_READ_SIGNATURE;
    signature[1] = boot_signature_byte_get(0); //Device Signature Byte 1
    signature[2] = boot_signature_byte_get(2); //Device Signature Byte 2
    signature[3] = boot_signature_byte_get(4); //Device Signature Byte 3
    signature[4] = boot_signature_byte_get(1); //RC Oscillator Calibration Byte
    hdlc.frameDecode(signature, 5);
}

// ECHO the data back, for testing CRC sum
void command_echo_data(const uint8_t *framebuffer, uint16_t framelength) {
    hdlc.frameDecode((const char*)framebuffer, framelength);
}

void command_error(void) {
    char data[2] = {(uint8_t)RESPONSE_ERROR, 1};
    //data[0] = RESPONSE_ERROR;
    hdlc.frameDecode(data, 2);
}

void command_default(void) {
    ;
}

void setup() {
    pinMode(1,OUTPUT); // Serial port TX to output
    servo1.attach(SERVO_PIN); // Servo on analog pin A0
    servo1.write(512);
    pinMode(LED1_PIN, OUTPUT);
    // initialize serial port to 9600 baud
    Serial.begin(9600);
}

void loop() {
    
}

/*
SerialEvent occurs whenever a new data comes in the
hardware serial RX.  This routine is run between each
time loop() runs, so using delay inside loop can delay
response.  Multiple bytes of data may be available.
*/
void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        hdlc.charReceiver(inChar);
    }
}
