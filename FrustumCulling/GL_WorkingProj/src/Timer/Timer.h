#pragma once
#include <windows.h>

class Timer
{
public:
	Timer(void);
	~Timer(void);

	void StartTiming();
	double TimeElapsed();
	double TimeElapsedInMS();

private:
	UINT64 StartTime;
	double ElapsedTime;

};

void InitTimeOperation();