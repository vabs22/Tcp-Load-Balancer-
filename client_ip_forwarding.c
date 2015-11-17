
struct trie_node * create_node()
{
	//printf("Inside create node\n");
	struct trie_node * tmp = malloc(sizeof(struct trie_node));
	tmp->end = 0;
	tmp->left = NULL;
	tmp->right = NULL;
	tmp->sptr = NULL;
	//printf("Exiting create node\n");
	return tmp;
}

struct trie_node * head ;

void trie_insert(struct trie_node * ptr , char str[] , int ind , int lim , struct server_node* sptr)
{
	//printf("Inside trie_insert\n");
	if(ind == lim)
	{
		ptr->end = 1;
		ptr->sptr = sptr;
		return ;
	}
	if(ptr == NULL)
	{
		ptr = create_node();
	}
	struct trie_node * tmp  = create_node();
	//printf("%c\n",str[ind]);
	if(str[ind] == '0')
	{
		if(ptr->left == NULL)
		{
			//printf("here ?\n");
			ptr->left = tmp;

		}
		trie_insert(ptr->left , str , ind+1 , lim , sptr);
	}
	else
	{
		if(ptr->right == NULL)
			ptr->right = tmp;
		trie_insert(ptr->right , str , ind+1 , lim , sptr);
	}
}

struct server_node * get_server_trie(char str[])
{
	struct trie_node * tmp = head;
	int ind = 0 , lim = strlen(str);
	if(tmp == NULL)
	{
		printf("Head is null\n");
	}
	while(1)
	{
		if(ind == lim)
			return tmp->sptr;

		if(str[ind] == '0' && tmp->left != NULL )
		{
			tmp = tmp->left;
			ind++;
		}
		else if(str[ind] == '1' && tmp->right != NULL)
		{
			tmp = tmp->right;
			ind++;
		}
		else
		{
			while(ind < lim)
			{
				if(tmp->left != NULL)
					tmp = tmp->left;
				else
					tmp = tmp->right;
				ind++;
			}
			return tmp->sptr;
		}
	}
}


void gen_bitarray1(char str[] , char targ[])
{
    int len = strlen(str);
    int i = 0 , arr[6] = {0} , j=0 , num = 0 , k=0;
    //printf("%s_%d\n",str,len);
    j = 0;
    while(i<len)
    {
        if(str[i] == '.')
        {
            arr[j] = num;
            j++;
            num = 0;
        }
        else
        {
            num = num*10 + str[i] - '0';
        }
        i++;
    }
    arr[j] = num;
    j = 31;
    for(k=3;k>=0;k--)
    {
        num = arr[k];
        //printf("%d",num);
        for(i=0;i<8;i++)
        {
            if(num&1)
                targ[j] = '1';
            else
                targ[j] = '0';
            num/=2;
            j--;
        }
    }

    targ[32] = '\0';
    return ;
    //printf("%d_%s\n",len,targ);
}


void init_ipforward(int* srv_num , char* f1name , char *f2name)
{
	FILE *fp;
	int srv_num_local = 0 , i , entries , j , check[105] = {0} , id_num;
	char node_name[50]  , ip_addr[35] , str_ip[35];
	char str_list[50][50];

	fp = fopen(f1name, "r"); 
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
        check[id_num] = 1;
        //printf("%d_%s\n",id_num , srv_array[id_num]->ip_addr);
    }


	fp = fopen(f2name, "r"); 
    if(NULL == fp)
    {
        printf("Error opening file\n probably file not found\n");
        return ;
    }
    else
    	printf("File successfully opened\n");

    //printf("%d\n" , srv_num_local);
    fscanf(fp,"%d",&entries);
    for(i=0;i<entries;i++)
    {
        fscanf(fp , "%d" ,&id_num);
    	fscanf(fp , "%s" ,ip_addr);
    	fscanf(fp , "%s" ,node_name);
    	
        if(id_num <= 100 && check[id_num] == 1)
        {
            gen_bitarray1(ip_addr , str_ip);
            trie_insert(head , str_ip , 0 , strlen(str_ip) , srv_array[id_num]);
        }
        else
        {
            printf("Error , unknown server name found\n");
            return ;
        }
    	
    }

    return ;	
}