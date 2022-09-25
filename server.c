#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <ftw.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
int sConn,status=0,portNo=0;
char *npip;

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}
int delete_files(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int removed_file = remove(fpath);

    if (removed_file)
        perror(fpath);

    return removed_file;
}

int rmrf(char *path)
{
    return nftw(path,delete_files, 64, FTW_DEPTH | FTW_PHYS);
}
// Function designed for chat between client and server.
void func(int sConn)
{
	char buff[MAX];
	int n;
	char *old_name;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		read(sConn, buff, sizeof(buff));
		// print buffer which contains the client contents
		


		//signal handler.
		if (strncmp("exit", buff, 4) == 0) {
			printf("Client Exited...\nWaiting for new\n");
			break;
		}
		else if (strncmp("PORT",buff,4)==0){
			char* cmd = strtok(buff," ");
			char * prt  = strtok(NULL," ");
			//removing \n in port number
			// prt[strlen(prt)-1] = '\0';
			npip = prt;
			// printf("%d\n",portNo);
			write(sConn,"OK\n",3);
			
		}
		else if (strncmp("RETR",buff,4)==0){
			char* cmd = strtok(buff," ");
			char * fname  = strtok(NULL," ");
			//removing \n in fname
			fname[strlen(fname)-1] = '\0';
			status = 1; // file process strted
			int fd,fdp;
			fd = open(fname,O_RDONLY);
			if(fd==-1)
			{
				write(sConn,"ERROR 2\n",strlen("ERROR 2\n"));
				continue;
			}
			fdp = open(npip,O_WRONLY);
			char buf;
			while(read(fd, &buf, 1)!=0)
			{
				printf("%c",buf);
				write(fdp, &buf, 1 );
			}
			status=0; // file process completed
			write(sConn,"OK\n",2);
			close(fd);
			close(fdp);
		}
		else if (strncmp("STOR",buff,4)==0){
			char* cmd = strtok(buff," ");
			char * fname  = strtok(NULL," ");
			//removing \n in fname
			fname[strlen(fname)-1] = '\0';
			status = 1; // file process strted
			int fd,fdp;
			fd = open(fname,O_WRONLY);
			if(fd==-1)
			{
				write(sConn,"ERROR 2\n",strlen("ERROR 2\n"));
				continue;
			}
			fdp = open(npip,O_RDONLY);
			char buf;
			while(read(fdp, &buf, 1)!=0)
			{
				printf("%c",buf);
				write(fd, &buf, 1 );
			}
			status=0; // file process completed
			write(sConn,"OK\n",2);
			close(fd);
			close(fdp);
		}
		else if (strncmp("USER",buff,4)==0){
			printf("Connection Successful\n");
			write(sConn,"OK\n",2);
		}
		else if (strncmp("RNFR",buff,4)==0){
			char* cmd = strtok(buff," ");
			char * rnff  = strtok(NULL," ");
			old_name = malloc(sizeof(rnff));
			strcpy(old_name,rnff);
			write(sConn,"OK",2);
		}
		else if (strncmp("RNTO",buff,4)==0){
			char *cmd = strtok(buff," ");
			char * new_name = strtok(NULL," ");
			old_name = trimwhitespace(old_name);
			new_name = trimwhitespace(new_name);
			int x = rename(old_name,new_name);
			printf("renamed from %s to %s",old_name,new_name);
			if(x!=0)
			{
				write(sConn,"ERROR",5);
				printf("%d\n",errno);
			}
			else
			{
				printf("renamed from %s to %s",old_name,new_name);
				write(sConn,"OK",2);
			}
		}
		else if (strncmp("DELE",buff,4)==0){
			char *cmd = strtok(buff," ");
			char *fname = strtok(NULL," ");
			fname[strlen(fname)-1] = '\0';
			//fname = trimwhitespace(fname);
			if(remove(fname)==0)
				write(sConn,"OK\n",3);
			else
				write(sConn,"ERROR\n",6);
		}
		else if (strncmp("RMD",buff,3)==0){
			char *cmd = strtok(buff," ");
			char *directory = strtok(NULL," ");
			directory[strlen(directory)-1] = '\0';
			printf("%d\n",rmrf(directory));
			write(sConn,"OK\n",3);
		}
		else if (strncmp("MKD",buff,3)==0){
			char *cmd = strtok(buff," ");
			char *directory = strtok(NULL," ");
			directory[strlen(directory)-1] = '\0';
			if(mkdir(directory,0777)!=-1)
			{
				write(sConn,"OK\n",3);
			}
			else
				write(sConn,"ERROR\n",6);
		}
		else if (strncmp("PWD",buff,3)==0){
			char cwd[1024];
			if(getcwd(cwd, sizeof(cwd))!=0)
			{
				int len = strlen(cwd);
				write(sConn,cwd,len);
			}
			else
				write(sConn,"ERROR\n",6);
		}
		else if (strncmp("LIST",buff,4)==0){
			DIR *dp;
			struct dirent *dirp;
			dp = opendir("./");
			char *list=(char*)malloc(2048);
			int len=0;
			while((dirp=readdir(dp)) != NULL)
			{
				char *files = dirp->d_name;
				strcat(files,"\n");
				len += strlen(files);
				strcat(list,files);
			}
			write(sConn,list,len);
			close(dp);
			free(list);
		}
		else if (strncmp("NOOP",buff,4)==0){
			write(sConn,"OK\n",3);
		}
	}
}

void signal_recieved()
{
	exit(0);
}

int main()
{
	int sockfd,len;
	struct sockaddr_in servaddr, cli;

	signal(SIGINT,signal_recieved);
	signal(SIGTSTP,signal_recieved);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);
	
	while(1){
		sConn = accept(sockfd, (SA*)&cli, &len);
		if(sConn!=-1)
		{
			printf("Client Connected\n");
			func(sConn);
		}
	}
}

