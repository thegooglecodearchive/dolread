#ifndef __PA_TIMER_SIM_H__
#define __PA_TIMER_SIM_H__ 1

/*
 * this is a millisecond timer
 */
#define TIME_MGR_CAPACITY 16

struct tTimer
{
	unsigned int     nPauseTick;
	unsigned int		nRefTick;
	unsigned char	bRunning;
};

struct tTimeMgr
{
	/* timer 0 as master timer */
	struct tTimer	rBank[TIME_MGR_CAPACITY];
	unsigned char				nCount;
};

extern void StartTime(unsigned char bNew);
extern void PauseTime();

extern unsigned char	NewTimer(unsigned char bStarted);
extern void	StartTimer(unsigned char nTimerId);
extern unsigned int	Tick(unsigned char	nTimerId);
extern void	PauseTimer(unsigned char nTimerId);
extern void	UnpauseTimer(unsigned char nTimerId);
extern void	ResetTimer(unsigned char);

extern void sim_timer_start(void);

#endif // __PA_TIMER_SIM_H__
