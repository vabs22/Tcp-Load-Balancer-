int strtoint(std::string str)
{
	int i=0 , len = str.length() , num=0;
	for(i=0;i<len;i++)
	{
		num = num*10 + str[i]-'0';
	}
	return num;
}

int readConfigurationFile(char * fname)
{

	ifstream infile;
	infile.open(fname);

	if (!infile)
    {
    	cout<<"Configuration file not found or couldn't be opened\n";
    	return -1;
    }

    std::string data;
    int flag = 0;
    while(infile)
    {
    	infile >> data;

    	if(data == "HEAD")
    	{
    		std::string ip;
    		int port , tailNum;
    		infile>>ip;
    		infile>>port>>tailNum;
    		infile>>data;

    		ptrHeadNode = new HeadNode(port , tailNum , ip);
    		if(data == "##")
    			continue;
    		else
    		{
    			flag = 1;
    			break;
    		}
    	}
    	else if(data == "TAIL")
    	{
    		int tailNum , srvnum=0;
    		std::string algo;
    		infile>>tailNum>>algo;
    		ptrTailNode[tailNum] = new TailNode(tailNum , algo);
    		while(1)
    		{
    			infile >> data;
    			if(data == "##")
    				break;
    			else
    			{
    				srvnum = strtoint(data);
    				ptrTailNode[tailNum]->addserver(srvnum);
    			}
    		}

    	}
    	else if(data == "SERVER")
    	{
    		int srvnum , port , check=0;
    		std::string ip;
    		infile>>data;
    		while(1)
    		{
    			if(data == "##")
    				break;
    			srvnum = strtoint (data);
    			infile >> ip >> port >> data;
    			if(data == "CHECK")
    			{
    				check = 1;
    				infile>>data;
    			}
    			ptrServer[srvnum] = new Server(srvnum , port , 1 , ip , check);
    			check = 0;
    		}
    	}
    	else if(data == "ROUNDROB")
    	{
    		int srvnum = 0 , weight = 0;
    		infile >> data;
    		while(1)
    		{
    			if(data == "##")
    				break;
    			srvnum = strtoint (data);
    			infile >> weight;
    			ptrServer[srvnum]->setweight(weight);

    			infile >> data;
    		}
    	}
    	else if(data == "IPFOR")
    	{
    		int mapsrv=0;
    		infile >> data;
    		while(data != "##")
    		{
    			infile >> mapsrv;
    			network_mapping.push_back(make_pair(data , mapsrv));
    			infile >> data;
    		}
    	}
    	else if(data == "REQTYPES")
    	{
    		int reqnum=0;
    		infile >> data ;
    		while(data != "##")
    		{
    			infile >> reqnum;
    			request_types.push_back(make_pair(data , reqnum));
    			infile >> data;
    		}
    	}
    	else if(data == "REQFOR")
    	{
    		int reqnum=0 , srvmap = 0;
    		infile >> data ;
    		while(data != "##")
    		{
    			reqnum = strtoint (data);
    			infile >> srvmap;
    			request_mapping.push_back(make_pair(reqnum , srvmap));
    			infile >> data;
    		}
    	}
    }
    return 1;
}

struct trie_node * create_trie_node()
{
	//printf("Inside create node\n");
	struct trie_node * tmp = (struct trie_node *)malloc(sizeof(struct trie_node));
	tmp->child = NULL;
	tmp->left = NULL;
	tmp->right = NULL;
	tmp->num_prefixes = 0;
	//printf("Exiting create node\n");
	return tmp;
}

struct trie_leaf* create_trie_leaf()
{
	//printf("Inside create node\n");
	struct trie_leaf * tmp = (struct trie_leaf *)malloc(sizeof(struct trie_leaf));
	tmp->srvnum = -1;
	tmp->prev = NULL;
	tmp->next = NULL;
	//printf("Exiting create node\n");
	return tmp;
}

void trie_setup()
{
	head = create_trie_node();
	start = create_trie_leaf();
	end = create_trie_leaf();

	start->next = end;
	end->prev = start;

	struct trie_node* tmp = head;
	for(int i=0;i<32;i++)
	{
		tmp->left = create_trie_node();
		tmp->num_prefixes++;
		tmp = tmp->left;
		tmp->child = start;
	}

	tmp = head;
	for(int i=0;i<32;i++)
	{
		tmp->right = create_trie_node();
		tmp->num_prefixes++;
		tmp = tmp->right;
		tmp->child = end;
	}

	head->child = start;
}

