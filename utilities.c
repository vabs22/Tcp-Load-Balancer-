#include <stdio.h>

struct server_node
{
	char node_name[50];
	char ip_addr[40];
	int port;
	int flag_check;
};


struct args
{
    int socket_desc;
    char client_ip_addr[40];
    int client_port;
    int algo_type;
    struct server_node * server_details;
};


struct trie_node
{
	int end;
	struct server_node * sptr;
	struct trie_node * left , *right;
};

