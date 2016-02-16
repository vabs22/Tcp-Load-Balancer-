std::string convertStrToBin(std::string ip)
{
    int i=0 , j=0 , lim = ip.length();
    int num=0;
    std::string binip;
    vector <int> v;
    for(i=0;i<lim;i++)
    {
        if(ip[i] == '.')
        {
            v.push_back(num);
            num=0;
        }
        else
        {
            num = num*10 + (ip[i]-'0');
        }
    }
    v.push_back(num);
    for(i=3;i>=0;i--)
    {
        num = v[i];
        for(j=0;j<8;j++)
        {
            if(num&1)
                binip.insert(0,"1");
            else
                binip.insert(0,"0");
            num = num/2;
        }
    }
    return binip;
}

class HeadNode
{
	int port , tailNum;
	std::string ip , binip;

    public :
    HeadNode(int port , int tailNum , std::string ip)
    {
    	this->port = port;
    	this->tailNum = tailNum;
    	this->ip = ip;
    	this->binip = convertStrToBin(ip);
    }

    void settailNum(int tailNum)
    {
    	this->tailNum = tailNum;
    }

    int gettailNum()
    {
    	return tailNum;
    }
    std::string getip()
    {
    	return ip;
    }
    int getport()
    {
    	return port;
    }

};

class TailNode
{
	int tailNum;
	std::string algo;
    bool srvnum[MAXSERVERS];


    public :
    TailNode(int tailNum , std::string algo)
    {
    	this->tailNum = tailNum;
    	this->algo = algo;
    	memset(srvnum , 0 , sizeof(srvnum));
    }

    int addserver(int id)
    {
    	if(id < MAXSERVERS)
    		return srvnum[id] = 1;
    	else
    		return -1;
    }

    int checkserver(int id)
    {
    	if(id < MAXSERVERS)
    		return srvnum[id];
    	else
    		return -1;
    }
    int getalgo()
    {
    	if(algo == "IPFOR")
    		return IPFORCODE; 		
    	else if(algo == "ROUNDROB")
    		return ROUNDROBCODE; 		
    }
};

class Server
{
	int srvnum , port , weight;
	std::string ip , binip;
    bool check;

    public :
    Server(int srvnum , int port , int weight , std::string ip , bool check)
    {
    	this->srvnum = srvnum;
    	this->port = port;
    	this->weight = weight;
    	this->ip = ip;
    	this->binip = convertStrToBin(ip);
    	this->check = check;
    }

    void setweight(int weight)
    {
    	this->weight = weight;
    }

    void setbinip(std::string binip)
    {
    	this->binip = binip;
    }

    int getcheck()
    {
    	return check;
    }

    const char* getip()
    {
    	return ip.c_str();
    }

    int getport()
    {
    	return port;
    }
};

// node of binary tree
struct trie_node
{
	int num_prefixes;						// for future use
	struct trie_node * left , *right;		// pointers to left and right children
	struct trie_leaf * child;				// pointer to rightmost child leaf node
};

// leaf node of binary tree
struct trie_leaf
{
	int srvnum;							// number of server
	struct trie_leaf * prev , *next;		// pointer to previous and next leaf nodes 
};

struct thread_args
{
    int host_socket_desc;
    char client_ip_addr[40];                // maybe useful in future  
    int client_port , srvnum;
};

