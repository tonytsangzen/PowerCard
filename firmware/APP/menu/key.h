#ifndef __KEY_H__
#define __KEY_H__

#define KEY_NUM		3
#define WHEEL_NUM	1

void RegisterKeyFunc(int keyCode, void(*shortPress)(void), void(*longPress)(void), int repeat);

void RegisterWheelFunc(int keyCode, void(*wheelUp)(void), void(*wheelDown)(void));

void KeyLoop(void);

#endif
