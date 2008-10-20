/* PAlib Timer Demo
 * by Chris Liu
 */

#include <PA9.h>  // PAlib include

enum {
    POS_X = 6,
    POS_Y = 11,
    CENTI_X = POS_X + 16,
    SEC_X = POS_X + 11,
    MIN_X = POS_X + 6
};

void outputUsage(bool screen) {
    char *usageMsg[] = {"A: Start", "B: Pause", "X: Clear"};
    int pos = 1, i;

    for (i = 0; i < 3; i++, pos += 2) {        
        PA_OutputSimpleText(screen, 1, pos, usageMsg[i]);        
    }
}

void clearTime(bool screen) {
    PA_OutputSimpleText(screen, POS_X, POS_Y, "000 : 00 : 00 : 00");
}

void refreshTime(bool screen, u32 tick) {
    u32 centiSec = (tick/10);
    u32 sec = centiSec / 100;
    u32 min = sec / 60;
    u32 hr = min / 60;
    static u32 oldCentiSec, oldSec, oldMin, oldHr;

    centiSec %= 100;
    sec %= 60;
    min %= 60;
    if (centiSec != oldCentiSec) {
        if (centiSec < 10) {
            PA_OutputText(screen, CENTI_X, POS_Y,"0%d", centiSec);
        } else {
            PA_OutputText(screen, CENTI_X, POS_Y,"%d", centiSec);
        }
        oldCentiSec = centiSec;
    }

    if (sec != oldSec) {
        if (sec < 10) {
            PA_OutputText(screen, SEC_X, POS_Y,"0%d", sec);
        } else {
            PA_OutputText(screen, SEC_X, POS_Y,"%d", sec);
        }
        oldSec = sec;
    }

    if (min != oldMin) {
        if (min < 10) {
            PA_OutputText(screen, MIN_X, POS_Y,"0%d", min);
        } else {
            PA_OutputText(screen, MIN_X, POS_Y,"%d", min);
        }
        oldMin = min;
    }

    if (hr != oldHr) {
        if (hr < 10) {
            PA_OutputText(screen, POS_X, POS_Y,"00%d", hr);
        } else if (hr < 100) {
            PA_OutputText(screen, POS_X, POS_Y,"0%d", hr);
        } else {
            PA_OutputText(screen, POS_X, POS_Y,"%d", hr);
        }
        oldHr = hr;
    }
}

int main(void)	{
    u8 timerId;
    bool timerStarted = false, cleared = false;

    // PAlib Inits
    PA_Init();
    PA_InitVBL();

    // Text Init
    PA_InitText(1, // Top screen...
            2);
    PA_InitText(0, // Bottom screen...
            2);

    // Write usage text...
    outputUsage(0);

    // Reset time at the first step
    clearTime(1);
    StartTime(true);
    timerId = NewTimer(true);
    PauseTimer(timerId);
    while(1)    { // Inifinite loop
        if (Pad.Newpress.A) {
            //Start timer
            if (! timerStarted) {
                UnpauseTimer(timerId);
                timerStarted = true;
            }
        } else if (Pad.Newpress.B) {
            //Pause Timer
            if (timerStarted) {
                PauseTimer(timerId);
                timerStarted = false;
            }
        } else if (Pad.Newpress.X) {
            //Clear Timer
            ResetTimer(timerId);
            PauseTimer(timerId);
            timerStarted = false;
            if (! cleared) {                
                clearTime(1);
                cleared = true;
            }
            refreshTime(1, 0);
        }
        if (timerStarted) {
            refreshTime(1, Tick(timerId));
            cleared = false;
        }
        PA_WaitForVBL();
    }

    return 0;
}

