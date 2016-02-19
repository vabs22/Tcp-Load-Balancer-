#include "includes.cpp"
#include "utilities.cpp"

HeadNode * ptrHeadNode = NULL;
TailNode* ptrTailNode[MAXTAILNODES];
Server* ptrServer[MAXSERVERS];
int serverWeights[MAXSERVERS] = {0} , algonum = 0 , tailnum = -1 , totalplaces =0 , iter = 0;

struct trie_node* head = NULL;
struct trie_leaf* start = NULL , *end = NULL;
vector < pair < std::string , int > > network_mapping , request_types;
vector < pair<int,int> >request_mapping; 
int server_allotment[MAXROUNDROBSLOTS] = {0} , server_availability[MAXSERVERS] = {0} , tmp_availability[MAXSERVERS] = {0};

pthread_mutex_t lock_availability;

sem_t sem_check_connection_threads;

#include "functions.cpp"

void* handle_connection(void* arg)
{
    cout<<"inside handler"<<endl;
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
    //cout<<client_port<<"\n"<<client_ip<<"\n"<<srvnum<<endl;
    

    // socket setup
    //printf("inside handler\n");
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
        printf("Couldn't create socket\n");
    //printf("%d\n",srvnum);
    //printf("%s\n%d\n",ptrServer[srvnum]->getip(),ptrServer[srvnum]->getport());
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
    free(args);
    return 0;
}

void* check_connection(void* x)
{
    int socket_desc;
    struct sockaddr_in server;
    int srvnum = *((int*)x) , result = 0 , status;

    free(x);

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  
    if(socket_desc == -1)
        printf("Couldn't create socket\n");
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ptrServer[srvnum]->getip());
    server.sin_port = htons( ptrServer[srvnum]->getport() );

    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        result = 0;
        printf("Server %d not available\n" , srvnum);
    }
    else
    {
        result = 1;
        close(socket_desc);
    }

    sem_wait(&sem_check_connection_threads);

    tmp_availability[srvnum] = result;

    sem_post(&sem_check_connection_threads);
}

void* server_periodic_health_check(void* args)
{
    int i , j;
    pthread_t check_thread_id;

    // Initialize the semaphore with a value of 1.
    // Note the second argument: passing zero denotes
    // that the semaphore is shared between threads (and
    // not processes).
    if(sem_init(&sem_check_connection_threads, 0, 1))
    {
        printf("Could not initialize a semaphore\n");
        return NULL;
    }

    while(1)
    {
        memset(tmp_availability , 0 , sizeof(tmp_availability));
        for(i=0;i<MAXSERVERS;i++)
        {
            if(ptrServer[i] != NULL)
            {
                int * tmp = (int *) malloc(sizeof(int));
                *tmp = i;
                if( pthread_create( &check_thread_id , NULL , check_connection , (void*) tmp) < 0)
                {
                    printf("could not create thread");
                    continue;
                    //return 1;
                }
                
            }
        }
        pthread_join(check_thread_id , NULL);

        pthread_mutex_lock(&lock_availability);
        for(i=0;i<MAXSERVERS;i++)
        {
            server_availability[i] = tmp_availability[i];
        }
        pthread_mutex_unlock(&lock_availability);

        sleep(TIME_CHECK_SERVER);
    }
    sem_destroy(&sem_check_connection_threads);
}

void initialise_forwarding_environment()
{
    // algorithm gathered from the configuration file
    tailnum = ptrHeadNode->gettailNum();
    algonum = ptrTailNode[tailnum]->getalgo();
    if(algonum == ROUNDROBCODE)
    {
        totalplaces = setup_roundrobin();
        if(totalplaces <= 0)
        {
            printf("Due to some unknown (obviously) error , round robin environment couldn't be setup\n");
            return ;
        }
        iter = 0;
    }
    else if(algonum == IPFORCODE)
    {
        trie_setup();
        pair<std::string , int>p;
        for(int u=0;u<network_mapping.size();u++)
        {
            //cout<<u<<endl;
            p = network_mapping[u];
            if(ptrTailNode[tailnum]->checkserver(p.second))
            {
                //cout<<"srvnum : "<<p.second<<"_"<<convertStrToBin(p.first)<<endl;
                trie_insert(head , NULL , convertStrToBin(p.first) , 0 , 32 , p.second);
                
            }
        }
    }
}

