/* timer.c */

#include "../ptpd.h"

#define TIMER_INTERVAL 1
int elapsed;

void catch_alarm(int sig)
{
  elapsed += TIMER_INTERVAL;
  
  /*
   * DBGV() calls vsyslog() which doesn't seem to be reentrant:
   * with Linux 2.6.23 and libc 2.5 (RH5) this even locked up the
   * system.
   *
   * DBGV("catch_alarm: elapsed %d\n", elapsed);
   */
}

void initTimer(void)
{
  struct itimerval itimer;
  
  DBG("initTimer\n");
  
  signal(SIGALRM, SIG_IGN);
  
  elapsed = 0;
  itimer.it_value.tv_sec = itimer.it_interval.tv_sec = TIMER_INTERVAL;
  itimer.it_value.tv_usec = itimer.it_interval.tv_usec = 0;
  
  signal(SIGALRM, catch_alarm);
  setitimer(ITIMER_REAL, &itimer, 0);
}

void timerUpdate(IntervalTimer *itimer)
{
  int i, delta;
  
  delta = elapsed;
  elapsed = 0;
  
  if(delta <= 0)
    return;
  
  for(i = 0; i < TIMER_ARRAY_SIZE; ++i)
  {
    if(itimer[i].interval > 0 && (itimer[i].left -= delta) <= 0)
    {
      itimer[i].left = itimer[i].interval;
      itimer[i].expire = PTRUE;
      DBGV("timerUpdate: timer %u expired\n", i);
    }
  }
}

void timerStop(UInteger16 index, IntervalTimer *itimer)
{
  if(index >= TIMER_ARRAY_SIZE)
    return;
  
  itimer[index].interval = 0;
}

void timerStart(UInteger16 index, UInteger16 interval, IntervalTimer *itimer)
{
  if(index >= TIMER_ARRAY_SIZE)
    return;
  
  itimer[index].expire = PFALSE;
  itimer[index].left = interval;
  itimer[index].interval = itimer[index].left;
  
  DBGV("timerStart: set timer %d to %d\n", index, interval);
}

ptpdBoolean timerExpired(UInteger16 index, IntervalTimer *itimer)
{
  timerUpdate(itimer);
  
  if(index >= TIMER_ARRAY_SIZE)
    return PFALSE;
  
  if(!itimer[index].expire)
    return PFALSE;
  
  itimer[index].expire = PFALSE;
  
  return PTRUE;
}

ptpdBoolean nanoSleep(TimeInternal *t)
{
  struct timespec ts, tr;

  ts.tv_sec = t->seconds;
  ts.tv_nsec = t->nanoseconds;

  if(nanosleep(&ts, &tr) < 0)
  {
    t->seconds = (Integer32)tr.tv_sec;
    t->nanoseconds = (Integer32)tr.tv_nsec;
    return PFALSE;
  }

  return PTRUE;
}

void timerNow(TimeInternal *time)
{
  struct timeval tv;

  gettimeofday(&tv, 0);
  time->seconds = (Integer32)tv.tv_sec;
  time->nanoseconds = (Integer32)(tv.tv_usec*1000);
}
