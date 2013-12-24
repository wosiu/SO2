#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mesg.h"
#include "err.h"

int main( int argc, const char* argv[] )
{
	int k, n, s, queClSrvId, queThrClId, queClThrId;
    ClientServerMsg msgClSrv;
    ThreadClientMsg msgThrCl;
    ClientThreadMsg msgClThr;
	short int myPID = getpid();

	if ( argc == 4 ) {
		k = atoi( argv[1] );
		n = atoi( argv[2] );
		s = atoi( argv[3] );
	} else {
		syserr( "Incorrect number of arguments" );
	}

    if ( ( queClSrvId = msgget( CLIENT_SERVER_MKEY, 0 ) ) == -1 )
        syserr( "msgget CLIENT_SERVER_MKEY" );
    if ( ( queThrClId = msgget( THREAD_CLIENT_MKEY, 0 ) ) == -1 )
        syserr( "msgget THREAD_CLIENT_MKEY" );
    if ( ( queClThrId = msgget( CLIENT_THREAD_MKEY, 0 ) ) == -1 )
        syserr( "msgget CLIENT_THREAD_MKEY" );

	// wysylanie rzadania o zasoby do serwera
    msgClSrv.mesg_type = 1L;
    msgClSrv.k = k;
    msgClSrv.n = n;
    msgClSrv.pid = myPID;

    if ( msgsnd( queClSrvId, (char *) &msgClSrv, ClientServerMsgSize, 0 ) != 0 )
        syserr( "msgsnd queClSrvId" );

    // oczekiwanie na zasoby
    if ( msgrcv( queThrClId, &msgThrCl, ThreadClientMsgSize, (long) myPID, 0 ) != ThreadClientMsgSize );
    	syserr("msgrcv queThrClId");

	// wykonywanie zadania
	printf("%d %d %hu %hu\n", k, n, myPID, msgThrCl.partner_pid);
	sleep(s);

	// wyslanie informacji o zwrocie zasobow
    msgClThr.mesg_type = (long) myPID;
    msgClThr.finished = (char) 1;
    if ( msgsnd( queClThrId, (char *) &msgClThr, ClientThreadMsgSize, 0 ) != 0 )
        syserr( "msgsnd queClThrId" );

    return 0;
}
