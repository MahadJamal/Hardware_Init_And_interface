// Sound.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// This routine calls the 4-bit DAC

#include "Sound.h"
#include "DAC.h"
#include "..//tm4c123gh6pm.h"

unsigned int sinewave[64] = {8,8,9,10,10,11,12,12,13,13,14,14,14,15,15,15,15,15,15,15,14,14,14,13,13,12,11,11,10,9,
9,8,7,6,6,5,4,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,3,4,5,5,6,7,8};
unsigned int DAC_value = 0;
unsigned int sound_status = 0;


// **************Sound_Init*********************
// Initialize Systick periodic interrupts
// Also calls DAC_Init() to initialize DAC
// Input: none
// Output: none
void Sound_Init(void)
{
	DAC_Init();		// initialise DAC
	NVIC_ST_CTRL_R = 0;           								// disable SysTick 
  NVIC_ST_CURRENT_R = 0;        								// any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0  
	NVIC_ST_CTRL_R = 0x07; // enable SysTic	  
}

// **************Sound_Tone*********************
// Change Systick periodic interrupts to start sound output
// Input: interrupt period
//           Units of period are 12.5ns
//           Maximum is 2^24-1
//           Minimum is determined by length of ISR
// Output: none
void Sound_Tone(unsigned long period)
{
	NVIC_ST_CTRL_R = 0; // disable SysTick
	NVIC_ST_RELOAD_R = period -1;
	sound_status = 1;
	DAC_value = 0;
	NVIC_ST_CTRL_R = 0x07; // enable SysTick
}


// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void)
{
	sound_status = 0;
	DAC_Out(0);
}


// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void)
{
	if(sound_status == 1)
	{
		DAC_Out(sinewave[DAC_value]);
		DAC_value = (DAC_value + 1) % 64;
	}   
}
