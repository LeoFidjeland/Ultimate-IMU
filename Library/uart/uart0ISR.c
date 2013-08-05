/*
	UART0
	Interrupt Service Routine
	
	Alerts the main program when an interrupt is triggered on UART0
	
	Written by Ryan Owens
	9/27/10
	
*/
#include "LPC214x.h"
#include "uart0ISR.h"

char uart0Message[250];
int m0index=0;
int row=1;
char uart0MessageComplete=0;

void ISR_UART0(void)
{
	char val = (char)U0RBR;
	
	if(val=='\r'){ 	
		uart0Message[m0index++]='\n';
		if(row==4){
			uart0Message[m0index++]='\0';
			m0index=0;
			row=1;
			uart0MessageComplete=1;
		}else{
			row+=1;
		}
	}
	else if(uart0MessageComplete==0){
		if(val != '\n')uart0Message[m0index++]=val;
	}
	
	VICVectAddr =0;	//Update the VIC priorities
}