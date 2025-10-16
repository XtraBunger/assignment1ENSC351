#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "hal/led.h"
#include "hal/joystickSPI.h"

// Get time
static long long getTimeInMs(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    return seconds * 1000 + nanoSeconds / 1000000;
}

// Sleep
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
    srand(time(NULL)); // Random num gen
    
    // Initialize HAL stuff
    led_init();           
    Joystick_init();
    
    printf("Hello embedded world, from me, Aaron!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n\n");
    
    long long bestTime = -1;  
    bool keepRunning = true;
    
    while (keepRunning) {

        // Start
        printf("Get ready...\n");
        for (int i = 0; i < 4; i++) {
            led_setGreen(true);   
            sleepMs(250);
            led_setGreen(false);

            led_setRed(true);
            sleepMs(250);
            led_setRed(false); 
        }
        
        if (joystickMoved()) {    
            printf("Please let go of joystick\n");
            while (joystickMoved()) {
                sleepMs(50);
            }
        }
        
        // Random wait time
        int waitTime = 500 + (rand() % 2501); 
        sleepMs(waitTime);
        
        if (joystickMoved()) {     
            printf("too soon\n");
            continue;
        }
        
        // Random direction picker
        bool shouldPressUp = (rand() % 2) == 0;
        if (shouldPressUp) {
            printf("Press UP now!\n");
            led_setGreen(true);   
        } else {
            printf("Press DOWN now!\n");
            led_setRed(true);      
        }
        
        long long startTime = getTimeInMs();
        JoystickDirection response = JOYSTICK_NONE;
        
        // Wait for user response
        while (response == JOYSTICK_NONE) {
            response = Joystick_read();
            
            if (getTimeInMs() - startTime > 5000) {
                printf("No input within 5000ms; quitting!\n");
                keepRunning = false;
                break;
            }
            
            sleepMs(10);  // Small delay to avoid busy-waiting
        }
        
        led_setGreen(false);     
        led_setRed(false);         
        
        if (!keepRunning) break;
        
        long long reactionTime = getTimeInMs() - startTime;
        
        // Results
        if (response == JOYSTICK_LEFT || response == JOYSTICK_RIGHT) {
            printf("User selected to quit.\n");
            keepRunning = false;
        } else if ((shouldPressUp && response == JOYSTICK_UP) ||
                   (!shouldPressUp && response == JOYSTICK_DOWN)) {
            printf("Correct!\n");
            
            if (bestTime == -1 || reactionTime < bestTime) {
                printf("New best time!\n");
                bestTime = reactionTime;
            }
            
            printf("Your reaction time was %lldms; best so far in game is %lldms.\n",
                   reactionTime, bestTime);
            
            led_flashGreen(5, 1000);   // 5x in 1 second
        } else {
            printf("Incorrect.\n");
            
            led_flashRed(5, 1000);     // 5x in 1 second
        }
        
        printf("\n");
    }
    
    led_cleanup();      
    Joystick_cleanup();
    
    return 0;
}