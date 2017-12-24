// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0 SS3 to be triggered by
// software and trigger a conversion, wait for it to finish,
// and return the result. 
// Daniel Valvano
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

#include "ADC.h"
#include "..//tm4c123gh6pm.h"

// This initialization function sets up the ADC 
// Max sample rate: <=125,000 samples/second
// SS3 triggering event: software trigger
// SS3 1st sample source:  channel 1
// SS3 interrupts: enabled but not promoted to controller
void ADC0_Init(void){ 
  volatile unsigned long delay;
//  SYSCTL_RCGC0_R |= 0x00010000; // 1) activate ADC0 (legacy code)
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // 1) activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_AMSEL_R = 0x04;	
  GPIO_PORTE_DIR_R &= ~0x04;         // 5) PE2 IN
  GPIO_PORTE_AFSEL_R |= 0x04;      // 6) disable alt funct on PE2
  GPIO_PORTE_DEN_R &= ~0x04;         // 7) disable digital I/O on PE2
	
	GPIO_PORTE_PCTL_R &= ~0xF0000000; // 4) PCTL GPIO on PA7
	
	ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x0123;          // 9) Sequencer 3 is highest priority //Sequence prioritizer
  ADC0_ACTSS_R &= ~0x0008;        // 10) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;         // 11) seq3 is software trigger //ADC is initited by setting SSn bit in PSSI
  
	//ADC0_SSMUX3_R = 0x1;         
  ADC0_SSMUX3_R &= ~0x000F;       // 11.1) clear SS3 field
  ADC0_SSMUX3_R += 1;             //    set channel Ain1 (PE2). 12) set channel AIN1->PE2 for SS3. 8 possible. Only using
	
	ADC0_SSCTL3_R = 0x0006;         // 13)  no TS0 D0, yes IE0 END0 // 0 1 1 0 //First sample
	
  ADC0_IM_R &= ~0x0008;           // 14) disable SS2 interrupts // Interrupt MASK // The status of SS3 in ADC0 does not effect its interrupt status
	
  ADC0_ACTSS_R |= 0x0008;         // 13) enable sample sequencer 3 //ADC0 SS3 Enable// 16 bit to show of ADC is busy or idle
	
	
}


//------------ADC0_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC0_In(void){  
  return 0; // replace this line with proper code
}
