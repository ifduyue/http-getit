#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 2048

static char buf[MAXSIZE+1];
static char *argv0;

int get_socket( const char *host, unsigned short port );
void http_get( const char *url, int outfd );
void usage();

int main( int argc, char **argv ) {
   argv0 = argv[0];
   if ( argc != 2 ) usage();
   http_get(argv[1], 1);
   return 0;
}

int get_socket( const char *hostname, unsigned short port ) {
   struct hostent *he = NULL;
   if ( ( he = gethostbyname( hostname ) ) == NULL ) {
      herror("gethostbyname error");
      exit( 1 );
   }
   
   int sockfd = socket( he->h_addrtype, SOCK_STREAM, 0 );
   if ( sockfd < 0 ) {
      perror( "can't get a sockfd" );
      exit( 1 );
   }
   
   struct sockaddr_in remote;
   remote.sin_family = he->h_addrtype;
   remote.sin_addr = *( (struct in_addr *) he->h_addr );
   remote.sin_port = htons( port );
   if ( connect( sockfd, (struct sockaddr *) &remote, sizeof( remote ) ) < 0 ) {
      perror( "connect error" );
      exit( 1 );
   }
   return sockfd;
}

void http_get( const char *url, int outfd ) {
   char *i;
   char host[MAXSIZE+1], get[MAXSIZE+1];
   unsigned short port = 80;
   if ( strncmp( url, "http://", 7 ) == 0 ) {
      url += 7;
   }
   if ( ( i = strchr( url, '/' ) ) == NULL ) {
      strcpy( get, "/" );
   }
   else {
      *i = 0;
      snprintf( get, MAXSIZE, "/%s", i+1 );
   }
   if ( ( i = strchr( url, ':' ) ) != NULL ) {
      *i = 0;
      port = atoi(i+1);
   }
   strncpy( host, url, MAXSIZE);
   
   int sockfd = get_socket(host, port);
   snprintf( buf, MAXSIZE, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", get, host );
   int bytes;
   write( sockfd, buf, strlen(buf) );
   fprintf( stderr, "host: %s, get: %s, port: %u\n", host, get, port);
   for (;;) {
      if ( read( sockfd, buf, 1 ) <= 0 ) break;
      if ( *buf == '\n' ) {
         if ( bytes == 0 ) break;
         bytes = 0;
      }
      else if (*buf != '\r') {
         bytes = 1;
      }
   }
   for (;;) {
      if ( ( bytes = read( sockfd, buf, MAXSIZE ) ) <= 0 ) break;
      write( outfd, buf, bytes );
   }
}

void usage() {
   fprintf( stderr, "useage: %s url\n", argv0 );
   exit( 1 );
}
