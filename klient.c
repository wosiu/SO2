/**
* SO zadanie zaliczeniowe nr 2
* Michal Wos mw336071
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mesg.h"
#include "err.h"

int main(int argc, const char* argv[])
{
	int k, n, s, queClSrvId, queThrClId, queClThrId;
    ClientServerMsg msgClSrv;
    ThreadClientMsg msgThrCl;
    ClientThreadMsg msgClThr;
	short int myPID = getpid();

	if (argc == 4) {
		k = atoi(argv[1]);
		n = atoi(argv[2]);
		s = atoi(argv[3]);
	} else {
		syserr("from %s, line %d: Incorrect number of arguments");
	}

    if ((queClSrvId = msgget(CL_SRV_MKEY, 0)) == -1)
        syserr("from %s, line %d: msgget CL_SRV_MKEY", __FILE__, __LINE__);
    if ((queThrClId = msgget(THR_CL_MKEY, 0)) == -1)
        syserr("from %s, line %d: msgget THR_CL_MKEY", __FILE__, __LINE__);
    if ((queClThrId = msgget(CL_THR_MKEY, 0)) == -1)
        syserr("from %s, line %d: msgget CL_THR_MKEY", __FILE__, __LINE__);

	// wysylanie rzadania o zasoby do serwera
    msgClSrv.mesg_type = 1L;
    msgClSrv.k = k;
    msgClSrv.n = n;
    msgClSrv.pid = myPID;

    if (msgsnd(queClSrvId, (char *) &msgClSrv, CliSrvMsgSize, 0) != 0)
        syserr("from %s, line %d: msgsnd queClSrvId", __FILE__, __LINE__);

    // oczekiwanie na zasoby
    if (msgrcv(queThrClId, &msgThrCl, ThrCliMsgSize, (long) myPID, 0) != ThrCliMsgSize)
    	syserr("from %s, line %d: msgrcv queThrClId", __FILE__, __LINE__);

	// wykonywanie zadania
	printf("%d %d %hu %hu\n", k, n, myPID, msgThrCl.partner_pid);
	fflush(stdout);
	sleep(s);

	// wyslanie informacji o zwrocie zasobow
    long mtype = (msgThrCl.partner_pid > myPID) ? myPID : msgThrCl.partner_pid;
    msgClThr.mesg_type = mtype;
    msgClThr.finished = (char) 1;
    if (msgsnd(queClThrId, (char *) &msgClThr, CliThrMsgSize, 0) != 0)
        syserr("from %s, line %d: msgsnd queClThrId, __FILE__, __LINE__");

	printf("%hu KONIEC\n", myPID);
	fflush(stdout);
    return 0;
}
