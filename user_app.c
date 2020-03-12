/*
 * user_app.c
 *
 *  Created on: Jan 2, 2020
 *      Author: yufeng.wang@clife.cn
 */
#include "user_nvm3.h"
#include "user_app.h"
#include "sw.h"

#if defined(emberAfOnOffClusterPrintln)//(...)
#undef emberAfOnOffClusterPrintln//(...)
#define emberAfOnOffClusterPrintln(...) emberAfPrintln(2,  __VA_ARGS__)//
#else
#define emberAfOnOffClusterPrintln(...) emberAfPrintln(2,  __VA_ARGS__)//
#endif

EmberEventControl onOffAttrReportEventControl;

EmberEventControl onOffSingleEP1ReportEventControl;
EmberEventControl onOffSingleEP2ReportEventControl;
EmberEventControl onOffSingleEP3ReportEventControl;

static void vReportOnOffAttribute(uint8_t endpoint, bool is_on)
{
// The way 1
	 uint8_t data[] = { LOW_BYTE(ZCL_ON_OFF_ATTRIBUTE_ID), HIGH_BYTE(ZCL_ON_OFF_ATTRIBUTE_ID), ZCL_BOOLEAN_ATTRIBUTE_TYPE, 0};
	 data[3] = is_on;
	 emberAfFillCommandGlobalServerToClientReportAttributes( ZCL_ON_OFF_CLUSTER_ID, data, sizeof(data) );
	 emberAfSetCommandEndpoints (endpoint, 1);
	 emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);		// send to ZC
	 emberAfOnOffClusterPrintln("vReportOnOffAttribute endpoint %d, value %d\r\n", endpoint, is_on);
}

EmberAfStatus onOffSingleEPxReportHandler(uint8_t ep)
{
	EmberAfStatus status;
	bool currentValue;//, newValue

	status = emberAfReadAttribute(	ep,
									ZCL_ON_OFF_CLUSTER_ID,
			                        ZCL_ON_OFF_ATTRIBUTE_ID,
			                        CLUSTER_MASK_SERVER,
			                        (uint8_t *)&currentValue,
			                        sizeof(currentValue),
			                        NULL); // data type
	if (status != EMBER_ZCL_STATUS_SUCCESS) {
		emberAfOnOffClusterPrintln("ERR: ep %d reading on/off %x", ep, status);
		return status;
	}

	vReportOnOffAttribute(ep, currentValue);
	return EMBER_ZCL_STATUS_SUCCESS;
}

void onOffSingleEP1ReportEventHandler(void)
{
	emberEventControlSetInactive(onOffSingleEP1ReportEventControl);
	onOffSingleEPxReportHandler(SWITCH_ONOFF_ENDPOINT);
}
void onOffSingleEP2ReportEventHandler(void)
{
	emberEventControlSetInactive(onOffSingleEP2ReportEventControl);
	onOffSingleEPxReportHandler(USB0_ONOFF_ENDPOINT);
}

void onOffSingleEP3ReportEventHandler(void)
{
	emberEventControlSetInactive(onOffSingleEP3ReportEventControl);
	onOffSingleEPxReportHandler(USB1_ONOFF_ENDPOINT);
}

void onOffAttrReportEventHandler(void)
{

}

EmberAfStatus OnOffCMDControl(uint8_t cmd, uint8_t ep)
{
	uint8_t command = cmd;
	uint8_t xled = ep-1;
	switch(command)
	{
		case ZCL_OFF_COMMAND_ID:
			halClearSW(xled); //halClearLed(xled);
			if(!xled)
			{
				halClearLed(xled);
			}
			break;
		case ZCL_ON_COMMAND_ID:
			halSetSW(xled); //halSetLed(xled);
			if(!xled)
			{
				halSetLed(xled);
			}
			break;
		case ZCL_TOGGLE_COMMAND_ID:
			halToggleSW(xled); //halToggleLed(xled);
			if(!xled)
			{
				halToggleLed(xled);
			}
			break;
		default:
			break;
	}
	return EMBER_ZCL_STATUS_SUCCESS;
}


//remote command Report
static void remoteReportEventSet(uint8_t ep)
{
	uint8_t xEp = ep;
	switch(xEp)
	{
	case 1:
		emberEventControlSetDelayMS(onOffSingleEP1ReportEventControl, ONOFF_REPORT_DELAY_MS);
		break;
	case 2:
		emberEventControlSetDelayMS(onOffSingleEP2ReportEventControl, ONOFF_REPORT_DELAY_MS * 2);
		break;
	case 3:
		emberEventControlSetDelayMS(onOffSingleEP3ReportEventControl, ONOFF_REPORT_DELAY_MS * 3);
		break;
	default:

		break;
	}
}

//local command Report
static void localReportEventSet(uint8_t ep)
{
	remoteReportEventSet(ep);
}

