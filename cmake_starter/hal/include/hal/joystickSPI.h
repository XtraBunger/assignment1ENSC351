#pragma once

#include <stdbool.h>

// Joystick directions
typedef enum {
    JOYSTICK_NONE = 0,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT
} JoystickDirection;

// Initializes  the joystick SPI stuff
void Joystick_init(void);

// Cleanup the joystick SPI stuff
void Joystick_cleanup(void);

// Read the joystick direction
JoystickDirection Joystick_read(void);

// Check if joystick moved from center position
bool joystickMoved(void);