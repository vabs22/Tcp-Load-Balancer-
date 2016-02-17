#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main(int argc , char *argv[])
{
	int socket_desc , host , c;
	struct sockaddr_in server , client;
	char message[2000]  , reply[2000];
	int num = atoi(argv[1]);

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);  
	if(socket_desc == -1)
		printf("Couldn't create socket\n");
	
	server.sin_addr.s_addr = inet_addr("192.168.1.35");
	server.sin_family = AF_INET;
	server.sin_port = htons( 6000 + num );

	// binding port to address
	if (bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("Binding failed\n");
		return 1;
	}
	else
		printf("Binding established\n");

	// listening for connections
	listen(socket_desc,5);

	// accept incoming connections
	c =  sizeof( struct sockaddr_in);

	while(host = accept(socket_desc , (struct sockaddr *)&client ,(socklen_t*)&c))
	{
		printf("New connection accepted:%d\n\n",host);
		

		if(recv(host , message , sizeof(message) , 0)< 0)
	    {
	        puts("recv failed");
	    }
	    printf("%s\n",message);
	    
		sprintf(reply ,"Hello client ! Greetings from server %d\n" , num);

		write(host ,reply , sizeof(reply));
	}
	
	close(socket_desc);
	return 0;
}