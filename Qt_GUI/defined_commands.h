#ifndef defined_commands_h
#define defined_commands_h

enum serial_commands {
    COMMAND_ERROR = 0, // 0
    COMMAND_SET_SERVO_POSITION,
    COMMAND_TOGGLE_LED,
    COMMAND_READ_SIGNATURE,
    COMMAND_ECHO_DATA,
};

enum serial_responses {
    RESPONSE_ERROR = 0, // 0
    RESPONSE_VERSION,
    RESPONSE_BUTTON_PRESS,
    RESPONSE_READ_SIGNATURE,
    RESPONSE_ECHO_DATA,
};

#endif
