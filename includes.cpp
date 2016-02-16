#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define MAXSERVERS 70
#define MAXTAILNODES 257
#define MAXNETWORKS 1000
#define NUMCONNECTIONS 100
#define IPFORCODE	1729			// C.P. Ramanujam's number
#define ROUNDROBCODE 151031			// A.P.J Abdul kalam sir's birth date 
using namespace std;


/*
std::string convertStrToBin(std::string ip);
int readConfigurationFile(char * fname);
struct trie_node * create_trie_node();
struct leaf_node* create_leaf_node();
void trie_setup();
void trie_insert(struct trie_node* ptr , struct leaf_node* leaf_anc, std::string const& binip , int ind , int lim ,int srvnum);
void trie_remove(struct trie_node* ptr , std::string const& binip , int ind , int lim);
struct leaf_node* trie_getserver(struct trie_node* ptr , std::string const& binip);
*/
