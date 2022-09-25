#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#define MAX 1024
#define PORT 8080
#define SA struct sockaddr
int clconn;
char *npip;

void func(int clconn)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the commands : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		if ((strncmp(buff,"exit",4))==0){
			printf("Exiting");
			write(clconn,buff,sizeof(buff));
			break;
		}
		else if((strncmp(buff,"PORT",4))==0){
			// printf();
			char* cmd = strtok(buff," ");
			char * prt  = strtok(NULL," ");
			//removing \n in fname
			// prt[strlen(prt)-1] = '\0';
			// int port_no = atoi(prt);
			npip=prt;
		}
		else if((strncmp(buff,"RETR",4))==0){
			// printf();
			char* cmd = strtok(buff," ");
			char * fname  = strtok(NULL," ");
			fname  = strtok(NULL," ");

			fname[strlen(fname)-1] = '\0';
			
			int fd,fdp;
			fd = open(fname,O_WRONLY);
			if(fd==-1)
			{
				write(clconn,"FILE DOESNOT EXISTS\n",strlen("FILE DOESNOT EXISTS\n"));
				continue;
			}
			fdp = open(npip,O_RDONLY);
			char buf;
			while(read(fdp, &buf, 1)!=0)
			{
				printf("%c",buf);
				write(fd, &buf, 1 );
			}
			write(clconn,"OK\n",2);
			close(fd);
			close(fdp);
		}
		write(clconn, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(clconn, buff, sizeof(buff));
		printf("From Server : %s", buff);
	}
}

void signal_recieved()
{
	write(clconn,"exit",4);
	close(clconn);
	exit(0);
}

int main()
{
	int connfd;
	struct sockaddr_in servaddr, cli;

	signal(SIGINT,signal_recieved);
	signal(SIGTSTP,signal_recieved);

	clconn = socket(AF_INET, SOCK_STREAM, 0);
	if (clconn == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	if (connect(clconn, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	func(clconn);

	close(clconn);
}

