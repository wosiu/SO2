globalne:
czeka_na_para_PID[k][0] = czeka_na_para_PID[k][1] = -1;
wolne_zasoby[k] = {N,...,N};


// mutex gdzie trzeba, ale najpierw musze poczytac o co cho

// SERWER:

// do serwera przyszlo zapytanie o zasob k:
if ( żodyn nie czeka na pare, np: czeka_na_para_PID[k] == -1 ) {
	czeka_na_para_PID[k] = myPID;
} else {
	partnerPID = czeka_na_para_PID[k];
	czeka_na_para_PID[k] = -1;
	// zrob watek, nadaj mu WID, odpal z argumentami myPID, partnerPID
}



// WATEK:
pthread_mutex_lock( mutex[k] );

// sprawdza zasoby:
// czy ktos inny czeka na zasoby
if ( not empty( na_zasob[k] ) ) {
	wait( inni[k], mutex[k] );
	lock( mutex[k] );
}
// nikt inny nie czeka na zasob, wiec:
if ( n + m > wolne_zasoby[k] ) {
	czeka_na_zasoby[k] = n + m;
	wait( na_zasob[k] );
}

// wyszedl wiec sa zasoby
wolne_zasoby[k] -= n + m;
czeka_na_zasoby[k] = 0;
// byc moze sa tez inni, ktorzy czekali na zasob, teraz moge jednego z nich uwolnic
signal( inni[k] );

// wysyla informacje do klientow
// gdzie w IPC typkom ustawiamy odpowiednio na PID[0] i PID[1]
// musi to byc w ochronie na wypadek, gdyby klient zdazyl wyslac zakonczenie
// nim obecny watek usnie

// czeka info od klientow, ze zakonczyli
zawiesza sie na read od klientow

// zwalnia zasoby
wolne_zasoby[k] += n + m
// jesli zwolnienie zasoby powoduje, ze moge uruchomic pierwszy czekajacy proces
if ( czeka_na_zasoby[k] <= wolne_zasoby[k] ) {
	signal( na_zasob[k] );
}
