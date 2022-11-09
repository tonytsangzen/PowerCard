#ifndef __POWER_CARD_H__
#define __POWER_CARD_H__
extern uint32_t TMOS_GetSystemClock( void );

void FusbSetRequestVolatage(int output);
void FusbInitial(void);
void FusbLoop(void);

void PowerCardInit(void);
void PowerCardEnableOutPut(int en);
void PowerCardSetCurrentLimit(int cur);
void PowerCardPoll(void);


#endif
