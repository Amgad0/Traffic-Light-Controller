/*
 * GPTM.h
 *
 *  Created on: Jan 13, 2022
 *      Author: amgad
 */

#ifndef GPTM_H_
#define GPTM_H_

#include "GPIO_INIT.h"

void Init_TimerInt(void);
void Timer0_Handler(void);
void Timer_Delay(void);

#endif /* GPTM_H_ */
