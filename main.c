#include <msp430.h>
#include "dtc.h"
#include <libemb/conio/conio.h>

unsigned int num = 0;
unsigned int toggle;		//globals
int digitChange;
int colorChange;

//p1 digit
int hexadecimalPort1Show [16] = {0,0,0,0,0,2,2,0,0,0,0,2,2,0,2,2};
//p2 digit
int hexadecimalPort2Show [16] = {0b01000000,0b11101001,0b00100100,0b10100000,0b10001001,0b10000000,0b00000000,0b11101000,0b00000000,0b10000000,0b00001000,0b00000001,0b00100101,0b00100001,0b00000100,0b00001100};

int RGBPort1[3] = {0b00100010, 0b01000100, 0b10000110};	//rgb for p1
int RGBPort2[3] = {0b00101101, 0b10000000, 0b00000011};	//rgb for p2

int main(void){
	
	WDTCTL   =   WDTPW  |  WDTHOLD;
	BCSCTL1  =   CALBC1_1MHZ;
	DCOCTL   =   CALDCO_1MHZ;

	P1DIR = 0b11100111;	//all but two as inputs
	P1OUT = 0b00001110;	//initially turn on red LED, all digs off
	
	P1SEL   |= BIT2;
	P1REN   |= BIT3;		//enable button interrupt set high
	P1IE    |= BIT3;
	P1IES   |= BIT3; 
	P1IFG   &= ~BIT3;	//clear interrupt

	P2SEL   &=  ~(BIT6+BIT7);	//clear for use
	P2SEL   |=  (BIT1+BIT4);

	P2DIR   |= 0b11111111;	//active low for 7-segment display, going up
	P2OUT   |= 0b11101101;

	TA0CCTL0 = CCIE;			//timer a0
	TA0CTL = TASSEL_2 | MC_1 | ID_0;
	TA0CCR0 = 1024;
	TA0CCTL1 = OUTMOD_7;

	TA1CTL = TASSEL_2 | MC_1 | ID_0;	//timer a1
	TA1CCR0 = 1024;
	TA1CCTL1 = OUTMOD_7;
	TA1CCTL2 = OUTMOD_7;

	initialize_dtc(INCH_4, &num);

	_BIS_SR(LPM0_bits | GIE);	//global interrupts 
}
//7 segment
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR(void){

	toggle = num;
	if(colorChange == 0){
		TA0CCR1 = toggle;
		cio_printf("red");
	}
	else if(colorChange == 1){
		TA1CCR1 = toggle;
		cio_printf("blue");
	}
	else{
		cio_printf("green");
		TA1CCR2 = toggle;
	}

	P1OUT &= ~(BIT5+BIT6+BIT7);	//clear the display

	toggle >>=2;
	toggle &= 0xff;
	int remainder[2] = {0};
	int i = 0;


	while(toggle > 0){
		remainder[i] = toggle%16;
		toggle = toggle/16;
		i++;
	}

	i = 0;

	if(digitChange == 0){
		if(colorChange == 0){
		P1OUT = RGBPort1[0] | BIT3;	//green show
		P2OUT = RGBPort2[0];
		}
		else if(colorChange == 1){	//blue show
		P1OUT = RGBPort1[1] | BIT3;
		P2OUT = RGBPort2[1];
		}
		else{
			P1OUT = RGBPort1[2] | BIT3;	//red show
			P2OUT = RGBPort2[2];
		}
	
		P1OUT |= BIT5;	//connected to 3rd digit

	}
	else if(digitChange == 1){
		P1OUT = hexadecimalPort1Show[remainder[1]] | BIT3;
		P2OUT = hexadecimalPort2Show[remainder[1]];
		P1OUT |= BIT6;	//connected to 2nd digit
	}
	else if(digitChange == 2){
		P1OUT = hexadecimalPort1Show[remainder[0]] | BIT3;
		P2OUT = hexadecimalPort2Show[remainder[0]];
		P1OUT |= BIT7;			//connected to 3rd digit
		digitChange = -1;
	}
	digitChange++;

}

#pragma vector=PORT1_VECTOR
__interrupt void buttonToggle(void){


	if(colorChange == 0){		//green print
		cio_printf("green");
	}
	else if(colorChange == 1){	//blue print
		cio_printf("blue");
	}
	else if(colorChange == 2){	//red print
		cio_printf("red");
		colorChange = -1;
	}
	while(!(BIT3 & P1IN)){}//input delay
	__delay_cycles(32000); 

	P1IFG &= ~BIT3;	//clear
	colorChange++;


}


	






