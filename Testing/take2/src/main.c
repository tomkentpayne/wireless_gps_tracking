#include "stm32f4xx_conf.h"

#define PAUSE_LONG  4000000L
#define PAUSE_SHORT 1000000L

GPIO_InitTypeDef GPIO_InitStructure;

static void delay(__IO uint32_t nCount)
{
    while(nCount--)
        __asm("nop"); // do nothing
}

static void setup(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_Init(GPIOC,  &GPIO_InitStructure);
}


int main(void)
{
    setup();

    for(;;) {
        GPIO_SetBits(GPIOC, GPIO_Pin_14);
        delay(PAUSE_LONG);
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);
        delay(PAUSE_LONG);
    }

    return 0;
}
