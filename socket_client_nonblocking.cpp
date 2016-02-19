#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc , char *argv[])
{
	int socket_desc , status;
	struct sockaddr_in server;
	char message[2000] , reply[2000];
	extern int errno;

	struct timeval  timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);  
	if(socket_desc == -1)
		printf("Couldn't create socket\n");
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.1.35");
	server.sin_port = htons( 5050 );


    fd_set set;
    FD_ZERO(&set);
    FD_SET(socket_desc, &set);

    fcntl(socket_desc, F_SETFL, O_NONBLOCK);

    if ( (status = connect(socket_desc, (struct sockaddr *)&server , sizeof(server))) < 0)
    {
    	printf("Connection error , status %d\n",status);
        //if ( errno != EINPROGRESS )
        //    return status;
    }
    status = select(socket_desc+1, NULL, &set, NULL, &timeout);
    printf("status %d\n",status);
	close(socket_desc);
	return 0;
}
