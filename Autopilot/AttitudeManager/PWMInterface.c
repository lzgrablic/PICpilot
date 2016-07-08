/*
* File:   PWMInterface.c
* Author: Ian Hill
*
* Created on June 29, 2016
*/

#include "PWMInterface.h"
#include <math.h>

struct channelData pwmChannel[NUM_PWM_CHANNELS - 1]; //Indexed from 0 for PWM outputs so subtract 1
char pwmData[3];
char dataByte;
int onPoint, offPoint;
char dataLength; //In bytes

/*
* The PCA9685 has 4 8bit registers for each of the 16 outputs it supports, 2 for ON point and 2 for OFF point
* The OFF and ON points are values between 0 and 4095 where the output is triggered to 0 and 1 states respectively (stored in bits 0 to 7 of less significant register and bits 0 to 3 of more significant)
* Race conditions occur if the ON point == OFF point, so bit 4 in the more significant registers toggle constant 1 or 0, constant OFF taking precedence
* This module controls the square wave by setting OFF point to 0, and shifting ON point as needed. Note that the offset can phase shift this square wave if necessary in the future
* Input for PWM values is by default between 0 and 1024, so a scale factor of 4 is applied to reach the input values needed for the registers, however this scaling is adjustable if desired
*
* The datasheet for the board can be found here https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
*/

void initPWMInterface() {
	//Initialize the connection
	initI2C();

	//Check that the device is present, an error is sent if not connected correctly
	if (checkDevicePresence(PWM_SLAVE_ADDRESS, 0x00) == 0) {
#if DEBUG
		error("PWM extension (PCA9685) board not found");
#endif
        return;
	}

	//Set up the structs containing the data for the PWM outputs, default scaleFactor is 4 to allow input to be between 0 and 1024, to match the PWM module
    int i = 0;
	while (i < NUM_PWM_CHANNELS) {
		pwmChannel[i].firstRegAddress = ((4 * i) + 6);
		pwmChannel[i].pwmOffset = 0;
		pwmChannel[i].scaleFactor = 4;
        i++;
	}

	//PCA9685 board has a number of stored settings, we configure those settings here, enabling auto-increment to speed up setting of PWM values
	dataByte = 0x21;
	dataLength = 1;
	sendMessage(PWM_SLAVE_ADDRESS, 0x00, &dataByte, dataLength, '0');
}
	//Since all PWM outputs default to full off on restart, any that should have a value other than 0 by default would have to be set here

void resetPWMInterface() {
	//This command sets the reset bit to 1, performing a restart of the board, which returns all settings to default and all PWM outputs to 0
	dataByte = 0xA1;
    dataLength = 1;
	sendMessage(PWM_SLAVE_ADDRESS, 0x00, &dataByte, '1', '0');
}

void configurePWMOutput(int channel, int offsetValue, float scaleValue) {
    //Error check for invalid channel number
    if (channel >= NUM_PWM_CHANNELS) {
#if DEBUG
		error("PWMInterface: invalid PWM channel argument.");
#endif 
        return;
    }
	//Offset value phase shifts the square wave of the PWM output, doesn't change the duty cycle, so has minimal practical use
	pwmChannel[channel].pwmOffset = offsetValue;
	//Scale value changes how much the argument PWM is compressed/expanded
	pwmChannel[channel].scaleFactor = scaleValue;
}

void setPWMValue(int channel, int pwmValue) {
    //Error check for invalid channel number
    if (channel >= NUM_PWM_CHANNELS) {
#if DEBUG
		error("Attempted read of invalid PWM output channel.");
#endif 
        return;
    }
	//Max/min PWM values need to be handled differently to prevent race conditions on the PCA9685
	if (pwmValue * pwmChannel[channel].scaleFactor >= MAX_I2C_PWM) {
		pwmData[0] = 0x00;
		pwmData[1] = 0x10;
		pwmData[2] = 0x00;
		pwmData[3] = 0x00;
		dataLength = 4;
		sendMessage(PWM_SLAVE_ADDRESS, pwmChannel[channel].firstRegAddress, pwmData, dataLength, '0');
	}
	else if (pwmValue <= 0) {
		pwmData[0] = 0x00;
		pwmData[1] = 0x00;
		pwmData[2] = 0x00;
		pwmData[3] = 0x10;
		dataLength = 4;
		sendMessage(PWM_SLAVE_ADDRESS, pwmChannel[channel].firstRegAddress, pwmData, dataLength, '0');
	}
	else {
		//Higher PWM corresponds to a lower ON point, e.g. PWM 1 -> ON point of 4095
		pwmValue = (pwmValue - (int)floor(4096 / pwmChannel[channel].scaleFactor)) * -1;

		onPoint = floor((pwmValue + pwmChannel[channel].pwmOffset) * pwmChannel[channel].scaleFactor);
		offPoint = floor(pwmChannel[channel].pwmOffset * pwmChannel[channel].scaleFactor);
		//We perform a bit operation with 0000000011111111 to extract the least significant byte, and bit-shift to get the most significant
		pwmData[0] = onPoint & 0xFF;
		pwmData[1] = onPoint >> 8;
		pwmData[2] = offPoint & 0xFF;
		pwmData[3] = offPoint >> 8;
		dataLength = 4;
		sendMessage(PWM_SLAVE_ADDRESS, pwmChannel[channel].firstRegAddress, pwmData, dataLength, '0');
	}

}

void setPWMValues(int *channels, int *pwmValues, int numChannels) {
	//Each PWM channel to be set is cycled through, and the individual set PWM function is called
    int i = 0;
	while (i < numChannels) {
		setPWMValue(channels[i], pwmValues[i]);
        i++;
	}
}

int getPWMValue(int channel) {
    //Error check for invalid channel number
    if (channel >= NUM_PWM_CHANNELS) {
#if DEBUG
		error("PWMInterface: invalid PWM channel argument.");
#endif 
        return -1;
    }
	//Read the data from the 2 ON point registers for the desired channel, the second register value is bit-shifted left so they add together correctly
	int readValue1 = (int)readMessage(PWM_SLAVE_ADDRESS, pwmChannel[channel].firstRegAddress);
	int readValue2 = (int)readMessage(PWM_SLAVE_ADDRESS, pwmChannel[channel].firstRegAddress + 1) << 8;

	//Return the PWM value, which is the value of the registers minus the offset, and divided by the scale factor
	return (int)floor(((readValue2 + readValue1) / pwmChannel[channel].scaleFactor) - pwmChannel[channel].pwmOffset - (4096 / pwmChannel[channel].scaleFactor) * -1);
}

int* getPWMValues(int *channels, int numChannels, int *pwmValues) {
	//For each desired channel, we read the PWM values by calling the single channel get PWM
    int i = 0;
	while (i < numChannels) {
		pwmValues[i] = getPWMValue(channels[i]);
        i++;
	}
	return pwmValues;
}
