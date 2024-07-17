/*
 * GPIO_INIT.c
 *
 *  Created on: Jan 13, 2022
 *      Author: amgad
 */

#include "GPIO_INIT.h"
#include "GPTM.h"


void Port_Init(void)
{
    /*enable port clock*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /*wait for clock*/
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)){}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)){}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)){}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    /*set output pins*/
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);  //first traffic red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_3 | GPIO_PIN_2); //Pedestrian red and green LEDs
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_6); //first traffic yellow LED
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7); //first traffic green LED and second traffic red, yellow and green LEDs

    /*set input pins*/
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_4);  //tiva C included push button
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0);  //tiva C included push button

    /*set Pull up resistor*/
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_4,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  //set tiva C included push button as input
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  //set tiva C included push button as input

}

void GPIO_Int(void)
{
  GPIOIntDisable(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);  //disable the interrupt for pin 4 and 0 in port F

  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);  //clear the interrupt for pin 4 and 0 in port F

  GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0,GPIO_FALLING_EDGE);  //set the interrupt type falling edge for pin 4 and 0 in port F

  GPIOIntRegister(GPIO_PORTF_BASE, Pedestrian_Crossing);   // initialize the name of the ISR function

  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4|GPIO_INT_PIN_0);   //GPIO enable the interrupt for pin 4 and 0 in port F
}

void Pedestrian_Crossing(void)
{
    if( (GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)) == 0)  //If condition check the press on button 1 while button 2 is off and flag is 0
        {
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_2);      //Green light is ON at 1st Pedestrian LEDs and red is off
            GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4);     //Red light is ON at Traffic 1 LEDs
            GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_6, 0x0);    //Yellow light is OFF atTraffic 1 LEDs
            GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7, 0x0);    //Green light is OFF atTraffic 1 LEDs

            /*If condition loop for 2 times to delay 2 seconds     */
            int i;
            for(i = 0; i<2 ;i++)
                {
                   Timer_Delay();   //wait for 1 second
                }

            GPIOPinWrite(GPIO_PORTB_BASE,  GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);          //Red light is OFF at 1st Pedestrian LEDs

        }
    /*    If condition check the press on button 2 while button 1 is off and flag is 0     */
    else if ( (GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3)) == 0)
        {
            GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_3);          //Green light is ON at 2nd Pedestrian LEDs and red is off
            GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_6); //Red light is ON,Yellow and green light is OFF at Traffic 1 LEDs

            /*If condition loop for 2 times to delay 2 seconds     */
            int i;
            for(i = 0; i<2 ;i++)
                {
                    Timer_Delay();                  //wait for 1 second
                }

            GPIOPinWrite(GPIO_PORTB_BASE,  GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_3);          //Red light is OFF at traffic 1
            //TimerEnable(TIMER0_BASE, TIMER_A);

        }
     GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0);     //Clear the GPIO interrupt flag

}
