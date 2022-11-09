#ifndef __ENV_H__
#define __ENV_H__

#define ENV_ID_VOL_CUR	0
#define ENV_ID_CALIBRATION	1


int EnvLoadInt(int id);
void EnvSaveInt(int id, int value);

float EnvLoadFloat(int id);
void EnvSaveFloat(int id, float value);
#endif
