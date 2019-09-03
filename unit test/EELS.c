/*
 * EELS.c
 *
 * Created: 7/24/2019 11:58:42 AM
 *  Author: hasan.jaafar
 */ 

#include "EELS.h"
#define __EELS_DBG__ (1)
#include <math.h>

uint32_t _EELS_ReadAddr(uint8_t slotNumber, uint32_t addr);

uint8_t _NumOfSlot;
ee_slot_t _slot_arr[EELS_MAX_SLOTS];

struct eeprom_func{
	int32_t (*eeprom_read)(uint32_t addr, uint8_t* const buf, uint16_t length);
	int32_t (*eeprom_write)(uint32_t addr, uint8_t* const buf, uint16_t length);
};



/* ==================================================================== */
/* 						PUBLIC FUNCTIONS 								*/
/* ==================================================================== */
struct eeprom_func _eeprom_obj;

uint8_t EELS_Init(int32_t (*readFunc)(uint32_t, uint8_t* const, uint16_t), int32_t (*writeFunc)(uint32_t,uint8_t* const, uint16_t)){
	_eeprom_obj.eeprom_read = readFunc;
	_eeprom_obj.eeprom_write = writeFunc;
	return 0;
}



uint8_t EELS_SetSlot(uint8_t slotNumber, uint32_t begin_addr, uint16_t length, uint8_t data_length) {
	if (slotNumber >= EELS_MAX_SLOTS)
		return 1; //out of boundary!
	_slot_arr[slotNumber].begining = begin_addr;
	_slot_arr[slotNumber].length = length;
	/*user data length*/
	_slot_arr[slotNumber].raw_data_size = data_length;
	/*calculate initial counter max*/ // +1 at least 1 byte for counter
	uint16_t init_counter_max = (uint16_t)ceil(length / data_length + _EELS_CRC_BYTE_LENGTH+1) + 2; //always add +2 extra
	uint8_t bits_required = (uint8_t)ceil(log2(init_counter_max));
	_slot_arr[slotNumber]._counter_bytes = (uint8_t)ceil(bits_required / 8.0f);
	/*Slot datalength*/
	_slot_arr[slotNumber].slot_log_length = data_length + _EELS_CRC_BYTE_LENGTH + _slot_arr[slotNumber]._counter_bytes;
	/*Recalculate counter max*/
	_slot_arr[slotNumber]._counter_max = (uint16_t)ceil(length / _slot_arr[slotNumber].slot_log_length)+2; //readjust max counter

	_EELS_FindLastPos(slotNumber); //now
	//printf("Max counter: %d", _slot_arr[slotNumber]._counter_max);
	//printf(" - counter bytes: %d\r\n", _slot_arr[slotNumber]._counter_bytes);

	return 0;
}


void EELS_InsertLog(uint8_t slotNumber, uint8_t* data) {
	
	#if (__EELS_DBG__)
	_EELS_FindLastPos(slotNumber);
	#endif
	uint32_t cnt = _slot_arr[slotNumber].current_counter;
	uint32_t mypos = _slot_arr[slotNumber].current_position;
	uint32_t S_begin = _slot_arr[slotNumber].begining;

	//-------- instead of this: lets make proper last pos identifier:
	if (!(_slot_arr[slotNumber].current_position == S_begin && _slot_arr[slotNumber].current_counter == 0)) // put description of this situation...
		mypos += _slot_arr[slotNumber].slot_log_length;
	//=================================================
	if (++cnt > _slot_arr[slotNumber]._counter_max) //if counter goes beyond counter max -> set back to 1
		cnt = 1;
	
	if ((mypos + _slot_arr[slotNumber].slot_log_length) > S_begin + _slot_arr[slotNumber].length) // set position to slot begining
	mypos = S_begin;
	
	/*=== LETS DO A WRITE ===*/
	char char_cnt[4] = { (char)(cnt), (char)(cnt >> 8), (char)(cnt >> 16), (char)(cnt >> 24) };
	_eeprom_obj.eeprom_write(mypos, (uint8_t*)char_cnt, _slot_arr[slotNumber]._counter_bytes);
	mypos += _slot_arr[slotNumber]._counter_bytes;
	uint8_t mycrc = xcrc8(data, _slot_arr[slotNumber].raw_data_size);
	_eeprom_obj.eeprom_write(mypos, (uint8_t*)(&mycrc), _EELS_CRC_BYTE_LENGTH); 
	mypos += _EELS_CRC_BYTE_LENGTH;
	_eeprom_obj.eeprom_write(mypos, data,_slot_arr[slotNumber].raw_data_size);
	mypos += _slot_arr[slotNumber].raw_data_size;
	
	#if (__EELS_DBG__)
		_EELS_FindLastPos(slotNumber);
	#else
		_slot_arr[slotNumber].current_counter = cnt;
		_slot_arr[slotNumber].current_position = mypos;
	#endif
}


