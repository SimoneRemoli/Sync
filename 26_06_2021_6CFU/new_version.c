#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int file_descriptor;
int file_descriptor_app;
FILE* file;
FILE* file2;
FILE* file_star_app;
int file_descriptor2;
char buffer[1024];
pid_t pid;
void run();
int check(int, char**);
void create_name(char*, char*);
char name_shadow[1024];
char *suffisso = "shadow_";
char *nome_file;
void handle(int);

int main(int argc, char **argv)
{
	if(check(argc,argv))
	{
		fprintf(stderr, "Errore [*] - Devi passare il nome di un file come argomento - [*] \n");
		return 0;
	}
	nome_file = argv[1];
	printf("Nome del file passato come argomento = %s . \n", argv[1]);

	file_descriptor = open(nome_file, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file = fdopen(file_descriptor, "w+");

	file_descriptor_app = open(nome_file, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file_star_app = fdopen(file_descriptor_app, "w+");

	create_name(name_shadow, suffisso);
	printf("Nome del file shadow = %s\n", name_shadow);

	file_descriptor2 = open(name_shadow, O_CREAT|O_TRUNC|O_RDWR, 0666);
	file2 = fdopen(file_descriptor2, "w+");

	pid = fork();
	if(pid==0)
		run();
	if(pid==-1)
		printf("Errore generazione figlio [*] \n");

	signal(SIGINT, handle);
	while(1)
	{
		printf(">>");
		if(fgets(buffer, 1024, stdin))
			buffer[strcspn(buffer, "\n")] = 0;

		printf("Update file - %s - \n", argv[1]);
		fprintf(file, "%s\n", buffer);
		fflush(file);
	}
	return 0;
}
int check(int argc, char**argv)
{
	if(argc==2)
		return 0;
	else
		return 1;
}
void run()
{
	char buffered[1024];
	signal(SIGINT, SIG_IGN);
	while(1)
	{
		sleep(10);
		fseek(file_star_app, 0, SEEK_CUR);
		while( EOF != fscanf(file_star_app, "%[^\n]\n", buffered))
		{
			printf("\n");
			printf("\t Copio nel file backup  = %s\n", buffered);
			printf("\n");
			fprintf(file2, "%s\n", buffered);
			fflush(file2);
		}
	}

}
void create_name(char *name_shadow, char* suffisso)
{
	for(int i=0;i<strlen(suffisso);i++)
		name_shadow[i] = suffisso[i];
	for(int i=strlen(suffisso),j=0;j<strlen(nome_file);i++,j++)
		name_shadow[i] = nome_file[j];

}
void handle(int unused)
{
	char buffered[1024];
	fseek(file_star_app, 0, SEEK_CUR);
	while( EOF != fscanf(file_star_app, "%[^\n]\n", buffered))
	{
		printf("\n");
		printf("\t Copio nel file backup  = %s\n", buffered);
		printf("\n");
		fprintf(file2, "%s\n", buffered);
		fflush(file2);
	}
}