/*----------------------------------------------------------------------------
 * Name:    USART1.c
 * Purpose: Initializes USART1 on PA9 and PA10.
 * Date: 		6/18/15
 * Author:	Christopher Jordan - Denny
 *----------------------------------------------------------------------------
 * Note(s): This is created to be used with the adafruit GPS module
 *----------------------------------------------------------------------------*/

/*---------------------------------Include Statements-------------------------*/
#include "stm32l053xx.h"			//Specific Device Header
#include <stdio.h>
#include "USART1.h"
#include "Serial.h"
#include <string.h>
/*---------------------------------Define Statments---------------------------*/
#define USART1EN		0x4000		//USART1 Clock enable bit
#define IOPAEN			0x1				//Enable port A clock

//Baud rate calculation
#define __DIV(__PCLK, __BAUD)       ((__PCLK*25)/(4*__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

//Globals
char line[120] ="0";

void USART1_Init(){
	RCC->IOPENR   |=   IOPAEN;			/* Enable GPIOA clock */
	RCC->APB2ENR  |=   USART1EN;    /* Enable USART#1 clock */
	
	//Make PA8 an input with a pull up resistor
	GPIOA->MODER &= ~(( 3ul << 2* 8) | ( 3ul << 2* 8) ); /* Set to input */
	GPIOA->PUPDR &= ~(( 3ul << 2* 8) | ( 3ul << 2* 8) ); /* Set to 0 */
	GPIOA->PUPDR |=  (( 2ul << 2* 8) | ( 2ul << 2* 8) ); /* Set to pull up */
	
	//Configure PA9 to USART1_TX, PA10 to USART1_RX
  GPIOA->AFR[1] &= ~((15ul << 4* 1) | (15ul << 4* 2) );		/* Set to 0 */
  GPIOA->AFR[1] |=  (( 4ul << 4* 1) | ( 4ul << 4* 2) );		/* Set to alternate function 4 */
  GPIOA->MODER  &= ~(( 3ul << 2* 9) | ( 3ul << 2* 10) );		/* Set to 0 */
  GPIOA->MODER  |=  (( 2ul << 2* 9) | ( 2ul << 2* 10) );		/* Set to alternate function mode */
	
	USART1->BRR  = __USART_BRR(32000000ul, 9600ul);  /* 9600 baud @ 32MHz   */
  USART1->CR3    = 0x0000;                 /* no flow control */
  USART1->CR2    = 0x0000;                 /* 1 stop bit */
  USART1->CR1    = ((   1ul <<  2) |       /* enable RX  */
                    (   1ul <<  3) |       /* enable TX  */
                    (   0ul << 12) |       /* 8 data bits */
                    (   0ul << 28) |       /* 8 data bits */
                    (   1ul <<  0) );      /* enable USART */
}

/*----------------------------------------------------------------------------
  Read character from Serial Port
 *----------------------------------------------------------------------------*/
int USART1_GetChar(void) {

  if (USART1->ISR & USART_ISR_RXNE){
    return (USART1->RDR);
	}
	else return (-1);
	
}

char USART1_PutChar(char ch) {

	//Wait for buffer to be empty
  while ((USART1->ISR & USART_ISR_TXE) == 0){
			//Nop
	}
	
	//Send character
  USART1->TDR = (ch);

  return (ch);
}

void USART1_Read(void){
	
	USART1_GetChar();

}

void USART1_Send(char c){
	
	USART1_PutChar(c);
	
}
