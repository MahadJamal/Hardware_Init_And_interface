// Lab13.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// edX Lab 13 
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"

unsigned long piano_key;
unsigned long prev_piano_key;

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay(unsigned long msec);
int main(void){ // Real Lab13 
	// for the real board grader to work 
	// you must connect PD3 to your DAC output
  TExaS_Init(SW_PIN_PE3210, DAC_PIN_PB3210,ScopeOn); // activate grader and set system clock to 80 MHz
// PortE used for piano keys, PortB used for DAC        
  Sound_Init(); // initialize SysTick timer and DAC
  Piano_Init();
  EnableInterrupts();  // enable after all initialization are done
	
	piano_key = 0;
	prev_piano_key = 0;
	
  while(1)
	{  
		piano_key = Piano_In();
		
		if(piano_key != prev_piano_key)
		{
			if(piano_key == 4)
			{
				Sound_Off();
			}
			else if(piano_key == 0)
			{
				Sound_Tone(2389);
			}
			else if(piano_key == 1)
			{
				Sound_Tone(2128);
			}
			else if(piano_key == 2)
			{
				Sound_Tone(1880);
			}
			else if(piano_key == 3)
			{
				Sound_Tone(1594);
			}
			//delay(10);
		}
		
		prev_piano_key = piano_key;
  }            
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}


