#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main(int argc , char *argv[])
{
	int socket_desc;
	struct sockaddr_in server;
	char message[2000] , reply[2000];


	socket_desc = socket(AF_INET , SOCK_STREAM , 0);  
	if(socket_desc == -1)
		printf("Couldn't create socket\n");
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( 8088 );

	// connecting to server
	if (connect(socket_desc , (struct sockaddr *)&server,sizeof(server)) < 0)
	{
		printf("Connection error\n");
		return 1;
	}
	else
		printf("Connection established\n");	

	sprintf(message , "This is client 140\nHow u doin :P");
	
	if( send(socket_desc , message , sizeof(message),0) < 0)
    {
        puts("Send failed");
        return 1;
    }
    else
    	printf("Message sent\n");


    if(recv(socket_desc , reply ,  sizeof(reply) , 0)< 0)
    {
        puts("recv failed");
    }
    else
    	printf("Message received:\n");

   	printf("%s",reply);
	close(socket_desc);
	return 0;
}