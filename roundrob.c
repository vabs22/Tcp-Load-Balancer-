int initialise_roundrob (int* srv_num , char* fname)
{
	FILE *fp;
	int srv_num_local = 0 , i , iter = 0 , weight[105] = {0} , id[105]  = {0} , id_num=0;
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

    for(i=0;i<srv_num_local;i++)
    {
        fscanf(fp , "%d" ,&id_num);
        srv_array[id_num] = (struct server_node * )malloc(sizeof(struct server_node));
        srv_array[id_num]->id_num = id_num;
    	fscanf(fp , "%s" ,srv_array[id_num]->node_name );
        fscanf(fp , "%s" ,srv_array[id_num]->ip_addr);
    	fscanf(fp , "%d" ,&srv_array[id_num]->port);
    	fscanf(fp , "%d" ,&srv_array[id_num]->flag_check);
        fscanf(fp , "%d" ,&srv_array[id_num]->weight);
        weight_list[iter] = srv_array[id_num]->weight;
        srv_id[iter] = id_num;
        ++iter;
        //printf("%d_%s\n",id_num , srv_array[id_num]->ip_addr);
    }
    //return srv_array
    return iter;
}


