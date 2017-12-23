// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
//void PLL_Init(void);
void SysTick_Init(void);
void SysTick_Wait10ms(unsigned long delay);

struct STATE{
	unsigned char out;
	unsigned short wait;
	unsigned char Next[9];
};

typedef const struct STATE FSM;

#define GOE 0
#define StopE 1
#define GON 2
#define StopN 3
#define Walk 4
#define FlashWalk1 5
#define FlashWalk2 6
#define FlashWalk3 7
#define FlashWalk4 8


FSM TrafficLightFSM[9] = {
	{0x32, 100, {GOE, 	 GOE,  				 StopE, 			StopE, 			StopE, 	StopE, 			StopE, 			StopE			}},
	{0x52, 50, {GOE, 	 GOE,   			 GON, 				GON, 				Walk, 	Walk, 			GON, 				Walk				}},
	{0x86, 100, {GON, 	 StopN,  			 GON, 				StopN, 			StopN, 	StopN, 			StopN, 			StopN			}},
	{0x8A, 50, {GON, 	 GOE,   			 GON, 				GOE, 			  Walk, 	GOE, 				Walk, 			GOE			}},
	{0x91, 100, {Walk,  FlashWalk1,   FlashWalk1,  FlashWalk1, Walk, 	FlashWalk1, FlashWalk1, FlashWalk1}},
	{0x90, 20, {Walk,  FlashWalk2,   FlashWalk2,  FlashWalk2, Walk, 	FlashWalk2, FlashWalk2, FlashWalk2}},
	{0x92, 20, {Walk,  FlashWalk3,   FlashWalk3,  FlashWalk3, Walk, 	FlashWalk3, FlashWalk3, FlashWalk3}},
	{0x90, 20, {Walk,  FlashWalk4,   FlashWalk4,  FlashWalk4, Walk, 	FlashWalk4, FlashWalk4, FlashWalk4}},
	{0x92, 20, {Walk,  GOE,   			 GON, 				GOE, 				Walk, 	GOE, 				GON, 				GON				}},
};

void PortB_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x02;          // 1) activate Port B
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PB7-0
  GPIO_PORTB_AMSEL_R = 0x00;       // 3) disable analog function on PB7-0
  GPIO_PORTB_PCTL_R = 0x00000000;  // 4) configure PB7-0 as GPIO
  GPIO_PORTB_DIR_R = 0xFF;         // 5) make PB7-0 out
  GPIO_PORTB_AFSEL_R = 0x00;       // 6) disable alt funct on PB7-0
  GPIO_PORTB_DR8R_R = 0xFF;        // enable 8 mA drive on PB7-0
  GPIO_PORTB_DEN_R = 0xFF;         // 7) enable digital I/O on PB7-0
}

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}

void PortE_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x10;          // 1) activate Port E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PE1-0
  GPIO_PORTE_AMSEL_R &= ~0x03;     // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF;// 4) configure PE1-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x07;       // 5) make PE2-0 in
  GPIO_PORTE_AFSEL_R &= ~0x03;     // 6) disable alt funct on PE1-0
  GPIO_PORTE_DEN_R |= 0x07;        // 7) enable digital I/O on PE2-0
}

void output(unsigned char outputpattern)
{
	unsigned char out;
	out = (outputpattern & 0x02) | ((outputpattern << 3 ) & 0x08);
	GPIO_PORTF_DATA_R = out;
	GPIO_PORTB_DATA_R = ((outputpattern & 0xFC)>>2);
}

// ***** 3. Subroutines Section *****
unsigned char cstate;
unsigned char input;
int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	
 
	PLL_Init();                      
  SysTick_Init();
  
  EnableInterrupts();
	PortB_Init();
	PortF_Init();
	PortE_Init();
	
	cstate = GOE;
  while(1){
		output(TrafficLightFSM[cstate].out);
		
		SysTick_Wait10ms(TrafficLightFSM[cstate].wait);
		
		input = GPIO_PORTE_DATA_R&0x07;
		
		cstate = TrafficLightFSM[cstate].Next[input];
		
  }
}




//-------------PLL INITIALIZATION ---------------//
/*
void PLL_Init(void){
  // 0) Use RCC2
  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}

*/


///---------------SysTick Wait --------------------------------------////
#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}




