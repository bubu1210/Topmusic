#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
 while (1){
  /* citirea mesajului */
  bzero (msg, 900);
  printf ("[client]Introduceti o comanda (sau utilizati comanda help): ");
  fflush (stdout);
  read (0, msg, 300);
  /* trimiterea mesajului la server */
  if (write (sd, msg, 300) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }
  if (strstr(msg,"exit")!=NULL)break;
  /* citirea raspunsului dat de server 
     (apel blocant pana cand serverul raspunde) */
  if (strstr(msg,"show")!=NULL){
      int nr_songs;
      if (read (sd, &nr_songs, 4) < 0)
      {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
      }
  
      for (int ii=0;ii<nr_songs;ii++){
          if (read (sd, msg, 900) < 0)
          {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
          }
          /* afisam mesajul primit */
          printf ("%s", msg);
    }

  }
  if (read (sd, msg, 900) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  /* afisam mesajul primit */
  printf ("[client]Mesajul primit este: %s\n", msg);
 }
  /* inchidem conexiunea, am terminat */
  close (sd);
}
