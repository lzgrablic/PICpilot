/* 
 * File:   PWMInterface.h
 * Author: Ian Hill
 *
 * Created on July 6, 2016, 3:53 PM
 */

#ifndef PWMINTERFACE_H
#define	PWMINTERFACE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "main.h"
#include "../Common/I2C.h"
#include "../Common/debug.h"

#define NUM_PWM_CHANNELS 16
#define MAX_I2C_PWM 4096
#define MIN_I2C_PWM 0

#define PWM_SLAVE_ADDRESS 0x80

struct channelData {
	char firstRegAddress;
	int pwmOffset; //Note that the scale factor is applied to the offset, so pwmOffset = 4 and scaleFactor = 4 gives a total offset of 16
	float scaleFactor;
};

void initPWMInterface();
void resetPWMInterface();

void configurePWMOutput(int channel, int offsetValue, float scaleValue);

void setPWMValue(int channel, int newValue);
void setPWMValues(int *channels, int *newValues, int numChannels);

int  getPWMValue(int channel);

/*****************************************************************************
 * Function: int* getPWMValues(int *channels, int numChannels, int *pwmValues);
 *
 * Preconditions: The initPWMInterface function in this module must have been run prior to this function call;
 *
 * Overview: This function takes a variable number of channels, and returns the current PWM value for each. Note that the third argument
 * should be empty, as it will be overwritten. This allows for client side memory management, preventing leakage or the use of a global array.
 *
 * Input:   int *channels is an array up to size 16 containing the channels the caller wants the current PWM values of.
 *          int numChannels is the number of elements in the array 'channels'.
 *          int *pwmValues is an array of identical size to 'channels' that will be overwritten with the retrieved values
 *
 * Output:  Returns a pointer to the modified integer array, with all retrieved values indexed corresponding to the channels in the *channels array.
 *
 *****************************************************************************/
int* getPWMValues(int *channels, int numChannels, int *pwmValues);

#ifdef	__cplusplus
}
#endif

#endif	/* PWMINTERFACE_H */