void trie_insert(struct trie_node* ptr , struct trie_leaf* leaf_anc, std::string const& binip , int ind , int lim ,int srvnum)
{
	//printf("Inside trie_insert\n");
	ptr->num_prefixes++;
	if(ind == lim)
	{
		struct trie_leaf * tmp = create_trie_leaf();
		tmp->next = leaf_anc->next;
		tmp->prev = leaf_anc;
		leaf_anc->next = tmp;
		tmp->srvnum = srvnum;

		ptr->child = tmp;
		return ;
	}
	if(binip[ind] == '0')
	{
		if(ptr->left == NULL)
			ptr->left = create_trie_node();
		trie_insert(ptr->left , leaf_anc , binip , ind+1 , lim , srvnum);
	}
	else
	{
		if(ptr->right == NULL)
			ptr->right = create_trie_node();
		if(ptr->child != NULL)
			leaf_anc = ptr->child;
		trie_insert(ptr->right , leaf_anc , binip , ind+1 , lim , srvnum);
	}

	if(ptr->left != NULL)
		ptr->child = ptr->left->child;

	if(ptr->right != NULL)
		ptr->child = ptr->right->child;
}

void trie_remove(struct trie_node* ptr , std::string const& binip , int ind , int lim)
{
	ptr->num_prefixes--;
	if(ind == lim)
	{
		struct trie_leaf * tmp = ptr->child;
		tmp->next->prev = tmp->prev;
		tmp->prev->next = tmp->next;
		ptr->child = NULL;
		free(tmp);
		return ;
	}
	if(binip[ind] == '0')
	{
		trie_remove(ptr->left , binip , ind+1 , lim);
		if(ptr->left->num_prefixes == 0)
		{
			free(ptr->left);
			ptr->left = NULL;
		}
	}
	else
	{
		trie_remove(ptr->right , binip , ind+1 , lim);
		if(ptr->right->num_prefixes == 0)
		{
			free(ptr->right);
			ptr->right = NULL;
		}
	}

	if(ptr->left == NULL && ptr->right == NULL)
		ptr->child = NULL;

	if(ptr->left != NULL)
		ptr->child = ptr->left->child;

	if(ptr->right != NULL)
		ptr->child = ptr->right->child;
}

struct trie_leaf* trie_getserver(struct trie_node* ptr , std::string const& binip)
{
	int ind = 0 , lim = binip.length() , flag=0;
	if(ptr == NULL)
	{
		printf("Head is null\n");
		return NULL;
	}
	while(1)
	{
		if(ind == lim)
			return ptr->child;

		if(flag)
		{
			if(ptr->left != NULL)
			{
				ptr = ptr->left;
				ind++;
			}
			else if(ptr->right != NULL)
			{
				ptr = ptr->right;
				ind++;
			}
		}
		else
		{
			if(binip[ind] == '0' && ptr->left != NULL )
			{
				ptr = ptr->left;
				ind++;
			}
			else if(binip[ind] == '1' && ptr->right != NULL)
			{
				ptr = ptr->right;
				ind++;
			}
			else
				flag = 1;
		}
	}
}
int gcd(int a , int b)
{
	if(a<b)
		return gcd(b,a);
	return b==0?a:gcd(b,a%b);
}
int setup_roundrobin()
{
	/*
	Supposing that there is a server set S = {S0, S1, …, Sn-1};
	W(Si) indicates the weight of Si;
	i indicates the server selected last time, and i is initialized with -1;
	cw is the current weight in scheduling, and cw is initialized with zero; 
	max(S) is the maximum weight of all the servers in S;
	gcd(S) is the greatest common divisor of all server weights in S;
	*/
	vector < pair <int,int> > vec;
	int i=0 , totalplaces = 0 , iter , totalservers = 0, cw = 0 , maxweight = 0, gcdweights = 1 , numservers = 0;

	memset(server_allotment , 0 , sizeof(server_allotment));

	for(int i=0;i<MAXSERVERS;i++)
	{
		if(ptrTailNode[tailnum]->checkserver(i))
		{
			vec.push_back(make_pair(ptrServer[i]->getweight(),i));
			totalplaces += ptrServer[i]->getweight();
			numservers ++;
		}
	}

	sort(vec.begin() , vec.end() , std::greater< pair <int , int > >());
	if(vec.size() >= 2)
	{
		maxweight = max(vec[0].first , vec[1].first);
		gcdweights = gcd(vec[0].first , vec[1].first);
		for(i=2;i<vec.size();i++)
		{
			maxweight = max(maxweight , vec[i].first);
			gcdweights = gcd(gcdweights , vec[i].first);
		}
		
		i = -1;
		for(iter=0;iter<totalplaces;iter++)
		{
			while (true) 
			{
			    i = (i + 1) % numservers;
			    if (i == 0) 
			    {
			        cw = cw - gcdweights; 
			        if (cw <= 0) 
			        {
			            cw = maxweight;
			            if (cw == 0)
			            	return 0;
			        }
			    } 
			    if (vec[i].first >= cw) 
			    {
			    	server_allotment[iter] = vec[i].second;
			    	break;
			    }
			}
		}
	}
	for(i=0;i<totalplaces;i++)
		cout<<server_allotment[i]<<endl;
	return totalplaces;
	
}
