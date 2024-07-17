/*
 * GPIO_INIT.h
 *
 *  Created on: Jan 13, 2022
 *      Author: amgad
 */

#ifndef GPIO_INIT_H_
#define GPIO_INIT_H_

#include "inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


void Port_Init(void);
void GPIO_Int(void);
void Pedestrian_Crossing(void);

#endif /* GPIO_INIT_H_ */
