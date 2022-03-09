#include "GPIO_LPC17xx.h"
#include "SPI_LPC17xx.h"
#include "RTE_Device.h"
#include "RTE_Components.h"
#include "Driver_SPI.h"
#include "lcd.h"

#define puerto_pin 0
#define pin_reset  8
#define pin_A0     6
#define pin_cs     18

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI * SPIdrv = &Driver_SPI1;
char buffer[512];


void retardoMicro(void){  //retardo 6us
int i;
for(i=0;i<36;i++); 
}

void retardoS(void){  //returndo 1ms
int j;
for( j=0;j<625000;j++);
}

void retardoMili(void){  //returndo 1ms
int j;
for( j=0;j<6250;j++);
}


//Resetear LCD
void resetDisplay(void){
GPIO_PinWrite(puerto_pin, pin_reset, 0); //Reset (activo a nivel bajo)
retardoMicro();
GPIO_PinWrite(puerto_pin, puerto_pin, 1); //Reset (deja de hacer el reset)
retardoMili();
}

//Inicializar display
void init(void){

GPIO_SetDir(puerto_pin,pin_reset,GPIO_DIR_OUTPUT);
GPIO_SetDir(puerto_pin,pin_A0,GPIO_DIR_OUTPUT);
GPIO_SetDir(puerto_pin,pin_cs,GPIO_DIR_OUTPUT);
GPIO_PinWrite(puerto_pin,pin_reset,1);  //reset (deja de hacer reset)
GPIO_PinWrite(puerto_pin,pin_A0,1);  //A0 = 1, lo inicializo para escrubir datos
GPIO_PinWrite(puerto_pin,pin_cs,1);  //Cs
	
	//Inicializamos SPI Driver
SPIdrv->Initialize(NULL);

  //Encendemos el periferico SPI a full use
SPIdrv->PowerControl(ARM_POWER_FULL);

//COnfiguramos el SPI con Master, 8 bits y 20MHz de frecuencia
SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB |  ARM_SPI_DATA_BITS(8),20000000); //8 bits, 20MHz

resetDisplay();
	
}


//Para escribir en el LCD datos
void wr_data(unsigned char data){
uint32_t numDatos = sizeof(data); //Mide la longitud del dato a introducir
  // Seleccionar CS = 0;
  // Seleccionar A0 = 1;
    GPIO_PinWrite(puerto_pin,pin_cs,0);//Cs a 0, activo chip
    GPIO_PinWrite(puerto_pin,pin_A0,1);//A0 a 1 escribe datos
  // Escribir un dato (data) usando la función SPIDrv->Send(…);
    SPIdrv->Send(&data,numDatos); 
// Seleccionar CS = 1;
    GPIO_PinWrite(puerto_pin,pin_cs,1);//Cs a 1, deshabilito chip
}

//Para escribir en el LCD comandos
void wr_cmd(unsigned char cmd){
  uint32_t numDatos = sizeof(cmd); //Mide la longitud del comando a introducir
  // Seleccionar CS = 0; para activar el chip puesto que es activo a niver bajo
  GPIO_PinWrite(puerto_pin,pin_cs,0);//Cs a 0, activamos chip
  // Seleccionar A0 = 0;
     GPIO_PinWrite(puerto_pin,pin_A0,0);//A0 a 0 escribe comandos
  // Escribir un comando (cmd) usando la función SPIDrv->Send(…);
     SPIdrv->Send(&cmd,numDatos);
  // Seleccionar CS = 1;
  GPIO_PinWrite(puerto_pin,pin_cs,1);//Cs a 1, deshabiito el chip select
}

//resetea y configura el lcd contraste etc
void  LCD_reset(void){
wr_cmd(0xAE);   //Display off
wr_cmd(0xA2);   //Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
wr_cmd(0xA0);   //El direccionamiento de la RAM de datos del display es la normal
wr_cmd(0xC8);   //El scan en las salidas COM es el normal
wr_cmd(0x22);   //Fija la relación de resistencias interna a 2
wr_cmd(0x2F);   //Power on
wr_cmd(0x40);   //Display empieza en la línea 0
wr_cmd(0xAF);   //Display ON
wr_cmd(0x81);   //Contraste
wr_cmd(0x17);   //Valor Contraste
wr_cmd(0xA4);   //Display all points normal
wr_cmd(0xA6);   //LCD Display normal
}

//escribe en el lcd

void copy_to_lcd(void){
	
    int i;
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB0);      // Página 0
    
    for(i=0;i<128;i++){
        wr_data(buffer[i]);
        }
  
     
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB1);      // Página 1
     
    for(i=128;i<256;i++){
        wr_data(buffer[i]);
        }
    
    wr_cmd(0x00);       
    wr_cmd(0x10);      
    wr_cmd(0xB2);      //Página 2
				
				
				
    for(i=256;i<386;i++){
        wr_data(buffer[i]);
        }
    
    wr_cmd(0x00);       
    wr_cmd(0x10);       
    wr_cmd(0xB3);      // Pagina 3
     
     
    for(i=384;i<512;i++){
        wr_data(buffer[i]);
        }
}

void limpiar_LCD(void){
	
 int i;

 for(i=0; i<512; i++){
	buffer[i]=0x00;
 }
 copy_to_lcd();
 }
