/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3519" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
   char command;
   char filename[MAXDATASIZE];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	//printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
   while(command != 'q'){
      printf("Please type a command (enter 'h' for help)\n");
      fgets(buf, MAXDATASIZE, stdin);
      sscanf(buf, "%c %s", &command, filename);
      send(sockfd, buf, MAXDATASIZE-1, 0);

      switch(command){
         case 'l':
	         if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	            perror("recv");
	            exit(1);
	         }
	
            buf[numbytes] = '\0';

            printf("%s", buf);
            close(sockfd);
            break;
         case 'q':
            close(sockfd);
            //return 0;
            close(sockfd);
            break;
         case 'h':
            printf("'l' is for list\n");
            printf("'c' is for check\n");
            printf("'d' is for download\n");
            printf("'p' is for display\n");
            printf("'q' is for quit\n");
            break;
         case 'c':
            if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
               perror("recv");
               exit(1);
            }
            buf[numbytes] = '\0';
            printf("%s\n", buf);
            close(sockfd);
            //break;

/*         case 'p':
            while((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0){
               printf("%s", buf);
            }
            close(sockfd);
            */
      }
   }
   close(sockfd);

   return 0;
}

