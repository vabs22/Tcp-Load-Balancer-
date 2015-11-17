#include <stdio.h>

struct server_node
{
	int id_num;
	char node_name[50];
	char ip_addr[40];
	int port;
	int flag_check;
	int weight;
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

struct server_node * srv_array[100];
int weight_list[105] = {0} , srv_id[105] = {0} , iter = 0 , base = 0 , end = 0;

void merge_sort(int low , int high)
{
    if(low >= high)
        return ;
    int mid = low + (high - low)/2;
    int temp_weight[105] = {0} , temp_srv_id[105] = {0};
    merge_sort(low , mid);
    merge_sort(mid+1 , high);
    int i = low , j = mid+1 , k=0;
    while(i <= mid && j <= high)
    {
        if(temp_weight[i] >= temp_weight[j] )
        {
            temp_weight[k] = weight_list[i];
            temp_srv_id[k] = srv_id[i];
            k++;
            i++;
        }
        else
        {
            temp_weight[k] = weight_list[j];
            temp_srv_id[k] = srv_id[j];
            k++;
            j++; 
        }
    }

    while( i <= mid )
    {
        temp_weight[k] = weight_list[i];
        temp_srv_id[k] = srv_id[i];
        k++;
        i++; 
    }

    while( j <= high)
    {
        temp_weight[k] = weight_list[j];
        temp_srv_id[k] = srv_id[j];
        k++;
        j++; 
    }
    j = 0;
    for(i = low ; i <= high ; i++ , j++)
    {
        weight_list[i] = temp_weight[j];
        srv_id[i] = temp_srv_id[j];
    }
}

