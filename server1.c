/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3510"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int my_fd[2];		//our file descriptor for the pipe this is an integer array
	char my_buffer[MAXDATASIZE];	//our buffer for the pipe
	char command;
	char filename[MAXDATASIZE];
	int status;
	size_t nbytes;
	/* Added on 2/1/17*/
	int in[2], out[2], n, pid;
	

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		pipe(my_fd);		//allocates memory in the computer for a pipe (with a buffer) with file descriptor.
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		recv(new_fd, my_buffer, MAXDATASIZE, 0);
		sscanf(my_buffer, "%c %s", &command, filename);
		//printf("%s", filename);
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		//printf("server: got connection from %s\n", s);
		//printf("server: I received this command  %c %s\n", &command, filename);

	switch(command){
		case 'l':
		if(!fork()){	//this is a child to the child aka grandchild. will process the ls command
			dup2(new_fd, 1);	//close the input side of the pipe
			execl("/usr/bin/ls", "ls", (char*)NULL);
			close(new_fd);
		}
		break;
		case 'c':
		if(!fork()){
			FILE* fp = NULL;
			fp = fopen(filename, "r");
			if(fp == NULL){
			   send(new_fd, "File Does not exist!", 23, 0);
			}
			else {
			fclose(fp);
			send(new_fd, "File Exists!", 23, 0);
			}
			close(new_fd); 
		}
		break;
		case 'p':
		if(!fork()){
		  dup2(new_fd, 1);
		  execl("/usr/bin/cat", "cat ", filename, (char*)NULL);
		  close(new_fd);
		}
		break;
		case 'd':
		if(!fork()) {
		  FILE* fp = NULL;
		  fp = fopen(filename, "r");
		  if(fp == NULL){
		    send(new_fd,"File Does Not Exist\n",  50, 0);
		  }
		  else{
		     fclose(fp);
		     dup2(new_fd,1);
		     execl("/usr/bin/cat", "cat", filename, (char*)NULL);
		     close(new_fd);
		  }
		}
		break;

	}
	close(new_fd);
	}

	return 0;
}
