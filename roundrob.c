struct server_node* initialise_roundrob (int* srv_num , char* fname)
{
	FILE *fp;
	int srv_num_local = 0 , i;
	char node_name[50]  , ip_addr[34];

	fp = fopen(fname, "r"); 
    if(NULL == fp)
    {
        printf("Error opening file\n probably file not found\n");
        return ;
    }
    else
    	printf("File successfully opened\n");

    fscanf(fp,"%d",&srv_num_local);
    *(srv_num) = srv_num_local;
    //printf("%d\n",srv_num_local);

    struct server_node * srv_array = malloc(srv_num_local * sizeof(struct server_node));

    for(i=0;i<srv_num_local;i++)
    {
    	fscanf(fp , "%s" ,srv_array[i].node_name );
    	fscanf(fp , "%s" ,srv_array[i].ip_addr);
    	fscanf(fp , "%d" ,&srv_array[i].port);
    	fscanf(fp , "%d" ,&srv_array[i].flag_check);
        fscanf(fp , "%d" ,&srv_array[i].weight);

        //printf("%s_%s_%d_%d" , srv_array[i].node_name , srv_array[i].ip_addr , srv_array[i].port , srv_array[i].flag_check);
        //printf("\n");
    }
    return srv_array;
}

int array_order(int list[] ,struct server_node* srv_array , int srv_num)
{
    int aux[50] = {0};
    int i = 0 , sum = 0 , k = 0;

    for(i=0;i<srv_num;i++)
    {
        aux[i] = srv_array[i].weight;
        sum += aux[i];
    }
    i = 0;
    while(sum > 0)
    {
        if(aux[i] > 0)
        {
            list[k] = i;
            k++;
            sum --;
        }
        i++;
        if(i == srv_num)
            i = 0;
    }
    return k;
}