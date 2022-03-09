/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2014 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "cmsis_os.h"                   /* CMSIS RTOS definitions             */
#include "rl_net.h"                     /* Network definitions                */

#include "Board_GLCD.h"
#include "GLCD_Config.h"
#include "Board_LED.h"
#include "Board_Buttons.h"
#include "Board_ADC.h"
#include "lcd.h"


#include "GPIO_LPC17xx.h"
#include "Arial12x12.h"
#include "string.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

bool LEDrun;
bool LCDupdate;
char lcd_text[2][20+1];


static void Display (void const *arg);

osThreadDef(Display, osPriorityNormal, 1, 0);


extern char buffer[512];
int posicionL1 = 0;
int posicionL2 = 0;

int EscribeLetra_L1(int letra){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	comienzo = 25 * ( letra - ' ');
	
	for( i = 0; i < 12; i++){
		
		valor1 = Arial12x12[comienzo + i*2 + 1];
		valor2 = Arial12x12[comienzo + i*2 + 2];
			
		buffer[i + posicionL1] = valor1;
		buffer[i+128 + posicionL1] = valor2;
	}
	copy_to_lcd();
	posicionL1 = posicionL1 + Arial12x12[comienzo];
  
	return 0;
}

int EscribeLetra_L2(int letra){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	comienzo =25 * ( letra - ' ');
	
	for( i = 0; i < 12; i++){
		
		valor1 = Arial12x12[comienzo + i*2 + 1];
		valor2 = Arial12x12[comienzo + i*2 + 2];
			
		buffer[i+256 + posicionL2] = valor1;
		buffer[i+384 + posicionL2] = valor2;
	
	}
	copy_to_lcd();
	posicionL2 = posicionL2 + Arial12x12[comienzo];
  
	return 0;
}



 void imprimir1 (char cadena[]){
	 int e;
	 int i;
	 for( e = 0; e < 256; e++){
		 buffer[e] = 0x00;
	 }
	 for( i = 0; i<strlen(cadena); i++){	
		if(posicionL1<116){
		   EscribeLetra_L1 (cadena [i]);		   
		}
		if(posicionL1>=116){
		   i = strlen(cadena);
			 for( e = 0; e < 256; e++){
		     buffer[e] = 0x00;
	     }
	   }
	 }
   if(posicionL1 >= 116){	
      posicionL1  = 0;					 
	 
	}
}
 
 
 void imprimir2 (char cadena[]){
	 int e;
	 int i;
	 int s;
		for( e = 256; e < 512; e++){
		  buffer[e] = 0;
	  } 
	  for( i = 0; i<strlen(cadena); i++){	
      if(posicionL2 < 116){			 
		    EscribeLetra_L2 (cadena [i]);
		  }
			if(posicionL2 >= 116){				
				  i = strlen(cadena);
				 for( s = 256; s < 512; s++){
		        buffer[s] = 0;
	      }	
			}
		}	 
		if(posicionL2 >= 116){     		
       posicionL2 = 0;							 
		}
}



/// Read analog inputs
uint16_t AD_in (uint32_t ch) {
  int32_t val = 0;

  if (ch == 0) {
    ADC_StartConversion();
    while (ADC_ConversionDone () < 0);
    val = ADC_GetValue();
  }
  return (val);
}

/// Read digital inputs
uint8_t get_button (void) {
  return (Buttons_GetState ());
}

/// IP address change notification
void dhcp_client_notify (uint32_t if_num,
                         dhcpClientOption opt, const uint8_t *val, uint32_t len) {
  if (opt == dhcpClientIPaddress) {
    // IP address has changed
    sprintf (lcd_text[0],"IP address:");
    sprintf (lcd_text[1],"%s", ip4_ntoa (val));
    LCDupdate = true;
  }
}

/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
static void Display (void const *arg) {

	init();	
	LCD_reset();  	
  limpiar_LCD();


  while(1) {
    if (LCDupdate == true) {

			imprimir1(lcd_text[0]);
			imprimir2(lcd_text[1]);

      LCDupdate = false;
    }
    osDelay (250);
  }
}


/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
int main (void) {
  LED_Initialize();

  Buttons_Initialize ();
  ADC_Initialize     ();
  net_initialize     ();

  osThreadCreate (osThread(Display), NULL);
	
  //LED_On(1);
//  LED_On(2);
//  LED_On(3);
//  LED_On(4);

  while(1) {
		
		ADC_GetValue();
		
    net_main ();
    osThreadYield ();
  }
}
