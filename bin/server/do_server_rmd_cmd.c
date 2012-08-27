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

void do_server_rmd_cmd(f_sockd, m_sockd){
  char *other, *filename;
  char buf[256];
  memset(buf, 0, sizeof(buf));
  if(recv(f_sockd, buf, sizeof(buf), 0) < 0){
    perror("Errore nella ricezione del nome della cartella");
    onexit(f_sockd, m_sockd, 0, 2);
  }
  other = NULL;
  filename = NULL;
  other = strtok(buf, " ");
  filename = strtok(NULL, "\n");
  if(strcmp(other, "RMD") == 0){
    printf("Ricevuta richiesta RMDIR\n");
  } else onexit(f_sockd, m_sockd, 0, 2);
  
  if(rmdir(filename) != 0){
    fprintf(stderr, "Impossibile eliminare la cartella '%s'\n", filename);
    strcpy(buf, "ERRORE: Cartella non eliminata\0");
    if(send(f_sockd, buf, strlen(buf), 0) < 0){
      perror("Errore durante invio");
      onexit(f_sockd, m_sockd, 0, 2);
    }
    onexit(f_sockd, m_sockd, 0, 2);
  }
  strcpy(buf, "OK\0");
  if(send(f_sockd, buf, strlen(buf), 0) < 0){
    perror("Errore durante invio");
    onexit(f_sockd, m_sockd, 0 ,2);
  }
  memset(buf, 0, sizeof(buf));
}