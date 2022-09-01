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

#define PORT "3510" // the port client will be connecting to 

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
	char new_filename[MAXDATASIZE];
	int i = 3;
	char reply;
	
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
while(1){

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
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

   printf("Please type a command (enter 'h' for help)\n");
 
 	//printf("Please insert a command: [l, c, d, p, q]");
	fgets(buf, MAXDATASIZE,stdin);
	sscanf(buf,"%c %s", &command, filename);
	send(sockfd, buf, MAXDATASIZE-1, 0); 
  //send(sockfd, command, sizeof(command)-1, 0);
	switch(command){
	case 'l':
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("recv");
		    exit(1);
		}

		buf[numbytes] = '\0';
	
		printf("client: received '%s'\n",buf);
		close(sockfd);
		break;

	case 'c':
		if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1){
		   perror("recv");
		   exit(1);
		}
		buf[numbytes] = '\0';
		printf("client: recieved, '%s'\n", buf);
		close(sockfd);
		break;

	case 'p':
		printf("client: received");
		while((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0){
			printf("%s", buf);
		}
		close(sockfd);
		break;
	
	case 'd':
		;
		FILE* fp = NULL;
		fp = fopen(filename, "r");
		if(fp != NULL) {
		  printf("%s already exists in this directory, download anyway? (y for yes, n for no)\n", filename);
		  scanf("%c", &reply); 
		}
	
		if(reply == 'n') {
		  printf("Processing has ended \n");
		  fclose(fp);
		  close(sockfd);
		  break;
		}
	
		else if(reply == 'y' || fp ==NULL) {
		    printf("What would you like to save it as?\n");
		    scanf("%s", new_filename);
		    fp = fopen(new_filename, "w+");
		    while((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) > 0) {
		   	//printf("%s", buf);
			fputs(buf, fp);
		    }
		    fclose(fp);
		}

		printf("Processing has ended \n");
		close(sockfd);
		break;
	
	case 'q':
		close(sockfd);
		freeaddrinfo(servinfo); // all done with this structure

		return 0;
		break;
	}

}
	close(sockfd);
	freeaddrinfo(servinfo); // all done with this structure

	return 0;
}