static EmberAfStatus localControlComandHandle(uint8_t cmd, uint8_t ep, bool isInNWK)
{
	EmberAfStatus status;
	bool currentValue, newValue, mainValue;
	if(SWITCH_ONOFF_ENDPOINT != ep)
	{
		status = emberAfReadAttribute(SWITCH_ONOFF_ENDPOINT,
					                      ZCL_ON_OFF_CLUSTER_ID,
					                      ZCL_ON_OFF_ATTRIBUTE_ID,
					                      CLUSTER_MASK_SERVER,
					                      (uint8_t *)&mainValue,
					                      sizeof(mainValue),
					                      NULL); // data type
		if (status != EMBER_ZCL_STATUS_SUCCESS) {
			emberAfOnOffClusterPrintln("ERR: reading ZCL_ON_OFF_CLUSTER_ID on/off %x", status);
			return status;
		}

	}
	status = emberAfReadAttribute(ep,
			                      ZCL_ON_OFF_CLUSTER_ID,
			                      ZCL_ON_OFF_ATTRIBUTE_ID,
			                      CLUSTER_MASK_SERVER,
			                      (uint8_t *)&currentValue,
			                      sizeof(currentValue),
			                      NULL); // data type
	if (status != EMBER_ZCL_STATUS_SUCCESS) {
		emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
		return status;
	}



	if ((!currentValue && cmd == ZCL_OFF_COMMAND_ID)
			|| (currentValue && cmd == ZCL_ON_COMMAND_ID)) {
		emberAfOnOffClusterPrintln("On/off already set to new value");
		return EMBER_ZCL_STATUS_SUCCESS;
	}
	newValue = !currentValue;
	if(currentValue == mainValue)
	{
//		emberAfOnOffClusterPrintln(" localControlComandHandle currentValue == mainValue OnOffCMDControl cmd %d ep %d ", cmd, ep);
		status = OnOffCMDControl(cmd, ep);
	}
	if(status != EMBER_ZCL_STATUS_SUCCESS)
	{
		emberAfOnOffClusterPrintln("ERR: OnOffCMDControl status %x", status);
		return status;
	}
	if(isInNWK)
	{
		localReportEventSet(ep);//87001
	}
	status = emberAfWriteAttribute(ep,
			    			  	   ZCL_ON_OFF_CLUSTER_ID,
			      	               ZCL_ON_OFF_ATTRIBUTE_ID,
			      	               CLUSTER_MASK_SERVER,
			      	               (uint8_t *)&newValue,
			      	               ZCL_BOOLEAN_ATTRIBUTE_TYPE);
	if (status != EMBER_ZCL_STATUS_SUCCESS) {
		emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
		return status;
	}
	return EMBER_ZCL_STATUS_SUCCESS;
}



static EmberAfStatus remoteControlComandHandle(uint8_t cmd, uint8_t ep, bool isInNWK)
{
	EmberAfStatus status;
	bool currentValue, newValue;
	status = emberAfReadAttribute(ep,
		                          ZCL_ON_OFF_CLUSTER_ID,
		                          ZCL_ON_OFF_ATTRIBUTE_ID,
		                          CLUSTER_MASK_SERVER,
		                          (uint8_t *)&currentValue,
		                          sizeof(currentValue),
		                          NULL); // data type
	if (status != EMBER_ZCL_STATUS_SUCCESS) {
		emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
		return status;
	}
	if ((!currentValue && cmd == ZCL_OFF_COMMAND_ID)
			|| (currentValue && cmd == ZCL_ON_COMMAND_ID)) {
		emberAfOnOffClusterPrintln("On/off already set to new value");
		return EMBER_ZCL_STATUS_SUCCESS;
	}

	newValue = !currentValue;
	emberAfOnOffClusterPrintln(" remoteControlComandHandle OnOffCMDControl cmd %d ep %d ", cmd, ep);
	status = OnOffCMDControl(cmd, ep);


	if(status != EMBER_ZCL_STATUS_SUCCESS)
	{
		emberAfOnOffClusterPrintln("ERR: OnOffCMDControl status %x", status);
		return status;
	}
	if(isInNWK)
	{
		remoteReportEventSet(ep);
	}

	//write flag bit
	{
		// write the new on/off value
		status = emberAfWriteAttribute(ep,
		    			  	  	  	  ZCL_ON_OFF_CLUSTER_ID,
		      	                      ZCL_ON_OFF_ATTRIBUTE_ID,
		      	                      CLUSTER_MASK_SERVER,
		      	                      (uint8_t *)&newValue,
		      	                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);
		if (status != EMBER_ZCL_STATUS_SUCCESS) {
			emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
			return status;
		}
	}
	return EMBER_ZCL_STATUS_SUCCESS;
}


EmberAfStatus ControlComandHandle(uint8_t cmd, uint8_t ep, bool isInNWK, bool isLocalControl)
{
//	emberAfOnOffClusterPrintln("userNvm3Init");
//	userNvm3Init();
	if(isLocalControl)
	{
		return localControlComandHandle(cmd, ep, isInNWK);
	}else{
		return remoteControlComandHandle(cmd, ep, isInNWK);
	}
}
