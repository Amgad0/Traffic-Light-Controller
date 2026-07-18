/*
 * GPTM.h
 *
 *  Created on: Jan 13, 2022
 *      Author: amgad
 */

#ifndef GPTM_H_
#define GPTM_H_

#include "GPIO_INIT.h"

/* ---- Timing configuration (all timing derives from the 16 MHz default system clock) ---- */
#define SYS_CLOCK_HZ        16000000UL      /* default system clock frequency          */
#define TIMER_ONE_SEC_LOAD  SYS_CLOCK_HZ    /* Timer load value for a 1-second interval */

/* ---- Traffic-light phase schedule, expressed in 1 Hz ticks of Timer0A ----
 *   tick 0 : active direction GREEN   (held 5 s)
 *   tick 5 : active direction YELLOW  (held 2 s)
 *   tick 7 : active direction RED
 *   tick 8 : 1 s all-red gap elapsed -> hand over to the other direction
 */
#define TICK_GREEN          0
#define TICK_YELLOW         5
#define TICK_RED            7
#define TICK_HANDOVER       8

/* ---- Direction selector values for switch_traffic ---- */
#define TRAFFIC_1           0   /* North-South */
#define TRAFFIC_2           1   /* East-West   */

/* ---- Pedestrian crossing green duration, in 1 s Timer_Delay() steps ---- */
#define PEDESTRIAN_CROSS_S  2

void Init_TimerInt(void);
void Timer0_Handler(void);
void Timer_Delay(void);

#endif /* GPTM_H_ */
