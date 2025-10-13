#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "hal/led.h"
#include "hal/joystickSPI.h"

// Helper function to get time in milliseconds
static long long getTimeInMs(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    return seconds * 1000 + nanoSeconds / 1000000;
}

// Helper function to sleep for milliseconds
static void sleepMs(long long delayInMs) {
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, NULL);
}

int main(void) {
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize HAL modules
    led_init();           // FIXED: was LED_init()
    Joystick_init();
    
    printf("Hello embedded world, from <YOUR NAME>!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n\n");
    
    long long bestTime = -1;  // Best reaction time in ms
    bool keepRunning = true;
    
    while (keepRunning) {
        // 1. Print "get ready" and flash LEDs 4 times
        printf("Get ready...\n");
        for (int i = 0; i < 4; i++) {
            led_setGreen(true);    // FIXED: was LED_setGreen
            sleepMs(250);
            led_setGreen(false);   // FIXED: was LED_setGreen
            
            led_setRed(true);      // FIXED: was LED_setRed
            sleepMs(250);
            led_setRed(false);     // FIXED: was LED_setRed
        }
        
        // 2. Check if user is already pressing joystick
        if (joystickMoved()) {     // FIXED: Added this check
            printf("Please let go of joystick\n");
            while (joystickMoved()) {
                sleepMs(50);
            }
        }
        
        // 3. Wait random time (0.5s to 3.0s)
        int waitTime = 500 + (rand() % 2501);  // FIXED: was 2500, should be 2501 for inclusive 3000
        sleepMs(waitTime);
        
        // 4. Check if user pressed too soon
        if (joystickMoved()) {     // FIXED: Added this check
            printf("too soon\n");
            continue;
        }
        
        // 5. Pick random direction and light LED
        bool shouldPressUp = (rand() % 2) == 0;
        if (shouldPressUp) {
            printf("Press UP now!\n");
            led_setGreen(true);    // FIXED: was LED_setGreen
        } else {
            printf("Press DOWN now!\n");
            led_setRed(true);      // FIXED: was LED_setRed
        }
        
        // 6. Time user's response
        long long startTime = getTimeInMs();
        JoystickDirection response = JOYSTICK_NONE;
        
        while (response == JOYSTICK_NONE) {
            response = Joystick_read();
            
            // Check for timeout (5 seconds)
            if (getTimeInMs() - startTime > 5000) {
                printf("No input within 5000ms; quitting!\n");
                keepRunning = false;
                break;
            }
            
            sleepMs(10);  // Small delay to avoid busy-waiting
        }
        
        // Turn off LEDs
        led_setGreen(false);       // FIXED: was LED_setGreen
        led_setRed(false);         // FIXED: was LED_setRed
        
        if (!keepRunning) break;
        
        long long reactionTime = getTimeInMs() - startTime;
        
        // 7. Process user's response
        if (response == JOYSTICK_LEFT || response == JOYSTICK_RIGHT) {
            printf("User selected to quit.\n");
            keepRunning = false;
        } else if ((shouldPressUp && response == JOYSTICK_UP) ||
                   (!shouldPressUp && response == JOYSTICK_DOWN)) {
            // Correct!
            printf("Correct!\n");
            
            if (bestTime == -1 || reactionTime < bestTime) {
                printf("New best time!\n");
                bestTime = reactionTime;
            }
            
            printf("Your reaction time was %lldms; best so far in game is %lldms.\n",
                   reactionTime, bestTime);
            
            // Flash green LED 5 times in 1 second
            led_flashGreen(5, 1000);   // FIXED: was LED_flashGreen
        } else {
            // Incorrect
            printf("Incorrect.\n");
            
            // Flash red LED 5 times in 1 second
            led_flashRed(5, 1000);     // FIXED: was LED_flashRed
        }
        
        printf("\n");
    }
    
    // Cleanup
    led_cleanup();        // FIXED: was LED_cleanup
    Joystick_cleanup();
    
    return 0;
}