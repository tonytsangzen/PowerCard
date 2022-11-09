#include <string.h>
#include "avg.h"



void AvgInit(AvgFilter* f){
	memset(f, 0, sizeof(AvgFilter));
}



unsigned short AvgPut(AvgFilter* f, unsigned short cur){
	f->subTotal += cur;
	f->subCnt++;

	if(f->subCnt == SUB_FILTER_LEVEL){
		f->total += f->subTotal;
		f->history[f->idx] = f->subTotal;
		f->idx++;
		f->idx%=HISTORY_LENGH;
		f->total -= f->history[f->idx];
		f->subTotal = 0;
		f->subCnt = 0;
	}
	return f->total / (HISTORY_LENGH * SUB_FILTER_LEVEL);
}

