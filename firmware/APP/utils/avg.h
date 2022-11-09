#ifndef __AVG_H__
#define __AVG_H__

#define HISTORY_LENGH		32
#define SUB_FILTER_LEVEL	32

typedef struct {
	short history[HISTORY_LENGH];
	short subTotal;
	int total;
	unsigned char idx;
	unsigned char subCnt;
}AvgFilter;


void AvgInit(AvgFilter* f);
unsigned short AvgPut(AvgFilter* f, unsigned short cur);
#endif
