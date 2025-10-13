#include "hal/joystickSPI.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include <assert.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_SPEED 250000

// ADC channels for joystick axes
#define JOYSTICK_X_CHANNEL 0
#define JOYSTICK_Y_CHANNEL 1

// Threshold values (you'll need to calibrate these)
#define THRESHOLD_LOW 1000   // Below this = pressed in negative direction
#define THRESHOLD_HIGH 3000  // Above this = pressed in positive direction

static int spiFd = -1;
static bool isInitialized = false;

// Read one channel from the ADC (from the SPI guide)
static int readChannel(int channel) {
    assert(isInitialized);
    
    // Build the command to send to ADC (MCP3008)
    uint8_t tx[3] = {
        (uint8_t)(0x06 | ((channel & 0x04) >> 2)),
        (uint8_t)((channel & 0x03) << 6),
        0x00
    };
    uint8_t rx[3] = {0};
    
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 3,
        .speed_hz = SPI_SPEED,
        .bits_per_word = 8,
        .cs_change = 0
    };
    
    if (ioctl(spiFd, SPI_IOC_MESSAGE(1), &transfer) < 1) {
        fprintf(stderr, "ERROR: SPI transfer failed\n");
        return -1;
    }
    
    // Return 12-bit result
    return ((rx[1] & 0x0F) << 8) | rx[2];
}

void Joystick_init(void) {
    // Open SPI device
    spiFd = open(SPI_DEVICE, O_RDWR);
    if (spiFd < 0) {
        fprintf(stderr, "ERROR: Unable to open SPI device\n");
        return;
    }
    
    // Configure SPI mode, bits, and speed
    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;
    
    ioctl(spiFd, SPI_IOC_WR_MODE, &mode);
    ioctl(spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    
    isInitialized = true;
}

void Joystick_cleanup(void) {
    assert(isInitialized);
    
    if (spiFd >= 0) {
        close(spiFd);
        spiFd = -1;
    }
    
    isInitialized = false;
}

JoystickDirection Joystick_read(void) {
    assert(isInitialized);
    
    // Read X and Y values from ADC
    int xValue = readChannel(JOYSTICK_X_CHANNEL);
    int yValue = readChannel(JOYSTICK_Y_CHANNEL);
    
    // Debug output (remove later)
    // printf("X: %d, Y: %d\n", xValue, yValue);
    
    // Determine direction based on thresholds
    // TODO: You'll need to calibrate these thresholds based on your joystick
    
    // Check Y-axis first (up/down)
    if (yValue < THRESHOLD_LOW) {
        return JOYSTICK_DOWN;  // or UP, depending on wiring
    } else if (yValue > THRESHOLD_HIGH) {
        return JOYSTICK_UP;    // or DOWN, depending on wiring
    }
    
    // Check X-axis (left/right)
    if (xValue < THRESHOLD_LOW) {
        return JOYSTICK_LEFT;
    } else if (xValue > THRESHOLD_HIGH) {
        return JOYSTICK_RIGHT;
    }
    
    return JOYSTICK_NONE;
}