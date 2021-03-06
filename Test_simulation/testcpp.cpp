// testcpp.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"



#include "EELS.h"


#define EELS_S0_data_length 5
#define EELS_S1_data_length 8


void PrintSlot(EELS* LogSys, uint8_t SlotNumber, uint8_t linelength);

int main()
{

	//E24LC64 mymem(TWI0, I2CADDR);
	EELS LogSys(TWI0, I2CADDR, 2);
	LogSys.SetSlot(0, 0, 100, EELS_S0_data_length);
	LogSys.SetSlot(1, 100, 150, EELS_S1_data_length);
	/*
	uint8_t addrzero[] = { 0,0,0,0 };
	mymem.eeprom_write(0, addrzero, 4);
	*/
	LogSys.WriteEeprom(0, 0);
	LogSys.WriteEeprom(1, 0);
	LogSys.WriteEeprom(2, 0);
	LogSys.WriteEeprom(49, 0);
	LogSys.WriteEeprom(100, 0x00);
	uint8_t meh[50];
	memset(meh, 0xff, 50);

	char cinchar;
	do
	{
		LogSys.InsertLog(0, meh);
		LogSys.InsertLog(1, meh);
		PrintSlot(&LogSys, 0, LogSys.GetLogSize(0)*2);
		printf("Slot(0) last position:%d\r\n", LogSys.GetLastPos(0));
		PrintSlot(&LogSys, 1, LogSys.GetLogSize(1) * 2);
		printf("Slot(1) last position:%d\r\n", LogSys.GetLastPos(1));
		if (meh[0] == 0xff)
			memset(meh, 0xee, 50);
		else
			memset(meh, 0xff, 50);
		std::cin >> cinchar;
	} while (cinchar == '+');
	

	std::cin.get();
    return 0;
}


void PrintSlot(EELS* LogSys, uint8_t SlotNumber, uint8_t linelength){
	
	unsigned char x;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	uint16_t S_length = LogSys[0][SlotNumber].length;
	uint16_t S_begin = LogSys[0][SlotNumber].begining;

	for (uint16_t i = S_begin; i <S_begin+S_length; i++)
	{
		if ( (i- S_begin) % linelength == 0)
			printf("\r\n%6d\t\t", i);
		if (i == LogSys->GetLogPos(SlotNumber))
			SetConsoleTextAttribute(hConsole, 0x20);
		else
			SetConsoleTextAttribute(hConsole, 7);
		printf("%02x ", LogSys->ReadEeprom(i));
	}
	SetConsoleTextAttribute(hConsole, 7);
	printf("\r\n");

}
