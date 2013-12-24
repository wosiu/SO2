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
		syserr( "from %s, line %d: Incorrect number of arguments" );
	}

    if ( ( queClSrvId = msgget( CLIENT_SERVER_MKEY, 0 ) ) == -1 )
        syserr( "from %s, line %d: msgget CLIENT_SERVER_MKEY", __FILE__, __LINE__ );
    if ( ( queThrClId = msgget( THREAD_CLIENT_MKEY, 0 ) ) == -1 )
        syserr( "from %s, line %d: msgget THREAD_CLIENT_MKEY", __FILE__, __LINE__ );
    if ( ( queClThrId = msgget( CLIENT_THREAD_MKEY, 0 ) ) == -1 )
        syserr( "from %s, line %d: msgget CLIENT_THREAD_MKEY", __FILE__, __LINE__ );

	// wysylanie rzadania o zasoby do serwera
    msgClSrv.mesg_type = 1L;
    msgClSrv.k = k;
    msgClSrv.n = n;
    msgClSrv.pid = myPID;

    if ( msgsnd( queClSrvId, (char *) &msgClSrv, ClientServerMsgSize, 0 ) != 0 )
        syserr( "from %s, line %d: msgsnd queClSrvId", __FILE__, __LINE__ );

    // oczekiwanie na zasoby
    if ( msgrcv( queThrClId, &msgThrCl, ThreadClientMsgSize, (long) myPID, 0 ) != ThreadClientMsgSize );
    	syserr("from %s, line %d: msgrcv queThrClId", __FILE__, __LINE__);

	// wykonywanie zadania
	printf("%d %d %hu %hu\n", k, n, myPID, msgThrCl.partner_pid);
	sleep(s);

	// wyslanie informacji o zwrocie zasobow
    msgClThr.mesg_type = (long) myPID;
    msgClThr.finished = (char) 1;
    if ( msgsnd( queClThrId, (char *) &msgClThr, ClientThreadMsgSize, 0 ) != 0 )
        syserr( "from %s, line %d: msgsnd queClThrId, __FILE__, __LINE__" );

	printf( "KONIEC %hu\n", myPID );

    return 0;
}
