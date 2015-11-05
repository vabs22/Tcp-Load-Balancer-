#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

struct args
{
    int socket_desc;
    char ip[34];
    int port;
};

void *connection_handler(void * arg1)
{
    //Get the socket descriptor

    struct args * argum = (struct args*)arg1;
    int sock = argum->socket_desc;
    char server_ip[34];
    strcpy(server_ip , argum->ip);
    int server_port = argum->port;

    struct sockaddr_in server;
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
        printf("Couldn't create socket\n");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_port = htons( server_port );

    // connecting to server
    if (connect(socket_desc , (struct sockaddr *)&server,sizeof(server)) < 0)
    {
        printf("Connection with server error\n");
        return ;
    }

    int read_size;
    char *message , client_message[2000] , server_message[2000];
    
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message to server
        write(socket_desc , client_message , strlen(client_message));
        // server's message
        if(recv(socket_desc , server_message ,  sizeof(server_message) , 0)< 0)
        {
            puts("recv failed");
        }
        //send the message to client
        write(sock, server_message , sizeof(server_message));
        //clear the message buffer
        memset(client_message, 0, 2000);
        memset(server_message, 0, 2000);
    }
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    return 0;
} 

int main(int argc , char *argv[])
{
    int socket_desc , host , c , number = 1;
    struct sockaddr_in server , client;
    pthread_t thread_id;

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
        printf("Couldn't create socket\n");
   
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8088 );

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
        
        struct args * arg1 = malloc(sizeof(struct args));
        arg1->socket_desc = host;
        sprintf(arg1->ip , "127.0.0.1");
        arg1->port = 5000 + number*10;

        number++;
        if(number == 4)
            number = 1;

        if( pthread_create( &thread_id , NULL , connection_handler , (void*) arg1) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
    if (host < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    close(socket_desc);
    return 0;
}