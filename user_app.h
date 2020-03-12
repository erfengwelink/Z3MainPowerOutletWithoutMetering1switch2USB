/*
 * user_app.h
 *
 *  Created on: Jan 2, 2020
 *      Author: root
 */

#ifndef USER_APP_H_
#define USER_APP_H_

#include "app/framework/include/af.h"
//#include "error.h"
//#include "include/ember-types.h"

#define IS_IN_NWK true
#define NOT_IN_NWK false

#define BUTTON_DEBOUNCE_MS  100
#define BUTTON_LEAVE_MS 5000
#define BUTTON_JOIN_MS  2000
#define BUTTON_MID_MS BUTTON_JOIN_MS

#define ONOFF_REPORT_DELAY_MS  50

#define SWITCH_ONOFF_ENDPOINT 	(1)
#define USB0_ONOFF_ENDPOINT 	(2)
#define USB1_ONOFF_ENDPOINT 	(3)


//EmberAfStatus OnOffCMDControl(uint8_t cmd, uint8_t ep);

EmberAfStatus ControlComandHandle(uint8_t cmd, uint8_t ep, bool isInNWK, bool isLocalControl);

#endif /* USER_APP_H_ */
