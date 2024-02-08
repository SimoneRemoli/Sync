/******************************************************************
Welcome to the Operating System examination

You are editing the '/home/esame/prog.c' file. You cannot remove 
this file, just edit it so as to produce your own program according to
the specification listed below.

In the '/home/esame/'directory you can find a Makefile that you can 
use to compile this program to generate an executable named 'prog' 
in the same directory. Typing 'make posix' you will compile for 
Posix, while typing 'make winapi' you will compile for WinAPI just 
depending on the specific technology you selected to implement the
given specification. Most of the required header files (for either 
Posix or WinAPI compilation) are already included in the head of the
prog.c file you are editing. 

At the end of the examination, the last saved snapshot of this file
will be automatically stored by the system and will be then considered
for the evaluation of your exam. Modifications made to prog.c which are
not saved by you via the editor will not appear in the stored version
of the prog.c file. 
In other words, unsaved changes will not be tracked, so please save 
this file when you think you have finished software development.
You can also modify the Makefile if requesed, since this file will also
be automatically stored together with your program and will be part
of the final data to be evaluated for your exam.

PLEASE BE CAREFUL THAT THE LAST SAVED VERSION OF THE prog.c FILE (and of
the Makfile) WILL BE AUTOMATICALLY STORED WHEN YOU CLOSE YOUR EXAMINATION 
VIA THE CLOSURE CODE YOU RECEIVED, OR WHEN THE TIME YOU HAVE BEEN GRANTED
TO DEVELOP YOUR PROGRAM EXPIRES. 


SPECIFICATION TO BE IMPLEMENTED:
Scrivere un programma che riceva in input tramite argv[] un insime di N
stringhe, con N maggiore o uguale ad 1, che indicheremo con S1 ... SN.
Il main thread del programma dovra' leggere indefinitamente stringhe dallo standard input
e per ogni nuova stringa letta S dovra' attivare un nuovo thread, che riferiamo come T, 
passando a questo nuovo thread la stringa S letta come parametro.


Questo thread T dovra' controllare se la stringa S sia uguale a ciascuna
delle N stringhe S1 ... SN originariamente ricevute in input dal programma. Per ogni 
stringa trovata uguale all'atto del controllo, dovra' essere incrementato un contatore
apposito (si presuppne quindi che il programma gestisca N di questi contatori, uno per 
ognuna delle stringhe S1 ... SN ricevute tramite argv[]).

L'applicazione dovra' gestire il segnale  SIGINT (o CTRL_C_EVENT nel caso WinAPI) 
in modo tale che quando il processo venga colpito il suo main thread dovra' 
riportare su standard output il valore degli N contatori, su linee diverse dello 
stream di output, associando nel messaggio di output ciascuno dei valori alla relativa 
stringa S1 ... SN. La stessa informazione dovra' essere scritta all'interno di un file 
dal nome "output.txt" in modo che questo contenga sempre e solo i valori dei contatori 
piu' aggionati all'atto del processamento del segnale ricevuto.

In caso non vi sia immissione di dati sullo standard input e non vi siano segnalazioni, 
l'applicazione dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.

*****************************************************************/
#ifdef Posix_compile
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int check(int, char**);
int numero_stringhe;
char **file_names;
int semaforo_main;
int semaforo_thread;
char buffer[1024];
void* worker(void*);
int indice = 0;
FILE* file;
int file_descriptor;
char *nomefile = "output.txt";
void handle(int);
int array_contatori[1024];
void setting(int*);

/*
Studente : Simone Remoli
Matricola: 0280114

L'implementazione prevede l'ausilio di due semafori - in modo particolare di due singoli distributori di gettoni - 
che gestiranno la sincronizzazione fra il main thread e il singolo thread che verrà attivato.
Il semaforo del main inizialmente viene settato ad 1 in modo tale da permettere l'acquisizione di una stringa da stdin.
Il semaforo del thread nel frattempo è impostato a 0 poichè inizialmente il thread non può iniziare la sua esecuzione.
Sarà il semaforo (in particolare il singolo distributore di gettone) del main a rilasciare, tramite l'impacchettamento di una
operazione semaforica, un token da consumare al semaforo del thread.
Il thread partirà con la sua esecuzione e, per ogni stringa inserita tramite argv, confronterà se la stringa che correntemente sta gestendo è
esattamente uguale ad una di queste stringhe inserite.
Se il controllo dovesse risultare positivo allora incrementerà una locazione di memoria che mantiene le informazioni di quella singola stringa.
Ed infine restituisce il gettone al main in modo tale che quest'ultimo possa richiedere ancora l'acquisizione indefinita di stringhe da stdin.
*/

