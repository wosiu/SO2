#ifndef _ERR_
#define _ERR_

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie, kod bledu czytany z errno */
extern void syserr(const char *fmt, ...);

extern void sysmerr(int b, const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

#endif
