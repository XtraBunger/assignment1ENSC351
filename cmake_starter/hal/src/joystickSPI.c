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
#define SPI_SPEED 1000000
#define SPI_MODE SPI_MODE_0

// ADC channels for joystick axes
#define JOYSTICK_X_CHANNEL 0
#define JOYSTICK_Y_CHANNEL 1

// Center values and deadzone
#define CENTER_X 2048
#define CENTER_Y 2048
#define DEADZONE_IN 1200   // Must move this far from center to register
#define DEADZONE_OUT 900   // Must return within this to reset
#define AVG_SAMPLES 4

static int spiFd = -1;
static bool isInitialized = false;
static bool isMoved = false;
static JoystickDirection lastDirection = JOYSTICK_NONE;
static int centerX = CENTER_X;
static int centerY = CENTER_Y;

static inline int iabs(int x) { 
    return x < 0 ? -x : x; 
}

// Read one channel from the ADC (MCP3208)
static int readChannel(int channel) {
    if (spiFd < 0 || channel < 0 || channel > 7) return -1;
    
    // Build the command to send to ADC
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
        perror("SPI_IOC_MESSAGE");
        return -1;
    }
    
    // Return 12-bit result
    return ((rx[1] & 0x0F) << 8) | rx[2];
}

void Joystick_init(void) {
    if (spiFd >= 0) return;
    
    // Open SPI device
    spiFd = open(SPI_DEVICE, O_RDWR);
    if (spiFd < 0) {
        perror(SPI_DEVICE);
        return;
    }
    
    // Configure SPI mode, bits, and speed
    uint8_t mode = SPI_MODE;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED;
    
    if (ioctl(spiFd, SPI_IOC_WR_MODE, &mode) == -1)
        perror("SPI_IOC_WR_MODE");
    if (ioctl(spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1)
        perror("SPI_IOC_WR_BITS_PER_WORD");
    if (ioctl(spiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
        perror("SPI_IOC_WR_MAX_SPEED_HZ");
    
    centerX = CENTER_X;
    centerY = CENTER_Y;
    isMoved = false;
    lastDirection = JOYSTICK_NONE;
    isInitialized = true;
}

void Joystick_cleanup(void) {
    if (spiFd >= 0) {
        close(spiFd);
        spiFd = -1;
    }
    isInitialized = false;
}

JoystickDirection Joystick_read(void) {
    if (spiFd < 0) return isMoved ? lastDirection : JOYSTICK_NONE;
    
    // Average multiple samples
    long sumX = 0, sumY = 0;
    int validSamples = 0;
    
    for (int i = 0; i < AVG_SAMPLES; i++) {
        int x = readChannel(JOYSTICK_X_CHANNEL);
        int y = readChannel(JOYSTICK_Y_CHANNEL);
        if (x >= 0 && y >= 0) {
            sumX += x;
            sumY += y;
            validSamples++;
        }
        usleep(500);
    }
    
    if (validSamples == 0) {
        return isMoved ? lastDirection : JOYSTICK_NONE;
    }
    
    int avgX = (int)(sumX / validSamples);
    int avgY = (int)(sumY / validSamples);
    
    // Calculate distance from center
    int dx = avgX - centerX;
    int dy = avgY - centerY;
    long distanceSquared = (long)dx * dx + (long)dy * dy;
    
    const long DEADZONE_IN_SQ = (long)DEADZONE_IN * DEADZONE_IN;
    const long DEADZONE_OUT_SQ = (long)DEADZONE_OUT * DEADZONE_OUT;
    
    // State machine with hysteresis
    if (!isMoved) {
        if (distanceSquared >= DEADZONE_IN_SQ) {
            isMoved = true;
            // Determine direction based on larger displacement
            lastDirection = (iabs(dx) >= iabs(dy))
                          ? (dx < 0 ? JOYSTICK_LEFT : JOYSTICK_RIGHT)
                          : (dy > 0 ? JOYSTICK_UP : JOYSTICK_DOWN);
        } else {
            lastDirection = JOYSTICK_NONE;
        }
    } else {
        if (distanceSquared <= DEADZONE_OUT_SQ) {
            isMoved = false;
            lastDirection = JOYSTICK_NONE;
        } else {
            // Update direction while moved
            lastDirection = (iabs(dx) >= iabs(dy))
                          ? (dx < 0 ? JOYSTICK_LEFT : JOYSTICK_RIGHT)
                          : (dy > 0 ? JOYSTICK_UP : JOYSTICK_DOWN);
        }
    }
    
    return lastDirection;
}

bool joystickMoved(void) {
    return Joystick_read() != JOYSTICK_NONE;
}

void joystickRecenter(void) {
    // This function can be used to recalibrate center if needed
    // For now, just reset state
    isMoved = false;
    lastDirection = JOYSTICK_NONE;
}