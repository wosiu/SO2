/**
* SO zadanie zaliczeniowe nr 2
* Michal Wos mw336071
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include "mesg.h"
#include "err.h"

#define MAX_K 100

int queClSrvId, queThrClId, queClThrId;
short int K, N, serwer_praca = 1;
short int wolne_zasoby[MAX_K], czeka_na_zasoby[MAX_K];
typedef struct { short int pid[2], n[2], k; } para_t;
para_t do_sparowania[MAX_K];
pthread_mutex_t mutex[MAX_K], sprzatanie;
pthread_cond_t inni[MAX_K], na_zasob[MAX_K], na_brak_watkow;
short int aktywne_watki; // do porzadkow przy zamykaniu serwera

void serwer_off(int sig);

void init()
{
	int i, blad;

	// kolejki IPC
	if ((queClSrvId = msgget(CL_SRV_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
		syserr("from %s, line %d: msgget CL_SRV_MKEY", __FILE__, __LINE__);
	if ((queThrClId = msgget(THR_CL_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
		fatal("from %s, line %d: msgget THR_CL_MKEY", __FILE__, __LINE__);
		serwer_off(0);
	}
	if ((queClThrId = msgget(CL_THR_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
		fatal("from %s, line %d: msgget CL_THR_MKEY", __FILE__, __LINE__);
		serwer_off(0);
	}

	// mutex i cond
	if ((blad = pthread_mutex_init(&sprzatanie, 0) != 0)) {
		mfatal(blad, "mutex init");
		serwer_off(0);
	}
	if ((blad = pthread_cond_init(&na_brak_watkow, 0)) != 0) {
		mfatal(blad, "cond init");
		serwer_off(0);
	}

	for (i = 1; i <= K; i++) {
		if ((blad = pthread_mutex_init(mutex + i, 0) != 0)) {
			mfatal(blad, "mutex init");
			serwer_off(0);
		}
		if ((blad = pthread_cond_init(inni + i, 0)) != 0) {
			mfatal(blad, "cond init");
			serwer_off(0);
		}
		if ((blad = pthread_cond_init(na_zasob + i, 0)) != 0) {
			mfatal(blad, "cond init");
			serwer_off(0);
		}
	}

	// zasoby, liczniki
	for (i = 1; i <= K; i++) {
		wolne_zasoby[i] = N;
	}
}

void lock(pthread_mutex_t *mtx) {
	int blad;
	if ((blad = pthread_mutex_lock(mtx)) != 0 && serwer_praca) {
		mfatal(blad, "from %s: mutex lock", __FILE__);
		if(serwer_praca) serwer_off(0);
	}
}

void unlock(pthread_mutex_t *mtx) {
	int blad;
	if ((blad = pthread_mutex_unlock(mtx)) != 0 && serwer_praca) {
		mfatal(blad, "from %s: mutex unlock", __FILE__);
		if(serwer_praca) serwer_off(0);
	}
}

void cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx) {
	int blad;
	if ((blad = pthread_cond_wait(cond, mtx)) != 0 && serwer_praca) {
		mfatal(blad, "from %s: wait on cond", __FILE__);
		if(serwer_praca) serwer_off(0);
	}
}

void cond_signal(pthread_cond_t *cond) {
	int blad;
	if ((blad = (pthread_cond_signal(cond))) != 0 && serwer_praca) {
		mfatal(blad, "from %s: signal on cond", __FILE__);
		if(serwer_praca) serwer_off(0);
	}
}

// potrzebne do eleganckiego sprzatania
char inc_aktywne_watki()
{
	lock(&sprzatanie);
	// nie pozwala zwiekszac ilosci watkow, gdy nakazano wylaczenie serwera
	if ( serwer_praca == 1 ) {
		aktywne_watki++;
		unlock(&sprzatanie);
		return 1;
	}
	unlock(&sprzatanie);
	return 0;
}

void dec_aktywne_watki()
{
	lock(&sprzatanie);
	if (--aktywne_watki == 0)
		cond_signal(&na_brak_watkow);
	unlock(&sprzatanie);
}

void *klient(void *data)
{
	#define unlock_dec_exit() {unlock(mutex + k); dec_aktywne_watki(); return 0;}
	para_t para = *(para_t *)data;
	free(data);
  	long thread_id = pthread_self();
	int i, k = para.k;

	lock(mutex + k);
	if (!serwer_praca) unlock_dec_exit();

	// czy ktos inny juz czeka na zasoby
	while (czeka_na_zasoby[k] > 0) {
		cond_wait(inni + k, mutex + k);
		if (!serwer_praca) unlock_dec_exit();
	}

	// nikt inny nie czeka na zasob, wiec ja czekam na zasob (jesli nie ma):
	czeka_na_zasoby[k] = para.n[0] + para.n[1];
	while (czeka_na_zasoby[k] > wolne_zasoby[k]) {
		cond_wait(na_zasob + k, mutex + k);
		if (!serwer_praca) unlock_dec_exit();
	}

	// wyszedl wiec sa zasoby, biore je
	wolne_zasoby[k] -= czeka_na_zasoby[k];
	czeka_na_zasoby[k] = 0;

	// uwalniamy jednego z potencjalnych pozostalych czekajacych
	cond_signal(inni + k);
	unlock(mutex + k);

	printf("Wątek %ld przydziela %d+%d zasobów %d klientom %d %d, pozostało %d zasobów.\n",
		thread_id, para.n[0], para.n[1], para.k, para.pid[0], para.pid[1],
		wolne_zasoby[para.k]);
	fflush(stdout);

	ThreadClientMsg msgThrCl;
	ClientThreadMsg msgClThr;

	// wysyla informacje o partnerze do klientow
	msgThrCl.mesg_type = (long)para.pid[0];
	msgThrCl.partner_pid = para.pid[1];
	if (msgsnd(queThrClId, (char *) &msgThrCl, ThrCliMsgSize, 0) != 0
			&& serwer_praca) {
		fatal("from %s, line %d: msgsnd queThrClId", __FILE__, __LINE__);
		dec_aktywne_watki();
		serwer_off(0);
	}
	msgThrCl.mesg_type = (long)para.pid[1];
	msgThrCl.partner_pid = para.pid[0];
	if (msgsnd(queThrClId, (char *) &msgThrCl, ThrCliMsgSize, 0) != 0
			&& serwer_praca) {
		fatal("from %s, line %d: msgsnd queThrClId", __FILE__, __LINE__);
		dec_aktywne_watki();
		serwer_off(0);
	}

	// czeka na informacje od klientow, ze zakonczyli
	long mtype = (para.pid[0] > para.pid[1]) ? para.pid[1] : para.pid[0];

	for (i = 0; i < 2; i++) {
		if (msgrcv(queClThrId, &msgClThr, CliThrMsgSize, mtype, 0)
				!= CliThrMsgSize && serwer_praca) {
			fatal("from %s, line %d: msgrcv queClThrId", __FILE__, __LINE__);
			dec_aktywne_watki();
			serwer_off(0);
		}
		if (!serwer_praca) { dec_aktywne_watki(); return 0; }
	}

	lock(mutex + k);
	if (!serwer_praca) unlock_dec_exit();

	// zakonczyli, wiec zwalnia zasoby
	wolne_zasoby[k] += para.n[0] + para.n[1];
	// uruchamiam pierwszy czekajacy proces typu k, jesli zwolniono dostatecznie
	// duzo takich zasobow
	if (czeka_na_zasoby[k] <= wolne_zasoby[k]) {
		cond_signal(na_zasob + k);
	}

	unlock_dec_exit();
}

void serwer_off(int sig)
{
	int blad, i;

	lock(&sprzatanie);
	if (serwer_praca == 0) return;
	// jedyne miejsce w kodzie, gdzie zmieniamy wartosc serwer_praca
	serwer_praca = 0;
	unlock(&sprzatanie);

	// Usuwamy kolejki IPC
	// Jesli byl to SIGINT to po stronie serwera odbedzie sie to bez komunikatow
	// o bledach. Klienci natomiast zaraportuja blad po swojej stronie.
	if (msgctl(queClSrvId, IPC_RMID, 0) == -1)
		fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);
	if (msgctl(queThrClId, IPC_RMID, 0) == -1)
		fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);
	if (msgctl(queClThrId, IPC_RMID, 0) == -1)
		fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);

	// odwieszamy wszystkich czekajacych na wait,
	// zwroca mutex i zakoncza sie poniewaz serwer_praca = 0
	for (i = 1; i <= K; i++) {
		if ((blad = pthread_cond_broadcast(inni + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond broadcast", __FILE__,
					__LINE__);
		if ((blad = pthread_cond_broadcast(na_zasob + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond broadcast", __FILE__,
					__LINE__);
	}

	// Wyobrazmy sobie scenariusz w ktorym wiele watkow czekalo na mutexie.
	// Zatem lancuchowo kolejno blokuja i od razu zwalniaja mutex.
	// Lecz aby przejsc dalej i zniszczyc mutex, musimy miec pewnosc,
	// ze zaden mutex nie bedzie w posiadaniu jakiegos watku, zatem czekamy az
	// wszystkie watki sie zakoncza. Nalezy zwrocic uwage, iz nie czekamy az
	// klienci zwroca zasoby (albowiem kolejki IPC usunelismy najpierw).
	// Chcemy jedynie elegancko posprzatac po stronie serwera.
	lock(&sprzatanie);
	while (aktywne_watki > 0)
		cond_wait(&na_brak_watkow, &sprzatanie);
	unlock(&sprzatanie);

	// niszczenie mutex i cond
	// poniewaz wszystko zwolnione, mozemy niszczyc
	for (i = 1; i <= K; i++) {
		if ((blad = pthread_cond_destroy(inni + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond destroy", __FILE__,
					__LINE__);
		if ((blad = pthread_cond_destroy(na_zasob + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond destroy", __FILE__,
					__LINE__);
		if ((blad = pthread_mutex_destroy(mutex + i) != 0))
			mfatal (blad, "Exiting in %s, line %d: mutex destroy", __FILE__,
					__LINE__);
	}

	if ((blad = pthread_cond_destroy(&na_brak_watkow)) != 0)
		mfatal (blad, "Exiting in %s, line %d: cond destroy", __FILE__,
			__LINE__);
	if ((blad = pthread_mutex_destroy(&sprzatanie) != 0))
		mfatal (blad, "Exiting in %s, line %d: mutex destroy", __FILE__,
			__LINE__);


	exit(0);
}

int main(int argc, const char* argv[])
{
	if (argc == 3) {
		K = atoi(argv[1]);
		N = atoi(argv[2]);
	} else {
		syserr("Nieprawidlowa ilosc argumentow", __FILE__, __LINE__);
	}

	if (signal(SIGINT,  serwer_off) == SIG_ERR)
		syserr("from %s, line %d: SIGINT signal", __FILE__, __LINE__);

	ClientServerMsg msgClSrv;
  	pthread_t th;
  	pthread_attr_t attr;
	int blad;
	para_t* para = NULL;

	if ((blad = pthread_attr_init(&attr)) != 0)
		err(blad, "from %s, line %d: attrinit", __FILE__, __LINE__);

	if ((blad = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
		err(blad, "from %s, line %d: setdetach", __FILE__, __LINE__);

	init();

	while (serwer_praca) {
		if (msgrcv(queClSrvId, &msgClSrv, CliSrvMsgSize, 1L, 0) != CliSrvMsgSize
				&& serwer_praca) {
			fatal("from %s, line %d: msgrcv queClSrvId", __FILE__, __LINE__);
			serwer_off(0);
		}
		if (!serwer_praca) break;

		// sprawdzamy czy jest para
		if (do_sparowania[ msgClSrv.k ].k == 0) {
			do_sparowania[ msgClSrv.k ].k = msgClSrv.k;
			do_sparowania[ msgClSrv.k ].n[0] = msgClSrv.n;
			do_sparowania[ msgClSrv.k ].pid[0] = msgClSrv.pid;
		} else {
			do_sparowania[ msgClSrv.k ].n[1] = msgClSrv.n;
			do_sparowania[ msgClSrv.k ].pid[1] = msgClSrv.pid;
			para = (para_t*) malloc(sizeof(para_t));
			*para = do_sparowania[ msgClSrv.k ];
			do_sparowania[msgClSrv.k].k = 0;

			if ( !inc_aktywne_watki() ) break;
			// jest para i serwer dziala, tworzymy dla niej watek
			if ((blad = pthread_create (&th, &attr, klient, (void *)para)) != 0) {
	  			free(para);
	  			mfatal(blad, "from %s, line %d: create", __FILE__, __LINE__);

	  			if (serwer_praca) serwer_off(0);
			}
		}
	}
}
