/*
 * EELS.h
 *
 * Created: 7/24/2019 12:10:27 PM
 *  Author: hasan.jaafar
 */ 
 


//#include "hal_flash.h"
#include "App/communication/crc32.h"
/*
	page_size = flash_get_page_size(&FLASH_0);

	for (i = 0; i < page_size; i++) {
		src_data[i] = i;
	}

	flash_write(&FLASH_0, 0x3200, src_data, page_size);

	flash_read(&FLASH_0, 0x3200, chk_data, page_size);

*/ //.
#ifndef EELS_H_
#define EELS_H_


#define EELS_MAX_SLOTS 5
#define _EELS_CRC_BYTE_LENGTH 1


typedef struct {
	uint32_t begining;
	uint16_t length;
	uint32_t current_position;
	uint16_t current_counter;
	uint8_t slot_log_length;
	uint8_t raw_data_size;
	uint16_t _counter_max;
	uint8_t _counter_bytes;
}ee_slot_t;

// public functions
uint8_t EELS_Init(int32_t (*readFunc)(uint32_t, uint8_t* const, uint16_t), int32_t (*writeFunc)(uint32_t,uint8_t* const, uint16_t));
void EELS_InsertLog(uint8_t slotNumber, uint8_t* data);
uint8_t EELS_SetSlot(uint8_t slotNumber, uint32_t begin_addr, uint16_t length, uint8_t data_length);

//private functions
int32_t _EELS_FindLastPos(uint8_t slotNumber);
bool EELS_ReadLast(uint8_t slotNumber, uint8_t* const buf);

// dbg functions
uint8_t EELS_SlotLogSize(uint8_t slotNumber);
uint32_t EELS_SlotBegin(uint8_t slotNumber);
uint32_t EELS_SlotEnd(uint8_t slotNumber);

#endif 
/* EELS_H_ */ 