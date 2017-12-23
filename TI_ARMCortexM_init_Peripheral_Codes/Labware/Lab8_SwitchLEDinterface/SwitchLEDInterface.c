// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"


// ***** Port E preprocessing address set up *****



// ***** 2. Global Declarations Section *****
unsigned long SW;


// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void InitIO(void);
void Delay_ms(unsigned long);
// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).

void InitIO()
{
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  delay++;
	GPIO_PORTE_DIR_R |= 0x02;         // PE1 output
  GPIO_PORTE_DIR_R &= ~0x01;        // PE0 input 
  GPIO_PORTE_AFSEL_R &= ~0x03;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x03;      // no analog
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // bits for PE1,PE0
  GPIO_PORTE_DEN_R |= 0x03;         // enable PE1,PE0 as digital pins

}


void Delay_ms(unsigned long ms)
{
	unsigned long count;
	while(ms > 0 ) { // repeat while still halfsecs to delay
    count = 16000; 
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    ms--;
  
}
}


int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  InitIO();
	EnableInterrupts();           // enable interrupts for the grader
	GPIO_PORTE_DATA_R |= 0x02;
	
	
  while(1){
		Delay_ms(100);
		SW = GPIO_PORTE_DATA_R & 0x01;
		if (SW == 1)
		{
			GPIO_PORTE_DATA_R ^= 0x02;
		}
		else
		{
			GPIO_PORTE_DATA_R |= 0x02;
		}
    
  }
  
}




