/* Descrizione: Semplice server FTP iterativo sviluppato per il progetto di Reti di Calcolatori
 * Sviluppatore: Paolo Stivanin
 * Versione: 1.0-alpha4
 * Copyright: 2012
 * Licenza: GNU AGPL v3 <http://www.gnu.org/licenses/agpl-3.0.html>
 * Sito web: <https://github.com/polslinux/FTPUtils>
 */

#define _GNU_SOURCE /* per definire get_current_dir_name */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h> /* per usare uint32_t invece di size_t */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <inttypes.h> /* per printare il tipo di dato uint32_t */
#include "prototypes.h"

int main(int argc, char *argv[]){
	
	check_before_start(argc, argv);
	
	int sockd, newsockd, socket_len;
	int NumPorta = atoi(argv[1]);
	struct sockaddr_in serv_addr, cli_addr; /* strutture contenenti indirizzo del server e del client */
	struct hostent *local_ip; /* struttura contenente ip server */
    struct in_addr **pptr;
	static char buffer[512], saved_user[512]; /* dichiaro static così viene direttamente inizializzato a 0 l'array */
	char *user_string = NULL, *username = NULL, *pass_string = NULL, *password = NULL;
    char *serverdir = (char *)(intptr_t)get_current_dir_name();
	
	if((sockd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Errore creazione socket\n");
		exit(EXIT_FAILURE);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr)); /* bzero scrive dei null bytes dove specificato per la lunghezza specificata */
	serv_addr.sin_family = AF_INET; /* la famiglia dei protocolli */
	serv_addr.sin_port = htons(NumPorta); /* porta htons converte nell'ordine dei byte di rete */
	serv_addr.sin_addr.s_addr = INADDR_ANY; /* dato che è un server bisogna associargli l'indirizzo della macchina su cui sta girando */
	
	if(bind(sockd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		perror("Errore di bind\n");
		onexit(0, sockd, 0, 1);
	}

	if(listen(sockd, 5) < 0){
		perror("Errore nella funzione listen");
    onexit(0, sockd, 0, 1);
	}
	socket_len = sizeof(cli_addr);

	signal (SIGINT, ( void *)sig_handler); /* se premuto CTRL+C il server termina */
	
	while(1){
    memset(&cli_addr, 0, sizeof(cli_addr));
    memset(buffer, 0, sizeof(buffer));
    memset(&local_ip, 0, sizeof(local_ip));
    if((local_ip = gethostbyname("localhost")) == NULL){
      perror("gethostbyname()");
      exit(1);
    }
    /* La lista puntata da h_addr_list è un array di puntatori. Pertanto è necessario dereferenziare ogni indice due volte,
     * una per avere il puntatore alla lista e l'altra per prendere l'in_addr struct a cui esso punta. 
     * Fatto ciò bisogna convertire l'unsigned int a 32 bit in un indirizzo ip decimale */
    pptr = (struct in_addr **)local_ip->h_addr_list;
    
		if((newsockd = accept(sockd, (struct sockaddr *) &cli_addr, (socklen_t *) &socket_len)) < 0){
			perror("Errore nella connessione\n");
			onexit(newsockd, sockd, 0, 2);
		}
    /* inet_ntoa converte un hostname in un ip decimale puntato */
    fprintf(stdout, "Ricevuta richiesta di connessione dall' indirizzo %s\n", inet_ntoa(cli_addr.sin_addr));

    /************************* MESSAGGIO DI BENVENUTO *************************/
    sprintf(buffer, "220 FTPUtils Server [%s]", inet_ntoa(**(pptr)));
    if(send(newsockd, buffer, strlen(buffer), 0) < 0){
      perror("Errore durante l'invio");
			onexit(newsockd, sockd, 0, 2);
		}
		memset(buffer, 0, sizeof(buffer));
    memset(&local_ip, 0, sizeof(local_ip)); /* pulire la struttura altrimenti alla seconda connessione si ha un segfault */
    /************************* FINE MESSAGGIO DI BENVENUTO *************************/

		/************************* INIZIO PARTE LOGIN *************************/
    /************************* RICEVIAMO NOME UTENTE E MANDIAMO CONFERMA *************************/
		if(recv(newsockd, buffer, sizeof(buffer), 0) < 0){
    	perror("Errore nella ricezione del nome utente");
    	onexit(newsockd, sockd, 0, 2);
    }    	
    user_string = strtok(buffer, " ");
    username = strtok(NULL, "\n");
    fprintf(stdout, "%s %s\n", user_string, username);
    sprintf(saved_user, "%s", username);
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "USEROK\n");
    if(send(newsockd, buffer, strlen(buffer), 0) < 0){
      perror("Errore durante l'invio");
		  onexit(newsockd, sockd, 0, 2);
		}
		memset(buffer, 0, sizeof(buffer));
    /************************* FINE NOME UTENTE *************************/

    /************************* RICEVIAMO PASSWORD E MANDIAMO CONFERMA *************************/
    if(recv(newsockd, buffer, sizeof(buffer), 0) < 0){
    	perror("Errore nella ricezione del nome utente");
    	onexit(newsockd, sockd, 0, 2);
    }
    pass_string = strtok(buffer, " ");
    password = strtok(NULL, "\n");
    fprintf(stdout, "%s %s\n", pass_string, password);
    memset(buffer, 0, sizeof(buffer));
   	strcpy(buffer, "PASSOK\n");
    if(send(newsockd, buffer, strlen(buffer), 0) < 0){
      perror("Errore durante l'invio");
		  onexit(newsockd, sockd, 0, 2);
		}
		memset(buffer, 0, sizeof(buffer));
    /************************* FINE PASSWORD *************************/
    	
    /************************* INVIO CONFERMA LOG IN *************************/
    sprintf(buffer, "230 USER %s logged in\n", saved_user);
    if(send(newsockd, buffer, strlen(buffer), 0) < 0){
			perror("Errore durante l'invio");
			onexit(newsockd, sockd, 0, 2);
		}
		memset(buffer, 0, sizeof(buffer));
		/************************* FINE CONFERMA LOG IN *************************/
    /************************* FINE PARTE LOGIN *************************/

    /************************* RESTO IN ASCOLTO DELL'AZIONE DAL CLIENT *************************/
    exec_listen_action:
    memset(buffer, 0, sizeof(buffer));
    if(recv(newsockd, buffer, sizeof(buffer), 0) < 0){
      perror("Errore nella ricezione azione\n");
      close(newsockd);
      exit(1);
    }
    if(strcmp(buffer, "SYST") == 0) goto prepara;
    if(strcmp(buffer, "LIST") == 0) goto prepara;
    if(strcmp(buffer, "PWD") == 0) goto prepara;
    if(strcmp(buffer, "CWD") == 0) goto prepara;
    if(strcmp(buffer, "RETR") == 0) goto prepara;
    if(strcmp(buffer, "STOR") == 0) goto prepara;
    if(strcmp(buffer, "DELE") == 0) goto prepara;
    if(strcmp(buffer, "MKD") == 0) goto prepara;
    if(strcmp(buffer, "RMD") == 0) goto prepara;
    if(strcmp(buffer, "RNM") == 0) goto prepara;
    if(strcmp(buffer, "EXIT") == 0) goto prepara;

    prepara:
    if(strcmp(buffer, "SYST") == 0) goto exec_syst;
    if(strcmp(buffer, "LIST") == 0) goto exec_list;
    if(strcmp(buffer, "PWD") == 0) goto exec_pwd;
    if(strcmp(buffer, "CWD") == 0) goto exec_cwd;
    if(strcmp(buffer, "RETR") == 0) goto exec_retr;
    if(strcmp(buffer, "STOR") == 0) goto exec_stor;
    if(strcmp(buffer, "DELE") == 0) goto exec_delete;
    if(strcmp(buffer, "MKD") == 0) goto exec_mkdir;
    if(strcmp(buffer, "RMD") == 0) goto exec_rmdir;
    if(strcmp(buffer, "RNM") == 0) goto exec_rename;
    if(strcmp(buffer, "EXIT") == 0) goto send_goodbye;
    /************************* FINE PARTE ASCOLTO *************************/

    /************************* INIZIO AZIONE SYST *************************/
    exec_syst:
    do_server_syst_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE SYST *************************/

    /************************* INIZIO AZIONE LIST *************************/
    exec_list:
    do_server_list_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE LIST *************************/

    /************************* INIZIO AZIONE PWD *************************/
    exec_pwd:
    do_server_pwd_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE PWD *************************/

    /************************* INIZIO AZIONE CWD *************************/
    exec_cwd:
    do_server_cwd_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE CWD *************************/

    /************************* INIZIO AZIONE RETR *************************/
    exec_retr:
    do_server_retr_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE RETR *************************/

    /************************* INIZIO AZIONE STOR *************************/
    exec_stor:
    do_server_stor_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE STOR *************************/

    /************************* INIZIO AZIONE DELE *************************/
    exec_delete:
    do_server_dele_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE DELE *************************/

    /************************* INIZIO AZIONE MKD *************************/
    exec_mkdir:
    do_server_mkd_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE MKD *************************/

    /************************* INIZIO AZIONE RMD *************************/
    exec_rmdir:
    do_server_rmd_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE RMD *************************/

    /************************* INIZIO AZIONE RNM *************************/
    exec_rename:
    do_server_rnm_cmd(newsockd, sockd);
    goto exec_listen_action;
    /************************* FINE AZIONE RNM *************************/

    /************************* INIZIO SALUTO FINALE *************************/
    send_goodbye:
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "221 Goodbye\n");
    if(send(newsockd, buffer, strlen(buffer), 0) < 0){
      perror("Errore durante l'invio 221");
      onexit(newsockd, sockd, 0, 2);
    }
    if(chdir(serverdir) < 0){
      perror("chdir on default dir");
    }
    memset(buffer, 0, sizeof(buffer));
    printf("Ricevuta richiesta EXIT\n");
    close(newsockd);
    /************************* FINE SALUTO FINALE *************************/
	}
	return EXIT_SUCCESS;
}

void check_before_start(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stdout, "Uso: %s <numero porta>\n", argv[0]);
		exit(1);
	}
}

void sig_handler(int signo, int sockd, int newsockd, int file){
  if (signo == SIGINT){
    printf("Ricevuto SIGINT, esco...\n");
    if(newsockd) close(newsockd);
    close(sockd);
    if(file) close(file);
    exit(EXIT_SUCCESS);
  }
}