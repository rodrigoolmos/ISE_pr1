
#include "LPC17xx.h"
#include "PIN_LPC17xx.h"

#include "Board_ADC.h"

#define ADC_RESOLUTION        12U            /* Number of A/D converter bits  */
#define CLKDIV     1 //ADC clock-divider (ADC_CLOCK=PCLK/CLKDIV+1) = 12.5Mhz @ 25Mhz PCLK
#define PWRUP      (1<<21) //setting it to 0 will power it down
#define ADCR_SETUP_SCM ((CLKDIV<<8) | PWRUP)
#define SEL_AD0_0  (1<<4) //Select Channel AD0.0 

static volatile uint16_t AD_last;            /* Last converted value          */
static volatile uint8_t  AD_done;            /* AD conversion done flag       */

/* Interrupt handler prototype */
void ADC_IRQHandler (void);

/* ADC pins:
   - AD0.5: P0_25
*/

static const PIN ADC_PIN[] = {
  {1U, 30U},
};

/**
  \fn          int32_t ADC_Initialize (void)
  \brief       Initialize Analog-to-Digital Converter
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_Initialize (void) {
	
  LPC_SC->PCONP |= ((1U << 12) | (1U << 15)); /* enable power to ADC & IOCON  */

	//SystemInit(); //Gets called by Startup code, sets CCLK=100Mhz, PCLK=25Mhz

	LPC_ADC->ADCR =  ADCR_SETUP_SCM | SEL_AD0_0;
	LPC_PINCON->PINSEL3 |= (1<<28)  | (1<<29); //select AD0.0 for P0.23

  LPC_ADC->ADINTEN =  ( 1U <<  8);           /* global ADC enable interrupt   */

  NVIC_EnableIRQ(ADC_IRQn);                  /* enable ADC Interrupt          */
	
	  return 0;
}

/**
  \fn          int32_t ADC_Uninitialize (void)
  \brief       De-initialize Analog-to-Digital Converter
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_Uninitialize (void) {

  NVIC_DisableIRQ (ADC_IRQn);                /* Disable ADC Interrupt         */
  LPC_ADC->ADINTEN &= ~( 1U <<  8);          /* Disable global ADC enable interrupt */

  PIN_Configure (ADC_PIN[0].Portnum, ADC_PIN[0].Pinnum, 0U, 0U, 0U);
  
  LPC_SC->PCONP &= ~(1U << 12);              /* Disable ADC clock             */

  return 0;
}

/**
  \fn          int32_t ADC_StartConversion (void)
  \brief       Start conversion
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t ADC_StartConversion (void) {

  LPC_ADC->ADCR &= ~( 7U << 24);             /* stop conversion               */
  LPC_ADC->ADCR |=  ( 1U << 24);             /* start conversion              */

  return 0;
}

/**
  \fn          int32_t ADC_ConversionDone (void)
  \brief       Check if conversion finished
  \returns
   - \b  0: conversion finished
   - \b -1: conversion in progress
*/
int32_t ADC_ConversionDone (void) {
  return (AD_done ? 0 : -1);
}

/**
  \fn          int32_t ADC_GetValue (void)
  \brief       Get converted value
  \returns
   - <b> >=0</b>: converted value
   - \b -1: conversion in progress or failed
*/
int32_t ADC_GetValue (void) {

  if (AD_done) {
    AD_done = 0U;
    return AD_last;
  }
  return -1;
}

/**
  \fn          uint32_t ADC_GetResolution (void)
  \brief       Get resolution of Analog-to-Digital Converter
  \returns     Resolution (in bits)
*/
uint32_t ADC_GetResolution (void) {
  return ADC_RESOLUTION;
}

/**
  \fn          void ADC_IRQHandler (void)
  \brief       Analog-to-Digital Converter Interrupt Handler
*/
void ADC_IRQHandler(void) {
  volatile uint32_t adstat;

  adstat = LPC_ADC->ADSTAT;                  /* Read ADC clears interrupt     */

  AD_last = (LPC_ADC->ADGDR >> 4) & 0xFFFU;  /* Store converted value         */

  AD_done = 1U;
}
