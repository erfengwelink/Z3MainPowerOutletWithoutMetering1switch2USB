/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"

//#include EMBER_AF_API_NETWORK_CREATOR
//#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_TARGET
//#include EMBER_AF_API_ZLL_PROFILE

#include "user_app.h"

#if defined(emberAfOnOffClusterPrint)//(...)
#undef emberAfOnOffClusterPrint//(...)
#define emberAfOnOffClusterPrint(...) emberAfPrint(13, __VA_ARGS__)//
#else
#define emberAfOnOffClusterPrint(...) emberAfPrint(13, __VA_ARGS__)//
#endif

#if defined(emberAfOnOffClusterPrintln)//(...)
#undef emberAfOnOffClusterPrintln//(...)
#define emberAfOnOffClusterPrintln(...) emberAfPrintln(13,  __VA_ARGS__)//
#else
#define emberAfOnOffClusterPrintln(...) emberAfPrintln(13,  __VA_ARGS__)//
#endif


#define JOIN_BLINK_NUM 480
#define FACTORY_BLINK_NUM 4

#define NWK_LED BOARD_HEARTBEAT_LED

//#define BUTTON_DEBOUNCE_MS  100
//#define BUTTON_LEAVE_MS 5000
//#define BUTTON_JOIN_MS  2000
//#define BUTTON_MID_MS BUTTON_JOIN_MS

//#define ONOFF_REPORT_DELAY_MS  50
//
//#define SWITCH_ONOFF_ENDPOINT 	(1)
//#define USB0_ONOFF_ENDPOINT 	(2)
//#define USB1_ONOFF_ENDPOINT 	(3)

#define SWITCH_ONOFF_MASK 	(0x01<<0)
#define USB0_ONOFF_MASK 	(0x01<<1)
#define USB1_ONOFF_MASK 	(0x01<<2)
#define VALID_ONOFF_MASK    (SWITCH_ONOFF_MASK | USB0_ONOFF_MASK | USB1_ONOFF_MASK)

EmberEventControl commissioningLedEventControl;
EmberEventControl findingAndBindingEventControl;
EmberEventControl pressDownEventControl;
EmberEventControl releaseUpEventControl;
EmberEventControl leaveAndJoinEventControl;

EmberEventControl nwkLEDFlashEventControl;
//extern EmberEventControl emberAfPluginReportingTickEventControl;

typedef enum{
	NON_BLINK_MODE,
	BLINK_PER_250_MS_MODE,
	BLINK_PER_1000_MS_MODE
} blinkMode_t;

struct nwk_led_t
{
	blinkMode_t mode;
	uint16_t blinkCnt;
	uint32_t nwkLEDInterval;
};

typedef struct nwk_led_t nwkLed_t;
//static uint8_t  lastMask = 0;//0111
//static uint8_t  CurMask = 0; //0100

static uint32_t  buttonPressDurationMs[2] = {0};
static nwkLed_t  nwkLedVal = {0};


// network leave
static EmberStatus networkLeave(void)
{
  EmberStatus status;
  status = emberLeaveNetwork();
//  emberAfAppPrintln("%p 0x%x", "leave", status);
  return status;
}


void nwkJoinFailHandler(void)
{
	emberEventControlSetInactive(nwkLEDFlashEventControl);
	halSetLed(NWK_LED);

}

void nwkJoinOKHandler(void)
{
	emberEventControlSetInactive(nwkLEDFlashEventControl);
	halClearLed(NWK_LED);

}

void nwkLEDFlashEventHandler(void)
{
	emberEventControlSetInactive(nwkLEDFlashEventControl);
	switch(nwkLedVal.mode)
	{
	case NON_BLINK_MODE:

		break;
	case BLINK_PER_250_MS_MODE:
		if(nwkLedVal.blinkCnt)
		{
			if(JOIN_BLINK_NUM == nwkLedVal.blinkCnt)
			{
				if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
				{
					EmberStatus status = networkLeave();
					if(status != EMBER_ZCL_STATUS_SUCCESS)
					{
						emberAfOnOffClusterPrintln("networkLeave failed!!!");
						return;
					}
				}
				emberAfOnOffClusterPrintln("480 leaveAndJoinEventControl 2000ms");
				emberEventControlSetDelayMS(leaveAndJoinEventControl, 2000);//
			}
			halToggleLed(NWK_LED);
			nwkLedVal.blinkCnt--;
			emberEventControlSetDelayMS(nwkLEDFlashEventControl, nwkLedVal.nwkLEDInterval);
		}else{

		}
		break;
	case BLINK_PER_1000_MS_MODE:
		if(nwkLedVal.blinkCnt)
		{
			halToggleLed(NWK_LED);
			nwkLedVal.blinkCnt--;
			emberEventControlSetDelayMS(nwkLEDFlashEventControl, nwkLedVal.nwkLEDInterval);
		}else{
			//factory reset
			{
				if (emberAfNetworkState() == EMBER_JOINED_NETWORK)
				{
					EmberStatus status = networkLeave();

					if(status != EMBER_ZCL_STATUS_SUCCESS)
					{

						emberAfOnOffClusterPrintln("networkLeave failed!!!");
						return ;
					}

				}
			}
			nwkLedVal.mode = BLINK_PER_250_MS_MODE;
			nwkLedVal.nwkLEDInterval = 250;
			nwkLedVal.blinkCnt = JOIN_BLINK_NUM;
			emberEventControlSetDelayMS(nwkLEDFlashEventControl, nwkLedVal.nwkLEDInterval);
		}

		break;
	default:

		break;
	}

}


