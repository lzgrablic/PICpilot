/*
 * File:   LED.h
 * Author: Leonard Zgrablic
 *
 * Created on December 8, 2016, 12:39 AM
 *
 * This file contains macros for manipulating an LED on an I/O pin.
 * Argument led is the register bit for the pin.
 */

// Turn led on.
#define ledOn(led) { \
    (led) = 1; \
}

// Turn led off.
#define ledOff(led) { \
    (led) = 0; \
}

// Flash led once.
#define ledFlash(led) { \
    (led) ^= 1; \
    (led) ^= 1; \
}

// Flash led n times.
#define ledBurst(led, n) { \
    unsigned i; \
    for (i = 0; i < (n); i++) ledFlash(led); \
}
