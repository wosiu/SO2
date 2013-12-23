globalne:
czeka_na_para_PID[k][0] = czeka_na_para_PID[k][1] = -1;
wolne_zasoby[k] = {N,...,N};


// mutex gdzie trzeba, ale najpierw musze poczytac o co cho

// SERWER:

// do serwera przyszlo zapytanie o zasob k:
if ( żodyn nie czeka na pare, np: czeka_na_para_PID[k][0] == -1 ) {
	czeka_na_para_PID[k][0] = myPID;
	// zrob watek, nadaj mu WID, odpal
} else {
	para_PID[k][1] = myPID;
	signal( na_pare[k] );
}

// do serwera przyszla informacja o zakonczeniu klienta
wid = PIDtoWID[ PID ]; // pobieram wewnetrzne id watku, ktory zajmuje sie klientem
signal ( zakonczenie_klienta[wid] )


// WATEK:

if ( żodyn nie czeka na pare, np.: czeka_na_para_PID[k][0] == -1 ) {
	wait( na_pare[k] );
}
// wyszedl, wiec jest do pary
PID[0] = czeka_na_para_PID[k][0];
PID[1] = czeka_na_para_PID[k][1];
czeka_na_para_PID[k][0] = czeka_na_para_PID[k][1] = -1;

// sprawdza zasoby:
// czy ktos inny czeka na zasoby
if ( not empty( na_zasob[k] ) ) {
	wait( inni[k] );
}
// nikt inny nie czeka na zasob
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

// czeka na info od serwera, ze klienci zakonczyli
for (int i = 0; i < 2; i++ )
	wait( zakonczenie_klienta[ wid_procesu ] );

// zwalnia zasoby
wolne_zasoby[k] += n + m
// jesli zwolnienie zasoby powoduje, ze moge uruchomic pierwszy czekajacy proces
if ( czeka_na_zasoby[k] <= wolne_zasoby[k] ) {
	signal( na_zasob[k] );
}
