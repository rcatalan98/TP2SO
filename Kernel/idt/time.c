// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <time.h>

static unsigned long ticks = 0;
typedef struct
{
	void (*function)(void);
	int flag;
} timerFunction;
timerFunction timer = {0, 0};

void timerHandler()
{
	ticks++;
	if (timer.flag)
	{
		timer.function();
	}
}

int sTicksElapsed()
{
	return ticks;
}

int secondsElapsed()
{
	return ticks / 18;
}

void timerFunc(void(*f), int toDo)
{
	timer.flag = toDo;
	timer.function = f;
}