/*
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
lock( mutex[k] );

// czy ktos inny juz czeka na zasoby
// if ( not empty( na_zasob[k] ) ) {
while ( czeka_na_zasoby[k] > 0 ) {
	wait( inni[k], mutex[k] );
}
// nikt inny nie czeka na zasob, wiec ja czekam na zasob jesli trzeba:
czeka_na_zasoby[k] = n + m;
while ( czeka_na_zasoby[k] > wolne_zasoby[k] ) {
	wait( na_zasob[k], mutex[k] );
}
// wyszedl wiec sa zasoby
wolne_zasoby[k] -= n + m;
czeka_na_zasoby[k] = 0;
// byc moze sa tez inni, ktorzy czekali na zasob, teraz moge jednego z nich uwolnic
signal( inni[k] );
unlock( mutex[k] );

// wysyla informacje do klientow
// gdzie w IPC typkom ustawiamy odpowiednio na PID[0] i PID[1]

// czeka info od klientow, ze zakonczyli
zawiesza sie na read od klientow

lock( mutex[k] );
// zwalnia zasoby
wolne_zasoby[k] += n + m
// jesli zwolnienie zasoby powoduje, ze moge uruchomic pierwszy czekajacy proces
while ( czeka_na_zasoby[k] <= wolne_zasoby[k] ) {
	signal( na_zasob[k] );
}
unlock( mutex[k] );*/
