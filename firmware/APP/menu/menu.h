#ifndef __MENU_H__
#define __MENU_H__


void MenuInit();
void MenuShowVoltage(int voltage);
void MenuShowCurrent(int current);
void MenuRefresh(void);
void MenuShowDebug(const char* info);
void MenuSetMode(int mode);
void MenuSetInputVoltage(int voltage);
#endif
