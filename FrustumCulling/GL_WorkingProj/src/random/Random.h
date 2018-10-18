#ifndef _Random_h
#define _Random_h

#include <windows.h>

void rndInit(void);
float rnd(float from,float to);
float rnd01();
void rndSeed(int seed);
float rndTable(WORD index);

#endif
