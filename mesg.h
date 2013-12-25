/**
* SO zadanie zaliczeniowe nr 2
* Michal Wos mw336071
*/
#ifndef _MESG_
#define _MESG_

typedef struct {
	long mesg_type;
	unsigned short int k,n,pid;
} ClientServerMsg;

const int CliSrvMsgSize = sizeof(unsigned short int) * 3;

typedef struct {
	long mesg_type;
	unsigned short int partner_pid;
} ThreadClientMsg;

const int ThrCliMsgSize = sizeof(unsigned short int);

typedef struct {
	long mesg_type;
	char finished;
} ClientThreadMsg;

const int CliThrMsgSize = sizeof(char);

// Teoretycznie zamiast 2 osobnych kolejek CLIENT_* moglaby byc jedna.
// Dodatkowa wprowadzona ze wzgledu na zwiekszenie przeplywu czytelnosci
// wiadomosci w kodzie.
#define	CL_SRV_MKEY 1235L
#define	THR_CL_MKEY 1236L
#define	CL_THR_MKEY 1237L

#endif
