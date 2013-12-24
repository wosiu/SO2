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

int K, N, queClSrvId, queThrClId, queClThrId;
short int serwer_praca = 1;
short int wolne_zasoby[MAX_K], czeka_na_zasoby[MAX_K];
typedef struct { short int pid[2], n[2], k; } para_t;
para_t do_sparowania[MAX_K];
pthread_mutex_t mutex[MAX_K];
pthread_cond_t inni[MAX_K], na_zasob[MAX_K], porzadki;

void exit_server(int sig)
{
	int blad, i;

	serwer_praca = 0;

	// kolejki IPC
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

	// czekamy az wszystkie watki na pewno sie zakoncza
	// TODO

	// mutex i cond
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

    exit(0);
}

void init()
{
	int i, blad;

	// kolejki IPC
	if ((queClSrvId = msgget(CL_SRV_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
        syserr("from %s, line %d: msgget CL_SRV_MKEY", __FILE__, __LINE__);
    if ((queThrClId = msgget(THR_CL_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
        fatal("from %s, line %d: msgget THR_CL_MKEY", __FILE__, __LINE__);
        exit_server(0);
    }
    if ((queClThrId = msgget(CL_THR_MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
        fatal("from %s, line %d: msgget CL_THR_MKEY", __FILE__, __LINE__);
        exit_server(0);
    }

	// mutex i cond
	for (i = 1; i <= K; i++) {
		if ((blad = pthread_mutex_init(mutex + i, 0) != 0)) {
			mfatal(blad, "mutex init");
			exit_server(0);
		}
		if ((blad = pthread_cond_init(inni + i, 0)) != 0) {
			mfatal(blad, "cond init");
			exit_server(0);
		}
		if ((blad = pthread_cond_init(na_zasob + i, 0)) != 0) {
			mfatal(blad, "cond init");
			exit_server(0);
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
		exit_server(0);
	}
}

void unlock(pthread_mutex_t *mtx) {
	int blad;
	if ((blad = pthread_mutex_unlock(mtx)) != 0 && serwer_praca) {
		mfatal(blad, "from %s: mutex unlock", __FILE__);
		exit_server(0);
	}
}

void cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx) {
	int blad;
	if ((blad = pthread_cond_wait(cond, mtx)) != 0 && serwer_praca) {
		mfatal(blad, "from %s: wait on cond", __FILE__);
		exit_server(0);
	}
}

void cond_signal(pthread_cond_t *cond) {
	int blad;
	if ((blad = (pthread_cond_signal(cond))) != 0 && serwer_praca) {
		mfatal(blad, "from %s: signal on cond", __FILE__);
		exit_server(0);
	}
}

void *klient(void *data)
{
	para_t para = *(para_t *)data;
	free(data);
  	long thread_id = pthread_self();
	int i, k = para.k;

	lock(mutex + k);
	if (!serwer_praca) { unlock(mutex + k); return 0;}

	// czy ktos inny juz czeka na zasoby
	while (czeka_na_zasoby[k] > 0) {
		cond_wait(inni + k, mutex + k);
		if (!serwer_praca) { unlock(mutex + k); return 0;}
	}

	// nikt inny nie czeka na zasob, wiec ja czekam na zasob (jesli nie ma):
	czeka_na_zasoby[k] = para.n[0] + para.n[1];
	while (czeka_na_zasoby[k] > wolne_zasoby[k]) {
		cond_wait(na_zasob + k, mutex + k);
		if (!serwer_praca) { unlock(mutex + k); return 0;}
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
        exit_server(0);
    }
	msgThrCl.mesg_type = (long)para.pid[1];
	msgThrCl.partner_pid = para.pid[0];
	if (msgsnd(queThrClId, (char *) &msgThrCl, ThrCliMsgSize, 0) != 0
			&& serwer_praca) {
        fatal("from %s, line %d: msgsnd queThrClId", __FILE__, __LINE__);
		exit_server(0);
	}

	// czeka na informacje od klientow, ze zakonczyli
	long mtype = (para.pid[0] > para.pid[1]) ? para.pid[1] : para.pid[0];

	for (i = 0; i < 2; i++) {
		if (msgrcv(queClThrId, &msgClThr, CliThrMsgSize, mtype, 0)
				!= CliThrMsgSize && serwer_praca) {
			fatal("from %s, line %d: msgrcv queClThrId", __FILE__, __LINE__);
			exit_server(0);
		}
		if (!serwer_praca) { return 0; }
	}

	lock(mutex + k);
	if (!serwer_praca) { unlock(mutex + k); return 0;}

	// zakonczyli, wiec zwalnia zasoby
	wolne_zasoby[k] += para.n[0] + para.n[1];
	// uruchamiam pierwszy czekajacy proces typu k, jesli zwolniono dostatecznie
	// duzo takich zasobow
	if (czeka_na_zasoby[k] <= wolne_zasoby[k]) {
		cond_signal(na_zasob + k);
	}
	unlock(mutex + k);
	return 0;
}

int main(int argc, const char* argv[])
{
	if (argc == 3) {
		K = atoi(argv[1]);
		N = atoi(argv[2]);
	} else {
		syserr("Nieprawidlowa ilosc argumentow", __FILE__, __LINE__);
	}

	if (signal(SIGINT,  exit_server) == SIG_ERR)
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
			exit_server(0);
        }
		if (!serwer_praca) break;

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
			// jest para, tworzymy dla niej watek
    		if ((blad = pthread_create (&th, &attr, klient, (void *)para)) != 0) {
      			free(para);
      			mfatal(blad, "from %s, line %d: create", __FILE__, __LINE__);
      			if (serwer_praca)
      				exit_server(0);
    		}
		}
    }
}
