// I2C0.c
// Runs on LM4F120/TM4C123
// Provide a function that initializes, sends, and receives the I2C0 module
// interfaced with an HMC6352 compass or TMP102 thermometer.
// Daniel Valvano
// July 2, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// I2C0SCL connected to PB2 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// I2C0SDA connected to PB3 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND
#include <stdint.h>
#include "tm4c123gh6pm.h"


#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
void I2C_Init(void){
    /*-- I2C1 and Port A Activation --*/
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R1;                 // enable I2C Module 1 clock
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;               // enable GPIO Port A clock
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R0) == 0) {};  // allow time for activating
    
    /*-- Port A Set Up --*/
    GPIO_PORTA_AFSEL_R |= 0xC0;                            // enable alt function on PA6, 7
    GPIO_PORTA_ODR_R |= 0x80;                              // enable open drain on PA7
    GPIO_PORTA_DEN_R |= 0xC0;                              // enable digital I/O on PA6, 7
    GPIO_PORTA_PCTL_R &= ((~GPIO_PCTL_PA6_M) &             // clear bit fields for PA6
                          (~GPIO_PCTL_PA7_M));             // clear bit fields for PA7
    GPIO_PORTA_PCTL_R |= (GPIO_PCTL_PA6_I2C1SCL |          // configure PA6 as I2C1SCL
                          GPIO_PCTL_PA7_I2C1SDA);          // configure PA7 as I2C1SDA
    GPIO_PORTA_AMSEL_R &= ~0xC0;                           // disable analog functionality on PA6, 7
    
    /*-- I2C1 Set Up --*/
    I2C1_MCR_R = I2C_MCR_MFE;                              // master function enable
    I2C1_MTPR_R = 39;                                      // configure for 100 kbps clock
    // 20 * (TPR + 1) * 12.5ns = 10us, with TPR=24
}

