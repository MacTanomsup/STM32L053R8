#include "stm32l053xx.h"                  // Device header
#include "I2C.h"
#include "Timing.h"

//#define  Temp_Hum_Address  0x0000005F 		//Slave Address	for temp humidity sensor

uint32_t I2C1_RX_Data = 0;

/**
  \fn          void I2C1_IRQHandler(void)
  \brief       The Global handler for I2C1, RX is the only enabled interrupt
*/

//void I2C1_IRQHandler(void){
//	
//	//Handle RX
//	if((I2C1->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE){
//		//Data = recieve register, will clear RXNE flag once read
//		I2C1_RX_Data = I2C1->RXDR;
//	}
//}

/**
  \fn          void I2C_Init(void)
  \brief			I2C initialization
							*PORTB-8: SCL
							*PORTB-9: SDA
							*Digital Noise filter with supression of 1 I2Cclk
							*fast Mode @400kHz with I2CCLK = 16MHz, rise time = 100ns, fall time = 10ns
*/

void I2C_Init(void){
	
	int SCL = 8;											//SCL pin on PORTB alt fnc 4
	int SDA = 9;											//SDA pin on PORTB alt fnc 4
	
	//Reset Control Clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;			// Enable the clock for I2C1
	
	//Enable Port Clock
	RCC->IOPENR |=  (1UL << 1);							// Enable GPIOB clock
	
	//interrupt init
	NVIC_EnableIRQ(I2C1_IRQn);
	NVIC_SetPriority(I2C1_IRQn,0);
	
	//GPIO Setup
	I2C1->CR1 |= (1<<8);		//Digital Noise filter with supression of 1 I2Cclk
	GPIOB->MODER = ~((~GPIOB->MODER) | ((1 << 2*SCL) + (1 << 2*SDA)));						// Alternate Mode PORTB pin 8 and pin 9
	GPIOB->AFR[1] = 0x00000044;																										// Set alternate function 4 for both pin 8 an 9 AF[1] HIGH PINS AF[0] LOW PINS	
	
	//Configuration for I2C
	I2C1->TIMINGR = (uint32_t)0x00503D5A;			//Standard Mode @100kHz with I2CCLK = 16MHz, rise time = 100ns, fall time = 10ns
	I2C1->CR1 |= I2C_CR1_PE;		//Enable I2C1 peripheral and the RX interrupt
}

/**
  \fn					void I2C_Write(uint32_t Data)
  \brief			Starts a I2C write sequence
							Configuration
							*		AUTOEND: a STOP condition is automatically sent when NBYTES data are transferred.
							*		1 byte to be recieved
							*		Slave address bits 7:1, written with the 7 bit slave address mode
	\param			uint32_t Device: The slave address of the device you would like to write to
	\param			uint32_t Data: The data that you would like write (most likely a register value)
*/

