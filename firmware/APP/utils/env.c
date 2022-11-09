#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "CH58x_common.h"
#include "env.h"

#define ENV_PART_SIZE EEPROM_PAGE_SIZE
#define ENV_ITEM_SIZE 16
#define ENV_ITEM_NUM 256

typedef struct {
	uint8_t  id;
	uint8_t  data[13];
	uint16_t sum;
}EnvItem;


void EnvLoad(int id, EnvItem* item){
	EnvItem temp;

	for(int i = 0; i < ENV_ITEM_NUM; i++){
		EEPROM_READ(ENV_ITEM_SIZE * i, &temp, ENV_ITEM_SIZE);
		if(temp.id == id){
			memcpy(item, &temp, ENV_ITEM_SIZE);
		}
	}
}

void EnvSave(int id, EnvItem* item){
	EnvItem temp;
	for(int i = 0; i < ENV_ITEM_NUM; i++){
		EEPROM_READ(ENV_ITEM_SIZE * i, &temp, ENV_ITEM_SIZE);
		if(temp.id == 0xFF){
			EEPROM_WRITE(ENV_ITEM_SIZE * i, item, ENV_ITEM_SIZE);
			return;
		}
	}
}

void EnvInit(void){
	EnvItem item[32];
	int freeCnt = 0;

	for(int i = 0; i < ENV_ITEM_NUM; i++){
		EEPROM_READ(ENV_ITEM_SIZE * i, &item[0], ENV_ITEM_SIZE);
		if(item[0].id == 0xFF){
			freeCnt++;
		}
	}

	if(freeCnt >= 16)
		return;

	memset(item, 0xFF, sizeof(item));

	for(int i = 0; i<32; i++){
		EnvLoad(i, &item[i]);
	}

	EEPROM_ERASE(0, EEPROM_PAGE_SIZE);

	for(int i = 0; i < 32; i++){
		if(item[i].id != 0xFF)
			EnvSave(i, &item[i]);
	}
}


int EnvLoadInt(int id){
	int value = 0;
	EnvItem item;
	EnvLoad(id, &item);
	if(item.id == id){
		value = atoi(item.data);
	}
	return value;
}

void EnvSaveInt(int id, int value){
	EnvItem item;
	item.id = id;
	item.sum = id;
	sprintf(item.data, "%d", value);
	for(int j = 0; j < sizeof(item.data);  j++)
		item.sum += item.data[j];
	EnvSave(id, &item);
}



float EnvLoadFloat(int id){
	float value = 0;
	EnvItem item;
	EnvLoad(id, &item);
	if(item.id == id){
		value = atof(item.data);
	}
	return value;
}

void EnvSaveFloat(int id, float value){
	EnvItem item;
	item.id = id;
	item.sum = id;
	sprintf(item.data, "%f", value);
	for(int j = 0; j < sizeof(item.data);  j++)
		item.sum += item.data[j];
	EnvSave(id, &item);
}

