#include "timer.h"

Timer::Timer(void)
{
}

Timer::~Timer(void)
{
}

static UINT64 freq; //timer frequency


void InitTimeOperation()
{
 LARGE_INTEGER s;
 QueryPerformanceFrequency(&s);
 freq=s.QuadPart;
}

UINT64 Time()
{
 LARGE_INTEGER s;
 QueryPerformanceCounter(&s);
 return s.QuadPart;
}
  
UINT64 GetTicksTime()
{
 LARGE_INTEGER s;
 QueryPerformanceCounter(&s);
 return ((s.QuadPart)*1000000/freq);
}

void Timer::StartTiming()
{
	LARGE_INTEGER s;
	QueryPerformanceCounter(&s);
	StartTime=s.QuadPart;
}

//in seconds
double Timer::TimeElapsed()
{
	ElapsedTime=(double)((Time()-StartTime)/freq);
	return ElapsedTime;
}

double Timer::TimeElapsedInMS()
{
	ElapsedTime = (double)((Time() - StartTime) * 1000.0 / freq);
	return ElapsedTime;
}