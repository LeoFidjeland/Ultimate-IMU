/*
	FGPMMOPA6H Library
	
	created 6 Aug 2013
	by Leo Fidjeland
	
*/
#include "FGPMMOPA6H.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "LPC21xx_SFE.h"
#include "main.h"

extern "C"{
#include "serial.h"
#include "rprintf.h"
//#include "delay.h"
}

#define GLOBALOBJECT
//#define DEBUG

cFGPMMOPA6H gps(0, 12);

cFGPMMOPA6H::cFGPMMOPA6H(unsigned int port, unsigned int en_pin)
{
	_en_pin=en_pin;
	_port=port;
}

void cFGPMMOPA6H::begin(unsigned int baud_rate)
{
	//Make sure the Enable pin is set up correctly
	PINSEL0 &= ~(3<<((_en_pin+1)*2));	//Set the pin as GPIO in the Pin Select Register
	IODIR0 |= (1<<_en_pin);	//Sets the enable pin as an output
	IOCLR0 = (1<<_en_pin);		//Turns the GPS module off by default.
	
	if(_port == 0)
		init_serial0(baud_rate);
	else if(_port == 1)
		init_serial1(baud_rate);
		
	updated=0;
}

void cFGPMMOPA6H::on(void)
{
	IOSET0 = (1<<_en_pin);	//Turns the GPS module on by setting the enable pin high.
	//delay_ms(500);
}

void cFGPMMOPA6H::off(void)
{
	IOCLR0 = (1<<_en_pin);	//Turns the GPS module off by setting the enable pin low.
}

void cFGPMMOPA6H::enable( const char type[])
{
	configure(type);
}

void cFGPMMOPA6H::enable(void)
{
	const char type[6] = {1,1,1,1,1,1};
	configure(type);	//Enable all of the GPS messages by setting frequencies to 1
}

void cFGPMMOPA6H::disable(void)
{
	const char type[6] = {0,0,0,0,0,0};
	configure(type);	//Disable all of the GPS messages by setting frequencies to 0
}

int cFGPMMOPA6H::parse(char * inmessage)
{
	char delim[] = ",";	//GPS message delimiter is a comma
	char msg_cksum=0;
	char calc_cksum=0;
	char ck_msg[256];
	
	msg_cksum = (int)strtol(&inmessage[strlen(inmessage)-2], NULL, 16);
	//rprintf("\r%02x\r", msg_cksum);	

	*ck_msg='\0';
	strncat(ck_msg, &inmessage[1], strlen(inmessage)-4); 
	//rprintf("%s\r", ck_msg);
	GPS_CHECKSUM(ck_msg, calc_cksum);
	//rprintf("%02x\r", calc_cksum);
	if(calc_cksum != msg_cksum)return 0;
	
	strcpy(&ck_msg[0], inmessage);
	char * result = NULL;
	result = strtok( &ck_msg[0], delim);
	data.time = strtok(NULL, delim);
	data.status = strtok(NULL, delim);
	if(*data.status == 'A')
	{
		data.latitude.position = strtok(NULL, delim);
		data.latitude.direction = strtok(NULL, delim);
		data.longitude.position = strtok(NULL, delim);
		data.longitude.direction = strtok(NULL, delim);
		data.speed = strtok(NULL, delim);
		data.heading = strtok(NULL, delim);
		data.date = strtok(NULL, delim);
		data.mag = strtok(NULL, delim);
		data.cksum = strtok(NULL, delim);
	}
	else return 0;	
	
	return 1;
}

void cFGPMMOPA6H::configure(const char type[])
{
	char gps_string[46];
	unsigned char cksum=0;
	
	sprintf( gps_string, "PMTK314,%01d,%01d,%01d,%01d,%01d,%01d,0,0,0,0,0,0,0,0,0,0,0,0,0", type[GLL], type[RMC], type[VTG], type[GGA], type[GSA], type[GSV] );
	GPS_CHECKSUM(gps_string,cksum);

	if(_port == 1)
	{
		putc_serial1('$');
		for(int i=0; gps_string[i] != '\0'; i++)
		{
			putc_serial1(gps_string[i]);
		}
		putc_serial1('*');
		putc_serial1(((cksum&0xF0)>>4)+'0');
		putc_serial1((cksum&0x0F)+'0');
		putc_serial1('\r');
		putc_serial1('\n');
#ifdef	DEBUG		
		rprintf("$");
		for(int i=0; gps_string[i] != '\0'; i++)
		{
			rprintf("%c", gps_string[i]);
		}
		rprintf("*");
		rprintf("%c",((cksum&0xF0)>>4)+'0');
		rprintf("%c", (cksum&0x0F)+'0');
		rprintf("\r\n");
#endif
	}
	else if(_port == 0)
	{
		putc_serial0('$');
		for(int i=0; gps_string[i] != '\0'; i++)
		{
			putc_serial0(gps_string[i]);
		}
		putc_serial0('*');
		putc_serial0(((cksum&0xF0)>>4)+'0');
		putc_serial0((cksum&0x0F)+'0');
		putc_serial0('\r');
		putc_serial0('\n');
	}	
}	