bool EELS_ReadLast(uint8_t slotNumber, uint8_t* const buf){
	_EELS_FindLastPos(slotNumber);
	#if _EELS_CRC_BYTE_LENGTH == 1
		uint8_t slotcrc=  0 ;
		_eeprom_obj.eeprom_read(_slot_arr[slotNumber].current_position+ _EELS_CRC_BYTE_LENGTH, (uint8_t*)&slotcrc, _EELS_CRC_BYTE_LENGTH);
		_eeprom_obj.eeprom_read(_slot_arr[slotNumber].current_position + _slot_arr[slotNumber]._counter_bytes + _EELS_CRC_BYTE_LENGTH ,
								buf,
								_slot_arr[slotNumber].raw_data_size);
		uint8_t calculatedCrc = xcrc8(buf,_slot_arr[slotNumber].raw_data_size);
	#endif
	return slotcrc == calculatedCrc;
}




uint8_t EELS_SlotLogSize(uint8_t slotNumber){
	return _slot_arr[slotNumber].slot_log_length;
}

uint32_t EELS_SlotBegin(uint8_t slotNumber){
	return _slot_arr[slotNumber].begining;
}

uint32_t EELS_SlotEnd(uint8_t slotNumber){
	return _slot_arr[slotNumber].begining + _slot_arr[slotNumber].length;
}


/* ==================================================================== */
/* 						PRIVATE FUNCTIONS 								*/
/* ==================================================================== */

uint32_t _EELS_ReadAddr(uint8_t slotNumber, uint32_t addr)
{
	uint32_t ret_val = 0;
	
	_eeprom_obj.eeprom_read(addr, (uint8_t*)&ret_val, _slot_arr[slotNumber]._counter_bytes);
	/*uint8_t buff;
	for (int i = 0; i < _slot_arr[slotNumber]._counter_bytes ; i++) {
		ret_val <<= 8;
		_eeprom_obj.eeprom_read(addr, &buff,1);
		ret_val |= buff;
	}*/
	
	return ret_val;
}

int32_t _EELS_FindLastPos(uint8_t slotNumber) {
	uint16_t _counter_max = _slot_arr[slotNumber]._counter_max; //don't use 0s. although the EEPROM comes with 0xff filled
	uint16_t SL = _slot_arr[slotNumber].slot_log_length;
	uint32_t S_begin = _slot_arr[slotNumber].begining;
	uint16_t length = _slot_arr[slotNumber].length;
	
	uint32_t pos = S_begin;
	uint32_t cnt = _EELS_ReadAddr(slotNumber, S_begin);   
	if (cnt > _slot_arr[slotNumber]._counter_max)
		cnt = 0;
	uint32_t tmp_cnt = 0;

	for (uint32_t i=(S_begin+ SL); i<(S_begin + length); i+=SL) {
		if (i + SL > S_begin+length)
		continue;
		tmp_cnt = _EELS_ReadAddr(slotNumber, i); //read the address 

		if (tmp_cnt == cnt + 1){// if (current = previous +1)
			cnt = tmp_cnt;
			pos = i;
			}else if (tmp_cnt == 1 && cnt == _counter_max) { //if (previous == max AND current == 1)
			cnt = tmp_cnt;
			pos = i;
		}
	}
	if (cnt > _slot_arr[slotNumber]._counter_max)
		cnt = 1;
	_slot_arr[slotNumber].current_position = pos;
	_slot_arr[slotNumber].current_counter = cnt;
	return pos;
}