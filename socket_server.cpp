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
#include <pthread.h>
using namespace std;

int num=0;
void* handle_connection(void* arg)
{
	char message[2000]  , reply[2000];
	int host_desc = *((int*)arg);
	delete((int *)arg);
	if(recv(host_desc , message , sizeof(message) , 0)< 0)
    {
        puts("recv failed");
    }
    printf("%s\n",message);
    
	sprintf(reply ,"Hello client ! Greetings from server %d\n" , num);

	write(host_desc ,reply , sizeof(reply));

	close(host_desc);
}
int main(int argc , char *argv[])
{
	 pthread_t thread_id;
	int socket_desc , host , c;
	struct sockaddr_in server , client;
	num = atoi(argv[1]);

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);  
	if(socket_desc == -1)
		printf("Couldn't create socket\n");
	
	server.sin_addr.s_addr = inet_addr("10.60.95.144");
	server.sin_family = AF_INET;
	server.sin_port = htons( 6000 + num );
	//server.sin_port = htons( 5050 );

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
		int * tmp = new int;
		*tmp = host;
		if( pthread_create( &thread_id , NULL , handle_connection , (void *) tmp) < 0)
	    {
	        printf("could not create server health check thread\n");
	        return -1;
	    }
		
	}
	
	close(socket_desc);
	return 0;
}