void I2C_Write(uint32_t Device,uint32_t Data){
	
	I2C1->CR2 ^= (0 ^ I2C1->CR2) & I2C_CR2_RD_WRN;		//Clear WRN to enable writing
	
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	while((I2C1->ISR & I2C_ISR_TXE) == 0);
	
	/*
	AUTOEND: a STOP condition is automatically sent when NBYTES data are transferred.
	WRN: Request a write transfer
	(1<<16): Means NBYTES is equal to 1. 1 byte to be recieved
	(I2C1_SLAVE_ADDRESS<<1): Slave address bit 7:1, written with the 7 bit slave address
	*/
	I2C1->CR2 |= I2C_CR2_AUTOEND | (1UL<<16) | (Device<<1);
	
	I2C1->CR2 |= I2C_CR2_START;		//Start communication
	
	//Check Tx empty before writing to it
	if((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2C1->TXDR = Data;
	}
}

/**
  \fn					uint32_t I2C_Read(void)
  \brief			Starts a I2C read sequence (More than likely followed by I2C_Write())
							Configuration
							*		AUTOEND: a STOP condition is automatically sent when NBYTES data are transferred.
							*		WRN: Request a read transfer
							*		1 byte to be recieved
							*		Slave address bits 7:1, written with the 7 bit slave address mode
	\returns		uint32_t I2C1_RX_Data: The data read
*/

uint32_t I2C_Read(uint32_t Device){
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	
	/*
	AUTOEND: a STOP condition is automatically sent when NBYTES data are transferred.
	WRN: Request a read transfer
	(1<<16): Means NBYTES is equal to 1. 1 byte to be recieved
	(I2C1_SLAVE_ADDRESS<<1): Slave address bit 7:1, written with the 7 bit slave address
	*/
	I2C1->CR2 |= I2C_CR2_AUTOEND | (1UL<<16) | I2C_CR2_RD_WRN | (Device<<1);
	
	I2C1->CR2 |= I2C_CR2_START;		//Start communication
	
	return(I2C1_RX_Data);
}

/**
  \fn				void Reset_I2C(void)
  \brief		I2C Reset, clears and then sets I2C_CR1_PE
*/

void Reset_I2C(void){
	int x = 0;		//1 to set bit, 0 to clear bit
	
	I2C1->CR1 ^= (-x ^ I2C1->CR1) & I2C_CR1_PE;		//Clear bit for reset
	I2C1->CR1 |= I2C_CR1_PE;
}

/**
  \fn					uint32_t I2C_Read_Reg(uint32_t Register)
  \brief			Reads a register, the entire sequence to read (at least for HTS221)
	\param			uint32_t Device: The slave address of the device
	\param			uint32_t Register: The Register to read from
	\returns		uint32_t I2C1_RX_Data: The data read
*/

uint32_t I2C_Read_Reg(uint32_t Device,uint32_t Register){
	
	//Reset CR2 Register
	I2C1->CR2 = 0x00000000;
	
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	
	//Set CR2 for 1-byte transfer for Device
	I2C1->CR2 |=(1UL<<16) | (Device<<1);
	
	//Start communication
	I2C1->CR2 |= I2C_CR2_START;
	
	//Check Tx empty before writing to it
	if((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2C1->TXDR = Register;
	}
	
	//Wait for transfer to complete
	while((I2C1->ISR & I2C_ISR_TC) == 0);

	//Clear CR2 for new configuration
	I2C1->CR2 = 0x00000000;
	
	//Set CR2 for 1-byte transfer, in read mode for Device
	I2C1->CR2 |= (1UL<<16) | I2C_CR2_RD_WRN | (Device<<1);
	
	//Start communication
	I2C1->CR2 |= I2C_CR2_START;
	
	//Wait for transfer to complete
	while((I2C1->ISR & I2C_ISR_TC) == 0);
	
	//Send Stop Condition
	I2C1->CR2 |= I2C_CR2_STOP;
	
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);

	//Clear Stop bit flag
	I2C1->ICR |= I2C_ICR_STOPCF;
	
	return(I2C1_RX_Data);
}

/**
  \fn					uint32_t I2C_Read_Reg(uint32_t Register)
  \brief			Reads a register, the entire sequence to read (at least for HTS221)
	\param			uint32_t Device: The slave address to written to
	\param			uint32_t Register: The register that you would like to write to
	\param			uint32_t Data: The data that you would like to write to the register
*/

void I2C_Write_Reg(uint32_t Device,uint32_t Register, uint32_t Data){
	
	//Reset CR2 Register
	I2C1->CR2 = 0x00000000;
	
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	
	//Set CR2 for 2-Byte Transfer, for Device
	I2C1->CR2 |= (2UL<<16) | (Device<<1);
	
	//Start communication
	I2C1->CR2 |= I2C_CR2_START;
	
	//Check Tx empty before writing to it
	if((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
		I2C1->TXDR = Register;
	}
	
	//Wait for TX Register to clear
	while((I2C1->ISR & I2C_ISR_TXE) == 0);
	
	//Check Tx empty before writing to it
	if((I2C1->ISR & I2C_ISR_TXE) == I2C_ISR_TXE){
		I2C1->TXDR = Data;
	}
	
	//Wait for transfer to complete
	while((I2C1->ISR & I2C_ISR_TC) == 0);
	
	//Send Stop Condition
	I2C1->CR2 |= I2C_CR2_STOP;	
	
	//Check to see if the bus is busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
	
	//Clear Stop bit flag
	I2C1->ICR |= I2C_ICR_STOPCF;
}
