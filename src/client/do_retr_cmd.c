#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h> /* per usare uint32_t invece di size_t */
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "../prototypes.h"

#define BUFFGETS 255

int do_retr_cmd(const int f_sockd){
  int fd;
  ssize_t nread = 0, tx = 0;
  uint32_t fsize, fsize_tmp, total_bytes_read, size_to_receive, fn_len;
  char *filename = NULL, *conferma = NULL;
  void *filebuffer = NULL;
  char buf[256], dirp[256];
  
  memset(dirp, 0, sizeof(dirp));
  memset(buf, 0, sizeof(buf));
  printf("File to download: ");
  if(fgets(dirp, BUFFGETS, stdin) == NULL){
    perror("Fgets file name");
    return -1;
  }
  filename = NULL;
  filename = strtok(dirp, "\n");
  fn_len = 0;
  fn_len = strlen(filename)+1;
  if(send(f_sockd, &fn_len, sizeof(fn_len), 0) < 0){
    perror("Error on sending the file length");
    return -1;
  }
  sprintf(buf, "RETR %s", filename);
  if(send(f_sockd, buf, fn_len+5, 0) < 0){
    perror("Error on sending the file name");
    return -1;
  }
  if(recv(f_sockd, buf, 3, 0) < 0){
    perror("Error on receiving RETR confirmation");
    return -1;
  }
  conferma = NULL;
  conferma = strtok(buf, "\0");
  if(strcmp(conferma, "NO") == 0){
    printf("ERROR: the requested file doesn't exist\n");
    return -1;
  }
  if(recv(f_sockd, &fsize, sizeof(fsize), MSG_WAITALL) < 0){
    perror("Error on receiving the file size");
    return -1;
  }
  fd = open(filename, O_CREAT | O_WRONLY, 0644);
  if (fd  < 0) {
    perror("Error: the file cannot be created.");
    return -1;
  }
  fsize_tmp = fsize;
  filebuffer = malloc(fsize);
  if(filebuffer == NULL){
    perror("Error on memory allocation (malloc)");
    close(fd);
    return -1;
  }
  total_bytes_read = 0;
  nread = 0;
  printf("Downloading...\n");
  for(size_to_receive = fsize; size_to_receive > 0;){
    nread = read(f_sockd, filebuffer, size_to_receive);
    tx += nread;
    printf("\r%d%%", (tx * 100 / fsize));
    fflush(NULL);
    if(nread < 0){
      perror("read error on retr");
      close(fd);
      return -1;
    }
    if(write(fd, filebuffer, nread) != nread){
      perror("write error on retr");
      close(fd);
      return -1;
    }
    size_to_receive -= nread;
  }
  close(fd);
  printf("\n");

  memset(buf, 0, sizeof(buf));
  if(recv(f_sockd, buf, 34, 0) < 0){
    perror("Error on receiving the 226 message");
    return -1;
  }
  printf("%s\n", buf);
  memset(buf, 0, sizeof(buf));
  memset(dirp, 0, sizeof(dirp));
  free(filebuffer);
  return 0;
}