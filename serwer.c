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
// TODO wyzerowac pola w strukturze?
typedef struct { short int pid[2], n[2], k; } para_t;
para_t do_sparowania[MAX_K];

pthread_mutex_t mutex[MAX_K];
pthread_cond_t inni[MAX_K], na_zasob[MAX_K];

void init()
{
	int i, blad;

	/* kolejki IPC */
	if ( ( queClSrvId = msgget( CLIENT_SERVER_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "from %s, line %d: msgget CLIENT_SERVER_MKEY", __FILE__, __LINE__ );
    if ( ( queThrClId = msgget( THREAD_CLIENT_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "from %s, line %d: msgget THREAD_CLIENT_MKEY", __FILE__, __LINE__ );
    if ( ( queClThrId = msgget( CLIENT_THREAD_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "from %s, line %d: msgget CLIENT_THREAD_MKEY", __FILE__, __LINE__ );

	/* mutex i cond */
	for ( i = 1; i <= K; i++ ) {
		if ((blad = pthread_mutex_init(mutex + i, 0) != 0))
			err (blad, "mutex init");
		if ((blad = pthread_cond_init(inni + i, 0)) != 0)
			err (blad, "cond init");
		if ((blad = pthread_cond_init(na_zasob + i, 0)) != 0)
			err (blad, "cond init");
	}

	/* zasoby, liczniki */
	for ( i = 1; i <= K; i++ ) {
		wolne_zasoby[i] = N;
	}
}

void *klient( void *data )
{
	para_t para = *(para_t *)data;
	free(data);
  	pid_t thread_pid = getpid();

 	printf("Wątek %d przydziela %d+%d zasobów %d klientom %d %d, pozostało %d zasobów.\n",
		thread_pid, para.n[0], para.n[1], para.k, para.pid[0], para.pid[1],
		wolne_zasoby[para.k] );

	return 0;
}

void exit_server(int sig)
{
	int blad, i;
	/*int n = 10;
	char passed[n];
	for ( int i = 0; i < n; i++ ) passed[i] = 1;*/

	serwer_praca = 0;

	// broadcast here

	/* mutex i cond */
	for ( i = 1; i <= K; i++ ) {
		if ((blad = pthread_mutex_destroy(mutex + i) != 0))
			mfatal (blad, "Exiting in %s, line %d: mutex destroy", __FILE__, __LINE__);
		if ((blad = pthread_cond_destroy(inni + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond destroy", __FILE__, __LINE__);
		if ((blad = pthread_cond_destroy(na_zasob + i)) != 0)
			mfatal (blad, "Exiting in %s, line %d: cond destroy", __FILE__, __LINE__);
	}

	/* kolejki IPC */
    if (msgctl(queClSrvId, IPC_RMID, 0) == -1)
        fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);
    if (msgctl(queThrClId, IPC_RMID, 0) == -1)
        fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);
    if (msgctl(queClThrId, IPC_RMID, 0) == -1)
        fatal("Exiting in %s, line %d: msgctl RMID", __FILE__, __LINE__);

    exit(0);
}


int main( int argc, const char* argv[] )
{
	if ( argc == 3 ) {
		K = atoi( argv[1] );
		N = atoi( argv[2] );
	} else {
		syserr( "Nieprawidlowa ilosc argumentow", __FILE__, __LINE__ );
	}

	init();

	if (signal(SIGINT,  exit_server) == SIG_ERR)
        syserr("from %s, line %d: SIGINT signal", __FILE__, __LINE__);

    ClientServerMsg msgClSrv;
  	pthread_t th;
  	pthread_attr_t attr;
	int blad;
	para_t* para = NULL;

	if ( ( blad = pthread_attr_init( &attr ) ) != 0 )
		err( blad, "from %s, line %d: attrinit", __FILE__, __LINE__ );

	if ( ( blad = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED ) ) != 0 )
		err( blad, "from %s, line %d: setdetach", __FILE__, __LINE__ );

    while ( serwer_praca ) {
        if ( msgrcv(queClSrvId, &msgClSrv, ClientServerMsgSize, 1L, 0 ) != ClientServerMsgSize )
            syserr("from %s, line %d: msgrcv queClSrvId", __FILE__, __LINE__);

		if ( do_sparowania[ msgClSrv.k ].k == 0 ) {
			do_sparowania[ msgClSrv.k ].k = msgClSrv.k;
			do_sparowania[ msgClSrv.k ].n[0] = msgClSrv.n;
			do_sparowania[ msgClSrv.k ].pid[0] = msgClSrv.pid;
		} else {
			do_sparowania[ msgClSrv.k ].n[1] = msgClSrv.n;
			do_sparowania[ msgClSrv.k ].pid[1] = msgClSrv.pid;
			para = (para_t*) malloc( sizeof( para_t ) );
    		*para = do_sparowania[ msgClSrv.k ];
			do_sparowania[msgClSrv.k].k = 0;
			// jest para, tworzymy dla niej watek
    		if ( ( blad = pthread_create (&th, &attr, klient, (void *)para ) ) != 0 )
      			err ( blad, "from %s, line %d: create", __FILE__, __LINE__ );
		}
    }
    return 0;
}
