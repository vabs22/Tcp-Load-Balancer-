#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "utilities.c"
#include "roundrob.c"
#include "client_ip_forwarding.c"


void gen_bitarray1(char str[] , char targ[]);


void *connection_handler(void * arg1)
{
    //Get the socket descriptor

    struct args * argnum = (struct args*)arg1;
    char client_ip[40] ;
    char str_ip[35];
    char * error_mssg = "Service not available , server maybe temporarily down\n";

    int sock = argnum->socket_desc;
    strcpy(client_ip , argnum->client_ip_addr);
    int client_port = argnum -> client_port;
    int type = argnum -> algo_type;

    struct server_node *  server_details;
    if(type == 1)
        server_details = argnum -> server_details;  // for round robin
    else if(type == 2)
    {
        gen_bitarray1(client_ip , str_ip);
        //printf("%s\n",str_ip);
        // for client ip forwarding logic
        server_details = get_server_trie(str_ip);
    }

    struct sockaddr_in server;
    int socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
        printf("Couldn't create socket\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(server_details -> ip_addr);
    server.sin_port = htons( server_details -> port );
    printf("%s_%d\n",(server_details -> ip_addr),( server_details -> port ));

    //printf("%s:%d",server_details -> ip_addr,server_details -> port);


    // connecting to server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("Connection with server error\n");
        write(sock, error_mssg , 60);
        close(sock);
        return ;
    }

    int read_size;
    char *message , client_message[2000] , server_message[2000];
    printf("Starting query transaction\n");

    
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
    int socket_desc , host , c , current_srv_number = 0;
    struct sockaddr_in server , client;
    pthread_t thread_id;
    int type=0;
    if( argc == 2 )
        type = atoi(argv[1]);
    else
    {
        printf("Enter the algo as well in the commandline\n\t1 - roundrobin\n\t2 - client ip forwarding\n");
        return 0;
    }

    // for testing client ip method
    //char * names[5] = {"127.0.0.1" , "127.127.0.0" , "222.222.222.222" , "0.189.221.47"};

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

    int srv_num = 0;

    // round robin , if statements to be added 
    //->>>>>>>    

    struct server_node * srv_array;
    if(type == 1)
        srv_array = initialise_roundrob(&srv_num , "srv_confg.txt"); // round robin
    else if(type == 2)
    {
        // client's ip forwarding
        head = create_node();
        srv_array = init_ipforward(&srv_num , "srv_confg.txt" , "forwarding_scheme.txt");
    }


    while(host = accept(socket_desc , (struct sockaddr *)&client ,(socklen_t*)&c))
    {
        printf("New connection accepted:%d\n\n",host);
        
        struct args * arg1 = malloc(sizeof(struct args));
        arg1->socket_desc = host;

        //actual one
        strcpy(arg1->client_ip_addr , inet_ntoa(client.sin_addr));
        arg1->algo_type = type;
        //for testing
        //strcpy(arg1->client_ip_addr , names[current_srv_number]);
        //printf("%s\n",names[current_srv_number]);

        arg1->client_port = ntohs(client.sin_port);

        // for round robin
        arg1->server_details = &srv_array[current_srv_number];

        current_srv_number++;
        if(current_srv_number == srv_num)
            current_srv_number = 0;

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


