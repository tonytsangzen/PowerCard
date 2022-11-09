#include "CH58x_common.h"
#include "menu/menu.h"
#include "menu/key.h"
#include "utils/fir.h"
#include "utils/avg.h"
#include "utils/env.h"
#include "power_card.h"

static short RoughCalib_Value = 0;
static short voltage = 0;
static short current = 0;

static unsigned short SettingVoltage = 5000;
static unsigned short SettingCurrent = 1000;
static int mode = 0;

static FirFilter VoltageFilter;
static AvgFilter CurrentFilter;

void PowerCardSetVoltage(int mV){
	short pwm =  mV * 8000 /32000;
	if(pwm > 8000)
		pwm = 8000;
    TMR2_Disable();
    TMR2_PWMActDataWidth(pwm); // 占空比 50%, 修改占空比必须暂时关闭定时器
    TMR2_Enable();
}

void PowerCardEnableOutPut(int en){
	if(en){
	    GPIOB_SetBits(GPIO_Pin_21);
	}else{
	    GPIOB_ResetBits(GPIO_Pin_21);
	}
}

int PowerCardVoltage(){
	return voltage;
}

int PowerCardCurrent(){
	return current;
}


void keyUp(void){
	printf("keyUp\n");
	if(mode == 1 && SettingVoltage < 30000)
		SettingVoltage += 100;
	else if(mode == 2 && SettingCurrent < 4000)
		SettingCurrent += 10;

	MenuShowVoltage(SettingVoltage);
	MenuShowCurrent(SettingCurrent);
}



void keyDown(void){
	printf("keyDown\n");
	if(mode == 1 && SettingVoltage > 0)
		SettingVoltage -= 100;
	else if(mode == 2 && SettingCurrent > 0)
		SettingCurrent -= 10;

	MenuShowVoltage(SettingVoltage);
	MenuShowCurrent(SettingCurrent);
}

void keyEnter(void){
	printf("keyEnter\n");
	if(mode ==  1)
		mode = 2;
	else if(mode == 2)
		mode = 1;

	MenuSetMode(mode);
}

void keyLock(void){
	printf("keyLock\n");
	if(mode){
		mode = 0;
		EnvSaveInt(0, SettingVoltage);
		EnvSaveInt(1, SettingCurrent);

		FusbSetRequestVolatage(SettingVoltage);
		PowerCardSetVoltage(SettingVoltage);
		PowerCardSetCurrentLimit(SettingCurrent);
		PowerCardEnableOutPut(TRUE);
	}else{
		mode = 1;
		PowerCardEnableOutPut(FALSE);
		MenuShowVoltage(SettingVoltage);
		MenuShowCurrent(SettingCurrent);
	}
	MenuSetMode(mode);
}

void PowerCardInit(void){

	SettingVoltage = EnvLoadInt(0);
	SettingCurrent =  EnvLoadInt(1);

	FusbSetRequestVolatage(SettingVoltage);

	printf("%d %d\n", SettingVoltage, SettingCurrent);
	//set freq to 600Khz
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);

    //set voltage
	printf("init pwm...\n");
	GPIOA_ResetBits(GPIO_Pin_10); // 配置PWM1 PA10
	GPIOA_ModeCfg(GPIO_Pin_10, GPIO_ModeOut_PP_5mA);

	TMR1_PWMInit(High_Level, PWM_Times_1);
	TMR1_PWMCycleCfg(8000);
    TMR1_Disable();
    TMR1_PWMActDataWidth(1); // 占空比 50%, 修改占空比必须暂时关闭定时器
    TMR1_Enable();

    //set current
	GPIOA_ResetBits(GPIO_Pin_11); // 配置PWM1 PA10
	GPIOA_ModeCfg(GPIO_Pin_11, GPIO_ModeOut_PP_5mA);

	TMR2_PWMInit(High_Level, 1);
	TMR2_PWMCycleCfg(8000);
    TMR2_Disable();
    TMR2_PWMActDataWidth(1); // 占空比 50%, 修改占空比必须暂时关闭定时器
    TMR2_Enable();

	PowerCardSetVoltage(SettingVoltage);
	PowerCardSetCurrentLimit(SettingCurrent);

    //Enable Pin
	printf("Start Power Card!");
	GPIOB_ResetBits(GPIO_Pin_21);
	GPIOB_ModeCfg(GPIO_Pin_21, GPIO_ModeOut_PP_5mA);

	printf("init adc...\n");
    /* 单通道采样：选择adc通道11做采样，对应 PA7引脚， 带数据校准功能 */
    GPIOA_ModeCfg(GPIO_Pin_7, GPIO_ModeIN_Floating);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_Floating);
    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);

    //set pin
    RegisterKeyFunc(0, keyUp, keyUp, 1);
    RegisterKeyFunc(1, keyDown, keyDown, 1);
    RegisterKeyFunc(2, keyEnter, keyLock, 0);
    RegisterWheelFunc(0, keyUp, keyDown);

    FirInit(&VoltageFilter);
    AvgInit(&CurrentFilter);
}

void PowerCardSetCurrentLimit(int ma){
	short pwm =  ma* 8000 /4000;
	if(pwm > 8000)
		pwm = 8000;
    TMR1_Disable();
    TMR1_PWMActDataWidth(pwm); // 占空比 50%, 修改占空比必须暂时关闭定时器
    TMR1_Enable();
}


void PowerCardRunAdc(){

	short vol = 0;
	short cur = 0;
	static short step = 0;

		ADC_ChannelCfg(11);
		vol  = ADC_ExcutSingleConver() + RoughCalib_Value;
		vol = 525*vol/64;
		voltage = FirPut(&VoltageFilter, vol);

		ADC_ChannelCfg(12);
		cur = ADC_ExcutSingleConver() + RoughCalib_Value;
		cur =  525*cur/512;
		current = AvgPut(&CurrentFilter, cur);

		step++;
		if(step%128 == 0){
			RoughCalib_Value = ADC_DataCalib_Rough(); // 用于计算ADC内部偏差，记录到全局变量 RoughCalib_Value中
			if(mode == 0){
				MenuShowVoltage(voltage);
				MenuShowCurrent(current);
				printf("%d %d\n",voltage, current);
			}
			MenuRefresh();
		}
}



#define POLL_INTERVAL 1

void PowerCardPoll(void){

	static uint32_t lastTick = 0;
	uint32_t now = TMOS_GetSystemClock();

	if(now - lastTick >=  POLL_INTERVAL){
		PowerCardRunAdc();
		KeyLoop();
		lastTick = now;

	}
}

