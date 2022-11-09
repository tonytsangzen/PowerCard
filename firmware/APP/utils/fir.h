#ifndef SAMPLEFILTER_H_
#define SAMPLEFILTER_H_

#define TAP_NUM 29

typedef struct {
  short history[TAP_NUM];
} FirFilter;

void FirInit(FirFilter* f);
unsigned short FirPut(FirFilter* f, unsigned short input);
#endif
