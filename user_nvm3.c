/*
 * user_nvm3.c
 *
 *  Created on: Jan 5, 2020
 *      Author: root
 */

#include "nvm3.h"
#include "user_nvm3.h"
#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/common.h"

#if defined(embernvm3Println)//(...)
#undef embernvm3Println//(...)
#define embernvm3Println(...) emberAfPrintln(33,  __VA_ARGS__)//
#else
#define embernvm3Println(...) emberAfPrintln(33,  __VA_ARGS__)//
#endif

nvm3_Handle_t  nvm3_userHandleData;
nvm3_Handle_t *nvm3_userHandle = &nvm3_userHandleData;

extern nvm3_Init_t    nvm3_defaultInitData;
nvm3_Init_t   *nvm3_userInit = &nvm3_defaultInitData;


/***************************************************************************//**
 * @brief
 *  Read the object data identified with a given key from NVM.
 *
 * @param[in] h
 *   A pointer to an NVM3 driver handle.
 *
 * @param[in] key
 *   A 20-bit object identifier.
 *
 * @param[out] value
 *   A pointer to the application data buffer. The read function will copy
 *   data to this location.
 *
 * @param[in] maxLen
 *   The maximum object size in number of bytes. The @ref nvm3_getObjectInfo() function
 *   can be used to find the actual size.
 *
 * @return
 *   @ref ECODE_NVM3_OK on success or a NVM3 @ref Ecode_t on failure.
 ******************************************************************************/
//Ecode_t nvm3_readData(nvm3_Handle_t *h, nvm3_ObjectKey_t key, void *value, size_t maxLen);


#define NVM3_SW_KEY     0x3000
typedef struct
{
	uint8_t sw[3];
} swValue_t;

static swValue_t swVal = {0};
static uint8_t ti = 0;

EmberStatus userNvm3Init(void)
{
//	EmberStatus status = 0;
	Ecode_t ecode = 0;

//	ecode = nvm3_open(nvm3_userHandle, nvm3_userInit);

//	if (ecode != ECODE_NVM3_OK) {
//		embernvm3Println("nvm3_open failed!!!");
//	}
	ecode = nvm3_readData(nvm3_userHandle, NVM3_SW_KEY, &swVal, sizeof(swVal));
	if (ecode != ECODE_NVM3_OK) {
		embernvm3Println("nvm3_readData failed!!!");
	}
//	embernvm3Println("default -> nvm3_readData| %d | %d | %d |", swVal.sw[0], swVal.sw[1], swVal.sw[2]);

//	swVal.sw[0] = ti++;
//	swVal.sw[1] = ti++;
//	swVal.sw[2] = ti++;

	ecode = nvm3_writeData(nvm3_userHandle, NVM3_SW_KEY, &swVal, sizeof(swVal));
	if (ecode != ECODE_NVM3_OK) {
		embernvm3Println("nvm3_writeData failed!!!");
	}
//	embernvm3Println("set nvm3_writeData| %d | %d | %d |", swVal.sw[0], swVal.sw[1], swVal.sw[2]);

	ecode = nvm3_readData(nvm3_userHandle, NVM3_SW_KEY, &swVal, sizeof(swVal));
//	embernvm3Println("get nvm3_readData| %d | %d | %d |", swVal.sw[0], swVal.sw[1], swVal.sw[2]);

	if (ecode != ECODE_NVM3_OK) {
		embernvm3Println("nvm3_readData failed!!!");
	}
//	emberAfDeviceTablePointer();
	return (EmberStatus)ecode;
}

#if 0
#include "app/framework/util/af-main.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/common.h"

#if defined (USE_NVM3)
#include "nvm3.h"
#include "nvm3_hal_flash.h"
// Device table counter key and data key
#define NVM3_DT_CNT_KEY 0x1000
#define NVM3_DT_KEY     0x2000
#endif


void emAfDeviceTableSave(void)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint8_t i;
  uint32_t counter = 0;

  for (i = 0; i < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE; i++) {
    if (deviceTable[i].nodeId != EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      nvm3_writeData(nvm3_defaultHandle,NVM3_DT_KEY + counter, &deviceTable[i], sizeof(deviceTable[i]));
      counter++;
    }
  }
  // Write counter
  nvm3_writeCounter(nvm3_defaultHandle, NVM3_DT_CNT_KEY, counter);
}

void emAfDeviceTableLoad(void)
{
  EmberAfPluginDeviceTableEntry *deviceTable = emberAfDeviceTablePointer();
  uint8_t i;
  uint32_t counter = 0;
  // Read number of object of device table
  nvm3_readCounter(nvm3_defaultHandle, NVM3_DT_CNT_KEY, &counter);
  for (i = 0; i < counter; i++) {
    nvm3_readData(nvm3_defaultHandle,NVM3_DT_KEY + i, &deviceTable[i], sizeof(deviceTable[i]));
    deviceTable[i].state = EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED;
    deviceTable[i].lastMsgTimestamp = halCommonGetInt32uMillisecondTick();
  }
}


#endif

