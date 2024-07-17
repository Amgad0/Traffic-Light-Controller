/*
 * GPTM.c
 *
 *  Created on: Jan 13, 2022
 *
 *      Author: amgad
 */

#include "GPTM.h"

int counter = 0;  //variable which the timer ISR increments
int switch_traffic = 0;    //variable to switch from traffic 1 to 2

void Init_TimerInt(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);     //enable timer clock

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);      //wait for clock

    /* initialize and configure TimerA */
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerConfigure(TIMER0_BASE,TIMER_CFG_A_PERIODIC_UP);
    TimerClockSourceSet(TIMER0_BASE,TIMER_CLOCK_SYSTEM);

    TimerLoadSet(TIMER0_BASE,TIMER_A,16000000);   //set load values
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0_Handler);  //set timer ISR function name
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /* enable timer and timer interrupt */
    IntEnable(INT_TIMER0A);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer0_Handler(void)
{
    /*loop for 7 seconds on traffic 1 */
    if(switch_traffic == 0)
        {
          if( counter == 0 && switch_traffic == 0)
          {
             GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7, 0x0);
             GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
                          GPIO_PIN_6 | GPIO_PIN_7);   //Green light traffic 1 is ON and Red light traffic 2 is ON
             GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3 , GPIO_PIN_3);
             GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, 0x0);
             GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_2);
          }
          if( counter == 5 && switch_traffic == 0 )                                                             //wait for 5 seconds
          {
              GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_6);   //Yellow light traffic 1 is ON
              GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_6); //1st traffic Green light OFF and 2nd traffic RED light ON, yellow and green ligh OFF
              GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_3 , GPIO_PIN_3);  //Pedestrian red light ON
              GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4,GPIO_PIN_2); //1st traffic Red light ON
          }
          if( counter == 7 && switch_traffic == 0 )                                                             //wait for 2 more seconds
          {
             GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4); //Red light traffic 1 is ON
             GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6, 0x0);   //Yellow light is off
             GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_6);
             GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3 , GPIO_PIN_3);
             GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_2);
          }
        }
     /*loop for 7 seconds on traffic 2 */
     else if(switch_traffic == 1)  //check 1 flag
        {
          if( counter == 0 && switch_traffic == 1)
          {
             GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_4);           //Green light traffic 2 is ON
             GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6, 0x0);      //Red light traffic 2 is OFF
             GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3 , GPIO_PIN_3);
             GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4);
          }
          if( counter == 5 && switch_traffic == 1 )                                                             //wait for 5 seconds
          {
              GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_5);           //Yellow light traffic 2 is ON
              GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6, 0x0);
              GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3 , GPIO_PIN_3);
              GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4);
          }
          if( counter == 7 && switch_traffic == 1 )                                        //wait for 2 more seconds
          {
              GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_6);          //Red light traffic 2 is ON
              GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3 , GPIO_PIN_3);
              GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4);
          }
        }
    counter++;  //increment counter

     if (counter == 8 && switch_traffic == 0) //delay 1 second after the traffic 1 finishes
     {
       counter =0;                     //reset counter to 0
       switch_traffic = 1;             //set the switch_traffic to 1
     }

     if (counter == 8 && switch_traffic == 1) // delay 1 second after the traffic 2 finishes
     {
       counter =0;                //reset counter to 0
       switch_traffic = 0;        //reset switch_traffic to 0
     }

     TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


void Timer_Delay(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); //Enable the Timer0 peripheral

  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)){} //Wait until clock is enabled

  TimerDisable(TIMER1_BASE, TIMER_A); // Disable Timer1A

  TimerConfigure(TIMER1_BASE, (TIMER_CFG_PERIODIC)); //Set the type of the timer to periodic

  TimerLoadSet(TIMER1_BASE, TIMER_A, 16000000); //load the timer value

  TIMER0_ICR_R = 0x01;  //set flag for the timer and enable it


  TimerEnable(TIMER1_BASE, TIMER_A); //Enable timer1A

   /*    Set the timer to count 1 second */
   TimerEnable(TIMER1_BASE, TIMER_A);
   while((TIMER0_RIS_R & 0x1) == 0){}
}
