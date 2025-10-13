#pragma once

#include <stdbool.h>

void led_init(void);
void led_setGreen(bool on);
void led_setRed(bool on);
void led_flashGreen(int numFlashes, int totalTimeMs);
void led_flashRed(int numFlashes, int totalTimeMs);
void led_cleanup(void); 