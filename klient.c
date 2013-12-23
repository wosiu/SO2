#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "mesg.h"
#include "err.h"

int main()
{
    ClientServerMsg mesg;
    int id, n;

    if ( (id = msgget(MKEY, 0)) == -1)
        syserr("msgget");

    printf("File name: ");
    if (scanf("%hu",&mesg.pid) == 0)
        syserr("fgets");

    mesg.mesg_type = 1L;			/* send messages of this type */
    if (msgsnd(id, (char *) &mesg, ClientServerMsgSize, 0) != 0)
        syserr("msgsnd");

	printf("ClientServerMsgSize: %d\n", ClientServerMsgSize);
    if ((n = msgrcv(id, &mesg, 6, 2L, 0)) < 0)
    	syserr("msgrcv");

	printf("%hu %hu %hu\n", mesg.k, mesg.n, mesg.pid);

    return 0;
}