void leaveAndJoinEventHandler(void)
{
	emberEventControlSetInactive(leaveAndJoinEventControl);
	EmberStatus status = emberAfPluginNetworkSteeringStart();
	emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
}

void pressDownEventHandler(void)
{
//	emberEventControlSetInactive(pressDownEventControl);
//	emberAfCorePrintln("%p", "pressDownEventHandler");
	uint32_t during = 0;
	buttonPressDurationMs[1] = halCommonGetInt32uMillisecondTick();
	during = buttonPressDurationMs[1] - buttonPressDurationMs[0];
	if(during == 5000 )//&& during < 10000)//LED flash
	{
//		emberEventControlSetInactive(pressDownEventControl);
//		emberAfCorePrintln("during > 5000 && during < 10000 stop during Tick %d \n", during);
		nwkLedVal.mode = BLINK_PER_250_MS_MODE;
		nwkLedVal.nwkLEDInterval = 250;
		nwkLedVal.blinkCnt = JOIN_BLINK_NUM;
		emberEventControlSetDelayMS(nwkLEDFlashEventControl, nwkLedVal.nwkLEDInterval);
	}else if(during == 10000){// factory reset
		emberEventControlSetInactive(pressDownEventControl);
//		emberAfCorePrintln("> 10000 stop during Tick %d \n", during);
		nwkLedVal.mode = BLINK_PER_1000_MS_MODE;
		nwkLedVal.blinkCnt = FACTORY_BLINK_NUM;
		nwkLedVal.nwkLEDInterval = 1000;
		emberEventControlSetDelayMS(nwkLEDFlashEventControl, nwkLedVal.nwkLEDInterval);
	}else{
		// do nothing
	}

}


void releaseUpEventHandler(void)
{
//  emberAfCorePrintln("%p ", "releaseUpEventHandler");
  emberEventControlSetInactive(pressDownEventControl);
  emberEventControlSetInactive(releaseUpEventControl);
  bool isLocalControl = true;
//  buttonPressDurationMs[1]
	uint32_t timeDownUp = elapsedTimeInt32u(
  								buttonPressDurationMs[0],
  								halCommonGetInt32uMillisecondTick());
  buttonPressDurationMs[0] = 0;


  if(timeDownUp > BUTTON_DEBOUNCE_MS && timeDownUp < BUTTON_LEAVE_MS)
  {
	  uint8_t ep = 1;
	  bool isInNwk = emberAfNetworkState() == EMBER_JOINED_NETWORK ? IS_IN_NWK : NOT_IN_NWK;
	  for(; ep<4; ep++)
	  {
		  EmberAfStatus emStatus = ControlComandHandle(ZCL_TOGGLE_COMMAND_ID, ep, isInNwk, isLocalControl);
		  emberAfCorePrintln("%p ? %p", "ControlComandHandle  ZCL_TOGGLE_COMMAND_ID ", emStatus?"FAIL":"OK");
	  }

  }
}


void commissioningLedEventHandler(void)
{

}

void findingAndBindingEventHandler()
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberEventControlSetInactive(findingAndBindingEventControl);
    emberAfCorePrintln("Find and bind target start: 0x%X",
                       emberAfPluginFindAndBindTargetStart(SWITCH_ONOFF_ENDPOINT));
  }
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  // Note, the ZLL state is automatically updated by the stack and the plugin.
  if (status == EMBER_NETWORK_DOWN) {
    halClearLed(COMMISSIONING_STATUS_LED);
  } else if (status == EMBER_NETWORK_UP) {
    halSetLed(COMMISSIONING_STATUS_LED);
    emberEventControlSetActive(findingAndBindingEventControl);
  }

// This value is ignored by the framework.
  return false;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
//  emberEventControlSetActive(commissioningLedEventControl);
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

  if (status != EMBER_SUCCESS) {
	  nwkJoinFailHandler();
	  EmberStatus status = emberAfPluginNetworkSteeringStart();
	  emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
    // Initialize our ZLL security now so that we are ready to be a touchlink
    // target at any point.
//    status = emberAfZllSetInitialSecurityState();
//    if (status != EMBER_SUCCESS) {
//      emberAfCorePrintln("Error: cannot initialize ZLL security: 0x%X", status);
//    }
//
//    status = emberAfPluginNetworkCreatorStart(false); // distributed
//    emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
  }else{
	  nwkJoinOKHandler();
  }
}

/** @brief Complete
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed. Ver.: always
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels Ver.: always
 */
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Form distributed",
                     "complete",
                     EMBER_SUCCESS);
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    bool onOff;
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        halSetLed(ON_OFF_LIGHT_LED);
      } else {
        halClearLed(ON_OFF_LIGHT_LED);
      }
    }
  }
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */

void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
//	emberAfCorePrintln("%p button %d ", "emberAfHalButtonIsrCallback", button);
	if(state == BUTTON_PRESSED && button == BSP_BUTTON0_PIN)// && button == ON_OFF_LIGHT_LED
	{
		emberEventControlSetActive(pressDownEventControl);
		buttonPressDurationMs[0] = halCommonGetInt32uMillisecondTick();
	}
	if (state == BUTTON_RELEASED&& button == BSP_BUTTON0_PIN) {// && button == ON_OFF_LIGHT_LED
		emberEventControlSetActive(releaseUpEventControl);
	}
}
