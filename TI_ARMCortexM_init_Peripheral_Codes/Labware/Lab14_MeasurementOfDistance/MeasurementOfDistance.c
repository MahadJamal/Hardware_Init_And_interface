// MeasurementOfDistance.c
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to periodically initiate a software-
// triggered ADC conversion, convert the sample to a fixed-
// point decimal distance, and store the result in a mailbox.
// The foreground thread takes the result from the mailbox,
// converts the result to a string, and prints it to the
// Nokia5110 LCD.  The display is optional.
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// Slide pot pin 3 connected to +3.3V
// Slide pot pin 2 connected to PE2(Ain1) and PD3
// Slide pot pin 1 connected to ground


#include "ADC.h"
#include "..//tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "TExaS.h"
void UART_ConvertDistance(unsigned long n);
void EnableInterrupts(void);  // Enable interrupts

unsigned char String[10]; // null-terminated ASCII string
unsigned long Distance;   // units 0.001 cm
unsigned long ADCdata;    // 12-bit 0 to 4095 sample
unsigned long Flag;       // 1 means valid Distance, 0 means Distance is empty

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
  //return 0;  // replace this line with real code
	return (0.488*sample + 1);
	//return 1;
}


#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(){
	NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 1999999;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts

}



// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
	ADCdata = ADC0_In();
	Distance = Convert(ADCdata);
  UART_ConvertDistance(Distance); // from Lab 11
	Flag = 1;

}

//-----------------------UART_ConvertDistance-----------------------
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
void UART_ConvertDistance(unsigned long n){
  //unsigned i = 0;
	unsigned cnt = 0;
	char buffer[11];
  do{
    buffer[cnt] = n%10;// digit
    n = n/10;
    cnt++;
  } 
  while(n);// repeat until n==0

	if(cnt > 4)
	{
		String[0] = '*' ;String[1] = '*';String[2] = '*';String[3] = '*'; String[4] = ' '; String[5] = '\0';
	}
	else
	{
		if(cnt == 1)
		{
			String[3] = buffer[cnt-1]+0x30;
			String[2] = ' ';
			String[1] = ' ';
			String[0] = ' ';
		}
		if(cnt == 2)
		{
			String[3] = buffer[cnt-2]+0x30;
			String[2] = buffer[cnt-1]+0x30;
			String[1] = ' ';
			String[0] = ' ';

		}
		if(cnt == 3)
		{
			String[3] = buffer[cnt-3]+0x30;
			String[2] = buffer[cnt-2]+0x30;
			String[1] = buffer[cnt-1]+0x30;
			String[0] = ' ';
			
		}
		if(cnt == 4)
		{
			String[3] = buffer[cnt-4]+0x30;
			String[2] = buffer[cnt-3]+0x30;
			String[1] = buffer[cnt-2]+0x30;
			String[0] = buffer[cnt-1]+0x30;
		}
		
		String[4] = ' ';
		String[5] = '\0';
	}

}

// main1 is a simple main program allowing you to debug the ADC interface
//int main1(void){ 
//  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
//  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
//  EnableInterrupts();
//  while(1){ 
//    ADCdata = ADC0_In();
//  }
//}


// once the ADC is operational, you can use main2 to debug the convert to distance
int main(void){ 
  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_NoScope);
  ADC0_Init();    // initialize ADC0, channel 1, sequencer 3
  Nokia5110_Init();             // initialize Nokia5110 LCD
	SysTick_Init();
  EnableInterrupts();
  while(1){ 
    //ADCdata = ADC0_In();
   Nokia5110_SetCursor(0, 0);
   Nokia5110_OutString(String);    // output to Nokia5110 LCD (optional)
		Flag = 0;
  }
}


// once the ADC and convert to distance functions are operational,
// you should use this main to build the final solution with interrupts and mailbox
//int main(void){ 
//  volatile unsigned long delay;
//  TExaS_Init(ADC0_AIN1_PIN_PE2, SSI0_Real_Nokia5110_Scope);
// initialize ADC0, channel 1, sequencer 3
// initialize Nokia5110 LCD (optional)
// initialize SysTick for 40 Hz interrupts
// initialize profiling on PF1 (optional)
                                    //    wait for clock to stabilize

// EnableInterrupts();
// print a welcome message  (optional)
//  while(1){ 
// read mailbox
// output to Nokia5110 LCD (optional)
//		
//  }
//}