int main(int argc , char *argv[])
{
    int socket_desc , host , addr_size = 0 , srvnum = 0 , yes = 1 , prev_iter = -1;
    struct sockaddr_in server_addr , client_addr;
    pthread_t thread_id , healthcheck;
    std::string client_ip;
    char ips[][20] = {"192.168.1.0" , "1.1.1.0" , "200.200.200.0" , "190.0.0.0"};


    /*
            DATA WOULD BE GATHERED FROM CONFIGURATION FILE 
    */
   
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


    /*
            FRONT END OF LOAD BALANCER IS INITIALISED HERE
    */
    
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);  // creates an unbound socket
    if(socket_desc == -1)
    {
        printf("Couldn't create socket\n");
        return -1;
    }
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

    /*
            PTHREAD MUTEX LOCKS AND SEMAPHORE INITIALISATION
     */

    if (pthread_mutex_init(&lock_availability, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return -1;
    }


    // health check thread
    if( pthread_create( &healthcheck , NULL , server_periodic_health_check , NULL) < 0)
    {
        printf("could not create server health check thread\n");
        return -1;
    }

    /*
            ENVIRONMENT VARIABLES SETUP
     */
    initialise_forwarding_environment();


    /*
            MAIN WORKING LOOP OF THIS FUNCTION
     */
    
    // accept incoming connections
    addr_size =  sizeof( struct sockaddr_in);
    iter = 0;
    
    
    struct trie_leaf* tmp = start;
    while(tmp != end)
    {
        srvnum = tmp->srvnum;
        cout<<"srvnum1 :"<<srvnum<<endl;
        tmp = tmp->next;
    }
    tmp = end;
    while(tmp != start)
    {
        srvnum = tmp->srvnum;
        //cout<<"srvnum :"<<tmp->prev->srvnum<<endl;
        cout<<"srvnum2 :"<<srvnum<<endl;
        tmp = tmp->prev;
    }
    srvnum = 0;
    
   
    while(host = accept(socket_desc , (struct sockaddr *)&client_addr ,(socklen_t*)&addr_size))
    {
        printf("New connection accepted:%d\n",host);
        
        // client's data accumulation 
        //client_ip = inet_ntoa(client_addr.sin_addr);      // actual initialisation
        client_ip = ips[iter];                              // for testing ip forwarding
        
        struct thread_args * arg = (struct thread_args *)malloc(sizeof(struct thread_args));
        arg->host_socket_desc = host;
        strncpy(arg->client_ip_addr , inet_ntoa(client_addr.sin_addr) , sizeof(arg->client_ip_addr));
        arg->client_port = ntohs(client_addr.sin_port);

        //cout<<"variables fine"<<endl;
        pthread_mutex_lock(&lock_availability);
        if(algonum == ROUNDROBCODE)
        {
            srvnum = server_allotment[iter];
            prev_iter = iter;
            while(server_availability[srvnum] == 0)
            {
                iter = (iter+1)%totalplaces;
                srvnum = server_allotment[iter];
                if(iter == prev_iter)
                    break;
            }
            iter = (iter+1)%totalplaces;
        }
        else if(algonum == IPFORCODE)
        {
            struct trie_leaf* tmp = trie_getserver(head , convertStrToBin(client_ip));
            struct trie_leaf* aux = tmp;
            srvnum = tmp->srvnum;
            prev_iter = tmp->srvnum;
            // cout<<"inside if"<<endl;
            while(server_availability[srvnum] == 0 && tmp != start)
            {
                cout<<"srvnum1: "<<srvnum<<endl;
                tmp = tmp->prev;
                srvnum = tmp->srvnum;
            }
            if(tmp == start)
            {
                tmp = aux;
                srvnum = tmp->srvnum;
                while(server_availability[srvnum] == 0 && tmp != end)
                {
                    cout<<"srvnum2: "<<srvnum<<endl;
                    tmp = tmp->next;
                    srvnum = tmp->srvnum;
                }
                if(tmp == end)
                    srvnum = prev_iter;
            }
            iter = (iter+1)%4;
        }
        pthread_mutex_unlock(&lock_availability);
        arg->srvnum = srvnum;
        //cout<<"if is fine"<<endl;
        cout<<client_ip<<endl;
        printf("srvnum :%d , iter : %d\n\n",srvnum,iter);

        if( pthread_create( &thread_id , NULL , handle_connection , (void*) arg) < 0)
        {
            printf("could not create thread");
            continue;
        }
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        //puts("Handler assigned");
        //sleep(120);
    }
    if (host < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    pthread_mutex_destroy(&lock_availability);
    close(socket_desc);

    return 0;
}