#ifndef JOYSTICK_SPI_H
#define JOYSTICK_SPI_H

#include <stdbool.h>

// Joystick directions
typedef enum {
    JOYSTICK_NONE,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_LEFT,
    JOYSTICK_RIGHT
} JoystickDirection;

// Initialize the joystick module (sets up SPI)
void Joystick_init(void);

// Cleanup the joystick module
void Joystick_cleanup(void);

// Read the current joystick direction
JoystickDirection Joystick_read(void);

#endif