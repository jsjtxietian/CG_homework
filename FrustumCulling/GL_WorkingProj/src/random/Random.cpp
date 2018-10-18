#include <windows.h>
#include "Random.h"

//fast random funcs
float RNDtable[65536];
static WORD RNDcurrent=0;

void rndInit(void){ srand(0); for(int i=0;i<65536;i++) RNDtable[i]=((float)rand())/32767.0f; }
float rnd(float from,float to) { RNDcurrent++; return from+(to-from)*RNDtable[RNDcurrent]; if (RNDcurrent>=65535) RNDcurrent=0; }
float rnd01() { RNDcurrent++; return RNDtable[RNDcurrent]; if (RNDcurrent>=65535) RNDcurrent=0; }
void rndSeed(int seed) { RNDcurrent=seed; if (RNDcurrent>=65535) RNDcurrent=0; }
float rndTable(WORD index) {return RNDtable[index];}