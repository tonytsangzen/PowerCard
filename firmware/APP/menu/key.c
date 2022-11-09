#include "CH58x_common.h"
#include "key.h"

struct KeyInfo{
	int state;
	int pin;
	int ts;
	int repeat;
	void (*shortPress)(void);
	void (*longPress)(void);
};

struct WheelInfo{
	int state;
	int pinA;
	int pinB;
	void (*wheelUp)(void);
	void (*wheelDown)(void);
};

struct KeyInfo keys[KEY_NUM] = {
	{0, GPIO_Pin_8, 0, 1, 0, 0},
	{0, GPIO_Pin_16, 0, 1, 0, 0},
	{0, GPIO_Pin_17, 0, 0, 0, 0}
};

struct WheelInfo wheels[WHEEL_NUM] = {
	{0, GPIO_Pin_20, GPIO_Pin_18, 0, 0}
};


void RegisterWheelFunc(int keyCode, void(*wheelUp)(void), void(*wheelDown)(void)){
	if(keyCode < WHEEL_NUM){
		wheels[keyCode].wheelUp=wheelUp;
		wheels[keyCode].wheelDown=wheelDown;
		wheels[keyCode].state = 0;
	    GPIOB_ModeCfg(wheels[keyCode].pinA, GPIO_ModeIN_PU);
	    GPIOB_ModeCfg(wheels[keyCode].pinB, GPIO_ModeIN_PU);
	}
}

void RegisterKeyFunc(int keyCode, void(*shortPress)(void), void(*longPress)(void), int repeat){
	if(keyCode < KEY_NUM){
		keys[keyCode].shortPress = shortPress;
		keys[keyCode].longPress = longPress;
		keys[keyCode].repeat = repeat;
	    GPIOB_ModeCfg(keys[keyCode].pin, GPIO_ModeIN_PU);
	}
}


void WheelLoop(void){
	for(int i = 0; i < WHEEL_NUM; i++){
		switch(wheels[i].state){
		case 0://IDLE
			if(!GPIOB_ReadPortPin(wheels[i].pinA))
				wheels[i].state = 3;
			break;
		case 3://detect
			if(GPIOB_ReadPortPin(wheels[i].pinA)){
				if(!GPIOB_ReadPortPin(wheels[i].pinB)){
						if(wheels[i].wheelUp)
							wheels[i].wheelUp();
				}else{
					if(wheels[i].wheelDown)
						wheels[i].wheelDown();
				}

				wheels[i].state = 0;
			}
			break;
		default:
			wheels[i].state = 0;
		}

	}
}

void KeyLoop(void){

	for(int i = 0; i < KEY_NUM; i++){
		switch(keys[i].state){
			case 0://IDLE
				if(GPIOB_ReadPortPin(keys[i].pin) == 0)
					keys[i].state = 1;
				else
					keys[i]. ts = 0;
				break;
			case 1://filter
				if(GPIOB_ReadPortPin(keys[i].pin) == 0)
					keys[i].ts++;
				else
					keys[i].state = 0;
				if(keys[i]. ts >= 20)
					keys[i].state = 2;
				break;
			case 2://press
				if(GPIOB_ReadPortPin(keys[i].pin) == 0)
					keys[i].ts++;
				else{
					if(keys[i].shortPress)
						keys[i].shortPress();
					keys[i].state = 0;
				}
				if(keys[i].ts > 200){
					if(keys[i].longPress)
						keys[i].longPress();
					keys[i].state = 3;
				}
				break;
			case 3://long press
				if(GPIOB_ReadPortPin(keys[i].pin) == 0)
					keys[i].ts++;
				else {
					keys[i].state = 0;
				}
				if(keys[i].ts % 50 == 0 && keys[i].longPress && keys[i].repeat)
					keys[i].longPress();
				break;
			default:
				keys[i].state = 0;
				break;
		}
	}

	WheelLoop();
}
