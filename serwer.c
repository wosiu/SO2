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

int msg_qid;

void exit_server(int sig)
{
    if (msgctl(msg_qid, IPC_RMID, 0) == -1)
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

}

int main()
{
    ClientServerMsg mesg;
    int	n, filefd;
    char errmesg[256];

    if (signal(SIGINT,  exit_server) == SIG_ERR)
        syserr("signal");

    if ((msg_qid = msgget(MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
        syserr("msgget");

    for(;;)
    {
        if ((n = msgrcv(msg_qid, &mesg, 6, 1L, 0)) <= 0)
            syserr("msgrcv");

		printf("n: %d\n", n);
		//printf("sizeof: %lu %lu %lu %lu\n", sizeof(mesg), sizeof(char), sizeof(int), sizeof(long));
        //mesg.mesg_data[n] = '\0';		/* null terminate filename */
        mesg.k = 42;
        mesg.n = 43;
        mesg.pid = 44;
        mesg.mesg_type = 2L;		/* send messages of this type */

		if (msgsnd(msg_qid, (char *) &mesg, 6, 0) != 0)
			syserr("msgsnd");
    }
}
