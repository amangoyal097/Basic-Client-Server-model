#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 3000
#define MAX_CONNECTIONS 1
int min(int a , int b) {
	if(a < b) return a;
	return b;
}
int bufsiz = 1e6+5;
int main(int argc, char *argv[]) {
	struct sockaddr_in addr;
	int sock = 0,bytes_read;
	struct sockaddr_in serv_addr;
	char *buf = (char*)malloc(sizeof(char) * bufsiz);	
	 if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));// to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
	
	// Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    while(1) {
    	printf("client>");
    	size_t insiz = 1024;
    	char *input = (char*)malloc(insiz);
    	bytes_read = getline(&input,&insiz,stdin);
    	if(bytes_read < 0) {
    		printf("Failed to read input\n");
    		break;
    	}
    	char* comm = strtok(input," \t\r\n\a");
    	char* list[30];
		int ind = 0;
    	while(comm != NULL) {
    		list[ind] = (char*)malloc(insiz);
    		strcpy(list[ind],comm);
    		ind++;
    		comm = strtok(NULL," \t\r\n\a");
    	}
    	if(ind == 0)
    		continue;
    	if(strcmp(list[0],"exit") == 0)
    		break;
    	else if(strcmp(list[0],"get") == 0)
		    for(int i = 1 ; i < ind ; i++) {
		    	// printf("%s\n",list[i]);
			    send(sock, list[i], strlen(list[i]),0);
			    bytes_read = read(sock, buf, bufsiz);
			    if(bytes_read < 0) {
			    	puts("Unable to read from server");
			    	continue;
			    }
			    buf[bytes_read] = '\0';
			    int siz_of_file = atoi(buf);
			    if(siz_of_file < 0) {
			    	printf("No file with name %s found in the server directory or there was an error reading the file\n",list[i]);
			    	continue;
			    }
			    int fp = open(list[i],O_CREAT | O_WRONLY | O_TRUNC,0664);
			    if(fp < 0) {
			    	perror("Creating file");
			    	continue;
			    }
			    int done = 0;
			    send(sock,"got it",strlen("got it"),0);
			    char* per = (char*)malloc(256);
			    int flag = 0;
			    printf("Downloading file %s\n",list[i]);
			    while(1) {
			    	bytes_read  = read(sock,buf,min(bufsiz,siz_of_file-done));
			    	if(bytes_read < 0) {
			    		printf("Error reading from socket\n");
			    		flag = 1;
			    		break;
			    	}
			    	buf[bytes_read] = '\0';
			    	done += bytes_read;
			    	double percentage = ((long double)done / (long double)siz_of_file) * (long double)(100);
					sprintf(per,"\b\b\b\b\b\b\b%3.2lf%%",percentage);
					write(STDOUT_FILENO,per,strlen(per));
					fflush(stdout);
			    	write(fp,buf,strlen(buf));
			    	if(done >= siz_of_file)
		                break;
			    }
			    close(fp);
			    if(flag)
			    	continue;
			    printf("\b\b\b\b\b\b\b100.00%%");
			    puts("\nDone downloading the file");

		    }
		else 
			printf("Invalid command\n");
	}
    close(sock);
	return 0;
}