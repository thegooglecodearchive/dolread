#include <reader_core.h>

#define ASSERT assert

#define TICK (getSysTick())
#define DAY_TICKS (24 * 3600 * 1000)

struct tTimeMgr gTime;

static unsigned int getSysTick(void) {
  SYSTEMTIME st;
  GetSystemTime(&st);
  return (st.wHour * 3600 + st.wMinute * 60 + st.wSecond) * 1000 + st.wMilliseconds;
}

void StartTime(unsigned char bNew)
{
	struct tTimer * pTimer = &(gTime.rBank[0]);
	ASSERT(pTimer != NULL);

	pTimer->bRunning = true;

	if (!bNew)
	{
		pTimer->nRefTick -= TICK-pTimer->nRefTick;
		return;
	}

	pTimer->nRefTick	= 0;

  sim_timer_start();

	gTime.nCount		= 1;

}

void PauseTime()
{
	struct tTimer * pTimer = &(gTime.rBank[0]);
	ASSERT(pTimer != NULL);

	pTimer->bRunning = false;

}

unsigned char	NewTimer(unsigned char bStarted)
{
	struct tTimer * pTimer = NULL;

	ASSERT(gTime.nCount < TIME_MGR_CAPACITY-1);

	pTimer = &(gTime.rBank[gTime.nCount]);
	ASSERT(pTimer != NULL);

	pTimer->bRunning = bStarted;
	pTimer->nRefTick = 0;

	if (bStarted)
		pTimer->nRefTick = TICK;

	return gTime.nCount++;	
}

void	StartTimer(unsigned char nTimerId)
{
	struct tTimer * pTimer = &(gTime.rBank[nTimerId]);
	ASSERT(pTimer != NULL);

	pTimer->bRunning = true;
	pTimer->nRefTick = TICK-pTimer->nRefTick;
}

unsigned int	Tick(unsigned char	nTimerId)
{
	struct tTimer * pTimer = &(gTime.rBank[nTimerId]);
  int elapsed;
	ASSERT(pTimer != NULL);

  if (pTimer->bRunning == true) {
    elapsed = (int)(TICK-pTimer->nRefTick);
    //hour turned when a day passed
    if (elapsed < 0) {
      elapsed += DAY_TICKS;
    }
    //assert(elapsed > 0);
    if (elapsed <= 0) {
      char buf[32] = {0};
      sprintf(buf, "elasped=%d", elapsed);
      MessageBox(reader_hwnd, buf, "elaspsed value error", MB_OK);
    }
    return elapsed;
  }

	return pTimer->nRefTick;
}

void PauseTimer(unsigned char nTimerId)
{
	struct tTimer * pTimer = &(gTime.rBank[nTimerId]);
	ASSERT(pTimer != NULL);

	pTimer->nPauseTick = TICK;

	pTimer->bRunning = false;	
}

void UnpauseTimer(unsigned char nTimerId)
{
	struct tTimer * pTimer = &(gTime.rBank[nTimerId]);
	ASSERT(pTimer != NULL);


	pTimer->nRefTick += TICK - pTimer->nPauseTick;

	pTimer->bRunning = true;	
}

void ResetTimer(unsigned char nTimerId)
{
	struct tTimer * pTimer = &(gTime.rBank[nTimerId]);
	ASSERT(pTimer != NULL);

	pTimer->nRefTick = TICK;
}
