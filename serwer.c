#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mesg.h"
#include "err.h"

int K, N, queClSrvId, queThrClId, queClThrId;

void exit_server(int sig)
{
    if (msgctl(queClSrvId, IPC_RMID, 0) == -1)
        syserr("msgctl RMID");
    if (msgctl(queThrClId, IPC_RMID, 0) == -1)
        syserr("msgctl RMID");
    if (msgctl(queClThrId, IPC_RMID, 0) == -1)
        syserr("msgctl RMID");

    exit(0);
}
/*
void *watek (void *data)
{
  pid_t thread_pid = getpid();

  printf("Wątek %d przydziela %d+%d zasobów %d klientom %d %d, pozostało %d \
		zasobów.\n", thread_pid, m, n, k, pid0, pid1, wolne_zasoby[k] );

  return 0;
}
*/
void init()
{
	if ( ( queClSrvId = msgget( CLIENT_SERVER_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "msgget CLIENT_SERVER_MKEY" );
    if ( ( queThrClId = msgget( THREAD_CLIENT_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "msgget THREAD_CLIENT_MKEY" );
    if ( ( queClThrId = msgget( CLIENT_THREAD_MKEY, 0666 | IPC_CREAT | IPC_EXCL ) ) == -1 )
        syserr( "msgget CLIENT_THREAD_MKEY" );
}

int main( int argc, const char* argv[] )
{
	if ( argc == 3 ) {
		K = atoi( argv[1] );
		N = atoi( argv[2] );
	} else {
		syserr( "Incorrect number of arguments" );
	}

	init();

    ClientServerMsg msgClSrv;

    if (signal(SIGINT,  exit_server) == SIG_ERR)
        syserr("signal");

    for(;;)
    {
        if ( msgrcv(queClSrvId, &msgClSrv, ClientServerMsgSize, 1L, 0 ) != ClientServerMsgSize )
            syserr("msgrcv queClSrvId");

		printf("%hu %hu %hu\n", msgClSrv.k, msgClSrv.n, msgClSrv.pid);
		// zrob watek
    }
}
