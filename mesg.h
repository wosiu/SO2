typedef struct {
	long mesg_type;
	unsigned short int k,n,pid;
} ClientServerMsg;

const int ClientServerMsgSize = sizeof( unsigned short int ) * 3;

typedef struct {
	long mesg_type;
	unsigned short int partner_pid;
} ThreadClientMsg;

const int ThreadClientMsgSize = sizeof( unsigned short int );

typedef struct {
	long mesg_type;
	char finished;
} ClientThreadMsg;

const int ClientThreadMsgSize = sizeof( char  );

// Teoretycznie zamiast 2 osobnych kolejek CLIENT-* moglaby byc jedna
// ale z praktycznego puntku widzenia, dzieki tej dodatkowej mozna zreazlizowac
// ladniejsze sprzatanie podczas zlapania SIGINT (poniewaz oczekujemy wtedy
// na zwolenienie zasobow przez klientow, a nowi klienci potencjalnie mogliby
// zapchnac kolejke zgloszeniami, poniewaz serwer nie przyjmuje juz nowych
// zgloszen, stad pracujacy klienci nie upchneliby wiadodomosci o zwoleniu
// zasobow.
#define	CLIENT_SERVER_MKEY	1235L
#define	THREAD_CLIENT_MKEY	1236L
#define	CLIENT_THREAD_MKEY	1237L

