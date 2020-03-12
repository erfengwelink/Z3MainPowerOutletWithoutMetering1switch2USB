/*
 * sw.c
 *
 *  Created on: Dec 27, 2019
 *      Author: yufeng.wang@clife.cn
 *
 */
#include "sw.h"


typedef struct {
  GPIO_Port_TypeDef   port;
  unsigned int        pin;
} tSWArray;
static const tSWArray SWArray[BSP_SW_COUNT] = BSP_SW_INIT;


void halInternalInitSW(void)
{
  int i;
  uint8_t enableSWs[HAL_SW_COUNT] = HAL_SW_ENABLE;


  for ( i = 0; i < HAL_SW_COUNT; i++ ) {
  #if defined (BSP_SW_POLARITY) && (BSP_SW_POLARITY == 0)
    GPIO_PinModeSet(ledArray[enableLeds[i]].port,
                    ledArray[enableLeds[i]].pin,
                    gpioModePushPull,
                    1);
  #else
    GPIO_PinModeSet(SWArray[enableSWs[i]].port,
                    SWArray[enableSWs[i]].pin,
                    gpioModePushPull,
                    0);
  #endif
  }
}

void halSetSW(HalBoardSW sw)
{
#if defined (BSP_SW_POLARITY) && (BSP_SW_POLARITY == 0)
  GPIO_PinOutClear(SWArray[sw].port, SWArray[sw].pin);
#else
  GPIO_PinOutSet(SWArray[sw].port, SWArray[sw].pin);
#endif
}

void halClearSW(HalBoardSW sw)
{
#if defined (BSP_SW_POLARITY) && (BSP_SW_POLARITY == 0)
  GPIO_PinOutSet(SWArray[sw].port, SWArray[sw].pin);
#else
  GPIO_PinOutClear(SWArray[sw].port, SWArray[sw].pin);
#endif
}

void halToggleSW(HalBoardSW sw)
{
  GPIO_PinOutToggle(SWArray[sw].port, SWArray[sw].pin);
}
