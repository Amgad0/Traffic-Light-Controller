/**
 * main.c
 */

#include "GPTM.h"

int main(void)
{
    Port_Init();
    Init_TimerInt();
    GPIO_Int();

    return 0;
}