// receives one byte from specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
// Note for TMP102 thermometer only:
// Used to read the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
uint8_t I2C_Recv(int8_t slave){
  int retryCounter = 1;
  do{
				while (I2C1_MCS_R & I2C_MCS_BUSY) {};                // wait for I2C ready
				I2C1_MSA_R = (slave << 1) & I2C_MSA_SA_M;    // MSA[7:1] is slave address
				I2C1_MSA_R |= I2C_MSA_RS;                            // MSA[0] is 1 for receive
				
				I2C1_MCS_R = (I2C_MCS_STOP  |                        // generate stop
											I2C_MCS_START |                        // generate start/restart
											I2C_MCS_RUN);                          // master enable
				while (I2C1_MCS_R & I2C_MCS_BUSY) {};                // wait for transmission done
				retryCounter++;                                      // increment retry counter
		}                                                        // repeat if error
		while (((I2C1_MCS_R & (I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0) && (retryCounter <= MAXRETRIES));
  return (I2C1_MDR_R & I2C_MDR_DATA_M);          // usually returns 0xFF on error
}
// receives two bytes from specified slave
// Note for HMC6352 compass only:
// Used with 'A' commands
// Note for TMP102 thermometer only:
// Used to read the contents of the pointer register
uint16_t I2C_Recv2(int8_t slave){
  uint8_t data1,data2;
  int retryCounter = 1;
  do{
		while (I2C1_MCS_R & I2C_MCS_BUSY) {};                // wait for I2C ready
		I2C1_MSA_R = (slave << 1) & I2C_MSA_SA_M;    // MSA[7:1] is slave address
		I2C1_MSA_R |= I2C_MSA_RS;                            // MSA[0] is 1 for receive
		
		I2C1_MCS_R = (I2C_MCS_ACK   |                        // positive data ack
									I2C_MCS_START |                        // generate start/restart
									I2C_MCS_RUN);                          // master enable
		while (I2C1_MCS_R & I2C_MCS_BUSY) {};                // wait for transmission done
    data1 = (I2C1_MDR_R & I2C_MDR_DATA_M);       // MSB data sent first
		
		I2C1_MCS_R = (I2C_MCS_STOP |                         // generate stop
									I2C_MCS_RUN);                          // master enable
		while (I2C1_MCS_R & I2C_MCS_BUSY) {};                // wait for transmission done
    data2 = (I2C1_MDR_R & I2C_MDR_DATA_M);       // LSB data sent last
    retryCounter = retryCounter + 1;        // increment retry counter
  }                                         // repeat if error
	while (((I2C1_MCS_R & (I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0) && (retryCounter <= MAXRETRIES));
  return (data1<<8)+data2;                  // usually returns 0xFFFF on error
}

// sends one byte to specified slave
// Note for HMC6352 compass only:
// Used with 'S', 'W', 'O', 'C', 'E', 'L', and 'A' commands
//  For 'A' commands, I2C_Recv2() should also be called
// Note for TMP102 thermometer only:
// Used to change the pointer register
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send1(int8_t slave, uint8_t data1){
	while (I2C1_MCS_R & I2C_MCS_BUSY) {};                            // wait for I2C ready
	I2C1_MSA_R = (slave << 1) & I2C_MSA_SA_M;                // MSA[7:1] is slave address
	I2C1_MSA_R &= ~I2C_MSA_RS;                                       // MSA[0] is 0 for send
	
	I2C1_MDR_R = data1 & I2C_MDR_DATA_M;                       // prepare data byte
	I2C1_MCS_R = (I2C_MCS_STOP |                                 // generate stop
								I2C_MCS_RUN);                                  // master enable
	while (I2C1_MCS_R & I2C_MCS_BUSY) {};                        // wait for transmission done
	// return error bits
	return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
// sends two bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
//  For 'r' and 'g' commands, I2C_Recv() should also be called
// Note for TMP102 thermometer only:
// Used to change the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2){
	while (I2C1_MCS_R & I2C_MCS_BUSY) {};                            // wait for I2C ready
	I2C1_MSA_R = (slave << 1) & I2C_MSA_SA_M;                // MSA[7:1] is slave address
	I2C1_MSA_R &= ~I2C_MSA_RS;                                       // MSA[0] is 0 for send
		
  I2C1_MDR_R = data1&0xFF;         // prepare first byte
  I2C1_MCS_R = (0
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                    //   & ~I2C_MCS_STOP    // no stop
                       | I2C_MCS_START    // generate start/restart
                       | I2C_MCS_RUN);    // master enable
  while(I2C1_MCS_R&I2C_MCS_BUSY){};// wait for transmission done
                                          // check error bits
  if((I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C1_MCS_R = (0                // send stop if nonzero
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // stop
                     //  & ~I2C_MCS_START   // no start/restart
                     //  & ~I2C_MCS_RUN    // master disable
                        );   
                                          // return error bits if nonzero
    return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C1_MDR_R = data2&0xFF;         // prepare second byte
  I2C1_MCS_R = (0
                      // & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // generate stop
                      // & ~I2C_MCS_START   // no start/restart
                       | I2C_MCS_RUN);    // master enable
  while(I2C1_MCS_R&I2C_MCS_BUSY){};// wait for transmission done
                                          // return error bits
  return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
// sends three bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'w' and 'G' commands
// Note for TMP102 thermometer only:
// Used to change the contents of the pointer register
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3){
	while (I2C1_MCS_R & I2C_MCS_BUSY) {};                            // wait for I2C ready
	I2C1_MSA_R = (slave << 1) & I2C_MSA_SA_M;                // MSA[7:1] is slave address
	I2C1_MSA_R &= ~I2C_MSA_RS;                                       // MSA[0] is 0 for send
		
  I2C1_MDR_R = data1&0xFF;         // prepare first byte
  I2C1_MCS_R = (0
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                     //  & ~I2C_MCS_STOP    // no stop
                       | I2C_MCS_START    // generate start/restart
                       | I2C_MCS_RUN);    // master enable
  while(I2C1_MCS_R&I2C_MCS_BUSY){};// wait for transmission done
                                          // check error bits
  if((I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C1_MCS_R = (0                // send stop if nonzero
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // stop
                     //  & ~I2C_MCS_START   // no start/restart
                     //  & ~I2C_MCS_RUN   // master disable
                       );   
                                          // return error bits if nonzero
    return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C1_MDR_R = data2&0xFF;         // prepare second byte
  I2C1_MCS_R = (0
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                     //  & ~I2C_MCS_STOP    // no stop
                     //  & ~I2C_MCS_START   // no start/restart
                       | I2C_MCS_RUN);    // master enable
  while(I2C1_MCS_R&I2C_MCS_BUSY){};// wait for transmission done
                                          // check error bits
  if((I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR)) != 0){
    I2C1_MCS_R = (0                // send stop if nonzero
                     //  & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // stop
                     //  & ~I2C_MCS_START   // no start/restart
                     //  & ~I2C_MCS_RUN   // master disable
                        );
                                          // return error bits if nonzero
    return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
  }
  I2C1_MDR_R = data3&0xFF;         // prepare third byte
  I2C1_MCS_R = (0
                    //   & ~I2C_MCS_ACK     // no data ack (no data on send)
                       | I2C_MCS_STOP     // generate stop
                    //   & ~I2C_MCS_START   // no start/restart
                       | I2C_MCS_RUN);    // master enable
  while(I2C1_MCS_R&I2C_MCS_BUSY){};// wait for transmission done
                                          // return error bits
  return (I2C1_MCS_R&(I2C_MCS_DATACK|I2C_MCS_ADRACK|I2C_MCS_ERROR));
}
