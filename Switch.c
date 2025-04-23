/*
 * Switch.c
 *
 *  Created on: Nov 5, 2023
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this

      // Inputs
  // West sensor - PB0
  IOMUX->SECCFG.PINCM[PB0INDEX] = 0x40081;
  // South sensor - PB1
  IOMUX->SECCFG.PINCM[PB1INDEX] = 0x40081;
  // Walk sensor - PB2
  IOMUX->SECCFG.PINCM[PB2INDEX] = 0x40081;
  // Walk sensor - PB3
  IOMUX->SECCFG.PINCM[PB3INDEX] = 0x40081;
  // Walk sensor - PB4
  IOMUX->SECCFG.PINCM[PB4INDEX] = 0x40081;
 
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
  return (GPIOB->DIN31_0 & 0x03); // replace this line
}
