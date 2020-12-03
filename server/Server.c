#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PORT 3000
#define MAX_CONNECTIONS 1

int min(int a , int b) {
    if(a < b) return a;
    return b;
}

int bufsiz = 1e6+5;
int main(int argc, char *argv[]) {
	int serv_fd,new_socket,bytes_read;
	struct sockaddr_in addr;
	int true = 1;
	int len = sizeof(addr);
	char* buf;
	buf = (char*)malloc(sizeof(char) * bufsiz);
	char *testmsg = "wassup";

	// CREATING A SOCKET
	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(serv_fd < 0) {
		perror("socket creation");
		return -1;
	}
	// REMOVE WEIRD ERROR MSGS
    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&true, sizeof(true)))
    {
        perror("setsockopt");
        return -1;
    }
    addr.sin_family = AF_INET; // PROTOCOL
    addr.sin_port = htons ( PORT );
    addr.sin_addr.s_addr = INADDR_ANY; // NOT BIND TO ANY SPECIDIC IP
    // BIND THE SERVER TO PORT
    if (bind(serv_fd, (struct sockaddr *)&addr,len) < 0)
    {
        perror("bind failed");
        return -1;
    }
    // START LISTENING FOR CALLS TO THE SERVER
    if(listen(serv_fd,MAX_CONNECTIONS) < 0) {
    	perror("listen");
    	return -1;
    }
    // MAKING THE NEW SOCKET FOR THE CURRENTLY ACCPETED CONNECTION
    if ((new_socket = accept(serv_fd, (struct sockaddr *)&addr,(socklen_t*)&len)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    while((bytes_read = read(new_socket, buf, bufsiz)) > 0) {
        buf[bytes_read] = '\0';
        int fp = open(buf,O_RDONLY);
        if(fp < 0) {
            send(new_socket,"-1",strlen("-1"),0);
            continue;
        }
        int size_of_file = lseek(fp, 0L, 2);
        lseek(fp,0L,0);
        char *sizfile = (char *)malloc(256);
        sprintf(sizfile,"%d",size_of_file);
        send(new_socket,sizfile,strlen(sizfile),0);
        int done = 0;
        read(new_socket,buf,bufsiz);
        while(1) {
            bytes_read  = read(fp,buf,min(bufsiz,size_of_file - done));
            if(bytes_read < 0) {;
                break;
            }
            buf[bytes_read] = '\0';
            done += bytes_read;
            send(new_socket,buf,strlen(buf),0);
            if(done >= size_of_file)
                break;
        }
    }
    close(new_socket);
    close(serv_fd);
    return 0;
}