int main(int argc, char** argv)
{
	pthread_t T;
	struct sembuf oper;
	int ret;
	if(check(argc, argv))
	{
		printf("Aborted [*] \n");
		return 0;
	}
	numero_stringhe = argc - 1;
	file_names = argv;
	printf("Numero di stringhe passate =  %d\n", numero_stringhe);
	file_descriptor = open(nomefile, O_CREAT|O_TRUNC|O_RDWR, 0666);
	if(file_descriptor == -1)
		fprintf(stderr, "Errore nella creazione del file [*] \n");
	file = fdopen(file_descriptor, "w+");
	for(int i=0;i<numero_stringhe;i++)
		printf("Stringa [%d] inserita = %s \n", i+1, file_names[i+1]);
		
	array_contatori[numero_stringhe];
	setting(array_contatori);
	int *ptr = &array_contatori[0];
	semaforo_main = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	if(semaforo_main == -1)
		fprintf(stderr, "Errore sulla creazione del semaforo main [*] \n");
	semaforo_thread = semget(IPC_PRIVATE, 1, IPC_CREAT|IPC_EXCL|0666);
	if(semaforo_thread == -1)
		fprintf(stderr, "Errore sulla creazione del semaforo thread [*] \n");
	ret = semctl(semaforo_main, 0, SETVAL, 1);
	if(ret == -1)
		fprintf(stderr, "Errore nel settaggio semaforico [*] \n");
	ret = semctl(semaforo_thread, 0, SETVAL, 0);
	if(ret == -1)
		fprintf(stderr, "Errore nel settaggio semaforico [*] \n");
	
	//FQ no i thread vanno creati on demand quando le stringhe in input giungono
	pthread_create(&T, NULL, worker, ptr);
	//FQ gestore impostato non atomico
	signal(SIGINT, handle);	
	
	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;
		printf(">>");
		//FQ no stai prendendo linee non stringhe
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;
		
		//FQ avresti dovuto attivare thread in questa zona di codice
		oper.sem_op = 1;
		oper.sem_flg = 0;
		oper.sem_num = 0;
redo2:
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
	return 0;
}
int check(int argc, char** argv)
{
	if(argc == 1)
	{
	 	fprintf(stderr, "- Errore - Devi inserire delle stringhe come parametro [*] \n");
	 	return 1;
	}
	 else
	 	return 0;
}
void* worker( void* arg)
{
	int ret;
	struct sembuf oper;
	int *ptr =  arg;
	int check = 0;

	while(1)
	{
		oper.sem_num = 0;
		oper.sem_op = -1;
		oper.sem_flg = 0;
redo1:
		ret = semop(semaforo_thread, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo1;
		
		printf("- Sono il thread e gestisco la seguente stringa = %s [*] \n", buffer);
		for(int i=0;i<numero_stringhe;i++)
		{
			if(strcmp(buffer, file_names[i+1])==0)
			{
				*(ptr + i) = *(ptr + i) + 1;
				printf("Stringa %s trovata. Numero occorrenze = %d [*] \n", buffer, *(ptr + i));
				check = 1;
			}
		}
		if(check==0)
			printf("Stringa ''%s'' non trovata [NO MATCH] - \n", buffer);
		check = 0;
		
		oper.sem_num = 0;
		oper.sem_op = 1;
		oper.sem_flg = 0;
redo2:
		ret = semop(semaforo_main, &oper, 1);
		if(ret == -1)
			if(errno == EINTR)
				goto redo2;
	}
} 

void handle(int unused)
{
	//FQ gestore non atomico non corretto per il reale contenuto del file
	open(nomefile, O_TRUNC);
	printf("\n");
	for(int i = 0;i < numero_stringhe; i++)
	{
		printf("Valore del contatore [%d] che gestisce la stringa %s = %d \n",i+1,file_names[i+1], array_contatori[i]);
		fprintf(file, "Val contatore = %d che gestisce la stringa %s = %d \n",i+1,file_names[i+1], array_contatori[i]);
		fflush(file);
		
	}
	/*
	Il file "output.txt" sarà sempre la versione più aggiornata
	*/
}
void setting(int *arr)
{
	for(int i=0;i<numero_stringhe;i++)
	{
		*(arr + i ) = 0;
	}
}
