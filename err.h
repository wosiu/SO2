/**
* SO zadanie zaliczeniowe nr 2
* Michal Wos mw336071
*/
#ifndef _ERR_
#define _ERR_

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie, kod bledu czytany z errno */
extern void syserr(const char *fmt, ...);

/* jak wyzej, ale kod bledu czyta z argumentu b */
extern void err(int b, const char *fmt, ...);

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...);

/* jak wyzej, ale kod bledu czyta z argumentu b */
extern void mfatal(int b, const char *fmt, ...);

#endif
