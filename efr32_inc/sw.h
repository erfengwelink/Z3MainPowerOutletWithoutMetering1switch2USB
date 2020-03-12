/*
 * sw.h
 *
 *  Created on: Dec 27, 2019
 *      Author: yufeng.wang@clife.cn
 */

#ifndef EFR32_INC_SW_H_
#define EFR32_INC_SW_H_

#include "hal-config.h"


enum HalBoardSWPins{
#if (BSP_SW_COUNT > 0)
	BOARDSW1 = 0,
	BOARDSW2,
	BOARDSW3
#endif
};

typedef enum HalBoardSWPins HalBoardSW;

void halInternalInitSW(void);

void halSetSW(HalBoardSW sw);

void halClearSW(HalBoardSW sw);

void halToggleSW(HalBoardSW sw);

#endif /* EFR32_INC_SW_H_ */
