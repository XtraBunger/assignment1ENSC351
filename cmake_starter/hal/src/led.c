#include "hal/led.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

// LED paths for BeagleY-AI
#define GREEN_LED_TRIGGER "/sys/class/leds/ACT/trigger"
#define GREEN_LED_BRIGHTNESS "/sys/class/leds/ACT/brightness"
#define RED_LED_TRIGGER "/sys/class/leds/PWR/trigger"
#define RED_LED_BRIGHTNESS "/sys/class/leds/PWR/brightness"

static bool isInitialized = false;

// Helper function to write to the led files
static void writeToFile(const char* path, const char* value) {
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Unable to open %s\n", path);
        exit(1);
    }
    fprintf(file, "%s", value);
    fclose(file);
}

 // Initialize LEDs
void led_init(void) {
    // Set LED triggers to "none" so we can control them manually
    writeToFile(GREEN_LED_TRIGGER, "none");
    writeToFile(RED_LED_TRIGGER, "none");
    
    isInitialized = true;

    // Turn off both LEDs initially
    led_setGreen(false);
    led_setRed(false);
}

 // Cleanup LEDs
void led_cleanup(void) {
    assert(isInitialized);
    
    // Turn off both LEDs
    led_setGreen(false);
    led_setRed(false);
    
    isInitialized = false;
}

 // Writes to brightness file to turn on/off LED
void led_setGreen(bool on) {
    assert(isInitialized);
    
    const char* value = on ? "1" : "0";
    writeToFile(GREEN_LED_BRIGHTNESS, value);
}

 // Writes to brightness file to turn on/off LED
void led_setRed(bool on) {
    assert(isInitialized);
    
    const char* value = on ? "1" : "0";
    writeToFile(RED_LED_BRIGHTNESS, value);
}

// Sleep function
static void sleepMs(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

 // Flash LED x times over totalTimeMs
void led_flashGreen(int numFlashes, int totalTimeMs) {
    assert(isInitialized);
    
    // Each flash = on + off, so time per flash cycle
    int msPerFlash = totalTimeMs / numFlashes;
    int msOn = msPerFlash / 2;
    int msOff = msPerFlash / 2;
    
    for (int i = 0; i < numFlashes; i++) {
        led_setGreen(true);
        sleepMs(msOn);
        led_setGreen(false);
        sleepMs(msOff);
    }
}

  // Flash LED x times over totalTimeMs
void led_flashRed(int numFlashes, int totalTimeMs) {
    assert(isInitialized);
    
    int msPerFlash = totalTimeMs / numFlashes;
    int msOn = msPerFlash / 2;
    int msOff = msPerFlash / 2;
    
    for (int i = 0; i < numFlashes; i++) {
        led_setRed(true);
        sleepMs(msOn);
        led_setRed(false);
        sleepMs(msOff);
    }
}