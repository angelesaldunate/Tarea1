#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

FILE *ipg;

// Cuenten con este codigo monolitico en una funcion
// main como punto de partida.
// Idealmente, el codigo del programa deberia estar
// adecuadamente modularizado en distintas funciones,
// e incluso en archivos separados, con dependencias
// distribuidas en headers.


int main(int argc, char** argv) {
	//time_t t;
	
	size_t bufsize = 512;
	char* commandBuf = malloc(sizeof(char)*bufsize);
	int forks = 0;
	// Para guardar descriptores de pipe
	// el elemento 0 es para lectura
	// y el elemento 1 es para escritura.
	int bankPipe[2];
	//Pipes.
	int outwPipes[50][2];
	//Sucursales.
	char branches[50][4];
	char branchesac[50][7];
	char transc[20000][21];
	for (int i = 0; i<50;i++){
		pipe(outwPipes[i]);
	}
	char readbuffer[80]; // buffer para lectura desde pipe

	// Se crea un pipe...
	pipe(bankPipe);

	const int bankId = getpid() % 1000;
	printf("Bienvenido a Banco '%d'\n", bankId);

	while (true) {
		usleep(200000);
		printf(">>");
		getline(&commandBuf, &bufsize, stdin);
	
		// Manera de eliminar el \n leido por getline
		commandBuf[strlen(commandBuf)-1] = '\0';
		printf("Comando ingresado: '%s'\n", commandBuf);

		if (!strncmp("quit", commandBuf, strlen("quit"))) {
			break;
		}
		else if (!strncmp("list", commandBuf, strlen("list"))) {
			// Enviando saludo a la sucursal
			char asd[] = "list";
			for (int j = 0; j<forks;j++){
				if (branches[j]>0){
					write(outwPipes[j][1], asd, 5);
				}
			}	
			continue;
		}
		else if (!strncmp("init", commandBuf, strlen("init"))) {
			// OJO: Llamar a fork dentro de un ciclo
			// es potencialmente peligroso, dado que accidentalmente
			// pueden iniciarse procesos sin control.
			// Buscar en Google "fork bomb"
			char  * numerocuentas;
			int largo = strlen(commandBuf);
			numerocuentas = strtok(commandBuf, " ");
			char n[7] = "1000";
			numerocuentas=strtok(NULL," ");
			if (numerocuentas && atoi(numerocuentas)>0 && atoi(numerocuentas)<1000000){	
				strcpy(n,numerocuentas);
			}
			pid_t sucid = fork();	
			if (sucid > 0) {
				
				printf("Sucursal creada con ID '%d'\n", sucid);
				int sucId = sucid % 1000;
				sprintf(branches[forks],"%d",sucId);
				strcpy(branchesac[forks], n);
				forks +=1;
				// Enviando saludo a la sucursal
				//char msg[] = "Hola sucursal, como estas?";
				//char asd[4];
				//sprintf(asd,"%d",sucid);
				//write(bankPipe[1], asd, 5);
				continue;
			}
			// Proceso de sucursal
			else if (!sucid) {
				int sucId = getpid() % 1000;
				int cash[atoi(n)];
				srand(time(NULL));
				for (int i=0; i< atoi(n); i++){
					cash[i]=1000;
					cash[i]+=rand()%499999000;
				}
				printf("Hola, soy la sucursal '%d' y tengo '%s' cuentas \n", sucId, n);
				while (true) {
					// 100 milisegundos...
					int bytes = read(outwPipes[forks][0], readbuffer, sizeof(readbuffer));
					//printf("Soy la sucursal '%d' y me llego mensaje '%s' de '%d' bytes.\n", sucId, readbuffer, bytes);

					// Usar usleep para dormir una cantidad de microsegundos
					//usleep(1000000);

					// Cerrar lado de lectura del pipe
					//close(bankPipe[0]);

					// Para terminar, el proceso hijo debe llamar a _exit,
					// debido a razones documentadas aqui:
					// https://goo.gl/Yxyuxb
					if (!strncmp("list", readbuffer, strlen("list"))){					 	
						printf("id:%d \n",sucId);
						printf("cuentas %s \n",n);
						// no le gusta esta linea
					}
					if (!strncmp("kill", readbuffer, strlen("kill"))){					 	
						_exit(EXIT_SUCCESS);
					}
					if (!strncmp("dump_accs", readbuffer, strlen("dump_accs"))){
						ipg=fopen("dumpaccsPID.csv", "w");
					        if (ipg == NULL) {
						    fprintf(stderr, "No se puede abrir archivo de entrada\n");
						    exit(1);
					        }
						fprintf(ipg,"Numero de cuenta, saldo\n");
						for (int i=0; i< atoi(n); i++){
							fprintf(ipg,"%06d, %d \n", i, cash[i]);
						}
					        
					        if(ipg!=NULL) fclose(ipg);	
					
				}
				}
			}
			  // error
			else {
				fprintf(stderr, "Error al crear proceso de sucursal!\n");
				return (EXIT_FAILURE);
			}
		}else if (!strncmp("kill", commandBuf, strlen("kill"))) {
			// especifica el ID y se aplica _exit
			//printf(" hacemos %s de la sucursal con pid %s ", commandBuf,aux1);
			//le mando todo el comando kill y el pid
				
				char *id;
				id=strtok(commandBuf, " ");
				id=strtok(NULL," ");
				for (int j = 0; j<forks;j++){
					if(strcmp(branches[j],id)==0){
						strcpy(branches[j],"0");
						write(outwPipes[j][1], commandBuf, (strlen(commandBuf)+1));
					}
				}
				continue;
		}else if (!strncmp("dump_accs", commandBuf, strlen("dump_accs"))) {
				char *id;
				if (strlen(commandBuf)<12){
					printf("ERROR Se debe ingresar un id de sucursal\n");
					continue;
				}
				id=strtok(commandBuf, " ");
				id=strtok(NULL," ");
				for (int j = 0; j<forks;j++){
					if(strcmp(branches[j],id)==0){
						strcpy(branches[j],"0");
						write(outwPipes[j][1], commandBuf, (strlen(commandBuf)+1));
					}
				}
				continue;

		}
		else {
			fprintf(stderr, "Comando no reconocido.\n");
		}
		// Implementar a continuacion los otros comandos
	}
	for (int i = 0; i < forks; i++){
		if (atoi(branches[i])>0){	
			printf("id: %s %s\n",branches[i], branchesac[i]);
		}
	}
  	printf("Terminando ejecucion limpiamente...\n");
  	// Cerrar lado de escritura del pipe
  	close(bankPipe[1]);
  	return(EXIT_SUCCESS);
}
