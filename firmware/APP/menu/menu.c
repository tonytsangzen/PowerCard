#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fb_draw.h"

#define MAX_ICON	8
#define ICON_SIZE_X	16
#define ICON_SIZE_Y	1

static char* frameBuffer;
static void (*Refresh)(void);

struct Menu{
	char* icon[MAX_ICON];
	char* info;
	int setVoltage;
	int setCurrent;
	int current;
	int voltage;
	int inputVoltage;
	int mode;
	int blick;
};
static struct Menu menu = {
		0
};

void MenuInit(char* fb, void (*ref)(void)){
	memset(&menu, 0, sizeof(menu));
	Refresh = ref;
	frameBuffer = fb;
	menu.inputVoltage = 5000;
}

void MenuSetMode(int mode){
	menu.mode = mode;
}

void MenuSetInputVoltage(int voltage){
	menu.inputVoltage = voltage;
}

void MenuShowVoltage(int voltage){
	menu.voltage = voltage;
}

void MenuShowCurrent(int current){
	menu.current = current;
}


void MenuShowDebug(char* info){
	menu.info = info;
}

void MenuRefresh(void){
	char temp[16];
	menu.blick++;
	menu.blick%=4;

	for(int i = 0; i < MAX_ICON; i++){
		if(menu.icon[i]){
			DrawBitmap(frameBuffer, i*16, 0, 16, 1, menu.icon[i]);
		}
	}

	sprintf(temp, "Input  :     %dV", menu.inputVoltage/1000);
	DrawString(frameBuffer, 0, 2, temp);
	if(menu.mode == 1 && menu.blick > 1){
		sprintf(temp, "Output :       ");
		DrawString(frameBuffer, 0, 3, temp);
	}else{
		sprintf(temp, "Output : %4d.%1dV", menu.voltage/1000, menu.voltage/100 - (menu.voltage/1000) * 10);
		DrawString(frameBuffer, 0, 3, temp);
	}

	if(menu.mode == 2 && menu.blick > 1){
		sprintf(temp, "Current:      ");
		DrawString(frameBuffer, 0, 4, temp);
	}else{
		sprintf(temp, "Current: %5dmA", menu.current);
		DrawString(frameBuffer, 0, 4, temp);
	}

	if(menu.mode){
		sprintf(temp, "                ");
		DrawString(frameBuffer, 0, 5, temp);
	}else{
		sprintf(temp, "Power  : %5dmW", menu.current * menu.voltage/1000);
	}
	DrawString(frameBuffer, 0, 5, temp);

	if(menu.info)
		DrawString(frameBuffer, 0, 0, menu.info);

	Refresh();
}
