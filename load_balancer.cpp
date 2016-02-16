#include "includes.cpp"
#include "utilities.cpp"

HeadNode * ptrHeadNode = NULL;
TailNode* ptrTailNode[MAXTAILNODES];
Server* ptrServer[MAXSERVERS];
int serverWeights[MAXSERVERS] = {0} , algonum = 0;

struct trie_node* head = NULL;
struct trie_leaf* start = NULL , *end = NULL;
vector < pair < std::string , int > > network_mapping , request_types;
vector < pair<int,int> >request_mapping; 

#include "functions.cpp"

void* handle_connection(void* arg)
{

    char error_mssg[] = "Service not available , server maybe temporarily down\n";
    struct sockaddr_in server;
    int socket_desc = 0 , host_desc = 0 , client_port = 0 , read_size , srvnum = 0;
    char *message , client_message[2000] , server_message[2000];
    std::string binip;

    struct thread_args* args = (struct thread_args*)arg;
    host_desc = args->host_socket_desc;
    client_port = args->client_port;
    std::string client_ip(args->client_ip_addr);
    binip = convertStrToBin(client_ip);
    srvnum = args->srvnum;
    cout<<client_port<<"\n"<<client_ip<<"\n"<<srvnum<<endl;

    // socket setup
    printf("inside handler\n");
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
        printf("Couldn't create socket\n");
    printf("%d\n",srvnum);
    printf("%s\n%d\n",ptrServer[srvnum]->getip(),ptrServer[srvnum]->getport());
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ptrServer[srvnum]->getip());
    server.sin_port = htons( ptrServer[srvnum]->getport() );

    // connecting to server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("Connection with server error\n");
        write(host_desc, error_mssg , 60);
        close(host_desc);
        return NULL;
    }

    printf("Starting query transaction\n");
    while( (read_size = recv(host_desc , client_message , 2000 , 0)) > 0 )
    {
        //Send the message to server
        write(socket_desc , client_message , strlen(client_message));
        // server's message
        if(recv(socket_desc , server_message ,  sizeof(server_message) , 0)< 0)
        {
            puts("recv failed");
        }
        //send the message to client
        write(host_desc, server_message , sizeof(server_message));
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

void initialise_forwarding_environment()
{

}

int main(int argc , char *argv[])
{
    int socket_desc , host , addr_size = 0 , current_srv_number = 0 , limit = 0 , x = 0;
    struct sockaddr_in server_addr , client_addr;
    pthread_t thread_id;
    int type = 0 , status = 0 , yes = 1 , tailnum = -1 , client_port = 0;
    std::string client_ip;

    if( argc == 2 )
    {
        int result = readConfigurationFile(argv[1]);
        if(result <= 0)
        {
            printf("some error occured in reading file\n");
            return -1;
        }
    }
    else
    {
        printf("Configuration File not specified or incorrrectly specified\n");
        return -1;
    }

    // for testing client ip method
    //char * names[5] = {"127.0.0.1" , "127.127.0.0" , "222.222.222.222" , "0.189.221.47"};

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
    {
        printf("Couldn't create socket\n");
        return -1;
    }
   
    //memset(server_addr , 0 , sizeof(server_addr));
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr(ptrHeadNode->getip().c_str());
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( ptrHeadNode->getport() );

    // we make use of the setsockopt() function to make sure the port is not in use.
    // by a previous execution of our code.
    if(setsockopt(socket_desc , SOL_SOCKET , SO_REUSEADDR , &yes , sizeof(int)) < 0)
    {
        printf("Port already in use\n");
        return -1;
    }

    // binding port to address
    if(bind(socket_desc,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Binding failed\n");
        return -1;
    }
    else
        printf("Binding established\n");

    // listening for connections
    if(listen(socket_desc , NUMCONNECTIONS) < 0)
    {
        printf("listen error \n");
        return -1;
    }

    // algorithm gathered from the configuration file
    tailnum = ptrHeadNode->gettailNum();
    algonum = ptrTailNode[tailnum]->getalgo();
    if(algonum == ROUNDROBCODE)
    {

    }
    else if(algonum == IPFORCODE)
    {
        trie_setup();
        pair<std::string , int>p;
        for(int u=0;u<network_mapping.size();u++)
        {
            cout<<u<<endl;
            p = network_mapping[u];
            trie_insert(head , NULL , convertStrToBin(p.first) , 0 , 32 , p.second);
        }
    }
    // accept incoming connections
    addr_size =  sizeof( struct sockaddr_in);
    char ips[][20] = {"192.168.1.0" , "1.1.1.0" , "200.200.200.0" , "190.0.0.0"};
    int iter=0;
    while(host = accept(socket_desc , (struct sockaddr *)&client_addr ,(socklen_t*)&addr_size))
    {
        printf("New connection accepted:%d\n\n",host);
        
        // client's data accumulation 
        //client_ip = inet_ntoa(client_addr.sin_addr);
        client_ip = ips[iter];
        iter = (iter+1)%4;
        client_port = ntohs(client_addr.sin_port);

        struct thread_args * arg = (struct thread_args *)malloc(sizeof(struct thread_args));
        arg->host_socket_desc = host;
        strncpy(arg->client_ip_addr , inet_ntoa(client_addr.sin_addr) , sizeof(arg->client_ip_addr));
        arg->client_port = ntohs(client_addr.sin_port);

        //actual one
        if(algonum == ROUNDROBCODE)
        {

        }
        else if(algonum == IPFORCODE)
        {
            struct trie_leaf* tmp = trie_getserver(head , convertStrToBin(client_ip));
            arg->srvnum = tmp->srvnum;
            printf("srvnum :%d\n",tmp->srvnum);
        }

        if( pthread_create( &thread_id , NULL , handle_connection , (void*) arg) < 0)
        {
            printf("could not create thread");
            return 1;
        }
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
        //sleep(120);
    }
    if (host < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    close(socket_desc);
    return 0;
}